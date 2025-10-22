cmake_minimum_required(VERSION 3.15)
project(Blackhole)

set(CMAKE_CXX_STANDARD 17)

# Include directories
include_directories(include)

# Source files
set(SOURCES
    src/main.cpp
    src/glad.c
)

# Add all .cpp files from src/shader
file(GLOB SHADER_SOURCES "src/shader/*.cpp")

# Append them to SOURCES
list(APPEND SOURCES ${SHADER_SOURCES})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add executable
add_executable(Blackhole ${SOURCES})

if(WIN32)
    target_link_directories(Blackhole PRIVATE ${CMAKE_SOURCE_DIR}/libs)
    target_link_libraries(Blackhole glfw3 opengl32 gdi32)
elseif(UNIX)
    find_package(OpenGL REQUIRED)
    target_link_libraries(Blackhole glfw ${OPENGL_gl_LIBRARY} dl m pthread)
endif()
