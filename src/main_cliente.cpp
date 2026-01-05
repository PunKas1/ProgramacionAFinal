#include "../include/clienteSocket.h"
#include "../include/chat.h"
#include <SFML/Graphics.hpp>
#include <thread>
#include <optional>
#include <cstdint>

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

    if (!cliente.crear() || !cliente.conectar("127.0.0.1", 8080)) return -1;

    std::thread red(hiloRedCliente, &cliente, &miChat);
    red.detach();

    sf::ContextSettings settings;
    settings.antiAliasingLevel = 8;

    sf::RenderWindow window(sf::VideoMode({450, 700}), "Centro de Ayuda", sf::State::Windowed, settings);
    window.setFramerateLimit(60);

    sf::View viewChat(sf::FloatRect({0, 0}, {450, 700}));
    float currentScrollY = 0;
    float maxScrollY = 0;

    sf::Font font;
if (!font.openFromFile("arial.ttf")) {
    return -1;
}
    std::string inputTexto;

    while (window.isOpen()) {
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();

            if (const auto* mouseWheel = event->getIf<sf::Event::MouseWheelScrolled>()) {
                if (mouseWheel->wheel == sf::Mouse::Wheel::Vertical) {
                    currentScrollY -= mouseWheel->delta * 25.f;
                    if (currentScrollY < 0) currentScrollY = 0;
                }
            }

            if (const auto* texto = event->getIf<sf::Event::TextEntered>()) {
                std::uint32_t unicode = texto->unicode;
                if (unicode == '\n' || unicode == '\r') {
                    if (!inputTexto.empty()) {
                        cliente.enviar(inputTexto.c_str());
                        miChat.agregarMensaje("Yo", inputTexto, true);
                        inputTexto.clear();
                        currentScrollY = maxScrollY;
                    }
                } else if (unicode == 8 && !inputTexto.empty()) {
                    inputTexto.pop_back();
                } else if (unicode >= 32 && unicode < 128) {
                    inputTexto += static_cast<char>(unicode);
                }
            }
        }

        window.clear(sf::Color(245, 245, 250));

        // Mensajes con Scroll
        viewChat.setCenter({225, 350 + currentScrollY});
        window.setView(viewChat);

        float y = 90.f;
        for (const auto& m : miChat.obtenerHistorial()) {
            sf::Text msg(font, m.texto, 16);
            msg.setFillColor(m.esMio ? sf::Color::White : sf::Color(40, 40, 40));

            sf::FloatRect bounds = msg.getLocalBounds();
            sf::RectangleShape burbuja;
            float padding = 15.f;
            burbuja.setSize({bounds.size.x + (padding * 2), bounds.size.y + (padding * 2)});
            
            if (m.esMio) { // Cliente - Verde (Derecha)
                float xPos = window.getSize().x - burbuja.getSize().x - 20.f;
                burbuja.setPosition({xPos, y});
                burbuja.setFillColor(sf::Color(37, 211, 102));
                msg.setPosition({xPos + padding, y + padding - 4.f});
            } else { // Soporte - Blanco (Izquierda)
                burbuja.setPosition({20.f, y});
                burbuja.setFillColor(sf::Color::White);
                burbuja.setOutlineThickness(1.f);
                burbuja.setOutlineColor(sf::Color(220, 220, 220));
                msg.setPosition({20.f + padding, y + padding - 4.f});
            }
            window.draw(burbuja);
            window.draw(msg);
            y += burbuja.getSize().y + 12.f;
        }

        maxScrollY = (y > 600.f) ? (y - 600.f) : 0;
        if (currentScrollY < maxScrollY && currentScrollY > maxScrollY - 60.f) currentScrollY = maxScrollY;

        // UI Fija
        window.setView(window.getDefaultView());
        
        sf::RectangleShape header({450.f, 75.f});
        header.setFillColor(sf::Color(37, 211, 102));
        window.draw(header);
        sf::Text titulo(font, "Asistente de Ayuda", 20);
        titulo.setPosition({20, 20});
        window.draw(titulo);

        sf::RectangleShape footer({450.f, 90.f});
        footer.setPosition({0, 610});
        footer.setFillColor(sf::Color::White);
        window.draw(footer);

        sf::Text actual(font, inputTexto.empty() ? "Escriba su consulta..." : inputTexto + "|", 16);
        actual.setPosition({30, 640});
        actual.setFillColor(sf::Color(120, 120, 120));
        window.draw(actual);

        window.display();
    }
    return 0;
} 

