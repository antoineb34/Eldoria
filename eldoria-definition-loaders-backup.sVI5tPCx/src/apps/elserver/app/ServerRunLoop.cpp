#include "ServerRunLoop.h"

#include <iostream>

namespace eld::elserver {

void ServerRunLoop::tick() {
    std::cout << "ElServer run loop tick." << std::endl;
}

}
