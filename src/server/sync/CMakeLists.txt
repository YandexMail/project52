cmake_minimum_required (VERSION 2.6)
project(sync_server)
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

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
include_directories(${Boost_INCLUDE_DIRS} ../../)

# Add test executable target
add_executable(fork_server fork_server.cc)
target_link_libraries(fork_server ${Boost_LIBRARIES} )

add_executable(sync_server sync_server.cc)
target_link_libraries(sync_server ${Boost_LIBRARIES} )

