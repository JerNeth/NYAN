cmake_minimum_required(VERSION 3.14)

include(FetchContent)

FetchContent_Declare(
	EnTT_Imgui
	GIT_REPOSITORY https://github.com/Green-Sky/imgui_entt_entity_editor
	GIT_TAG        master
	GIT_SHALLOW    true
)
FetchContent_MakeAvailable(EnTT_Imgui)
