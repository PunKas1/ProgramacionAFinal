cmake_minimum_required(VERSION 3.15)
project(ProyectoFinal)

set(CMAKE_CXX_STANDARD 17)

# 1. Buscar SFML
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# 2. Archivos comunes (Lógica de red y ChatManager)
set(COMMON_SOURCES src/socket.cpp src/chat.cpp)

# 3. Compilar Servidor
add_executable(servidor src/main_server.cpp ${COMMON_SOURCES})
target_include_directories(servidor PUBLIC include)
target_link_libraries(servidor sfml-graphics sfml-window sfml-system pthread)

# 4. Compilar Cliente
add_executable(cliente src/main_cliente.cpp src/clienteSocket.cpp ${COMMON_SOURCES})
target_include_directories(cliente PUBLIC include)
target_link_libraries(cliente sfml-graphics sfml-window sfml-system pthread)