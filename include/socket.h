/**
 * @file ServerSocket.h
 * @author Valencia Cedeño Marcos Gael
 * @author Peralta Ordóñez Jesús
 * @author Esteves Flores Andrés
 * @brief Gestor central del servidor, conexiones TCP y lógica de colas.
 * @version 1.0
 * @date 06/01/2026
 * * Este archivo define la clase principal que actúa como el "cerebro" de la red.
 * Combina la programación de sockets con estructuras de datos
 * de alto nivel (colas, vectores) para gestionar múltiples clientes simultáneos.
 */

#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H

#include <netinet/in.h>
#include <string>
#include <queue>
#include <vector> // Necesario para std::vector
#include <mutex>

/**
 * @struct InfoCliente
 * @brief Datos asociados a un cliente conectado.
 * * Permite guardar información relevante del cliente para su administración.
 */
struct InfoCliente {
    int socket;         ///< El ID numérico del socket (File Descriptor).
    int id;             ///< ID único autoincremental asignado por nuestro sistema.
    std::string nombre; ///< Nombre para mostrar en la interfaz (ej. "Cliente 5").
};

/**
 * @class ServerSocket
 * @brief Clase administradora del servidor concurrente.
 * * Responsabilidades:
 * 1. Escuchar conexiones entrantes en un puerto específico.
 * 2. Gestionar la concurrencia mediante hilos (Acceptor Thread vs Main Thread).
 * 3. Administrar la cola de espera (FIFO) para atención al cliente.
 * 4. Mantener el registro de todos los clientes conectados.
 */
class ServerSocket {
private:
    int serverSocket;       ///< Descriptor del socket principal que escucha ("El Oído").
    int clienteActual;      ///< Socket del cliente que está siendo atendido actualmente (-1 si libre).
    sockaddr_in serverAddr; ///< Configuración de red (IP/Puerto).
    int contadorID;         ///< Contador para generar IDs únicos (1, 2, 3...).

public:
    /**
     * @brief Cola de espera "First-In, First-Out" (FIFO).
     * * Almacena los sockets de los clientes que están esperando turno.
     * Es pública para depuración, pero debería accederse con cuidado.
     */
    std::queue<int> colaClientes;

    /**
     * @brief Base de datos en memoria de todos los conectados.
     * * Se usa para buscar el nombre de un cliente a partir de su socket.
     */
    std::vector<InfoCliente> listaClientes;

    /**
     * @brief Mutex para proteger la cola de condiciones de carrera.
     * * Vital porque el Hilo Aceptador (background) mete gente a la cola
     * y el Hilo Principal (Main) saca gente de la cola. Sin esto, el programa crashea.
     */
    std::mutex mtxCola;

    /**
     * @brief Constructor. Inicializa el servidor en estado "apagado" y contadores en 0.
     */
    ServerSocket();

    /**
     * @brief Destructor. Cierra el socket principal para liberar el puerto del SO.
     */
    ~ServerSocket();

    /**
     * @brief Crea el socket del servidor usando la syscall socket().
     * @return true si se creó el descriptor correctamente.
     */
    bool crear();

    /**
     * @brief Configura la estructura sockaddr_in con la IP y Puerto deseados.
     */
    bool configurar(const char* ip, int puerto);

    /**
     * @brief Vincula el socket a la dirección IP/Puerto en el Sistema Operativo.
     * * Utiliza la syscall bind(). Si falla, usualmente es porque el puerto está ocupado.
     */
    bool bindear();

    /**
     * @brief Pone al socket en modo pasivo para escuchar conexiones.
     * @param espera Tamaño del backlog (cuántas conexiones pendientes admite el kernel).
     */
    bool escuchar(int espera = 5);

    /**
     * @brief Bucle infinito (para correr en un hilo aparte) que acepta conexiones.
     * * Cuando llega alguien, lo registra, lo mete a la cola y le envía señal de espera.
     * * Thread-Safe: Usa mtxCola al modificar la cola.
     */
    void aceptarClientes();      

    /**
     * @brief Extrae al siguiente cliente de la cola y lo marca como activo.
     * * Thread-Safe: Usa mtxCola.
     * @return true si había alguien en la cola y se pudo tomar, false si estaba vacía.
     */
    bool tomarSiguienteCliente(); 

    /**
     * @brief Busca en el vector listaClientes el nombre asociado a un socket.
     * @param socket El ID del socket a buscar.
     * @return El nombre del cliente o "Desconocido".
     */
    std::string obtenerNombrePorSocket(int socket);

    /**
     * @brief Recibe datos del cliente que está siendo atendido ACTUALMENTE.
     */
    std::string recibir();

    /**
     * @brief Envía datos al cliente que está siendo atendido ACTUALMENTE.
     */
    void enviar(const std::string& msg);

    /**
     * @brief Cierra la conexión con un cliente específico.
     */
    void cerrarCliente();

    /**
     * @brief Apaga todo el servidor.
     */
    void cerrarServidor();

    /**
     * @brief Getter para obtener el ID del socket activo.
     */
    int getClienteActual();

    /**
     * @brief Verifica si hay personas esperando en la fila.
     * @return true si la cola no está vacía. Thread-Safe.
     */
    bool hayClientesEnCola();

    /**
     * @brief Verifica si el agente está ocupado.
     * @return true si clienteActual != -1.
     */
    bool estoyAtendiendo();

    /**
     * @brief Resetea el estado del agente a "Libre".
     * * Se llama cuando el cliente actual se desconecta o termina la sesión.
     */
    void liberarClienteActual();
};

#endif