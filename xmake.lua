set_project("wplayer")
set_version("0.0.1")

target("wplayer")
    set_kind("binary")
    add_rules("mode.debug", "mode.release")
    add_files("src/*.cpp")
    add_links("SDL2", "avformat", "avcodec","avutil", "fmt", "pthread")
    set_pcxxheader("src/vendor.hpp")