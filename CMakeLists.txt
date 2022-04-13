cmake_minimum_required(VERSION 3.0.0)
project(BTLS VERSION 2.0.0)

configure_file (
  "${PROJECT_SOURCE_DIR}/config/BTLS_Config.h.in"
  "${PROJECT_SOURCE_DIR}/config/BTLS_Config.h"
)

option(Mac "|To build on MacOS|" OFF)
option(Win "|To build on Windows|" OFF)

if(Mac)
  set(CMAKE_C_COMPILER "/opt/homebrew/Cellar/llvm/13.0.1_1/bin/clang")
  set(CMAKE_CXX_COMPILER "/opt/homebrew/Cellar/llvm/13.0.1_1/bin/clang++")
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#[[
if(Win)
  add_subdirectory()
  list(APPEND EXTRA_LIBS )
  list(APPEND EXTRA_INCLUDES "${PROJECT_SOURCE_DIR}/")
endif()
]]

add_subdirectory(src)

include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
