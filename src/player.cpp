#include "player.h"
#include "media_source.h"

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

    render_timer = SDL_AddTimer(100, [] (std::uint32_t interval, void* data) -> std::uint32_t {
        auto self = static_cast<player*>(data);
        self->render(self->render_);
        return self->source_ ? (1000/self->source_->fps()) : 200;
    }, this);
}

player::~player() {
    SDL_RemoveTimer(render_timer);
    if(texture_) SDL_DestroyTexture(texture_);
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

void player::render(SDL_Renderer* r) {
    SDL_RenderClear(r);
    if(status_ & kPlaying) {
        if(source_->has_video()) {
            source_->next_video_frame([this] (AVFrame* f) {
                if(!f) {
                    if(!source_->more()) 
                        status_ ^= kPlaying;
                    return;
                }
                SDL_UpdateYUVTexture(texture_, &view_, f->data[0], f->linesize[0], f->data[1], f->linesize[1], f->data[2], f->linesize[2]);
                SDL_RenderCopy(render_, texture_, &view_, &view_);
            });
        }
    }
    else {
        if(source_) source_.reset();
    }
    SDL_RenderPresent(r);
    // int w, h;
    // std::tie(w, h) = size();
    // SDL_Rect rect {0, 0, w, h};
    // SDL_SetRenderDrawColor(r, 0, 0, 0, 0);
    // SDL_RenderFillRect(r, &rect);
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

void player::play(std::unique_ptr<media_source> src) {
    source_ = std::move(src);
    source_->play();

    if(source_->has_video()) {
        view_.x = 0;
        view_.y = 0;
        std::tie(view_.w, view_.h) = source_->size();
        if(texture_) SDL_DestroyTexture(texture_);
        texture_ = SDL_CreateTexture(render_, SDL_PixelFormatEnum::SDL_PIXELFORMAT_IYUV, SDL_TextureAccess::SDL_TEXTUREACCESS_STREAMING, view_.w, view_.h);
    }
    status_ |= kPlaying;
}

void player::stop() {
    status_ ^= kPlaying;
    source_->stop(); // stop and defer the destroying to the next render cycle
}
