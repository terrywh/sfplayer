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

    void play(std::unique_ptr<media_source> src);
    void stop();
protected:
    void render(SDL_Renderer* r);
private:
    SDL_Window* window_;
    boundary_t  window_size;
    SDL_Rect    view_;

    SDL_TimerID   render_timer;
    SDL_Renderer* render_;
    SDL_Texture*  texture_ = nullptr;
    std::unique_ptr<media_source> source_;
    std::uint32_t status_;

    friend class media_source;
};

#endif // PLAYER_H
