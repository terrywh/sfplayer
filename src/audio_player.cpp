#include "audio_player.h"
#include "media_source.h"

audio_player::audio_player(std::shared_ptr<media_source> src)
: src_(src) {
    SDL_AudioSpec want, have;
    SDL_zero(want);
    want.freq = src_->audio_rate();
    want.format = AUDIO_S16;
    want.channels = 2; // src_->audio_channels();
    // want.silence = 0;
    want.samples = src_->audio_size(); // ?? 测试音频中设置位 1024 时正确播放音频；
    want.callback = playback;
    want.userdata = this;
    dev_ = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);

    SDL_PauseAudioDevice(dev_, 0);
}

audio_player::~audio_player() {
    SDL_CloseAudioDevice(dev_);
}

void audio_player::playback(void *userdata, Uint8 * stream, int len) {
    audio_player* self = static_cast<audio_player*>(userdata);
    if(!self->src_->more()) {
        std::memset(stream, 0, len);
        delete self;
        SDL_PauseAudioDevice(self->dev_, 1);
        return;
    }
    SDL_memset(stream, 0, len);
    while(len > 0) {
        self->src_->next_aframe([stream, &len] (AVBufferRef* f) {
            if(f) {
                // std::cout << "len: " << len << " size: " << f->size << "\n";
                // SDL_MixAudioFormat(stream, f->data, AUDIO_S16, f->size, SDL_MIX_MAXVOLUME/2);
                std::memcpy(stream, f->data, len >= f->size ? f->size : len);
                len -= f->size;
            }
            else {
                len = 0;
            }
        });
    }
}