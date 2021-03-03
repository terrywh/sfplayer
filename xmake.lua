set_project("wplayer")
set_version("0.0.1")

target("sfplayer")
    set_kind("binary")
    add_rules("mode.debug", "mode.release")
    add_files("src/*.cpp")
    add_links("SDL2", "avformat", "avcodec", "swresample","avutil", "fmt", "pthread")
    set_pcxxheader("src/vendor.hpp")

target("test")
    set_kind("binary")
    add_files("test/*.cpp")