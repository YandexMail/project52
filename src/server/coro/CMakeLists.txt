cmake_minimum_required (VERSION 2.6)
project(coro_server)
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
             coroutine
             context
REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
include_directories(${Boost_INCLUDE_DIRS} ../../)

# Add test executable target
add_executable(coro_server coro_server.cc)
target_link_libraries(coro_server ${Boost_LIBRARIES} )

