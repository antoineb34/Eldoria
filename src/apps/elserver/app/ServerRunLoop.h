#pragma once

#include <chrono>
#include <memory>
#include <vector>

#include <asio.hpp>


namespace eld::elserver
{

    class Session;


    class ServerRunLoop
    {
    public:
        explicit ServerRunLoop(
            asio::io_context& context
        );


        void addSession(
            const std::shared_ptr<Session>& session
        );


        void start();


    private:
        void scheduleNextTick();


        void tick();


        static constexpr
        std::chrono::milliseconds
            TickInterval{
                600
            };


        asio::steady_timer
            timer_;


        std::vector<
            std::weak_ptr<Session>
        >
            sessions_;
    };

}
