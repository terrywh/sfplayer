#include "vendor.hpp"
#include "application.h"
#include <malloc.h>

int main(int argc, char* argv[]) {
    mallopt(M_TRIM_THRESHOLD, 16 * 1024);


    std::unique_ptr<application> app = std::make_unique<application>();
    app->run();
    return 0;
}