#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H

#include <netinet/in.h>

class ServerSocket {
private:
    int serverSocket;
    int clienteSocket;
    sockaddr_in serverAddr;
    sockaddr_in clienteAddr;
    socklen_t clienteLen;

public:
    ServerSocket();
    ~ServerSocket();

    bool crear();
    bool configurar(const char* ip, int puerto);
    bool bindear();
    bool escuchar(int espera = 5);
    bool aceptar();
    void recibir();
    void enviarRespuesta();
    void cerrar();

};

#endif // SERVERSOCKET_H
