#include "../include/socket.h"
#include "../include/chat.h"
#include <SFML/Graphics.hpp>
#include <thread>
#include <iostream>

// Función para recibir datos en segundo plano
void hiloRedServidor(ServerSocket* servidor, Chat* manager) {
    while (true) {
        std::string mensaje = servidor->recibir(); 
        if (!mensaje.empty()) {
            manager->agregarMensaje("Cliente", mensaje, false);
        }
    }
}

int main() {
    ServerSocket servidor;
    Chat miChat;

    if (!servidor.crear() || !servidor.configurar("127.0.0.1", 8080) || 
        !servidor.bindear() || !servidor.escuchar()) {
        return -1;
    }

    std::cout << "Servidor de Soporte listo. Esperando cliente..." << std::endl;
    
    if (servidor.aceptar()) {
        std::thread t(hiloRedServidor, &servidor, &miChat);
        t.detach();

        sf::RenderWindow window(sf::VideoMode(400, 600), "Panel de Soporte Tecnico");
        sf::Font font;
        font.loadFromFile("arial.ttf");

        sf::Text textoVisual;
        textoVisual.setFont(font);
        textoVisual.setCharacterSize(16);

        std::string inputTexto = "";

        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) window.close();

                if (event.type == sf::Event::TextEntered) {
                    if (event.text.unicode == '\r' || event.text.unicode == '\n') {
                        if (!inputTexto.empty()) {
                            servidor.enviarRespuesta(inputTexto);
                            miChat.agregarMensaje("Soporte", inputTexto, true);
                            inputTexto = "";
                        }
                    } else if (event.text.unicode == 8 && !inputTexto.empty()) {
                        inputTexto.pop_back();
                    } else if (event.text.unicode < 128) {
                        inputTexto += static_cast<char>(event.text.unicode);
                    }
                }
            }

            window.clear(sf::Color(25, 25, 25));
            
            auto historial = miChat.obtenerHistorial();
            float y = 10;
            for (auto& m : historial) {
                textoVisual.setString(m.emisor + ": " + m.texto);
                textoVisual.setPosition(10, y);
                textoVisual.setFillColor(m.esMio ? sf::Color::Cyan : sf::Color::White);
                window.draw(textoVisual);
                y += 25;
            }

            sf::Text actual("> " + inputTexto + "_", font, 18);
            actual.setPosition(10, 560);
            actual.setFillColor(sf::Color::Yellow);
            window.draw(actual);

            window.display();
        }
    }
    return 0;
}