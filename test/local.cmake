
# ---------------------------------------------------------------------------
# Files
# ---------------------------------------------------------------------------

set(TEST_CPP
    test/LinAlgTests.cpp
    test/Tester.cpp
)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
enable_testing()

add_executable(tester ${TEST_CPP})
target_link_libraries(tester ${GTEST_BOTH_LIBRARIES})

add_test(runUnitTests tester)