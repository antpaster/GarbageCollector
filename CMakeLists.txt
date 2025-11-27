# mkdir build
# cd build
# cmake ..
# cmake --build .
# cmake --build . --target clean # cleans the compiled files, object files, libraries, executables
cmake_minimum_required(VERSION 3.15)
project(GarbageCollector LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable testing
include(CTest)
enable_testing()

# Add library
add_library(gc
    src/gc_object.cpp
    src/gc_manager.cpp
)

target_include_directories(gc PUBLIC src)

# Add executable demo
add_executable(gc_demo src/main.cpp)
target_link_libraries(gc_demo PRIVATE gc)

# Add tests
add_subdirectory(tests)