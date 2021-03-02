#include "media_source.h"
#include "player.h"

media_source::media_source(const char* uri) {
    
    int r = avformat_open_input(fctx_, uri, nullptr, nullptr);
    if(r != 0) AV_THROW(r, "failed to open uri '{}'", uri);

    avformat_find_stream_info(fctx_, nullptr);
    av_dump_format(fctx_, 0, uri, 0);
    for(int i=0;i<fctx_->nb_streams; ++i)  {
        AVStream* s = fctx_->streams[i];
        if(s->codecpar->codec_type == AVMediaType::AVMEDIA_TYPE_VIDEO) vidx_ = i;
        else if(s->codecpar->codec_type == AVMediaType::AVMEDIA_TYPE_AUDIO) aidx_ = i;
    }
}

media_source::~media_source() {
    // status_ |= kStopping;
    stop();
    worker_->join();
}

AVStream* media_source::video() const {
    assert(has_video());
    return fctx_->streams[vidx_];
}

AVStream* media_source::audio() const {
    assert(has_audio());
    return fctx_->streams[aidx_];
}

std::pair<int, int> media_source::size() const {
    return has_video()
        ? std::pair<int, int> { video()->codecpar->width, video()->codecpar->height }
        : std::pair<int, int> { 0, 0 };
}

int media_source::fps() const {
    return has_video() ? av_q2d(video()->r_frame_rate) : 5;
}

void media_source::play() {
    if(worker_) {
        status_ |= kStopping;
        worker_->join();
        status_ ^= kStopping;
    }
    worker_.reset(new std::thread([] (media_source* self) {
        self->decode();
    }, this));
}

void media_source::decode() {
    std::unique_ptr<context_t> vctx_;
    std::unique_ptr<context_t> actx_;

    if(has_video()) {
        AVCodec* codec = avcodec_find_decoder(fctx_->streams[vidx_]->codecpar->codec_id);
        vctx_ = std::make_unique<context_t>(avcodec_alloc_context3(codec));
        avcodec_parameters_to_context(*vctx_, video()->codecpar);
        int r = avcodec_open2(*vctx_, codec, nullptr);
        if(r!=0) AV_THROW(r, "failed to open video codec '{}'", codec->name);
    }
    if(has_audio()) {
        AVCodec* codec = avcodec_find_decoder(fctx_->streams[aidx_]->codecpar->codec_id);
        actx_ = std::make_unique<context_t>(avcodec_alloc_context3(codec));
        avcodec_parameters_to_context(*actx_, audio()->codecpar);
        int r = avcodec_open2(*actx_, codec, nullptr);
        if(r!=0) AV_THROW(r, "failed to open audio codec '{}'", codec->name);
    }
    packet_t pkt(av_packet_alloc());
    while( !(status_ & kStopping) ) {
        if(av_read_frame(fctx_, pkt) <0) break;
        if(pkt->stream_index == vidx_) {
            int r = avcodec_send_packet(*vctx_, pkt);
            while(r >=0) {
                frame_t f = pool_.prepare();
                r = avcodec_receive_frame(*vctx_, f);
                if(r < 0) break;
                while(true) {
                    if(video_.push(f)) {
                        f.detach();
                        break;
                    }
                    else if(status_ & kStopping) break;
                    else SDL_Delay(5);
                }
            }
        }
        else if(pkt->stream_index == aidx_) {
            // TODO push_audio
        }
        av_packet_unref(pkt);
    }
    status_ |= kStopping;
}

void media_source::stop() {
    status_ |= kStopping; // no more decoding
}

bool media_source::more() {
    if(status_ & kStopping) return !video_.empty() || !audio_.empty();
    else return true;
}
