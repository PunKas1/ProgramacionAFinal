#include "../include/socket.h"
#include "../include/chat.h"

#include <SFML/Graphics.hpp>
#include <thread>
#include <iostream>
#include <optional>
#include <cstdint>

// ================= HILO DE RED =================
void hiloRedServidor(ServerSocket* servidor, Chat* manager)
{
    while (true) {
        std::string mensaje = servidor->recibir();
        if (!mensaje.empty()) {
            manager->agregarMensaje("Cliente", mensaje, false);
        }
    }
}

// ================= MAIN =================
int main()
{
    ServerSocket servidor;
    Chat miChat;

    if (!servidor.crear() ||
        !servidor.configurar("127.0.0.1", 8080) ||
        !servidor.bindear() ||
        !servidor.escuchar())
    {
        std::cerr << "Error al iniciar servidor\n";
        return -1;
    }

    std::cout << "Servidor listo. Esperando cliente...\n";

    if (!servidor.aceptar()) {
        std::cerr << "Error al aceptar cliente\n";
        return -1;
    }

    std::thread red(hiloRedServidor, &servidor, &miChat);
    red.detach();

    // ================= VENTANA =================
    sf::RenderWindow window(
        sf::VideoMode({400, 600}),
        "Panel de Soporte Tecnico"
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

            // Entrada de texto
            if (const auto* texto =
                event->getIf<sf::Event::TextEntered>())
            {
                std::uint32_t unicode = texto->unicode;

                if (unicode == '\n' || unicode == '\r') {
                    if (!inputTexto.empty()) {
                        servidor.enviarRespuesta(inputTexto);
                        miChat.agregarMensaje("Soporte", inputTexto, true);
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
        window.clear(sf::Color(25, 25, 25));

        float y = 10.f;
        for (const auto& m : miChat.obtenerHistorial()) {
            sf::Text linea(font, m.emisor + ": " + m.texto, 16);
            linea.setPosition({10.f, y});
            linea.setFillColor(
                m.esMio ? sf::Color::Cyan : sf::Color::White
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

