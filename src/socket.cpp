/**
 * @file socket.cpp
 * @author Valencia Cedeño Marcos Gael
 * @author Peralta Ordóñez Jesús
 * @author Esteves Flores Andrés
 * @brief Implementación de la lógica del servidor concurrente.
 * @version 1.0
 * @date 06/01/2026
 * * Este archivo contiene la implementación de las syscalls de red
 * y la lógica crítica de sincronización de hilos para la cola de clientes.
 */

#include "../include/socket.h"
#include <iostream>
#include <unistd.h>      // close()
#include <arpa/inet.h>   // inet_pton, htons
#include <cstring>       // memset

using namespace std;

/**
 * @brief Constructor. Inicializa los descriptores en -1 (estado inválido).
 */
ServerSocket::ServerSocket(){
    serverSocket = -1;
    clienteActual = -1;
    contadorID = 1;
}

/**
 * @brief Destructor. Llama a cerrarServidor() para asegurar limpieza de recursos.
 */
ServerSocket::~ServerSocket()
{
    cerrarServidor();
}

// 1 - Crear socket
/**
 * @brief Solicita un socket maestro al Kernel.
 * * AF_INET: IPv4.
 * * SOCK_STREAM: TCP (Orientado a conexión).
 */
bool ServerSocket::crear()
{
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    return serverSocket != -1;
}

// 2 - Configurar
/**
 * @brief Prepara la estructura de dirección.
 * * Utiliza htons y inet_pton para asegurar que los datos sean legibles
 * por la red (Big Endian).
 */
bool ServerSocket::configurar(const char *ip, int puerto)
{
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(puerto);
    // Convierte IP texto ("127.0.0.1") a binario de red.
    return inet_pton(AF_INET, ip, &serverAddr.sin_addr) > 0;
}

// 3 - Bind
/**
 * @brief "Amarra" el socket a un puerto específico de la máquina.
 * * Nota técnica: Se usa '::bind' (con dos puntos) para decirle al compilador
 * que use la función bind() original de C (global) y no std::bind de C++.
 */
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
/**
 * @brief Pone el socket en modo pasivo (escucha).
 * @param espera Longitud de la cola de conexiones pendientes del Kernel (Backlog).
 */
bool ServerSocket::escuchar(int espera)
{
    return listen(serverSocket, espera) >= 0;
}

// 5 - Aceptar clientes (HILO ACEPTADOR)
/**
 * @brief Bucle infinito que corre en un hilo secundario (Background).
 * * Su única tarea es esperar a que alguien se conecte y meterlo a la cola.
 * * CRÍTICO: Usa Mutex para proteger el acceso a 'colaClientes' y 'listaClientes'.
 */
void ServerSocket::aceptarClientes() {
    while (true) {
        sockaddr_in clienteAddr;
        socklen_t len = sizeof(clienteAddr);

        // accept(): SE BLOQUEA aquí hasta que alguien intente entrar.
        int nuevoSocket = accept(serverSocket, (sockaddr*)&clienteAddr, &len);

        if (nuevoSocket >= 0) {
            // BLOQUEO DE SEGURIDAD (Mutex)
            std::lock_guard<std::mutex> lock(mtxCola);
            
            // 1. Crear ficha técnica del cliente
            InfoCliente info;
            info.socket = nuevoSocket;
            info.id = contadorID++;
            info.nombre = "Cliente " + std::to_string(info.id);

            // 2. Guardar en registro histórico
            listaClientes.push_back(info);

            // 3. Meter a la cola de espera
            colaClientes.push(nuevoSocket);

            std::cout << "Nuevo: " << info.nombre << "\n";

            // 4. PROTOCOLO: Enviamos comando /WAIT para que el cliente se ponga en pantalla de espera
            std::string msg = "/WAIT";
            send(nuevoSocket, msg.c_str(), msg.size() + 1, 0); 
        }
    }
}

// 6 - Tomar siguiente cliente (HILO ATENCION)
/**
 * @brief Saca al siguiente cliente de la cola y empieza la sesión.
 * * Esta función es llamada por el 'Main Loop'.
 * * CRÍTICO: Usa Mutex porque modifica la misma cola que usa aceptarClientes().
 */
bool ServerSocket::tomarSiguienteCliente() {
    std::lock_guard<std::mutex> lock(mtxCola);

    if (colaClientes.empty()) return false;

    // Extraemos el primero de la fila (FIFO)
    clienteActual = colaClientes.front();
    colaClientes.pop();

    std::string nombre = obtenerNombrePorSocket(clienteActual);
    std::cout << "Atendiendo a: " << nombre << "\n";

    // PROTOCOLO: Enviamos comando /START para desbloquear la UI del cliente
    std::string msg = "/START";
    send(clienteActual, msg.c_str(), msg.size() + 1, 0);

    return true;
}

// 7 - Recibir
/**
 * @brief Lee datos del cliente actual.
 * * Usa memset para limpiar el buffer antes de leer, evitando basura de memoria.
 */
string ServerSocket::recibir()
{
    char buffer[1024];
    // memset llena de ceros todo el arreglo. Es vital para strings de C.
    memset(buffer, 0, sizeof(buffer));

    int bytes = recv(clienteActual, buffer, sizeof(buffer), 0);
    if (bytes <= 0)
        return ""; // Desconexión o error

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

/**
 * @brief Búsqueda lineal en el vector de clientes.
 * * Itera sobre la listaClientes para encontrar el nombre asociado a un socket.
 */
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
    std::lock_guard<std::mutex> lock(mtxCola); // Protección de lectura
    return !colaClientes.empty();
}

bool ServerSocket::estoyAtendiendo() {
    return clienteActual != -1;
}

void ServerSocket::liberarClienteActual() {
    clienteActual = -1; 
}