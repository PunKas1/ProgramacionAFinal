cmake_minimum_required(VERSION 3.24)
project(ProyectoFinal LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# --- BLOQUE PARA SFML 3 ---
include(FetchContent)
FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG      3.0.0
)
FetchContent_MakeAvailable(SFML)

# --- SERVIDOR ---
# Agregamos src/socket.cpp porque ahí es donde suele estar la lógica del servidor
add_executable(servidor 
    src/main_server.cpp 
    src/chat.cpp
    src/socket.cpp
)
target_include_directories(servidor PUBLIC include)
target_link_libraries(servidor PRIVATE sfml-graphics sfml-window sfml-system sfml-network)

# --- CLIENTE ---
# Agregamos src/clienteSocket.cpp porque ahí es donde definiste ClienteSocket
add_executable(cliente 
    src/main_cliente.cpp 
    src/chat.cpp
    src/clienteSocket.cpp
)
target_include_directories(cliente PUBLIC include)
target_link_libraries(cliente PRIVATE sfml-graphics sfml-window sfml-system sfml-network)

# Copiar fuente
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/arial.ttf"
     DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
