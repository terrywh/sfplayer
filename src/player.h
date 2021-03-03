#ifndef PLAYER_H
#define PLAYER_H

#include "vendor.hpp"
#include "utility.hpp"

class media_source;

class player {
public:
    


    player();
    ~player();

    enum {
        kPlaying    = 0x00000001,
        kDestory    = 0x00000002,
        kFullScreen = 0x00000010,
    };

    void show();
    std::pair<int, int> size() const {
        return {window_size.w, window_size.h};
    }
    void resize(std::pair<int, int> size);
    void fullscreen(bool full = true);
    void toggle_fullscreen();

    void play(const char* uri);
    void stop();
private:
    SDL_Window*   window_;
    SDL_Renderer* render_;
    boundary_t  window_size;
    std::shared_ptr<media_source> source_;
    std::uint32_t status_;

    friend class media_source;
};

#endif // PLAYER_H
