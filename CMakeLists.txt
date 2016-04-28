cmake_minimum_required (VERSION 2.6)

project(reflection C CXX)

set(FLAGS
  "-Wall"
  "-Wextra"
  "-fexceptions"
  "-pipe"
  "-std=c++11"
  "-DBOOST_PP_VARIADICS=1"
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
  chrono
)

include_directories(SYSTEM ${Boost_INCLUDE_DIRS})


find_library(yajl_LIBRARIES yajl)
message(STATUS "yajl:" ${yajl_LIBRARIES})

find_path(yajl_INCLUDE_DIR
  NAMES yajl/yajl_common.h
)
message(STATUS "yajl include dir:" ${yajl_INCLUDE_DIR})
include_directories(${yajl_INCLUDE_DIR})


find_package(Protobuf)
include_directories(${PROTOBUF_INCLUDE_DIRS})


subdirs(googletest)

set(gtest_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/googletest/googletest/include)
set(gmock_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/googletest/googlemock/include)

include_directories(${gtest_INCLUDE_DIR})
include_directories(${gmock_INCLUDE_DIR})


set(reflection_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/reflection/include)
include_directories(${reflection_INCLUDE_DIR})


include_directories(${PROJECT_SOURCE_DIR})


set(ext_CMAKE_ARGS
  -DCMAKE_USER_MAKE_RULES_OVERRIDE=${CMAKE_USER_MAKE_RULES_OVERRIDE}
  -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
  -DCMAKE_CC_COMPILER=${CMAKE_CC_COMPILER}
)

subdirs (reflection http app model)
