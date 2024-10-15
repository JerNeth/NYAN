cmake_minimum_required (VERSION 3.14)
include(FetchContent)


CPMAddPackage(NAME stb
	GITHUB_REPOSITORY  nothings/stb
	GIT_TAG master
	DOWNLOAD_ONLY YES
)

add_library(stb STATIC 
	stb.cpp
)
target_include_directories(stb SYSTEM PUBLIC
	"${stb_SOURCE_DIR}"
)