#include "TcpServer.h"

#include <iostream>
#include <memory>
#include <system_error>
#include <utility>

#include "TcpSession.h"


namespace eld::network
{

    TcpServer::TcpServer(
        asio::io_context& context,
        std::uint16_t port,
        SessionHandler sessionHandler
    )
        : acceptor_(
              context,
              asio::ip::tcp::endpoint(
                  asio::ip::tcp::v4(),
                  port
              )
          ),
          sessionHandler_(
              std::move(
                  sessionHandler
              )
          )
    {
    }


    std::uint16_t TcpServer::port() const
    {
        return
            acceptor_
                .local_endpoint()
                .port();
    }


    void TcpServer::start()
    {
        std::cout
            << "network | listening on 0.0.0.0:"
            << port()
            << "\n";


        acceptNext();
    }


    void TcpServer::acceptNext()
    {
        acceptor_.async_accept(
            [
                this
            ](
                const std::error_code& error,
                asio::ip::tcp::socket socket
            )
            {
                if (!error) {
                    auto transport =
                        std::make_shared<TcpSession>(
                            std::move(
                                socket
                            )
                        );


                    if (
                        sessionHandler_
                    ) {
                        sessionHandler_(
                            transport
                        );
                    }


                    transport->start();
                }
                else if (
                    error !=
                    asio::error::operation_aborted
                ) {
                    std::cerr
                        << "network | accept failed | "
                        << error.message()
                        << "\n";
                }


                if (
                    acceptor_.is_open()
                ) {
                    acceptNext();
                }
            }
        );
    }

}
