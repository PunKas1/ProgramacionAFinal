#ifndef CHAT_H
#define CHAT_H

#include <vector>
#include <string>
#include <mutex>

struct Mensaje{
    
    std::string emisor;
    std::string texto;
    bool esMio;

};

class Chat{

    private:
        std::vector<Mensaje> historial;
        std::mutex mtx;

    public:
        Chat(){};

        void agregarMensaje(std::string emisor,std::string texto,bool esMio);
        std::vector<Mensaje> obtenerHistorial();
};


#endif 
