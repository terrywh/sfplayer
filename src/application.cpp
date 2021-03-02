#include "application.h"

application::application() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_TIMER);
    player_ = new player();
}

application::~application() {
    delete player_;
    SDL_Quit();
}

void application::run() {
    player_->show();
    
    while(!(status_ & kExiting)) {
        handle_event();
        SDL_Delay( (status_ & kPlaying) ? 5 : 50 ); // 不需要处理得过快
    }
}

void application::handle_event() {
    while(SDL_PollEvent(&event_)) {
        switch(event_.type) {
        case SDL_QUIT:
            status_ |= kExiting;
            break;
        case SDL_WINDOWEVENT:
            // std::cout << "w: " << event.window.windowID << "\n";
            // switch(event.window.event) {
            // case SDL_WindowEventID::SDL_WINDOWEVENT_RESIZED:
            //     std::tie(width, height) = win.size();
            // }
            break;
        case SDL_KEYUP:
            switch (event_.key.keysym.sym) {
            case SDL_KeyCode::SDLK_ESCAPE:
                player_->stop();
                break;
            case SDL_KeyCode::SDLK_o: {
                SDL_Event e;
                e.type = kOnOpenMedia;
                e.user.data1 = const_cast<char*>("/home/wuhao/Downloads/SampleVideo_1280x720_10mb.mp4");
                SDL_PushEvent(&e);

                break;
            }
            // case SDL_KeyCode::SDLK_ESCAPE:
            //     win.fullscreen(false);
            //     break;
            // case SDL_KeyCode::SDLK_RETURN:
            //     win.toggle_fullscreen();
            //     break;
            }
            break;
        case kOnOpenMedia:
            player_->play(static_cast<const char*>(event_.user.data1));
            break;
        case kOnMediaDone:
            player_->stop();
        }
    }
}

void application::play(const char* uri) {
    ;
}
