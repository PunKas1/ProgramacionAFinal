cmake_minimum_required(VERSION 3.15)
project(ProyectoFinal LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


set(SFML_DIR "/opt/homebrew/lib/cmake/SFML")

find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System)

set(COMMON_SOURCES
    src/socket.cpp
    src/chat.cpp
)

# Servidor
add_executable(servidor
    src/main_server.cpp
    ${COMMON_SOURCES}
)

target_include_directories(servidor PUBLIC include)
target_link_libraries(servidor
    sfml-graphics
    sfml-window
    sfml-system
)

target_compile_definitions(servidor PRIVATE BUILD_SERVER)

# Cliente
add_executable(cliente
    src/main_cliente.cpp
    src/clienteSocket.cpp
    ${COMMON_SOURCES}
)

target_include_directories(cliente PUBLIC include)
target_link_libraries(cliente
    sfml-graphics
    sfml-window
    sfml-system
)

target_compile_definitions(cliente PRIVATE BUILD_CLIENT)


