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

# Configure GLFW options
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# Prevent GoogleTest from overriding compiler/linker options
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Make dependencies available
FetchContent_MakeAvailable(spdlog glm json glfw googletest)

# Find OpenMP if enabled
if(PAS_ENABLE_OPENMP)
    find_package(OpenMP)
endif()

# Source files
set(PAS_SOURCES
    src/main.cpp
    src/utils/Logger.cpp
    src/utils/Timer.cpp
)

set(PAS_HEADERS
    src/utils/Logger.hpp
    src/utils/Timer.hpp
    src/physics/Constants.hpp
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
        src/utils/Logger.cpp
        src/utils/Timer.cpp
    )

    add_executable(pas_tests ${PAS_TEST_SOURCES})

    target_include_directories(pas_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_link_libraries(pas_tests PRIVATE
        GTest::gtest
        GTest::gtest_main
        spdlog::spdlog
        glm::glm
    )

    include(GoogleTest)
    gtest_discover_tests(pas_tests)
endif()

# Installation
install(TARGETS pas RUNTIME DESTINATION bin)
install(DIRECTORY shaders/ DESTINATION bin/shaders)
install(DIRECTORY assets/ DESTINATION bin/assets)
install(DIRECTORY config/ DESTINATION bin/config)
