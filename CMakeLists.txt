cmake_minimum_required(VERSION 3.15)
project(Blackhole)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# ----------  ImGui  ----------
set(IMGUI_DIR ${PROJECT_SOURCE_DIR}/include/ImGui)

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# ---------- Sources ----------
set(SOURCES
    ${IMGUI_SOURCES}
    src/main.cpp
    src/glad.c
    src/models/shader.cpp
    src/models/particle.cpp
    src/models/sphere.cpp
    src/models/stb_image.cpp
    src/models/texture.cpp
    src/models/skybox.cpp
    src/models/startmenu.cpp
)

file(GLOB SHADER_SOURCES "src/shader/*.cpp")
list(APPEND SOURCES ${SHADER_SOURCES})
file(COPY ${CMAKE_SOURCE_DIR}/src/assets DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/src/shaders DESTINATION ${CMAKE_BINARY_DIR})
add_executable(Blackhole ${SOURCES})

# ---------- Includes ----------
target_include_directories(Blackhole PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/ImGui
    ${PROJECT_SOURCE_DIR}/include/ImGui/backends
)

# ---------- OpenGL ----------
find_package(OpenGL REQUIRED)

# ---------- GLFW via FetchContent ----------
include(FetchContent)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)

# ---------- Link ----------
target_link_libraries(Blackhole PRIVATE glfw OpenGL::GL)

if(UNIX AND NOT APPLE)
    target_link_libraries(Blackhole PRIVATE dl pthread m)
endif()

if (WIN32)
    target_link_libraries(Blackhole PRIVATE opengl32)
    target_compile_definitions(Blackhole PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
