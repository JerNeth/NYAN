cmake_minimum_required (VERSION 3.14)
include(FetchContent)


if(NOT DEFINED repo_stb OR NOT EXISTS ${repo_stb})
	message("STB not locally specified, fetching remote")
	set(repo_stb https://github.com/nothings/stb.git)
endif()
FetchContent_Declare(
	stb
	GIT_REPOSITORY ${repo_stb}
	GIT_TAG        master
	GIT_SHALLOW    true
)

FetchContent_MakeAvailable(stb)

add_library(stb STATIC 
	stb.cpp
)
target_include_directories(stb SYSTEM PUBLIC
	"${stb_SOURCE_DIR}"
)