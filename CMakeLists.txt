cmake_minimum_required(VERSION 3.10)

project(CLI11-yaml)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSION OFF)

find_package(CLI11 CONFIG REQUIRED)
find_package(Catch2 CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)

add_library(cli11-yaml
    ../include/cli11-yaml/cli11-yaml.hpp
    ./src/cli11-yaml.cpp)

add_library(cli11-yaml::cli11-yaml ALIAS cli11-yaml)

target_include_directories(cli11-yaml
    PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/${PROJECT_NAME}>
)

target_link_libraries(cli11-yaml PUBLIC CLI11::CLI11 yaml-cpp)



add_subdirectory(tests)
