#include "../include/chat.h"

void Chat::agregarMensaje(std::string emisor,std::string texto,bool esMio){
    std::lock_guard<std::mutex> lock(mtx);
    historial.push_back({emisor,texto,esMio});
}

std::vector<Mensaje> Chat::obtenerHistorial(){
    std::lock_guard<std::mutex> lock(mtx);
    return historial;
}