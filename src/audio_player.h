#ifndef AUDIO_PLAYER_H
#define AUDIO_PLAYER_H

#include "vendor.hpp"

class media_source;
class audio_player {
public:
    audio_player(std::shared_ptr<media_source> src);
    ~audio_player();
private:
    SDL_AudioDeviceID dev_;
    std::shared_ptr<media_source> src_;

    static void playback(void *userdata, Uint8 * stream, int len);
};

#endif // AUDIO_PLAYER_H