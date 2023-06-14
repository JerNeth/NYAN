cmake_minimum_required (VERSION 3.14)
include(FetchContent)

FetchContent_Declare(
  stb
  GIT_REPOSITORY https://github.com/nothings/stb.git
  GIT_TAG        master
  GIT_SHALLOW    true
)
FetchContent_MakeAvailable(stb)


file(WRITE "${stb_SOURCE_DIR}/stb.cpp" "#include \"stb_image.h\"" "\n" "#include \"stb_image_write.h\"")

add_library(stb STATIC 
	"${stb_SOURCE_DIR}/stb.cpp"
)
target_include_directories(stb SYSTEM PUBLIC
	"${stb_SOURCE_DIR}"
)
target_compile_definitions(stb PRIVATE 
    STB_IMAGE_IMPLEMENTATION
	STB_IMAGE_WRITE_IMPLEMENTATION
)