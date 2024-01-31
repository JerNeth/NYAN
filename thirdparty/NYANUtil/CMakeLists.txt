cmake_minimum_required (VERSION 3.14)
include(FetchContent)


if(NOT DEFINED repo_nyanutil OR NOT EXISTS ${repo_nyanutil})
	message("repo_nyanutil not locally specified, fetching remote")
	set(repo_nyanutil https://github.com/JerNeth/NYANUtil.git)
endif()
FetchContent_Declare(
	nyanutil
	GIT_REPOSITORY ${repo_nyanutil}
	GIT_TAG        master
	GIT_SHALLOW    true
)
FetchContent_GetProperties(nyanutil)
FetchContent_MakeAvailable(nyanutil)
