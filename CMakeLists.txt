###############################################################################
# Top level CMakeList for building the *EVT_BOARD_NAME* source code
###############################################################################
cmake_minimum_required(VERSION 3.15)

# Convert CMake flags to compiler flags
if(EVT_CORE_LOG_ENABLE)
    add_compile_definitions(EVT_CORE_LOG_ENABLE)
endif()

# Handle default selection of the target device
if(NOT TARGET_DEV)
    set(TARGET_DEV "STM32F334x8")
endif()

set(EVT_CORE_DIR      ${CMAKE_SOURCE_DIR}/libs/EVT-core)

# Link to the EVT-core library
add_subdirectory(libs/EVT-core/)

include(CMakeDependentOption)
include(${EVT_CORE_DIR}/cmake/evt-core_compiler.cmake)
include(${EVT_CORE_DIR}/cmake/evt-core_install.cmake)

###############################################################################
# Project Setup
###############################################################################
set(BOARD_LIB_NAME BOARD_NAME)
if("${BOARD_LIB_NAME}" STREQUAL BOARD_NAME)
    message(FATAL_ERROR
            "You must set the template project name in the top-level CMakeLists.txt")
endif()

file(STRINGS version.txt BOARD_VERSION)

project(${BOARD_LIB_NAME}
        VERSION ${BOARD_VERSION}
        LANGUAGES CXX C
        )

add_library(${PROJECT_NAME} STATIC)

# Add sources
target_sources(${PROJECT_NAME} PRIVATE
        src/dev/LED.cpp
        src/BOARD_NAME.cpp
        )

###############################################################################
# Handle dependencies
###############################################################################
target_link_libraries(${PROJECT_NAME}
        PUBLIC EVT
        )

###############################################################################
# Install and expose library
###############################################################################
install_and_expose(${PROJECT_NAME})

###############################################################################
# Build Target Code
###############################################################################
add_subdirectory(targets)
