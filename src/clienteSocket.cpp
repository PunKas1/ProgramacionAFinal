#include "../include/clienteSocket.h"
#include<iostream>
#include<unistd.h>
#include<arpa/inet.h>
#include<cstring>

using namespace std;

ClienteSocket::ClienteSocket() : clienteSocket(-1){}

ClienteSocket::~ClienteSocket(){
    cerrar();
}

bool ClienteSocket::crear(){

    clienteSocket = socket(AF_INET,SOCK_STREAM,0);
    if(clienteSocket < 0){
        cerr <<"Error al crear el socket del cleinte"<<endl;
        return false;
    }
    return true;
}

bool ClienteSocket::conectar(const char* ip, int puerto){
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(puerto);

    if (inet_pton(AF_INET,ip,&serverAddr.sin_addr)<= 0){
        cerr << "IP Invalida del servidor" << endl;
        return false;
    }
    
    if(connect(clienteSocket,(struct sockaddr*)&serverAddr,sizeof(serverAddr))< 0){
        cerr << "Error: no se pudo estabablecer conexion con el server"<<endl;
        return false;
    }
    cout<<"Conectado con el servidor exitosamente"<<endl;
    return true;
}

void ClienteSocket::enviar(const char* mensaje){
    send(clienteSocket,mensaje,strlen(mensaje),0);
}

void ClienteSocket::recibir(){
    char buffer[1024] = {0};
    int bytesLeidos = recv(clienteSocket,buffer,sizeof(buffer),0);
    if(bytesLeidos > 0){
        cout <<"SERVIDOR DICE: "<<buffer<<endl;
    }
}

void ClienteSocket::cerrar(){
    if(clienteSocket != -1){
        close(clienteSocket);
        clienteSocket = -1;
    }
}