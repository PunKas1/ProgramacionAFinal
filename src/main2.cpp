#include "../include/clienteSocket.h"// La clase que creamos arriba
#include <iostream>

int main() {
    ClienteSocket cliente;
    if (!cliente.crear()) return 1;
    
    std::cout << "Intentando conectar..." << std::endl;
    if (cliente.conectar("127.0.0.1", 8080)) {
        cliente.enviar("Hola servidor, soy el cliente!");
        cliente.recibir(); // Espera la respuesta del servidor
    }
    return 0;
}