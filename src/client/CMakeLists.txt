cmake_minimum_required (VERSION 2.6)
project(client)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 
  COMPONENTS system 
             iostreams 
             filesystem 
             timer 
             chrono
             thread 
             coroutine
             context
REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(".")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-variable -std=c++11")

add_executable(sync_client client.cc)
set_target_properties (sync_client PROPERTIES COMPILE_DEFINITIONS "IO=sync_strategy")
target_link_libraries(sync_client ${Boost_LIBRARIES} )

add_executable(async_client client.cc)
set_target_properties (async_client PROPERTIES COMPILE_DEFINITIONS "IO=async_strategy")
target_link_libraries(async_client ${Boost_LIBRARIES} )

add_executable(coro_client client.cc)
set_target_properties (coro_client PROPERTIES COMPILE_DEFINITIONS "IO=coro_strategy")
target_link_libraries(coro_client ${Boost_LIBRARIES} )
