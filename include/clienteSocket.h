#ifndef CLIENTESOCKET_H
#define CLIENTESOCKET_H

#include<netinet/in.h>
#include<string>

class ClienteSocket{
    private:

        int clienteSocket;
        sockaddr_in serverAddr;

    public:

        ClienteSocket();
        ~ClienteSocket();

        bool crear();
        bool conectar(const char* ip, int puerto);
        void enviar(const char* mensaje);
        std::string recibir();
        void cerrar();

};

#endif
