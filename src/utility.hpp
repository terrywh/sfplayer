#ifndef UTILITIES_H
#define UTILITIES_H

#include "vendor.hpp"

struct boundary_t {
    int w;
    int h;
};

template <class T>
class defer {
public:
    defer(const T& fn)
    : fn_(fn) {}

    ~defer() {
        fn_();
    }
private:
    T fn_;
};


template <typename T, void (*F)(T**)>
class wrapper {
public:
    wrapper()
    : t_(nullptr) {

    }
    wrapper(T* t)
    : t_(t) {}

    wrapper(const wrapper& w) = default;
    wrapper(wrapper&& w) {
        t_ = w.t_;
        w.t_ = nullptr;
    }

    operator T*() const {
        return t_;
    }

    operator T**() {
        return &t_;
    }

    T* operator ->() const {
        return t_;
    }

    void detach() {
        t_ = nullptr;
    }

    T*& data() {
        return t_;
    }

    ~wrapper() {
        if(t_) F(&t_);
    }
private:
    T* t_;
};

#define AV_THROW(error, style, ...) do { \
    char errstr[2048]; \
    av_strerror(error, errstr, sizeof(errstr)-1); \
    throw std::runtime_error( fmt::format( style ": {1}" , errstr, __VA_ARGS__) ); \
} while(0)

#endif // UTILITIES_H
