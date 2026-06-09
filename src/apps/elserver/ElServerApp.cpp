#include "ElServerApp.h"

#include "ServerRunLoop.h"

#include <iostream>

namespace eldoria::apps::elserver {

int ElServerApp::run() {
    ServerRunLoop runLoop;

    std::cout << "ElServer starting..." << std::endl;
    runLoop.tick();
    std::cout << "ElServer shutdown." << std::endl;

    return 0;
}

}
