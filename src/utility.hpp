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
    using value_type = T;
    
    wrapper()
    : x_(nullptr)
    , t_(&x_) {

    }

    wrapper(value_type* t)
    : x_(t)
    , t_(&x_) {}

    wrapper(T** t)
    : x_(nullptr)
    , t_(t) {}

    wrapper(const wrapper& w) {
        // std::cout << "wrapper copy\n";
        x_ = w.x_;
        t_ = w.t_;
    }

    wrapper(wrapper&& w) {
        // std::cout << "wrapper move\n";
        if(w.t_ == &w.x_) {
            x_ = w.x_;
            t_ = &x_;
        }
        else {
            x_ = nullptr;
            t_ = w.t_;
        }
        w.x_ = nullptr;
        w.t_ = nullptr;
    }

    operator bool() const {
        return *t_ != nullptr;
    }

    operator value_type*() const {
        return *t_;
    }

    operator value_type**() const {
        return t_;
    }

    value_type* operator ->() const {
        return *t_;
    }

    void detach() {
        x_ = nullptr;
        t_ = nullptr;
    }

    T*& data() {
        return *t_;
    }

    ~wrapper() {
        if(!t_ || !(*t_)) return;
        assert(!!t_ && !!(*t_));
        F(t_);
    }
private:
    value_type*  x_;
    value_type** t_;
};

#define AV_THROW(error, style, ...) do { \
    char errstr[2048]; \
    av_strerror(error, errstr, sizeof(errstr)-1); \
    throw std::runtime_error( fmt::format( style ": {1}" , errstr, __VA_ARGS__) ); \
} while(0)

#endif // UTILITIES_H
