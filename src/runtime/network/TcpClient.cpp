#include "TcpClient.h"

#include <iostream>
#include <span>
#include <system_error>
#include <utility>


namespace eld::network
{

    TcpClient::TcpClient()
        : socket_(
              io_
          )
    {
    }


    TcpClient::~TcpClient()
    {
        io_.stop();


        if (
            worker_.joinable()
        ) {
            worker_.join();
        }


        std::error_code error;


        socket_.close(
            error
        );
    }


    void TcpClient::setPacketHandler(
        PacketHandler handler
    )
    {
        packetHandler_ =
            std::move(
                handler
            );
    }


    bool TcpClient::connected() const noexcept
    {
        return
            connected_.load();
    }


    void TcpClient::connect(
        const std::string& host,
        std::uint16_t port
    )
    {
        asio::ip::tcp::resolver resolver(
            io_
        );


        const auto endpoints =
            resolver.resolve(
                host,
                std::to_string(
                    port
                )
            );


        asio::connect(
            socket_,
            endpoints
        );


        connected_.store(
            true
        );


        std::cout
            << "network | connected | "
            << host
            << ":"
            << port
            << "\n";


        read();


        worker_ =
            std::thread(
                [this]()
                {
                    io_.run();
                }
            );
    }


    void TcpClient::send(
        std::vector<std::uint8_t> frame
    )
    {
        if (
            !connected()
        ) {
            return;
        }


        asio::post(
            io_,

            [
                this,
                frame =
                    std::move(
                        frame
                    )
            ]() mutable
            {
                const bool writeInProgress =
                    !writeQueue_.empty();


                writeQueue_.push_back(
                    std::move(
                        frame
                    )
                );


                if (
                    !writeInProgress
                ) {
                    writeNext();
                }
            }
        );
    }


    void TcpClient::writeNext()
    {
        if (
            writeQueue_.empty()
        ) {
            return;
        }


        asio::async_write(
            socket_,

            asio::buffer(
                writeQueue_.front()
            ),

            [this](
                const std::error_code& error,
                std::size_t bytesWritten
            )
            {
                if (error) {
                    std::cerr
                        << "network | send failed | "
                        << error.message()
                        << "\n";


                    connected_.store(
                        false
                    );


                    return;
                }


                std::cout
                    << "network | sent | bytes="
                    << bytesWritten
                    << "\n";


                writeQueue_.pop_front();


                if (
                    !writeQueue_.empty()
                ) {
                    writeNext();
                }
            }
        );
    }


    void TcpClient::read()
    {
        socket_.async_read_some(
            asio::buffer(
                readBuffer_
            ),

            [this](
                const std::error_code& error,
                std::size_t bytesRead
            )
            {
                if (error) {
                    if (
                        error !=
                        asio::error::operation_aborted
                    ) {
                        std::cout
                            << "network | disconnected";

                        if (
                            error !=
                            asio::error::eof
                        ) {
                            std::cout
                                << " | "
                                << error.message();
                        }

                        std::cout
                            << "\n";
                    }


                    connected_.store(
                        false
                    );


                    return;
                }


                try {
                    decoder_.append(
                        std::span<const std::uint8_t>(
                            readBuffer_.data(),
                            bytesRead
                        )
                    );


                    while (
                        auto packet =
                            decoder_.next()
                    ) {
                        if (
                            packetHandler_
                        ) {
                            packetHandler_(
                                *packet
                            );
                        }
                    }
                }
                catch (
                    const std::exception& exception
                ) {
                    std::cerr
                        << "protocol | decode failed | "
                        << exception.what()
                        << "\n";


                    connected_.store(
                        false
                    );


                    std::error_code closeError;


                    socket_.close(
                        closeError
                    );


                    return;
                }


                read();
            }
        );
    }

}
