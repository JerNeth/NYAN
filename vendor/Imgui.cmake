cmake_minimum_required (VERSION 3.14)
include(FetchContent)
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        master
  GIT_SHALLOW    true
)
FetchContent_MakeAvailable(imgui)
set(IMGUI_SRC 
	${imgui_SOURCE_DIR}/imgui.cpp
	${imgui_SOURCE_DIR}/imgui_draw.cpp
	${imgui_SOURCE_DIR}/imgui_demo.cpp
	${imgui_SOURCE_DIR}/imgui_widgets.cpp
	)
include_directories(${imgui_SOURCE_DIR})