#include "application.h"


application::application() {

}

void application::run() {
    player_.show();
    
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
            // switch (event.key.keysym.sym) {
            // case SDL_KeyCode::SDLK_ESCAPE:
            //     win.fullscreen(false);
            //     break;
            // case SDL_KeyCode::SDLK_RETURN:
            //     win.toggle_fullscreen();
            //     break;
            // }
            ;
        }
    }
}