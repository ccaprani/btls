cmake_minimum_required(VERSION 3.4...3.22)
project(BtlsPy VERSION 2.0.0)

if(WIN32)
  option(Win "|To build on Windows|" ON)
endif()
option(Py "|To generate a Python package|" OFF)
option(Build "|To build an executable file|" OFF)

configure_file (
  "${PROJECT_SOURCE_DIR}/config/BTLS_Config.h.in"
  "${PROJECT_SOURCE_DIR}/config/BTLS_Config.h"
)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(Win)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
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