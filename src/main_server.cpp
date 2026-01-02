#include "../include/socket.h"
#include "../include/chat.h"
#include <SFML/Graphics.hpp>
#include <thread>
#include <optional>
#include <cstdint>

// Definición antes del main para evitar errores de ámbito
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
        !servidor.bindear() || !servidor.escuchar()) return -1;

    if (!servidor.aceptar()) return -1;

    std::thread red(hiloRedServidor, &servidor, &miChat);
    red.detach();

    sf::ContextSettings settings;
    settings.antiAliasingLevel = 8;

    // SFML 3.0: Uso de sf::State::Windowed
    sf::RenderWindow window(sf::VideoMode({450, 700}), "Soporte Tecnico - Panel Agente", sf::State::Windowed, settings);
    window.setFramerateLimit(60);

    // Sistema de Scroll
    sf::View viewChat(sf::FloatRect({0, 0}, {450, 700}));
    float currentScrollY = 0;
    float maxScrollY = 0;

    sf::Font font;
    if (!font.openFromFile("arial.ttf")) return -1;
    std::string inputTexto;

    while (window.isOpen()) {
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();

            // Scroll manual con rueda
            if (const auto* mouseWheel = event->getIf<sf::Event::MouseWheelScrolled>()) {
                if (mouseWheel->wheel == sf::Mouse::Wheel::Vertical) {
                    currentScrollY -= mouseWheel->delta * 25.f;
                    if (currentScrollY < 0) currentScrollY = 0;
                }
            }

            // Entrada de texto SFML 3.0
            if (const auto* texto = event->getIf<sf::Event::TextEntered>()) {
                std::uint32_t unicode = texto->unicode;
                if (unicode == '\n' || unicode == '\r') {
                    if (!inputTexto.empty()) {
                        servidor.enviarRespuesta(inputTexto);
                        miChat.agregarMensaje("Yo", inputTexto, true);
                        inputTexto.clear();
                        currentScrollY = maxScrollY; // Bajar al final
                    }
                } else if (unicode == 8 && !inputTexto.empty()) {
                    inputTexto.pop_back();
                } else if (unicode >= 32 && unicode < 128) {
                    inputTexto += static_cast<char>(unicode);
                }
            }
        }

        window.clear(sf::Color(240, 240, 245));

        // --- RENDERIZADO DE MENSAJES (VISTA MÓVIL) ---
        viewChat.setCenter({225, 350 + currentScrollY});
        window.setView(viewChat);

        float y = 90.f;
        for (const auto& m : miChat.obtenerHistorial()) {
            sf::Text msg(font, m.texto, 16);
            msg.setFillColor(m.esMio ? sf::Color::White : sf::Color(30, 30, 30));

            sf::FloatRect bounds = msg.getLocalBounds();
            sf::RectangleShape burbuja;
            float padding = 15.f;
            burbuja.setSize({bounds.size.x + (padding * 2), bounds.size.y + (padding * 2)});
            
            if (m.esMio) { // Soporte - Azul (Derecha)
                float xPos = window.getSize().x - burbuja.getSize().x - 20.f;
                burbuja.setPosition({xPos, y});
                burbuja.setFillColor(sf::Color(0, 102, 255));
                msg.setPosition({xPos + padding, y + padding - 4.f});
            } else { // Cliente - Blanco (Izquierda)
                burbuja.setPosition({20.f, y});
                burbuja.setFillColor(sf::Color::White);
                burbuja.setOutlineThickness(1.f);
                burbuja.setOutlineColor(sf::Color(210, 210, 210));
                msg.setPosition({20.f + padding, y + padding - 4.f});
            }
            window.draw(burbuja);
            window.draw(msg);
            y += burbuja.getSize().y + 12.f;
        }

        // Auto-scroll si hay mensajes nuevos
        maxScrollY = (y > 600.f) ? (y - 600.f) : 0;
        if (currentScrollY < maxScrollY && currentScrollY > maxScrollY - 60.f) currentScrollY = maxScrollY;

        // --- UI FIJA (HEADER Y FOOTER) ---
        window.setView(window.getDefaultView());
        
        sf::RectangleShape header({450.f, 75.f});
        header.setFillColor(sf::Color(0, 51, 102));
        window.draw(header);
        sf::Text titulo(font, "Agente de Soporte Online", 20);
        titulo.setPosition({20, 20});
        window.draw(titulo);

        sf::RectangleShape footer({450.f, 90.f});
        footer.setPosition({0, 610});
        footer.setFillColor(sf::Color::White);
        window.draw(footer);
        sf::Text actual(font, inputTexto.empty() ? "Responder..." : inputTexto + "|", 16);
        actual.setPosition({30, 640});
        actual.setFillColor(sf::Color(160, 160, 160));
        window.draw(actual);

        window.display();
    }
    return 0;
}