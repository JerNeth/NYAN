cmake_minimum_required (VERSION 3.14)
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(googletest)
set(GTEST_BOTH_LIBRARIES
    gtest_main 
    gtest
)