cmake_minimum_required (VERSION 2.6)
project(async_server)
#
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 
  COMPONENTS system 
             program_options
             iostreams 
             filesystem 
             timer 
             chrono
             thread 
             regex 
REQUIRED)

set(CMAKE_CXX_FLAGS 
  "${CMAKE_CXX_FLAGS} -std=c++11 -DBOOST_SPIRIT_THREADSAFE -DPHOENIX_THREADSAFE")
include_directories(${Boost_INCLUDE_DIRS})
include_directories(..)
file(GLOB_RECURSE SOURCES "*.cc")

# Add test executable target
add_executable(async_server ${SOURCES})
target_link_libraries( async_server ${Boost_LIBRARIES} )
