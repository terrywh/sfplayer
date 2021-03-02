#ifndef MEDIA_SOURCE_H
#define MEDIA_SOURCE_H

#include "vendor.hpp"
#include "utility.hpp"

class player;

class media_source {
public:
    using  format_t = wrapper<AVFormatContext, avformat_close_input>;
    using context_t = wrapper<AVCodecContext,  avcodec_free_context>;
    using   frame_t = wrapper<AVFrame,   av_frame_free>;
    using  packet_t = wrapper<AVPacket, av_packet_free>;
    class frame_pool {
    public:
        frame_pool()
        : free_(64) {}
        // 
        ~frame_pool() {
            free_.consume_all([] (AVFrame* f) {
                av_frame_free(&f);
            });
        }
        //
        frame_t prepare() {
            if(free_.empty()) 
                return frame_t(av_frame_alloc());
            else {
                AVFrame* f;
                free_.pop(f);
                return frame_t(f);
            }
        }
        void recycle(frame_t& f) {
            if(free_.push(f)) f.detach();
        }
    private:
        boost::lockfree::queue<AVFrame*> free_;
    };

    media_source(const char* uri);
    ~media_source();

    bool has_video() const {
        return vidx_ != -1;
    }
    
    bool has_audio() const {
        return aidx_ != -1;
    }

    std::pair<int, int> size() const;
    int fps() const;

    void play();
    void stop();
    template <class Fn>
    void next_video_frame(Fn&& fn) {
        frame_t f; // 使用 frame_t 防止 fn 失败时造成 frame 的内存泄漏
        if(!video_.pop(f.data())) fn(nullptr);
        else {
            fn(f);
            pool_.recycle(f);
        }
    }

    bool more();

    enum {
        kStopping = 0x00000001,
    };
private:
    format_t fctx_;
    std::uint32_t status_ = 0;

    AVStream* video() const;
    AVStream* audio() const;

    frame_pool pool_;

    int              vidx_ = -1;
    int              aidx_ = -1;
    boost::lockfree::queue<AVFrame*, boost::lockfree::capacity<64>> video_;
    boost::lockfree::queue<AVFrame*, boost::lockfree::capacity<64>> audio_;

    std::unique_ptr<std::thread> worker_;
    void decode();
};


#endif // MEDIA_SOURCE_H