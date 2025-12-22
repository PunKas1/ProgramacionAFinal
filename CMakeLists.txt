
CMAKE_MINIMUM_REQUIRED(VERSION 3.15)
project(ProyectoFinal)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX STANDARD_REQUIRED ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# incluir headers

include_directories(include)

file(GLOB SOURCES "src/*.cpp")

# crear ejecutable
add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC inlude)

