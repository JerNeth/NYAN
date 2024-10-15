cmake_minimum_required(VERSION 3.14)

include(FetchContent)


if(NOT DEFINED repo_entt OR NOT EXISTS ${repo_entt})
	message("repo_entt not locally specified, fetching remote")
	set(repo_entt https://github.com/skypjack/entt.git)
else()
	message("repo_entt locally specified: " ${repo_entt})
endif()
FetchContent_Declare(
	EnTT
	GIT_REPOSITORY ${repo_entt}
	GIT_TAG        master
	GIT_SHALLOW    true
)

FetchContent_MakeAvailable(EnTT)
