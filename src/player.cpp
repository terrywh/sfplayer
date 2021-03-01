#include "player.h"

player::player()
: window_size {640, 480}
, status_(0) {
    window_ = SDL_CreateWindow("wplayer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        window_size.w, window_size.h, 
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    
    render_ = SDL_CreateRenderer(window_, -1, SDL_RendererFlags::SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawColor(render_, 0, 0, 0, 0);
    SDL_RenderClear(render_);
    SDL_RenderPresent(render_);
    render_timer_ = SDL_AddTimer(40, [] (std::uint32_t interval, void* data) -> std::uint32_t {
        auto self = static_cast<player*>(data);
        self->render(self->render_);
        return (self->status_ & kScreenToDraw) ? 40 : 400;
    }, this);
}

player::~player() {
    SDL_RemoveTimer(render_timer_);
    SDL_DestroyRenderer(render_);
    SDL_DestroyWindow(window_);
}

void player::show() {
    SDL_ShowWindow(window_);
}

void player::resize(const window_size_t& s) {
    SDL_SetWindowSize(window_, s.w, s.h);
    SDL_GetWindowSize(window_, &window_size.w, &window_size.h);
    std::cout << window_size.w << "x" << window_size.h << "px\n";
}

void player::render(SDL_Renderer* r) {
    if(status_ & kScreenToDraw) {
        SDL_RenderClear(r);
        SDL_RenderPresent(r);
        // TODO frames queue empty ? reset status_
    }
    // int w, h;
    // std::tie(w, h) = size();
    // SDL_Rect rect {0, 0, w, h};
    // SDL_SetRenderDrawColor(r, 0, 0, 0, 0);
    // SDL_RenderFillRect(r, &rect);
}

void player::fullscreen(bool full) {
    if(full) {
        status_ |= kFullScreened;
        SDL_SetWindowFullscreen(window_, SDL_WINDOW_FULLSCREEN_DESKTOP);
    }
    else {
        status_ &= ~kFullScreened;
        SDL_SetWindowFullscreen(window_, 0);
    }

    SDL_GetWindowSize(window_, &window_size.w, &window_size.h);
    std::cout << window_size.w << "x" << window_size.h << "px\n";
}

void player::toggle_fullscreen() {
    fullscreen( !(status_ & kFullScreened) );
}