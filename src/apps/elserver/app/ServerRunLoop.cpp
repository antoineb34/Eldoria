#include "ServerRunLoop.h"

#include <algorithm>
#include <system_error>

#include "session/Session.h"


namespace eld::elserver
{

    ServerRunLoop::ServerRunLoop(
        asio::io_context& context
    )
        : timer_(
              context
          )
    {
    }


    void ServerRunLoop::addSession(
        const std::shared_ptr<Session>& session
    )
    {
        sessions_.push_back(
            session
        );
    }


    void ServerRunLoop::start()
    {
        scheduleNextTick();
    }


    void ServerRunLoop::scheduleNextTick()
    {
        timer_.expires_after(
            TickInterval
        );


        timer_.async_wait(
            [this](
                const std::error_code& error
            )
            {
                if (
                    error
                ) {
                    return;
                }


                tick();


                scheduleNextTick();
            }
        );
    }


    void ServerRunLoop::tick()
    {
        for (
            auto iterator =
                sessions_.begin();

            iterator !=
                sessions_.end();
        ) {
            if (
                auto session =
                    iterator->lock()
            ) {
                session->tick();


                ++iterator;
            }
            else {
                iterator =
                    sessions_.erase(
                        iterator
                    );
            }
        }
    }

}
