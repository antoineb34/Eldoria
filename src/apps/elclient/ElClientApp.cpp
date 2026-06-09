#include "ElClientApp.h"

#include "ClientState.h"

#include <iostream>

namespace eldoria::apps::elclient {

int ElClientApp::run() {
    ClientState state;

    std::cout << "ElClient starting..." << std::endl;
    std::cout << "ElClient screen: startup" << std::endl;

    state.screen = ClientScreen::PlaceholderWorld;

    std::cout << "ElClient shutdown." << std::endl;

    return 0;
}

}
