#include "vendor.hpp"
#include "application.h"


int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_TIMER);

    std::unique_ptr<application> app = std::make_unique<application>();
    app->run();
    return 0;
}