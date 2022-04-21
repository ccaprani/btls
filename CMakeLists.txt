cmake_minimum_required(VERSION 3.4...3.22)
project(BtlsPy VERSION 2.0.0)

# option(Mac "|To build on MacOS|" OFF)
option(Win "|To build on Windows|" OFF)
option(Py "|To generate a Python package|" OFF)
option(Build "|To build an executable file|" OFF)

configure_file (
  "${PROJECT_SOURCE_DIR}/config/BTLS_Config.h.in"
  "${PROJECT_SOURCE_DIR}/config/BTLS_Config.h"
)

# if(Mac)
#   set(CMAKE_C_COMPILER "/opt/homebrew/Cellar/llvm/13.0.1_1/bin/clang")
#   set(CMAKE_CXX_COMPILER "/opt/homebrew/Cellar/llvm/13.0.1_1/bin/clang++")
# endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(Win)
  # Please build based on Visual Studio
endif()

if(Py)
  add_subdirectory(pybind11)
  aux_source_directory(./src SRC_LIST)
  include_directories(./include)
  include_directories(./config)
  pybind11_add_module(BtlsPy ${SRC_LIST})
endif()

if(Build)
  add_subdirectory(src)

  include(CTest)
  enable_testing()

  set(CPACK_PROJECT_NAME ${PROJECT_NAME})
  set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
  include(CPack)
endif()