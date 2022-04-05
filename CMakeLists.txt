###############################################################################
# Top level CMakeList for building the EVT BMS source code
###############################################################################
cmake_minimum_required(VERSION 3.15)

set(EVT_CORE_DIR      ${CMAKE_SOURCE_DIR}/libs/EVT-core)

# Handle Selection of the target device
option(TARGET_DEV "Target device" "STM32F302x8")
if(NOT TARGET_DEV)
    set(TARGET_DEV "STM32F334x8")
endif()


if(TARGET_DEV STREQUAL "STM32F302x8")
    add_compile_definitions(STM32F302x8)
    add_compile_definitions(STM32F3xx)
elseif(TARGET_DEV STREQUAL "STM32F334x8")
    add_compile_definitions(STM32F334x8)
    add_compile_definitions(STM32F3xx)
else()
    message(FATAL_ERROR "The target device is not supported")
endif()


include(CMakeDependentOption)
include(${EVT_CORE_DIR}/cmake/evt-core_compiler.cmake)
include(${EVT_CORE_DIR}/cmake/evt-core_install.cmake)

# Link to the EVT-core library
add_subdirectory(libs/EVT-core/)

###############################################################################
# Project Setup
###############################################################################
set(BOARD_LIB_NAME   BMS)
if("${BOARD_LIB_NAME}" STREQUAL CHANGE_ME)
    message(FATAL_ERROR
            "You must set the template project name in CmakeLists.txt")
endif()

file(STRINGS version.txt BOARD_VERSION)

project(${BOARD_LIB_NAME}
        VERSION ${BOARD_VERSION}
        LANGUAGES CXX C
)

add_library(${PROJECT_NAME} STATIC)

# Add sources
target_sources(${PROJECT_NAME} PRIVATE
    src/BMS.cpp
    src/BMSLogger.cpp
    src/BQSettingStorage.cpp
    src/BQSetting.cpp
    src/dev/BQ76952.cpp
    src/dev/Interlock.cpp
    src/dev/SystemDetect.cpp
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
