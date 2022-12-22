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
add_library(imgui STATIC ${IMGUI_SRC})
target_include_directories(imgui SYSTEM PUBLIC
  ${imgui_SOURCE_DIR}
)
target_compile_definitions(imgui PUBLIC ImTextureID=ImU32)
#target_compile_definitions(imgui PRIVATE IMGUI_DISABLE_OBSOLETE_KEYIO)
#message(${imgui_SOURCE_DIR})
#message(${IMGUI_SRC})

FetchContent_Declare(
  implot
  GIT_REPOSITORY https://github.com/epezent/implot
  GIT_TAG        master
  GIT_SHALLOW    true
)
FetchContent_MakeAvailable(implot)
set(IMPLOT_SRC 
	${implot_SOURCE_DIR}/implot.cpp
	${implot_SOURCE_DIR}/implot_items.cpp
	)
set(IMPLOT_H
	${implot_SOURCE_DIR}/implot.h
	${implot_SOURCE_DIR}/implot_internal.h
	)
add_library(implot STATIC ${IMPLOT_SRC})
target_sources(
	implot
	PUBLIC 
		${IMPLOT_H}
	PRIVATE 
		${IMPLOT_SRC}
)
target_link_libraries(implot PRIVATE imgui)
target_include_directories(implot SYSTEM PUBLIC
  ${implot_SOURCE_DIR}
)

