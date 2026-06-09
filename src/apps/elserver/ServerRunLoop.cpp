#include "ServerRunLoop.h"

#include <iostream>

namespace eldoria::apps::elserver {

void ServerRunLoop::tick() {
    std::cout << "ElServer run loop tick." << std::endl;
}

}
