cmake_minimum_required(VERSION 3.11)

include(FetchContent)

if(NOT DEFINED repo_magicenum OR NOT EXISTS ${repo_magicenum})
	message("MagicEnum not locally specified, fetching remote")
	set(repo_magicenum https://github.com/Neargye/magic_enum.git)
endif()

FetchContent_Declare(
	magic_enum
	GIT_REPOSITORY ${repo_magicenum}
	GIT_TAG        master
	GIT_SHALLOW    true
)

FetchContent_MakeAvailable(magic_enum)