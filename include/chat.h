/**
 * @file Chat.h
 * @author Valencia Cedeño Marcos Gael
 * @author Peralta Ordóñez Jesús
 * @author Esteves Flores Andrés
 * @brief Definición de la estructura de datos para el historial de mensajes.
 * @version 1.0
 * @date 06/01/2026
 * * Este archivo contiene la lógica para almacenar y gestionar el flujo de conversación
 * de manera segura entre hilos 
 */

#ifndef CHAT_H
#define CHAT_H

#include <vector>
#include <string>
#include <mutex>

/**
 * @struct Mensaje
 * @brief Estructura de datos simple que representa un único mensaje de texto.
 * * Se utiliza struct en lugar de class porque solo necesitamos un contenedor de datos
 * público sin lógica compleja interna.
 */
struct Mensaje{
    
    std::string emisor; ///< Nombre o ID de quien envió el mensaje (ej. "Cliente 1", "Soporte").
    std::string texto;  ///< El contenido del mensaje.
    bool esMio;         ///< Flag booleana para la UI: true=Derecha (Verde), false=Izquierda (Blanco).

};

/**
 * @class Chat
 * @brief Clase gestora del historial de la conversación.
 * * Esta clase actúa como un recurso compartido entre:
 * 1. El Hilo de Red (que escribe mensajes recibidos).
 * 2. El Hilo Gráfico/Main (que lee mensajes para dibujarlos).
 * * Utiliza un Mutex para evitar Condiciones de Carrera (Race Conditions).
 */
class Chat{

    private:
        /**
         * @brief Contenedor dinámico de mensajes.
         * * Se eligió std::vector por sobre std::list o arreglos fijos porque:
         * 1. Permite acceso rápido e iteración secuencial (ideal para dibujar 60 FPS).
         * 2. Crece dinámicamente según sea necesario.
         */
        std::vector<Mensaje> historial;

        /**
         * @brief Semáforo de exclusión mutua.
         * * Este objeto es CRITICO. Bloquea el acceso al vector mientras un hilo
         * está escribiendo, obligando al otro a esperar. Sin esto, el programa
         * crashearía al intentar leer y escribir memoria simultáneamente.
         */
        std::mutex mtx;

    public:
        /**
         * @brief Constructor por defecto. Inicializa un historial vacío.
         */
        Chat(){};

        /**
         * @brief Inserta un nuevo mensaje en el historial de forma segura.
         * @param emisor El nombre de quien envía.
         * @param texto El contenido del mensaje.
         * @param esMio Define si el mensaje lo escribí yo (true) o me llegó (false).
         */
        void agregarMensaje(std::string emisor, std::string texto, bool esMio);

        /**
         * @brief Obtiene una COPIA del historial actual para ser dibujada.
         * @return std::vector<Mensaje> Una copia segura de los mensajes.
         * * Se devuelve por valor (copia) y no por referencia para que el hilo gráfico
         * pueda dibujar tranquilo sin bloquear al hilo de red por mucho tiempo.
         */
        std::vector<Mensaje> obtenerHistorial();

        /**
         * @brief Borra todos los mensajes almacenados.
         * * Utilizado cuando se cambia de cliente o se cierra una sesión,
         * para asegurar que el siguiente usuario empiece con la pantalla limpia.
         */
        void limpiarHistorial();

};

#endif