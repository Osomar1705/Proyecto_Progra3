cmake_minimum_required(VERSION 3.15)
project(streaming_platform LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build optimizado por defecto (criterio de eficiencia)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
# MSVC uses /O2; GCC/Clang use -O2
if(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "/O2")
else()
    set(CMAKE_CXX_FLAGS_RELEASE "-O2")
endif()

# Carpeta de cabeceras
include_directories(${CMAKE_SOURCE_DIR}/include)

# Recolectar todos los .cpp de src/
file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")

add_executable(streaming_platform ${SOURCES})

target_include_directories(streaming_platform PRIVATE ${CMAKE_SOURCE_DIR}/include)

find_package(Threads REQUIRED)
target_link_libraries(streaming_platform Threads::Threads)
