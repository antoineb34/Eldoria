#pragma once

namespace eldoria::apps::elclient {

enum class ClientScreen {
    Startup,
    PlaceholderWorld
};

struct ClientState {
    ClientScreen screen = ClientScreen::Startup;
};

}
