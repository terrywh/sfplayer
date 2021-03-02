#include <cstdint>

#include <list>
#include <memory>
#include <iostream>
#include <thread>
#include <utility>

#include <fmt/core.h>

#include <boost/lockfree/queue.hpp>

#include <SDL2/SDL.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}
