#ifndef PLAYER_H
#define PLAYER_H

#include "vendor.hpp"

class player {
public:
    player();
    ~player();

    enum {
        kFullScreened = 0x00000001,
        kScreenToDraw = 0x00000002,
    };

    void show();

    struct window_size_t {
        int w;
        int h;
    };
    const window_size_t& size() const {
        return window_size;
    }
    void resize(const window_size_t& size);
    void fullscreen(bool full = true);
    void toggle_fullscreen();
protected:
    void render(SDL_Renderer* r);
private:
    SDL_Window* window_;

    struct window_size_t window_size;

    SDL_TimerID render_timer_;
    SDL_Renderer* render_;

    std::uint32_t status_;
};

#endif // PLAYER_H
