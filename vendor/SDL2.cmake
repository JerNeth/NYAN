cmake_minimum_required (VERSION 3.14)
include(FetchContent)
FetchContent_Declare(
  SDL2
  URL https://www.libsdl.org/release/SDL2-2.0.12.tar.gz
)
FetchContent_MakeAvailable(SDL2)