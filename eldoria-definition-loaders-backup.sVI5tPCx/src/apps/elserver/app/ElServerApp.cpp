#include "ElServerApp.h"

#include "ServerRunLoop.h"

#include <iostream>

namespace eld::elserver {

int ElServerApp::run() {
    ServerRunLoop runLoop;

    std::cout << "ElServer starting..." << std::endl;
    runLoop.tick();
    std::cout << "ElServer shutdown." << std::endl;

    return 0;
}

}
