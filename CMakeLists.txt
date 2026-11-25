cmake_minimum_required(VERSION 3.20)
project(renderer VERSION 0.2.0 LANGUAGES C CXX)

option(RENDERER_BUILD_APP       "Build the OpenGL application (needs GLFW, glad, spdlog)" ON)
option(RENDERER_BUILD_BENCHMARK "Build the headless benchmark"                           ON)
option(RENDERER_BUILD_TESTS     "Build unit tests (downloads Catch2)"                    OFF)
option(RENDERER_ENABLE_TRACY    "Enable Tracy profiler zones (downloads Tracy)"          OFF)
set(RENDERER_SANITIZE "" CACHE STRING "Sanitizers for GCC/Clang, e.g. 'thread' or 'address,undefined'")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

if(RENDERER_SANITIZE AND NOT MSVC)
    add_compile_options(-fsanitize=${RENDERER_SANITIZE} -fno-omit-frame-pointer -g)
    add_link_options(-fsanitize=${RENDERER_SANITIZE})
endif()

function(renderer_set_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Wno-invalid-offsetof)
    endif()
endfunction()

include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.3
    GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(glm)

if(RENDERER_ENABLE_TRACY)
    FetchContent_Declare(tracy
        GIT_REPOSITORY https://github.com/wolfpld/tracy.git
        GIT_TAG        v0.11.1
        GIT_SHALLOW    TRUE)
    FetchContent_MakeAvailable(tracy)
endif()

add_library(renderer_core STATIC
    core/ParticleSystem.cpp
    core/ParticleSystem.h
    
    core/Random.h
    core/ThreadPool.h)
target_include_directories(renderer_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(renderer_core PUBLIC glm::glm)
find_package(Threads REQUIRED)
target_link_libraries(renderer_core PUBLIC Threads::Threads)
renderer_set_warnings(renderer_core)
if(RENDERER_ENABLE_TRACY)
    target_link_libraries(renderer_core PUBLIC Tracy::TracyClient)
    target_compile_definitions(renderer_core PUBLIC RENDERER_ENABLE_TRACY)
endif()



if(RENDERER_BUILD_APP)
    FetchContent_Declare(glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        3.4
        GIT_SHALLOW    TRUE)
    set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_WAYLAND  OFF CACHE BOOL "" FORCE) 

    FetchContent_Declare(glad
        GIT_REPOSITORY https://github.com/Dav1dde/glad.git
        GIT_TAG        v0.1.36
        GIT_SHALLOW    TRUE)

    FetchContent_Declare(spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG        v1.15.3
        GIT_SHALLOW    TRUE)

    FetchContent_MakeAvailable(glfw glad spdlog)

    add_executable(renderer
        src/main.cpp
        src/Application.cpp
        src/Application.h
        src/Camera.cpp
        src/Camera.h
        src/IndexBuffer.cpp
        src/IndexBuffer.h
        src/Paths.h
        src/Renderer.cpp
        src/Renderer.h
        src/Shader.cpp
        src/Shader.h
        src/VertexArray.cpp
        src/VertexArray.h
        src/VertexBuffer.cpp
        src/VertexBuffer.h
        src/Window.cpp
        src/Window.h)

    target_link_libraries(renderer PRIVATE renderer_core glfw glad spdlog::spdlog)
    target_compile_definitions(renderer PRIVATE
        GLFW_INCLUDE_NONE
        RENDERER_SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shaders")
    renderer_set_warnings(renderer)

    add_custom_command(TARGET renderer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:renderer>/shaders
        COMMENT "Copying shaders")

    if(MSVC)
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT renderer)
    endif()
endif()
