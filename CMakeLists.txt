cmake_minimum_required(VERSION 3.4...3.22)
project(BTLS VERSION 0.2.2)

# set macro
if(WIN32)
  option(Win "|To build on Windows|" ON)
endif()

configure_file (
  "${PROJECT_SOURCE_DIR}/c_src/config/BTLS_Config.h.in"
  "${PROJECT_SOURCE_DIR}/c_src/config/BTLS_Config.h"
)

# extra parameters to compiler
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
elseif(APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch arm64")   # -arch x86_64
  set(MACOSX_DEPLOYMENT_TARGET "10.15")
elseif(UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

# source files
aux_source_directory(${PROJECT_SOURCE_DIR}/c_src/src SRC_LIST)
list(REMOVE_ITEM SRC_LIST "${PROJECT_SOURCE_DIR}/c_src/src/py_main.cpp")
include_directories(${PROJECT_SOURCE_DIR}/c_src/include)
include_directories(${PROJECT_SOURCE_DIR}/c_src/config)

# compiling target
add_executable(BTLS ${SRC_LIST})
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# test
# include(CTest)
# enable_testing()

# compile as a lib
# set(CPACK_PROJECT_NAME ${PROJECT_NAME})
# set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# include(CPack)
