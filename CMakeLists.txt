cmake_minimum_required(VERSION 3.15)
project(streaming_platform LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Carpeta de cabeceras
include_directories(${CMAKE_SOURCE_DIR}/include)

# Recolectar todos los .cpp de src/
file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")

add_executable(streaming_platform ${SOURCES})

target_include_directories(streaming_platform PRIVATE ${CMAKE_SOURCE_DIR}/include)
