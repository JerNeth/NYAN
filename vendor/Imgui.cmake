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
	${imgui_SOURCE_DIR}/imgui_tables.cpp

	)
include_directories(${imgui_SOURCE_DIR})
add_library(imgui STATIC ${IMGUI_SRC})
target_include_directories(imgui SYSTEM PUBLIC
  ${imgui_SOURCE_DIR}
)
#target_compile_definitions(imgui PRIVATE IMGUI_DISABLE_OBSOLETE_KEYIO)
#message(${imgui_SOURCE_DIR})
#message(${IMGUI_SRC})