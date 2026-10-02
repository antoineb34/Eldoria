#include "Client.h"

#include <exception>
#include <iostream>

#include "network/ClientSession.h"


int main()
{
    eld::client::ClientSession
        server;


    try {
        server.connect(
            "127.0.0.1",
            43594
        );
    }
    catch (
        const std::exception& error
    ) {
        std::cerr
            << "network | connection failed | "
            << error.what()
            << "\n";
    }


    eld::client::Client
        client(
            server
        );


    return
        client.run();
}
