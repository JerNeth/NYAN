cmake_minimum_required (VERSION 3.23)

CPMAddPackage(NAME imgui
	GITHUB_REPOSITORY ocornut/imgui
	GIT_TAG	master
	DOWNLOAD_ONLY YES
)

set(IMGUI_SRC 
	${imgui_SOURCE_DIR}/imgui.cpp
	${imgui_SOURCE_DIR}/imgui_draw.cpp
	${imgui_SOURCE_DIR}/imgui_demo.cpp
	${imgui_SOURCE_DIR}/imgui_widgets.cpp
	${imgui_SOURCE_DIR}/imgui_tables.cpp
	)
set(IMGUI_H 
	${imgui_SOURCE_DIR}/imgui.h
	${imgui_SOURCE_DIR}/imgui_internal.h
	${imgui_SOURCE_DIR}/imconfig.h
	${imgui_SOURCE_DIR}/imstb_rectpack.h
	${imgui_SOURCE_DIR}/imstb_textedit.h
	${imgui_SOURCE_DIR}/imstb_truetype.h
	)
add_library(imgui STATIC ${IMGUI_SRC})

#target_include_directories(imgui SYSTEM PUBLIC
#  ${imgui_SOURCE_DIR}
#)

target_sources(
	imgui
	PUBLIC FILE_SET HEADERS BASE_DIRS ${imgui_SOURCE_DIR} FILES ${IMGUI_H}
	PRIVATE ${IMGUI_SRC}
)

target_compile_definitions(imgui PUBLIC ImTextureID=ImU64)
#target_compile_definitions(imgui PRIVATE IMGUI_DISABLE_OBSOLETE_KEYIO)

CPMAddPackage(NAME implot
	GITHUB_REPOSITORY epezent/implot
	GIT_TAG	master
	DOWNLOAD_ONLY YES
)

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
	PUBLIC FILE_SET HEADERS BASE_DIRS ${implot_SOURCE_DIR} FILES ${IMPLOT_H}
	PRIVATE ${IMPLOT_SRC}
)
target_link_libraries(implot PRIVATE imgui)
target_include_directories(implot SYSTEM PUBLIC
  ${implot_SOURCE_DIR}
)

#TODO:https://github.com/aiekick/ImGuiFileDialog
#TODO:https://github.com/CedricGuillemet/ImGuizmo