#ifndef RELEASE_GUARD_H
#define RELEASE_GUARD_H


class release_guard {
public:
    typedef void (*release_callback_t)(void*);
    template <class T>
    release_guard(T* t, void(* r)(T*))
    : t_(t), r_(reinterpret_cast<release_callback_t>(r)) {}
    ~release_guard() {
        r_(t_);
    }
private:
    void* t_;
    release_callback_t r_;
};

#endif // RELEASE_GUARD_H
