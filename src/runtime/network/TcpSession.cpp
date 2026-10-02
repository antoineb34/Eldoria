#include "TcpSession.h"

#include <iostream>
#include <span>
#include <system_error>
#include <utility>


namespace eld::network
{

    TcpSession::TcpSession(
        asio::ip::tcp::socket socket
    )
        : socket_(
              std::move(
                  socket
              )
          )
    {
    }


    void TcpSession::setPacketHandler(
        PacketHandler handler
    )
    {
        packetHandler_ =
            std::move(
                handler
            );
    }


    void TcpSession::start()
    {
        std::error_code error;


        const auto endpoint =
            socket_.remote_endpoint(
                error
            );


        if (!error) {
            std::cout
                << "network | client connected | "
                << endpoint.address().to_string()
                << ":"
                << endpoint.port()
                << "\n";
        }
        else {
            std::cout
                << "network | client connected\n";
        }


        read();
    }


    void TcpSession::send(
        std::vector<std::uint8_t> frame
    )
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


    void TcpSession::writeNext()
    {
        if (
            writeQueue_.empty()
        ) {
            return;
        }


        const auto self =
            shared_from_this();


        asio::async_write(
            socket_,

            asio::buffer(
                writeQueue_.front()
            ),

            [
                self
            ](
                const std::error_code& error,
                std::size_t bytesWritten
            )
            {
                if (error) {
                    std::cerr
                        << "network | send failed | "
                        << error.message()
                        << "\n";


                    return;
                }


                std::cout
                    << "network | sent | bytes="
                    << bytesWritten
                    << "\n";


                self
                    ->writeQueue_
                    .pop_front();


                if (
                    !self
                        ->writeQueue_
                        .empty()
                ) {
                    self->writeNext();
                }
            }
        );
    }


    void TcpSession::read()
    {
        const auto self =
            shared_from_this();


        socket_.async_read_some(
            asio::buffer(
                readBuffer_
            ),

            [
                self
            ](
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
                            << "network | client disconnected";

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


                    return;
                }


                try {
                    self->decoder_.append(
                        std::span<const std::uint8_t>(
                            self
                                ->readBuffer_
                                .data(),
                            bytesRead
                        )
                    );


                    while (
                        auto packet =
                            self
                                ->decoder_
                                .next()
                    ) {
                        if (
                            self->packetHandler_
                        ) {
                            self->packetHandler_(
                                *packet
                            );
                        }
                        else {
                            std::cout
                                << "network | packet dropped"
                                << " | no handler"
                                << "\n";
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


                    std::error_code closeError;


                    self
                        ->socket_
                        .close(
                            closeError
                        );


                    return;
                }


                self->read();
            }
        );
    }

}
