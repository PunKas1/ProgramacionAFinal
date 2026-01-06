#include "../include/socket.h"
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>

using namespace std;

ServerSocket::ServerSocket(){
    serverSocket = -1;
    clienteActual = -1;
    contadorID = 1;
}

ServerSocket::~ServerSocket()
{
    cerrarServidor();
}

// 1 - Crear socket
bool ServerSocket::crear()
{
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    return serverSocket != -1;
}

// 2 - Configurar
bool ServerSocket::configurar(const char *ip, int puerto)
{
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(puerto);
    return inet_pton(AF_INET, ip, &serverAddr.sin_addr) > 0;
}

// 3 - Bind
bool ServerSocket::bindear()
{
    if (::bind(serverSocket,
               (struct sockaddr *)&serverAddr,
               sizeof(serverAddr)) < 0)
    {
        cerr << "Error en el bind" << endl;
        return false;
    }
    return true;
}

// 4 - Listen
bool ServerSocket::escuchar(int espera)
{
    return listen(serverSocket, espera) >= 0;
}

// 5 - Aceptar clientes (HILO ACEPTADOR)
void ServerSocket::aceptarClientes() {
    while (true) {
        sockaddr_in clienteAddr;
        socklen_t len = sizeof(clienteAddr);

        int nuevoSocket = accept(serverSocket, (sockaddr*)&clienteAddr, &len);

        if (nuevoSocket >= 0) {
            std::lock_guard<std::mutex> lock(mtxCola);
            
            // 1. Crear datos
            InfoCliente info;
            info.socket = nuevoSocket;
            info.id = contadorID++;
            info.nombre = "Cliente " + std::to_string(info.id);

            // 2. Guardar en el VECTOR (al final de la lista)
            listaClientes.push_back(info);

            // 3. Meter a la cola de espera
            colaClientes.push(nuevoSocket);

            std::cout << "Nuevo: " << info.nombre << "\n";

            std::string msg = "/WAIT";
            send(nuevoSocket, msg.c_str(), msg.size() + 1, 0); 
        }
    }
}

// 6 - Tomar siguiente cliente (HILO ATENCION)
bool ServerSocket::tomarSiguienteCliente() {
    std::lock_guard<std::mutex> lock(mtxCola);

    if (colaClientes.empty()) return false;

    clienteActual = colaClientes.front();
    colaClientes.pop();

    // Usamos nuestra función de búsqueda
    std::string nombre = obtenerNombrePorSocket(clienteActual);
    std::cout << "Atendiendo a: " << nombre << "\n";

    std::string msg = "/START";
    send(clienteActual, msg.c_str(), msg.size() + 1, 0);

    return true;
}

// 7 - Recibir
string ServerSocket::recibir()
{
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    int bytes = recv(clienteActual, buffer, sizeof(buffer), 0);
    if (bytes <= 0)
        return "";

    return string(buffer);
}

// 8 - Enviar
void ServerSocket::enviar(const string &msg)
{
    if (clienteActual != -1)
    {
        send(clienteActual, msg.c_str(), msg.size(), 0);
    }
}

// 9 - Cerrar cliente
void ServerSocket::cerrarCliente()
{
    if (clienteActual != -1)
    {
        close(clienteActual);
        clienteActual = -1;
    }
}

// 10 - Cerrar servidor
void ServerSocket::cerrarServidor()
{
    cerrarCliente();
    if (serverSocket != -1)
    {
        close(serverSocket);
        serverSocket = -1;
    }
}

std::string ServerSocket::obtenerNombrePorSocket(int socketBuscado) {
    for(size_t i = 0; i < listaClientes.size(); ++i) {
        if (listaClientes[i].socket == socketBuscado) {
            return listaClientes[i].nombre;
        }
    }
    return "Desconocido";
}

int ServerSocket::getClienteActual() {
    return clienteActual;
}

bool ServerSocket::hayClientesEnCola() {
    std::lock_guard<std::mutex> lock(mtxCola);
    return !colaClientes.empty();
}

bool ServerSocket::estoyAtendiendo() {
    // Si es -1, significa que estoy libre
    return clienteActual != -1;
}

void ServerSocket::liberarClienteActual() {
    clienteActual = -1; // Reseteamos para que pueda entrar otro
}