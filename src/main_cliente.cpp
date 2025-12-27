#include "../include/clienteSocket.h"
#include "../include/chat.h"
#include <SFML/Graphics.hpp>
#include <thread>
#include <iostream>

// IMPORTANTE: Definir la función ANTES del main para evitar errores de compilación
void hiloRedCliente(ClienteSocket* cliente, Chat* manager) {
    while (true) {
        std::string mensaje = cliente->recibir();
        if (!mensaje.empty()) {
            manager->agregarMensaje("Soporte", mensaje, false);
        }
    }
}

int main() {
    ClienteSocket cliente;
    Chat miChat;

    // Conexión inicial
    if (!cliente.crear() || !cliente.conectar("127.0.0.1", 8080)) {
        std::cout << "Error: No se pudo conectar al servidor de soporte." << std::endl;
        return -1;
    }

    // Iniciar hilo de red de forma asíncrona
    std::thread t(hiloRedCliente, &cliente, &miChat);
    t.detach();

    // Configuración de Ventana SFML
    sf::RenderWindow window(sf::VideoMode(400, 600), "Chat de Atencion al Cliente");
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) return -1;

    sf::Text textoVisual;
    textoVisual.setFont(font);
    textoVisual.setCharacterSize(16);

    std::string inputTexto = "";

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            // Captura de texto por teclado
            if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode == '\r' || event.text.unicode == '\n') {
                    if (!inputTexto.empty()) {
                        cliente.enviar(inputTexto.c_str());
                        miChat.agregarMensaje("Yo", inputTexto, true);
                        inputTexto = "";
                    }
                } else if (event.text.unicode == 8 && !inputTexto.empty()) { // Backspace
                    inputTexto.pop_back();
                } else if (event.text.unicode < 128) {
                    inputTexto += static_cast<char>(event.text.unicode);
                }
            }
        }

        window.clear(sf::Color(45, 45, 45));

        // Renderizar historial de chat
        auto historial = miChat.obtenerHistorial();
        float y = 10;
        for (auto& m : historial) {
            textoVisual.setString(m.emisor + ": " + m.texto);
            textoVisual.setPosition(10, y);
            textoVisual.setFillColor(m.esMio ? sf::Color::Green : sf::Color::White);
            window.draw(textoVisual);
            y += 25;
        }

        // Mostrar lo que se está escribiendo actualmente
        sf::Text actual("> " + inputTexto + "_", font, 18);
        actual.setPosition(10, 560);
        actual.setFillColor(sf::Color::Yellow);
        window.draw(actual);

        window.display();
    }
    return 0;
}