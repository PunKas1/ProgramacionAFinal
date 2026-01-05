#pragma once
#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H

#include <netinet/in.h>
#include <string>
#include <queue>
#include <mutex>

class ServerSocket {
private:
    int serverSocket;
    int clienteActual;
    sockaddr_in serverAddr;

public:
    std::queue<int> colaClientes;
    std::mutex mtxCola;

    ServerSocket();
    ~ServerSocket();

    bool crear();
    bool configurar(const char* ip, int puerto);
    bool bindear();
    bool escuchar(int espera = 5);

    void aceptarClientes();      
    bool tomarSiguienteCliente(); 

    std::string recibir();
    void enviar(const std::string& msg);

    void cerrarCliente();
    void cerrarServidor();
};

#endif
