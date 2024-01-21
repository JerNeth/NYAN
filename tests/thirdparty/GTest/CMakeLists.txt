include(FetchContent)


if(NOT DEFINED repo_gtest OR NOT EXISTS ${repo_gtest})
	message("GTest not locally specified, fetching remote")
	set(repo_gtest https://github.com/google/googletest.git)
endif()
FetchContent_Declare(
	googletest
	GIT_REPOSITORY ${repo_gtest}
	GIT_TAG        main
	GIT_SHALLOW    true
)

FetchContent_MakeAvailable(googletest)
set(GTEST_BOTH_LIBRARIES
    gtest_main 
    gtest
)