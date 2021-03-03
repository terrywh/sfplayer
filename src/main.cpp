#include "vendor.hpp"
#include "application.h"
#include <malloc.h>

using  buffer_t = wrapper<AVBufferRef, av_buffer_unref>;

int main(int argc, char* argv[]) {
    mallopt(M_TRIM_THRESHOLD, 16 * 1024);

    std::unique_ptr<application> app = std::make_unique<application>();
    app->run();
    return 0;
}