cmake_minimum_required (VERSION 2.6)
project(sync_server)
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

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -stdlib=libc++")
include_directories(${Boost_INCLUDE_DIRS})
file(GLOB_RECURSE SOURCES "*.cc")

# Add test executable target
add_executable(sync_server ${SOURCES})
target_link_libraries( sync_server ${Boost_LIBRARIES} )
