cmake_minimum_required(VERSION 3.20)
project(ParticleAcceleratorSimulation VERSION 1.0.0 LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Options
option(PAS_BUILD_TESTS "Build unit tests" ON)
option(PAS_ENABLE_OPENMP "Enable OpenMP for parallel particle updates" ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Compiler warnings
if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Fetch external dependencies
include(FetchContent)

# spdlog - Logging library
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.12.0
)

# GLM - Math library
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)

# nlohmann_json - JSON library
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)

# GLFW - Window/Input management
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)

# GoogleTest - Testing framework
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)

# Dear ImGui - UI library
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.90.1
)

# Configure GLFW options
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# Prevent GoogleTest from overriding compiler/linker options
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Make dependencies available
FetchContent_MakeAvailable(spdlog glm json glfw googletest imgui)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Create GLAD library (OpenGL loader)
# We'll use embedded sources for simplicity
add_library(glad STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/src/glad.c
)
target_include_directories(glad PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include
)

# Create ImGui library
add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui PRIVATE glfw OpenGL::GL)

# Find OpenMP if enabled
if(PAS_ENABLE_OPENMP)
    find_package(OpenMP)
endif()

# Source files
set(PAS_SOURCES
    src/main.cpp
    src/utils/Logger.cpp
    src/utils/Timer.cpp
    src/physics/Particle.cpp
    src/physics/EMField.cpp
    src/physics/Integrator.cpp
    src/physics/ParticleSystem.cpp
    src/physics/PhysicsEngine.cpp
    src/accelerator/Component.cpp
    src/accelerator/Accelerator.cpp
    src/core/Window.cpp
    src/rendering/Shader.cpp
    src/rendering/Camera.cpp
    src/rendering/Mesh.cpp
    src/rendering/ParticleRenderer.cpp
    src/rendering/AcceleratorRenderer.cpp
    src/rendering/Renderer.cpp
    src/ui/ControlPanel.cpp
    src/ui/BeamStatsPanel.cpp
    src/config/Config.cpp
)

set(PAS_HEADERS
    src/utils/Logger.hpp
    src/utils/Timer.hpp
    src/physics/Constants.hpp
    src/physics/Particle.hpp
    src/physics/EMField.hpp
    src/physics/Integrator.hpp
    src/physics/ParticleSystem.hpp
    src/physics/PhysicsEngine.hpp
    src/accelerator/Component.hpp
    src/accelerator/Accelerator.hpp
    src/core/Window.hpp
    src/rendering/Shader.hpp
    src/rendering/Camera.hpp
    src/rendering/Mesh.hpp
    src/rendering/ParticleRenderer.hpp
    src/rendering/AcceleratorRenderer.hpp
    src/rendering/Renderer.hpp
    src/ui/UIPanel.hpp
    src/ui/ControlPanel.hpp
    src/ui/BeamStatsPanel.hpp
    src/config/Config.hpp
)

# Main executable
add_executable(pas ${PAS_SOURCES} ${PAS_HEADERS})

target_include_directories(pas PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(pas PRIVATE
    spdlog::spdlog
    glm::glm
    nlohmann_json::nlohmann_json
    glfw
    glad
    imgui
    OpenGL::GL
)

if(OpenMP_CXX_FOUND AND PAS_ENABLE_OPENMP)
    target_link_libraries(pas PRIVATE OpenMP::OpenMP_CXX)
    target_compile_definitions(pas PRIVATE PAS_ENABLE_OPENMP)
endif()

# Copy assets to build directory
add_custom_command(TARGET pas POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:pas>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:pas>/assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/config $<TARGET_FILE_DIR:pas>/config
    COMMENT "Copying resources to build directory"
)

# Tests
if(PAS_BUILD_TESTS)
    enable_testing()

    set(PAS_TEST_SOURCES
        tests/test_main.cpp
        tests/utils/test_timer.cpp
        tests/utils/test_logger.cpp
        tests/physics/test_constants.cpp
        tests/physics/test_particle.cpp
        tests/physics/test_emfield.cpp
        tests/physics/test_integrator.cpp
        tests/physics/test_particlesystem.cpp
        tests/physics/test_physicsengine.cpp
        tests/accelerator/test_component.cpp
        tests/accelerator/test_accelerator.cpp
        tests/rendering/test_camera.cpp
        tests/rendering/test_mesh.cpp
        src/utils/Logger.cpp
        src/utils/Timer.cpp
        src/physics/Particle.cpp
        src/physics/EMField.cpp
        src/physics/Integrator.cpp
        src/physics/ParticleSystem.cpp
        src/physics/PhysicsEngine.cpp
        src/accelerator/Component.cpp
        src/accelerator/Accelerator.cpp
        src/rendering/Camera.cpp
        src/rendering/Mesh.cpp
    )

    add_executable(pas_tests ${PAS_TEST_SOURCES})

    target_include_directories(pas_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include
    )

    target_link_libraries(pas_tests PRIVATE
        GTest::gtest
        GTest::gtest_main
        spdlog::spdlog
        glm::glm
        glad
        OpenGL::GL
    )

    include(GoogleTest)
    gtest_discover_tests(pas_tests)
endif()

# Installation
install(TARGETS pas RUNTIME DESTINATION bin)
install(DIRECTORY shaders/ DESTINATION bin/shaders)
install(DIRECTORY assets/ DESTINATION bin/assets)
install(DIRECTORY config/ DESTINATION bin/config)
