cmake_minimum_required (VERSION 2.6)

project(reflection C CXX)

set(reflection_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/reflection/include)

set(FLAGS
  "-Wall"
  "-Wextra"
  "-fexceptions"
  "-pipe"
  "-std=c++11"
)

foreach(FLAG ${FLAGS})
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAG}")
endforeach()

find_package(Threads REQUIRED)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED COMPONENTS
  atomic
  filesystem
  regex
  system
  thread
  coroutine
  context
)

include_directories(${reflection_INCLUDE_DIR})
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

set(ext_CMAKE_ARGS
  -DCMAKE_USER_MAKE_RULES_OVERRIDE=${CMAKE_USER_MAKE_RULES_OVERRIDE}
  -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
  -DCMAKE_CC_COMPILER=${CMAKE_CC_COMPILER}
)

subdirs (reflection http)
