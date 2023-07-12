
# ---------------------------------------------------------------------------
# Files
# ---------------------------------------------------------------------------

set(TEST_CPP
    test/LinAlgTests.cpp
    test/Tester.cpp
    test/UtilityTests.cpp
)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
enable_testing()

add_executable(tester ${TEST_CPP} ${MATH_SRC})
target_link_libraries(tester ${GTEST_BOTH_LIBRARIES})

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    #target_compile_options(Main PUBLIC /WX /std:c++latest) 
    #target_compile_options(tester PUBLIC /WX /std:c++latest)
    #target_compile_options(tester PUBLIC /WX)
    #target_compile_options(tester PUBLIC /Mtd)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    #target_compile_options(tester PUBLIC -std=c++20 -Wall -Wextra)
    #target_compile_options(tester PUBLIC -Wall -Wextra)
endif()

target_compile_features(tester PUBLIC 
    cxx_std_20
)
add_test(NAME Utility.UnitTests 
        COMMAND tester)