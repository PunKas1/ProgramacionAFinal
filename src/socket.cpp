#include "../include/socket.h"
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <string>

using namespace std;

ServerSocket::ServerSocket()
    : serverSocket(-1), clienteSocket(-1) {
    clienteLen = sizeof(clienteAddr);
}

ServerSocket::~ServerSocket() {
    cerrar();
}

// 1 - Crear socket
bool ServerSocket::crear() {
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        cerr << "Error al crear el socket" << endl;
        return false;
    }
    return true;
}

// 2 - Configurar direccion
bool ServerSocket::configurar(const char* ip, int puerto) {
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(puerto);

    if (inet_pton(AF_INET, ip, &serverAddr.sin_addr) <= 0) {
        cerr << "IP invalida" << endl;
        return false;
    }
    return true;
}

// 3 - Bind
bool ServerSocket::bindear() {
    if (::bind(serverSocket,
        (struct sockaddr*)&serverAddr,
        sizeof(serverAddr)) < 0) {

        cerr << "Error en el bind" << endl;
        return false;
    }
    return true;
}

// 4 - Listen
bool ServerSocket::escuchar(int espera) {
    if (listen(serverSocket, espera) < 0) {
        cerr << "Error en el listen" << endl;
        return false;
    }
    return true;
}

// 5 - Accept
bool ServerSocket::aceptar() {
    clienteSocket = accept(serverSocket,
        (struct sockaddr*)&clienteAddr,
        &clienteLen);

    if (clienteSocket < 0) {
        cerr << "Error al aceptar la conexion" << endl;
        return false;
    }

    cout << "Cliente conectado!!!" << endl;
    return true;
}

// 6 - Recibir
std::string ServerSocket::recibir() {
    char buffer[1024] = {0};
    int bytesRead = recv(clienteSocket, buffer, sizeof(buffer), 0);

    if (bytesRead > 0) {
        return std::string(buffer);
    }
    return "";
}

// 7 - Enviar
void ServerSocket::enviarRespuesta(std::string mensaje) {
    send(clienteSocket, mensaje.c_str(), mensaje.length(), 0);
}

// 8 - Cerrar
void ServerSocket::cerrar() {
    if (clienteSocket != -1) {
        close(clienteSocket);
        clienteSocket = -1;
    }
    if (serverSocket != -1) {
        close(serverSocket);
        serverSocket = -1;
    }
}
