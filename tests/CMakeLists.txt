include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

FetchContent_MakeAvailable(googletest)

add_executable(gc_tests test_gc.cpp)
target_link_libraries(gc_tests PRIVATE gc gtest_main)

add_test(NAME gc_tests COMMAND gc_tests)
