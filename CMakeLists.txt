cmake_minimum_required(VERSION 3.15)
project(rbfx-tps-example)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find rbfx SDK
# Adjust this path to match your rbfx installation
set(RBFX_SDK_PATH "" CACHE PATH "Path to rbfx SDK")

if(NOT RBFX_SDK_PATH)
    message(FATAL_ERROR "Please set RBFX_SDK_PATH to your rbfx SDK installation directory")
endif()

# Include rbfx CMake modules
list(APPEND CMAKE_MODULE_PATH "${RBFX_SDK_PATH}/CMake/Modules")
include(rbfx)

# Find rbfx package
find_package(rbfx REQUIRED)

# Source files
set(SOURCES
    Source/Main.cpp
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link against rbfx
target_link_libraries(${PROJECT_NAME} rbfx)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${RBFX_SDK_PATH}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

# Copy rbfx DLLs to output directory on Windows
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${RBFX_SDK_PATH}/bin/rbfx.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()

# Install rules
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
