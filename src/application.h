#ifndef APPLICATION_H
#define APPLICATION_H

#include "vendor.hpp"
#include "player.h"
#include "media_source.h"

class application {
public:
    application();
    ~application();
    void run();
    enum {
        kExiting = 0x00000001,
        kPlaying = 0x00000002,

        kOnOpenMedia = SDL_USEREVENT + 1,
        kOnMediaDone = SDL_USEREVENT + 2,
    };
private:
    std::uint32_t status_ = 0;
    
    SDL_Event      event_;
    player*       player_ = nullptr;

    void handle_event();
    void play(const char* uri);
    void stop();
};

#endif // APPLICATION_H
