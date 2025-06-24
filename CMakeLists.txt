cmake_minimum_required(VERSION 3.15...3.26)

# Set macros
option(Binary "Build as a C++ terminal program" ON)
option(PythonLib "Build as a Python package" OFF)
if(PythonLib)
  set(Binary OFF)
endif()

# Set project name and version
if(Binary)
  set(PROJECT_NAME "BTLS")
  set(PROJECT_VERSION "1.3.6")
elseif(PythonLib)
  set(PROJECT_NAME "libbtls")
  set(PROJECT_VERSION "${SKBUILD_PROJECT_VERSION}")
endif()

project(${PROJECT_NAME} VERSION ${PROJECT_VERSION} LANGUAGES CXX)

# Extra flags to compiler
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_STANDARD 17)
set(CPP_EXTRA_COMPILE_ARGS "-fPIC")  # To generate position independent code
if(DEBUG_BUILD)
  list(APPEND CPP_EXTRA_COMPILE_ARGS "-g")
endif()

# Platform-specific settings
if(APPLE)
  list(APPEND CPP_EXTRA_COMPILE_ARGS "-stdlib=libc++" "-mmacosx-version-min=10.15")
elseif(UNIX)
  list(APPEND CPP_EXTRA_COMPILE_ARGS "")  # For warnings: -Wall -Wextra & then -Weffc++ for extra fun
elseif(WIN32)
  list(APPEND CPP_EXTRA_COMPILE_ARGS "")
endif()

# Search for source files
aux_source_directory(${PROJECT_SOURCE_DIR}/cpp/src SRC_LIST)
if(Binary)
  list(REMOVE_ITEM SRC_LIST "${PROJECT_SOURCE_DIR}/cpp/src/py_main.cpp")
elseif(PythonLib)
  list(REMOVE_ITEM SRC_LIST "${PROJECT_SOURCE_DIR}/cpp/src/main.cpp")
endif()

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/cpp/include)

# Find Python and pybind11
if(PythonLib)
  find_package(
    Python
    COMPONENTS Interpreter Development.Module
    REQUIRED)
  find_package(pybind11 CONFIG REQUIRED)
endif()

# Compiling target options
if(Binary)
  add_executable(${PROJECT_NAME} ${SRC_LIST})
  target_compile_definitions(
    ${PROJECT_NAME} 
    PRIVATE VERSION_INFO=${PROJECT_VERSION}
    PRIVATE BTLS
  )
  target_compile_options(
    ${PROJECT_NAME} 
    PRIVATE ${CPP_EXTRA_COMPILE_ARGS}
  )
  set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
elseif(PythonLib)
  pybind11_add_module(${PROJECT_NAME} MODULE ${SRC_LIST})
  target_compile_definitions(
    ${PROJECT_NAME}
    PRIVATE VERSION_INFO=${PROJECT_VERSION}
    PRIVATE PyBTLS
  )
  target_compile_options(
    ${PROJECT_NAME}
    PRIVATE ${CPP_EXTRA_COMPILE_ARGS}
    )
  install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION pybtls/lib)
endif()
