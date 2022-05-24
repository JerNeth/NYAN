cmake_minimum_required(VERSION 3.14)

include(FetchContent)

FetchContent_Declare(
	EnTT
	GIT_REPOSITORY https://github.com/skypjack/entt.git
	GIT_TAG        master
	GIT_SHALLOW    true
)
FetchContent_MakeAvailable(EnTT)
