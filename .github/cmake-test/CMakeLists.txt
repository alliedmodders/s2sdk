cmake_minimum_required(VERSION 3.16)
project(s2sdk_cmake_test LANGUAGES C CXX)

# A plugin-like shared library linking s2sdk the way a consumer does
set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreaded)
add_subdirectory(../.. s2sdk)

add_library(cmake_test SHARED plugin.cpp)
target_link_libraries(cmake_test PRIVATE s2sdk::s2sdk)
