/**
 * @file clienteSocket.cpp
 * @author Valencia Cedeño Marcos Gael
 * @author Peralta Ordóñez Jesús
 * @author Esteves Flores Andrés
 * @brief Implementación de la comunicación de red lado Cliente.
 * @version 1.0
 * @date 06/01/2026
 * * Este archivo traduce las solicitudes de alto nivel de nuestra aplicación
 * a llamadas de bajo nivel (syscalls) del sistema operativo Linux/Unix.
 */

#include "../include/clienteSocket.h"
#include <iostream>
#include <unistd.h>      // Para close()
#include <arpa/inet.h>   // Para inet_pton, htons
#include <cstring>       // Para strlen

using namespace std;

/**
 * @brief Constructor.
 * * Inicializa el descriptor del socket en -1 para indicar que está "vacío" o "no asignado".
 * Esto evita que intentemos cerrar o usar un socket basura por accidente.
 */
ClienteSocket::ClienteSocket() : clienteSocket(-1){}

/**
 * @brief Destructor.
 * * Garantiza que, si el objeto ClienteSocket se destruye (ej. al cerrar la ventana),
 * la conexión se cierre correctamente liberando recursos del sistema.
 */
ClienteSocket::~ClienteSocket(){
    cerrar();
}

/**
 * @brief Solicita un socket al Kernel.
 * * AF_INET: Indica que usaremos IPv4.
 * * SOCK_STREAM: Indica que usaremos TCP (protocolo fiable, ordenado, con conexión).
 * * 0: Protocolo IP automático.
 */
bool ClienteSocket::crear(){
    clienteSocket = socket(AF_INET, SOCK_STREAM, 0);
    
    if(clienteSocket < 0){
        cerr << "Error al crear el socket del cliente" << endl;
        return false;
    }
    return true;
}

/**
 * @brief Establece la conexión física/lógica con el servidor.
 * * Convierte la IP de texto a binario.
 */
bool ClienteSocket::conectar(const char* ip, int puerto){
    serverAddr.sin_family = AF_INET;
    
    // htons (Host TO Network Short): Convierte el número de puerto al orden de bytes
    // de la red (Big Endian) para que el router lo entienda.
    serverAddr.sin_port = htons(puerto);

    // inet_pton (Presentation TO Network): Convierte "127.0.0.1" a formato binario seguro.
    // Es más moderno y seguro que el antiguo inet_addr.
    if (inet_pton(AF_INET, ip, &serverAddr.sin_addr) <= 0){
        cerr << "IP Invalida del servidor" << endl;
        return false;
    }
    
    // connect(): Inicia el saludo TCP.
    // Si esta función retorna 0, significa que el cable está conectado virtualmente.
    if(connect(clienteSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0){
        cerr << "Error: no se pudo establecer conexion con el server" << endl;
        return false;
    }
    
    cout << "Conectado con el servidor exitosamente" << endl;
    return true;
}

/**
 * @brief Envía bytes crudos al servidor.
 * * Utiliza 'send' en lugar de 'write' porque es específico para sockets.
 * @param mensaje Cadena de caracteres estilo C.
 */
void ClienteSocket::enviar(const char* mensaje){
    // strlen calcula la longitud exacta del texto(sin basura extra).
    send(clienteSocket, mensaje, strlen(mensaje), 0);
}

/**
 * @brief Escucha y captura la respuesta del servidor.
 * * Esta función es BLOQUEANTE por defecto (espera hasta que llegue algo).
 */
std::string ClienteSocket::recibir(){
    // Buffer de 1KB en el Stack (memoria rápida).
    // {0} inicializa todo en nulos para evitar basura de memoria.
    char buffer[1024] = {0};
    
    // recv(): Lee del buffer de entrada de la tarjeta de red.
    // Retorna la cantidad de bytes que realmente llegaron.
    int bytesLeidos = recv(clienteSocket, buffer, sizeof(buffer), 0);
    
    if(bytesLeidos > 0){
        // Convertimos el array de char a string de C++ automáticamente.
        return std::string(buffer);
    }
    
    // Si bytesLeidos es 0, significa que el servidor cerró la conexión.
    return "";
}

/**
 * @brief Cierra el descriptor de archivo.
 * * Libera el puerto y la memoria en el Kernel.
 */
void ClienteSocket::cerrar(){
    if(clienteSocket != -1){
        close(clienteSocket);
        clienteSocket = -1; // Marcamos como cerrado para no cerrarlo dos veces.
    }
}