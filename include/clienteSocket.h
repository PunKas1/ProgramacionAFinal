/**
 * @file ClienteSocket.h
 * @author Valencia Cedeño Marcos Gael
 * @author Peralta Ordóñez Jesús
 * @author Esteves Flores Andrés
 * @brief Encapsulamiento de la lógica de sockets cliente para TCP/IP.
 * @version 1.0
 * @date 06/01/2026
 * * Este archivo define la interfaz para conectarse a un servidor remoto,
 * abstraer las llamadas al sistema (syscalls) de Linux y manejar el flujo de datos.
 */

#ifndef CLIENTESOCKET_H
#define CLIENTESOCKET_H

#include <netinet/in.h> // Estructuras necesarias para direcciones de internet (sockaddr_in)
#include <string>       // Para manejar cadenas de texto dinámicas (std::string)

/**
 * @class ClienteSocket
 * @brief Clase que representa al cliente en la arquitectura Cliente-Servidor.
 * * Su función principal es iniciar la comunicación, mantener el
 * descriptor de archivo del socket y convertir los mensajes de texto de C++
 * en paquetes de bytes para la red.
 */
class ClienteSocket{
    private:
        /**
         * @brief Descriptor de archivo del socket.
         * * En Linux, todo es un archivo. El socket se identifica con un simple número entero (int).
         * Si es -1, significa que el socket no es válido o hubo error.
         */
        int clienteSocket;

        /**
         * @brief Estructura de datos para IPv4.
         * * Contiene la Familia (AF_INET), el Puerto (htons) y la Dirección IP (inet_addr)
         * del servidor al que nos queremos conectar.
         */
        sockaddr_in serverAddr;

    public:

        /**
         * @brief Constructor. Inicializa las variables en un estado seguro/nulo.
         */
        ClienteSocket();

        /**
         * @brief Destructor. Se asegura de cerrar la conexión si el objeto se destruye.
         * * Implementa el principio RAII (Resource Acquisition Is Initialization) para evitar
         * fugas de recursos.
         */
        ~ClienteSocket();

        /**
         * @brief Solicita al Sistema Operativo la creación de un endpoint de comunicación.
         * * Utiliza la syscall 'socket()'.
         * @return true si el SO nos asignó un ID de socket válido, false si falló.
         */
        bool crear();

        /**
         * @brief Intenta establecer el "Túnel" (TCP) con el servidor.
         * * Utiliza la syscall 'connect()'. Es una operación bloqueante por defecto.
         * @param ip Dirección IP del servidor (ej. "127.0.0.1").
         * @param puerto Puerto de escucha del servidor (ej. 8080).
         * @return true si el servidor aceptó la conexión.
         */
        bool conectar(const char* ip, int puerto);

        /**
         * @brief Envía datos a través del túnel.
         * * Serializa el texto y lo empuja al buffer de salida de la tarjeta de red.
         * @param mensaje El texto crudo (string) a enviar.
         */
        void enviar(const char* mensaje);

        /**
         * @brief Espera y captura datos provenientes del servidor.
         * * Lee el buffer de entrada de la tarjeta de red.
         * @return std::string El mensaje reconstruido como objeto string de C++.
         */
        std::string recibir();

        /**
         * @brief Cierra ordenadamente la conexión.
         * * Envía un paquete FIN para terminar el handshake TCP y libera el descriptor.
         */
        void cerrar();

};

#endif