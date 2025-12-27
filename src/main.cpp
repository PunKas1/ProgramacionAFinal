#include "../include/socket.h"
#include <iostream>

int main() {
    ServerSocket servidor;
    if (!servidor.crear()) return 1;
    if (!servidor.configurar("127.0.0.1", 8080)) return 1;
    if (!servidor.bindear()) return 1;
    if (!servidor.escuchar()) return 1;

    std::cout << "Esperando al cliente..." << std::endl;
    if (servidor.aceptar()) {
        servidor.recibir();          // Lee lo que envíe el cliente
        servidor.enviarRespuesta();  // Responde "Hola desde el servidor"
    }
    return 0;
}