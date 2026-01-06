/**
 * @file Chat.cpp
 * @author Valencia Cedeño Marcos Gael
 * @author Peralta Ordóñez Jesús
 * @author Esteves Flores Andrés
 * @brief Implementación de la lógica de almacenamiento de mensajes.
 * @version 1.0
 * @date 06/01/2026
 * * Este archivo contiene el código ejecutable de la clase Chat.
 * Aquí es donde se aplica la exclusión mutua (Mutex) para proteger la memoria.
 */

#include "../include/chat.h"

/**
 * @brief Agrega un mensaje al vector de forma segura (Thread-Safe).
 * * Utiliza el patrón RAII para el bloqueo: El mutex se bloquea al crear 'lock'
 * y se desbloquea AUTOMÁTICAMENTE cuando la función termina (se cierra la llave).
 * * @param emisor Quién envía el mensaje.
 * @param texto Contenido del mensaje.
 * @param esMio Booleano para determinar el color de la burbuja en la UI.
 */
void Chat::agregarMensaje(std::string emisor, std::string texto, bool esMio){
    // CRÍTICO: Bloqueamos el acceso antes de tocar el vector.
    // Si otro hilo intenta entrar aquí, se quedará congelado esperando hasta que terminemos.
    std::lock_guard<std::mutex> lock(mtx);
    
    // Operación de escritura en memoria compartida
    historial.push_back({emisor, texto, esMio});
    
    // Al llegar a esta llave '}', el lock_guard se destruye y libera el mutex.
}

/**
 * @brief Devuelve una copia instantánea del chat actual.
 * * Es necesario bloquear también para LEER, porque si el vector se redimensiona
 * (realloc) mientras lo leemos, el programa crashearía.
 * * @return std::vector<Mensaje> Copia completa del historial.
 */
std::vector<Mensaje> Chat::obtenerHistorial(){
    std::lock_guard<std::mutex> lock(mtx);
    return historial; // Se devuelve una copia, no el original.
}

/**
 * @brief Vacia el vector de mensajes.
 * * Se utiliza al cambiar de cliente para limpiar la pantalla.
 */
void Chat::limpiarHistorial() {
    std::lock_guard<std::mutex> lock(mtx); 
    historial.clear(); 
}