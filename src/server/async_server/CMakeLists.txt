cmake_minimum_required (VERSION 2.6)
project(async_server)
#
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 
  COMPONENTS system 
             iostreams 
             filesystem 
             timer 
             chrono
             thread 
REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
include_directories(${Boost_INCLUDE_DIRS})
file(GLOB_RECURSE SOURCES "*.cc")

# Add test executable target
add_executable(async_server ${SOURCES})
target_link_libraries( async_server ${Boost_LIBRARIES} )
