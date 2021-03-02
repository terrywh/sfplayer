#include "player.h"
#include "media_source.h"
#include "video_render.h"

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
}

player::~player() {
    SDL_DestroyRenderer(render_);
    SDL_DestroyWindow(window_);
}

void player::show() {
    SDL_ShowWindow(window_);
}

void player::resize(std::pair<int,int> s) {
    SDL_SetWindowSize(window_, s.first, s.second);
    SDL_GetWindowSize(window_, &window_size.w, &window_size.h);
    std::cout << window_size.w << "x" << window_size.h << "px\n";
}

void player::fullscreen(bool full) {
    if(full) {
        status_ |= kFullScreen;
        SDL_SetWindowFullscreen(window_, SDL_WINDOW_FULLSCREEN_DESKTOP);
    }
    else {
        status_ &= ~kFullScreen;
        SDL_SetWindowFullscreen(window_, 0);
    }

    SDL_GetWindowSize(window_, &window_size.w, &window_size.h);
    std::cout << window_size.w << "x" << window_size.h << "px\n";
}

void player::toggle_fullscreen() {
    fullscreen( !(status_ & kFullScreen) );
}

void player::play(const char* uri) {
    stop();
    source_.reset(new media_source(uri));
    if(source_->has_video())
        new video_render(source_, render_, window_);
    
    source_->play();
}

void player::stop() {
    if(source_) {
        source_->stop(); // stop source and the audio_playback / video_render will stop accordingly
        source_.reset();
    }
}
