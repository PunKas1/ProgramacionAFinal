#include "../include/clienteSocket.h"
#include "../include/chat.h"

#include <SFML/Graphics.hpp>
#include <thread>
#include <iostream>
#include <cstdint>

// ================= HILO DE RED =================
void hiloRedCliente(ClienteSocket* cliente, Chat* manager)
{
    while (true) {
        std::string mensaje = cliente->recibir();
        if (!mensaje.empty()) {
            manager->agregarMensaje("Soporte", mensaje, false);
        }
    }
}

// ================= MAIN =================
int main()
{
    ClienteSocket cliente;
    Chat miChat;

    if (!cliente.crear() ||
        !cliente.conectar("127.0.0.1", 8080))
    {
        std::cerr << "No se pudo conectar al servidor\n";
        return -1;
    }

    std::thread red(hiloRedCliente, &cliente, &miChat);
    red.detach();

    // ================= VENTANA =================
    sf::RenderWindow window(
        sf::VideoMode({400, 600}),
        "Chat - Cliente"
    );

    sf::Font font;
    if (!font.openFromFile("arial.ttf")) {
        std::cerr << "No se pudo abrir la fuente\n";
        return -1;
    }

    std::string inputTexto;

    // ================= LOOP =================
    while (window.isOpen()) {

        while (auto event = window.pollEvent()) {

            // Cerrar ventana
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            // Entrada de texto (SFML 3)
            if (const auto* texto =
                event->getIf<sf::Event::TextEntered>())
            {
                std::uint32_t unicode = texto->unicode;

                if (unicode == '\n' || unicode == '\r') {
                    if (!inputTexto.empty()) {
                        cliente.enviar(inputTexto.c_str());
                        miChat.agregarMensaje("Yo", inputTexto, true);
                        inputTexto.clear();
                    }
                }
                else if (unicode == 8 && !inputTexto.empty()) {
                    inputTexto.pop_back();
                }
                else if (unicode < 128) {
                    inputTexto += static_cast<char>(unicode);
                }
            }
        }

        // ================= DIBUJO =================
        window.clear(sf::Color(45, 45, 45));

        float y = 10.f;
        for (const auto& m : miChat.obtenerHistorial()) {
            sf::Text linea(font, m.emisor + ": " + m.texto, 16);
            linea.setPosition({10.f, y});
            linea.setFillColor(
                m.esMio ? sf::Color::Green : sf::Color::White
            );
            window.draw(linea);
            y += 24.f;
        }

        sf::Text actual(font, "> " + inputTexto + "_", 18);
        actual.setPosition({10.f, 560.f});
        actual.setFillColor(sf::Color::Yellow);
        window.draw(actual);

        window.display();
    }

    return 0;
}
