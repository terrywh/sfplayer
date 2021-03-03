#include "video_render.h"
#include "media_source.h"
#include "application.h"

video_render::video_render(std::shared_ptr<media_source> src, SDL_Renderer* r, SDL_Window* win)
: source_(src)
, render_(r) {

    int w, h;
    std::tie(w, h) = source_->size();
    sview_ = {0, 0, w, h};
    texture_ = SDL_CreateTexture(render_, SDL_PixelFormatEnum::SDL_PIXELFORMAT_IYUV, SDL_TextureAccess::SDL_TEXTUREACCESS_STREAMING, w, h);

    SDL_GetWindowSize(win, &w, &h);
    dview_ = {0, 0, w, h};
    
    timer_ = SDL_AddTimer(1000/source_->video_rate(), [] (std::uint32_t interval, void* data) -> std::uint32_t {
        auto self = static_cast<video_render*>(data);
        return self->do_render();
    }, this);
}

video_render::~video_render() {
    SDL_RemoveTimer(timer_);
    SDL_DestroyTexture(texture_);
}

std::uint32_t video_render::do_render() {
    SDL_RenderClear(render_);
    if(!source_->more()) {
        SDL_RenderPresent(render_);
        delete this; // !! 线程、任务型对象，需要自行销毁
        SDL_Event e;
        e.type = application::kOnMediaDone;
        SDL_PushEvent(&e);
        return 0;
    }
    source_->next_vframe([this] (AVFrame* f) {
        if(f) {
            SDL_UpdateYUVTexture(texture_, &sview_, f->data[0], f->linesize[0], f->data[1], f->linesize[1], f->data[2], f->linesize[2]);
            SDL_RenderCopy(render_, texture_, &sview_, &dview_);
        }
    });
    SDL_RenderPresent(render_);
    return 1000/source_->video_rate();
}
