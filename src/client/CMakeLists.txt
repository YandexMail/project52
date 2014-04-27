cmake_minimum_required (VERSION 2.6)
project(client)

if(BUILD_STATIC_EXECUTABLES) 
  set(CMAKE_EXE_LINKER_FLAGS -static) 
  set(CMAKE_FIND_LIBRARY_SUFFIXES .a) 
  set(CMAKE_EXE_LINK_DYNAMIC_C_FLAGS)       # remove -Wl,-Bdynamic 
  set(CMAKE_EXE_LINK_DYNAMIC_CXX_FLAGS) 
  set(CMAKE_SHARED_LIBRARY_C_FLAGS)         # remove -fPIC 
  set(CMAKE_SHARED_LIBRARY_CXX_FLAGS) 
  set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)    # remove -rdynamic 
  set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS) 
  # Maybe this works as well, haven't tried yet. 
  # set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS FALSE) 
else(BUILD_STATIC_EXECUTABLES) 
  # Set RPATH to use for installed targets; append linker search path 
  set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${LOFAR_LIBDIR}") 
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE) 
endif(BUILD_STATIC_EXECUTABLES) 

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

find_package(ZLib)

include_directories(${Boost_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIR})
include_directories("/home/nikki/yamail/include")
include_directories("/Users/nikki/Work/github/yamail/include")
include_directories(".")

set(CMAKE_CXX_FLAGS 
  "${CMAKE_CXX_FLAGS} -Wno-unused-variable -std=c++11")

add_executable(async_client client.cc)
set_target_properties (async_client PROPERTIES COMPILE_DEFINITIONS "IO=async_strategy")
target_link_libraries(async_client ${Boost_LIBRARIES} ${ZLIB_LIBRARY})

if(BUILD_EXTRA_CLIENTS) 
add_executable(coro_client client.cc)
set_target_properties (coro_client PROPERTIES COMPILE_DEFINITIONS "IO=coro_strategy")
target_link_libraries(coro_client ${Boost_LIBRARIES} ${ZLIB_LIBRARY})

add_executable(sync_client client.cc)
set_target_properties (sync_client PROPERTIES COMPILE_DEFINITIONS "IO=sync_strategy")
target_link_libraries(sync_client ${Boost_LIBRARIES} ${ZLIB_LIBRARY})
endif(BUILD_EXTRA_CLIENTS)
