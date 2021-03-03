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
    std::cout << "~media_source\n";
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

int media_source::video_rate() const {
    return av_q2d(video()->r_frame_rate);
}

int media_source::audio_rate() const {
    return audio()->codecpar->sample_rate;
}

AVSampleFormat media_source::audio_format() const {
    return static_cast<AVSampleFormat>(audio()->codecpar->format);
}

int media_source::audio_channels() const {
    return audio()->codecpar->channels;
}

int media_source::audio_size() const {
    return audio()->codecpar->frame_size;
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

media_source::frame_pool  media_source::fpool_;
media_source::buffer_pool media_source::bpool_;

template <class Fn>
static void decode_packet(media_source::codec_context_t& ctx, media_source::packet_t& pkt, Fn&& fn) {
    int r = avcodec_send_packet(ctx, pkt);
    while(r >=0) {
        media_source::frame_t f = media_source::fpool_.prepare();
        r = avcodec_receive_frame(ctx, f);
        if(r < 0) break;
        fn(f);
    }
}

static media_source::buffer_t convert_sample_format(SwrContext* ctx, AVFrame* f) {
    // 不需要调整
    if(!ctx) return media_source::buffer_t(&f->buf[0]);

    ctx = swr_alloc_set_opts(nullptr,
                    // 输出
                    AV_CH_LAYOUT_STEREO, // av_get_default_channel_layout( 2),// fctx_->streams[aidx_]->codecpar->channels),
                    AV_SAMPLE_FMT_S16, // 配合 SDL 支持的类型
                    f->sample_rate,
                    // 输入
                    f->channel_layout,
                    static_cast<AVSampleFormat>(f->format),
                    f->sample_rate,
                // 日志
                0, nullptr);
    swr_init(ctx);

    int dst_nb_samples = av_rescale_rnd(
        swr_get_delay(ctx, f->sample_rate) + f->nb_samples, f->sample_rate, f->sample_rate, AV_ROUND_UP);

    // av_buffer_alloc(dst_nb_samples * av_get_bytes_per_sample(AV_SAMPLE_FMT_S32) * f->channels)
    media_source::buffer_t buffer = media_source::bpool_.prepare();
    int nb_samples = swr_convert(ctx, &buffer->data, dst_nb_samples, (const uint8_t**)f->data, f->nb_samples);
    buffer->size = av_samples_get_buffer_size(nullptr, 2, nb_samples, AV_SAMPLE_FMT_S16, 1); // f->channels * nb_samples * av_get_bytes_per_sample(AV_SAMPLE_FMT_S32);

    swr_free(&ctx);
    return buffer;
}

void media_source::decode() {
    std::unique_ptr<codec_context_t> vctx_;
    std::unique_ptr<codec_context_t> actx_;
    std::unique_ptr<swr_context_t>   aswr_;
    
    if(has_video()) {
        AVCodec* codec = avcodec_find_decoder(fctx_->streams[vidx_]->codecpar->codec_id);
        vctx_ = std::make_unique<codec_context_t>(avcodec_alloc_context3(codec));
        avcodec_parameters_to_context(*vctx_, video()->codecpar);
        int r = avcodec_open2(*vctx_, codec, nullptr);
        if(r!=0) AV_THROW(r, "failed to open video codec '{}'", codec->name);
    }
    if(has_audio()) {
        AVCodec* codec = avcodec_find_decoder(fctx_->streams[aidx_]->codecpar->codec_id);
        actx_ = std::make_unique<codec_context_t>(avcodec_alloc_context3(codec));
        avcodec_parameters_to_context(*actx_, audio()->codecpar);
        int r = avcodec_open2(*actx_, codec, nullptr);
        if(r!=0) AV_THROW(r, "failed to open audio codec '{}'", codec->name);
        // 标记需要转换
        if(static_cast<AVSampleFormat>(fctx_->streams[aidx_]->codecpar->format) != AV_SAMPLE_FMT_S32) {
            aswr_ = std::make_unique<swr_context_t>(swr_alloc_set_opts(nullptr,
                    // 输出
                    av_get_default_channel_layout( 2),// fctx_->streams[aidx_]->codecpar->channels),
                    AV_SAMPLE_FMT_S16, // 配合 SDL 支持的类型
                    fctx_->streams[aidx_]->codecpar->sample_rate,
                    // 输入
                    fctx_->streams[aidx_]->codecpar->channel_layout,
                    static_cast<AVSampleFormat>(fctx_->streams[aidx_]->codecpar->format),
                    fctx_->streams[aidx_]->codecpar->sample_rate,
                // 日志
                0, nullptr));
            int r = swr_init(*aswr_);
            assert(r == 0);
        }
    }
    packet_t pkt(av_packet_alloc());
    while( !(status_ & kStopping) ) {
        if(av_read_frame(fctx_, pkt) <0) break;
        if(pkt->stream_index == vidx_) {
            decode_packet(*vctx_, pkt, [this] (frame_t& f) {
                while(true) {
                    if(video_.push(f)) break;
                    else if(status_ & kStopping) break;
                    else SDL_Delay(1);
                }
            });
        }
        else if(pkt->stream_index == aidx_) {
            decode_packet(*actx_, pkt, [this, aswr = aswr_.get()] (frame_t& f) {
                buffer_t buf = convert_sample_format(*aswr, f);
                while(true) {
                    if(audio_.push(buf)) break;
                    else if(status_ & kStopping) break;
                    else SDL_Delay(1);
                }
            });
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
