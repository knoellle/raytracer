cmake_minimum_required(VERSION 3.10)

project(RayTracingInOneWeekend)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(TBB CONFIG REQUIRED)
find_package(Threads REQUIRED)

include_directories(/usr/include/stb/)

SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
SET(BUILD_SHARED_LIBS OFF)
SET(CMAKE_EXE_LINKER_FLAGS "-static")

add_executable(raytracer main.cpp)

target_link_libraries(raytracer PRIVATE
  TBB::tbb
  Threads::Threads
)
