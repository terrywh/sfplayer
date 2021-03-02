#ifndef VIDEO_RENDER_H
#define VIDEO_RENDER_H

#include "vendor.hpp"
class media_source;

class video_render {
public:
    video_render(std::shared_ptr<media_source> src, SDL_Renderer* r, SDL_Window* win);
    ~video_render();

    std::uint32_t do_render();
private:

    std::shared_ptr<media_source>  source_;
    SDL_Renderer*  render_;
    SDL_Texture*  texture_;
    SDL_TimerID     timer_;

    SDL_Rect        sview_;
    SDL_Rect        dview_;
};

#endif // VIDEO_RENDER_H