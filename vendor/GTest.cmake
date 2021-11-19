include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        main
    GIT_SHALLOW    true
)
FetchContent_MakeAvailable(googletest)
set(GTEST_BOTH_LIBRARIES
    gtest_main 
    gtest
)