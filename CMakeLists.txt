cmake_minimum_required (VERSION 2.6)

set(CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules/"
)

project(project52 C CXX)
set(TARGET_LIBRARY "project52")

set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src)
set(SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)

set(FLAGS
  "-Wall"
  "-Wextra"
  "-fexceptions"
  "-pipe"
)

foreach(FLAG ${FLAGS})
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAG}")
endforeach()

find_package(Threads REQUIRED)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.55 REQUIRED COMPONENTS
  atomic
  filesystem
  system
  thread
  coroutine
  context
)

include_directories(${INCLUDE_DIR})
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

set(ext_CMAKE_ARGS
  -DCMAKE_USER_MAKE_RULES_OVERRIDE=${CMAKE_USER_MAKE_RULES_OVERRIDE}
  -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
  -DCMAKE_CC_COMPILER=${CMAKE_CC_COMPILER}
)

include(ExternalProject)

# Import YaMail project
ExternalProject_Add(yamail
  GIT_REPOSITORY https://github.com/YandexMail/yamail.git
  TIMEOUT 10
  CMAKE_ARGS ${ext_CMAKE_ARGS}
  INSTALL_COMMAND ""
)

ExternalProject_Get_Property(yamail source_dir)
include_directories(${source_dir}/include)




subdirs (src/client src/server)
