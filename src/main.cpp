#include "vendor.hpp"
#include "application.h"


int main(int argc, char* argv[]) {
    std::unique_ptr<application> app = std::make_unique<application>();
    app->run();
    return 0;
}