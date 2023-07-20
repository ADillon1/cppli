cmake_minimum_required(VERSION 3.0)
include(FetchContent)

set(LIB_SOURCE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/cppli)
set(TEST_SOURCE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(BUILD_CPPLI_TESTS "Builds tests" OFF)

project(cppli)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# Debug build type configuration
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if (MSVC)
        add_compile_options(/Z7)
    else()
        add_compile_options(-g)
    endif()
    
    add_compile_definitions(_DEBUG)
endif()

# Release build type configuration
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
endif()

FetchContent_Declare(tokenize
GIT_REPOSITORY https://github.com/ADillon1/tokenize.git
GIT_TAG main
)

FetchContent_MakeAvailable(tokenize)

file(GLOB_RECURSE SOURCES
    "${LIB_SOURCE_DIRECTORY}/*.h"
    "${LIB_SOURCE_DIRECTORY}/*.hpp"
    "${LIB_SOURCE_DIRECTORY}/*.cpp"
    "${LIB_SOURCE_DIRECTORY}/*.inl"
)

add_library(cppli SHARED ${SOURCES})
target_link_libraries(cppli tokenize)
target_compile_definitions(cppli PRIVATE CPPLI_EXPORT)

target_include_directories(cppli PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if (BUILD_CPPLI_TESTS)
    message("Building tests")
    FetchContent_Declare(Catch2 
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v2.x
    )
    FetchContent_MakeAvailable(Catch2)
    add_executable(tests ${TEST_SOURCE_DIRECTORY}/main.cpp)
    target_link_libraries(tests Catch2)
    target_link_libraries(tests cppli)
else()
    message("Not building tests")
endif()