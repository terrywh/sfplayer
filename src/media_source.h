#ifndef MEDIA_SOURCE_H
#define MEDIA_SOURCE_H

#include "vendor.hpp"
#include "utility.hpp"

class player;

class media_source {
public:
    using  format_t = wrapper<AVFormatContext, avformat_close_input>;
    using codec_context_t = wrapper<AVCodecContext,  avcodec_free_context>;
    using swr_context_t = wrapper<SwrContext, swr_free>;
    using   frame_t = wrapper<AVFrame,   av_frame_free>;
    using  packet_t = wrapper<AVPacket, av_packet_free>;
    using  buffer_t = wrapper<AVBufferRef, av_buffer_unref>;

    class frame_pool {
    public:
        frame_pool()
        : free_(24) { }
        // 
        ~frame_pool() {
            free_.consume_all([] (AVFrame* f) {
                av_frame_free(&f);
            });
        }
        //
        frame_t prepare() {
            AVFrame* f = nullptr;
            if(!free_.pop(f)) {
                f = av_frame_alloc();
                assert(f);
            }
            assert(f);
            return frame_t(f);
        }
        void recycle(frame_t& f) {
            if(!f) return;
            assert(!!f);
            if(free_.push(f)) f.detach();
        }
    private:
        boost::lockfree::queue<AVFrame*> free_;
    };

    class buffer_pool {
    public:
        buffer_pool()
        : free_(24) { }
        // 
        ~buffer_pool() {
            free_.consume_all([] (AVBufferRef* f) {
                av_buffer_unref(&f);
            });
        }
        //
        buffer_t prepare() {
            AVBufferRef* f = nullptr;
            if(!free_.pop(f)) {
                f = av_buffer_alloc(8 * 4 * 4096);
                assert(f);
            }
            assert(f);
            return buffer_t(f);
        }

        void recycle(buffer_t& f) {
            if(!f) return;
            assert(!!f);
            if(free_.push(f))
                f.detach();
        }
    private:
        boost::lockfree::queue<AVBufferRef*> free_;
    };

    template <class T, std::size_t Capacity>
    class queue {
    public:
        using value_type = typename T::value_type;
        bool push(T& f) {
            if(queue_.push(f)) {
                f.detach();
                return true;
            }
            return false;
        }
        T pop() {
            value_type* f;
            if(queue_.pop(f)) return T(f);
            else return T(static_cast<value_type*>(nullptr));
        }
        bool empty() {
            return queue_.empty();
        }
    private:
        boost::lockfree::queue<value_type*,boost::lockfree::capacity<Capacity>> queue_;
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
    int video_rate() const; // frame per second
    int audio_rate() const; // sample per second
    AVSampleFormat audio_format() const;
    int audio_channels() const;
    int audio_size() const;

    void play();
    void stop();
    template <class Fn>
    void next_vframe(Fn&& fn) {
        frame_t f = video_.pop();
        fn(f);
        fpool_.recycle(f);
    }

    template<class Fn>
    void next_aframe(Fn&& fn) {
        buffer_t f = audio_.pop();
        fn(f);
        bpool_.recycle(f);
    }

    bool more();

    enum {
        kStopping = 0x00000001,
    };

    static frame_pool  fpool_;
    static buffer_pool bpool_;
    
private:
    format_t fctx_;
    std::uint32_t status_ = 0;

    AVStream* video() const;
    AVStream* audio() const;

    

    int              vidx_ = -1;
    int              aidx_ = -1;
    queue<frame_t, 32>  video_;
    queue<buffer_t, 64> audio_;

    std::unique_ptr<std::thread> worker_;
    void decode();
};


#endif // MEDIA_SOURCE_H