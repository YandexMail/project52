cmake_minimum_required (VERSION 2.6)
project(client)
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
             coroutine
REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
include_directories(${Boost_INCLUDE_DIRS})
include_directories(".")

add_executable(sync_client main_sync.cc)
target_link_libraries( sync_client ${Boost_LIBRARIES} )

add_executable(async_client main_async.cc)
target_link_libraries( async_client ${Boost_LIBRARIES} )

add_executable(coro_client main_coro.cc)
target_link_libraries( coro_client ${Boost_LIBRARIES} )