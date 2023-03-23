###############################################################################
# Top level CMakeList for building the EVT BMS source code
###############################################################################
cmake_minimum_required(VERSION 3.15)

# Convert CMake flags to compiler flags
if(EVT_CORE_LOG_ENABLE)
    add_compile_definitions(EVT_CORE_LOG_ENABLE)
endif()

# Allow for up to 8 CANopen TPDOs
add_compile_definitions(CO_TPDO_N=8)

add_compile_definitions(CANOPEN_QUEUE_SIZE=50)

add_compile_definitions(USE_HAL_DRIVER)

# Handle Selection of the target device
option(TARGET_DEV "Target device" "STM32F302x8")
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
set(BOARD_LIB_NAME   BMS)
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
    src/BMS.cpp
    src/BQSettingStorage.cpp
    src/BQSetting.cpp
    src/dev/BQ76952.cpp
    src/dev/Interlock.cpp
    src/dev/SystemDetect.cpp
    src/dev/ThermistorMux.cpp
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
