cmake_minimum_required (VERSION 3.1)
project("dariadb-example")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(ENABLE_TESTS OFF)
set(ENABLE_INTEGRATION_TESTS OFF)
set(ENABLE_SERVER OFF)
set(ENABLE_BENCHMARKS OFF)
set(ENABLE_SAMPLES OFF)


add_subdirectory(dariadb)
include_directories(dariadb/)
MESSAGE(STATUS ${PROJECT_BINARY_DIR})

add_executable(example main.cpp)
        
target_link_libraries(example ${DARIADB_STORAGE_LIBS})

