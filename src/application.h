#ifndef APPLICATION_H
#define APPLICATION_H

#include "vendor.hpp"
#include "player.h"

class application {
public:
    application();
    void run();
    enum {
        kExiting = 0x00000001,
        kPlaying = 0x00000002,
    };
private:
    std::uint32_t status_ = 0;
    player player_;
    SDL_Event event_;

    void handle_event();
};

#endif // APPLICATION_H
