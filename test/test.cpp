#include <iostream>

class A {
public:
    explicit A(int* a)
    : a_(a) {}

    operator int*() const {
        return a_;
    }

    operator int**() const {
        return b_;
    }

    operator bool() const {
        return a_ != nullptr;
    }
private:
    int *  a_;
    int ** b_;
};

int main() {
    int x = 10;
    A a {&x};

    if(a) {
        std::cout << "full\n";
    }
    else {
        std::cout << "none\n";
    }

    return 0;
}