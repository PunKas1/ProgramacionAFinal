#include "../include/clienteSocket.h"
#include "../include/chat.h"
#include <SFML/Graphics.hpp>
#include <thread>
#include <atomic> // Para manejar variables entre hilos de forma segura
#include <iostream>
#include <optional>
#include <cstdint>

// Variable global atómica para controlar el estado de espera
// True = Bloqueado (En cola), False = Libre (Atendiendo)
std::atomic<bool> enEspera(true); 

// ================= HILO DE RED =================
void hiloRedCliente(ClienteSocket* cliente, Chat* manager) {
    while (true) {
        std::string mensaje = cliente->recibir();
        
        if (mensaje.empty()) continue;

        // --- DETECTAR COMANDOS DEL SERVIDOR ---
        if (mensaje == "/WAIT") {
            enEspera = true;
            // Opcional: Limpiar chat o mostrar aviso en consola
            std::cout << "[SISTEMA] Puesto en cola de espera.\n";
        }
        else if (mensaje == "/START") {
            enEspera = false;
            std::cout << "[SISTEMA] Agente conectado. Iniciando chat.\n";
            // Opcional: Reproducir sonido de "Ding" aquí si tienes SFML Audio
        }
        else {
            // Es un mensaje normal de chat
            manager->agregarMensaje("Soporte", mensaje, false);
        }
    }
}

// ================= MAIN =================
int main() {
    ClienteSocket cliente;
    Chat miChat;

    // Intentar conectar
    if (!cliente.crear() || !cliente.conectar("127.0.0.1", 8080)) {
        std::cerr << "Error: No se pudo conectar al servidor.\n";
        return -1;
    }

    // Iniciar hilo de escucha
    std::thread red(hiloRedCliente, &cliente, &miChat);
    red.detach();

    // ================= CONFIGURACIÓN SFML 3.0 =================
    sf::ContextSettings settings;
    settings.antiAliasingLevel = 8;

    sf::RenderWindow window(
        sf::VideoMode({450, 700}),
        "Centro de Ayuda - Chat",
        sf::State::Windowed, // Corrección para SFML 3
        settings
    );
    window.setFramerateLimit(60);

    // --- SISTEMA DE SCROLL ---
    sf::View viewChat(sf::FloatRect({0, 0}, {450, 700}));
    float currentScrollY = 0;
    float maxScrollY = 0;

    sf::Font font;
    if (!font.openFromFile("arial.ttf")) {
        std::cerr << "Error: No se encontro arial.ttf\n";
        return -1;
    }

    std::string inputTexto;

    // ================= BUCLE PRINCIPAL =================
    while (window.isOpen()) {
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();

            // Scroll manual (Funciona siempre)
            if (const auto* mouseWheel = event->getIf<sf::Event::MouseWheelScrolled>()) {
                if (mouseWheel->wheel == sf::Mouse::Wheel::Vertical) {
                    currentScrollY -= mouseWheel->delta * 25.f;
                    if (currentScrollY < 0) currentScrollY = 0;
                }
            }

            // Entrada de texto (SOLO SI NO ESTÁ EN ESPERA)
            if (!enEspera) {
                if (const auto* texto = event->getIf<sf::Event::TextEntered>()) {
                    std::uint32_t unicode = texto->unicode;
                    
                    if (unicode == '\n' || unicode == '\r') {
                        if (!inputTexto.empty()) {
                            cliente.enviar(inputTexto.c_str());
                            miChat.agregarMensaje("Yo", inputTexto, true);
                            inputTexto.clear();
                            currentScrollY = maxScrollY; // Bajar al final
                        }
                    } 
                    else if (unicode == 8) { // Backspace
                        if (!inputTexto.empty()) inputTexto.pop_back();
                    } 
                    else if (unicode >= 32 && unicode < 128) {
                        inputTexto += static_cast<char>(unicode);
                    }
                }
            }
        }

        window.clear(sf::Color(240, 242, 245)); // Fondo gris suave

        // 1. --- DIBUJAR MENSAJES (Capa con movimiento) ---
        viewChat.setCenter({225, 350 + currentScrollY});
        window.setView(viewChat);

        float y = 90.f;
        for (const auto& m : miChat.obtenerHistorial()) {
            sf::Text msg(font, m.texto, 16);
            msg.setFillColor(m.esMio ? sf::Color::White : sf::Color(30, 30, 30));

            sf::FloatRect bounds = msg.getLocalBounds();
            sf::RectangleShape burbuja;
            float padding = 12.f;
            burbuja.setSize({bounds.size.x + (padding * 2), bounds.size.y + (padding * 2)});
            
            if (m.esMio) { // Cliente (Verde WhatsApp)
                float xPos = window.getSize().x - burbuja.getSize().x - 20.f;
                burbuja.setPosition({xPos, y});
                burbuja.setFillColor(sf::Color(37, 211, 102));
                msg.setPosition({xPos + padding, y + padding - 4.f});
            } else { // Soporte (Blanco)
                burbuja.setPosition({20.f, y});
                burbuja.setFillColor(sf::Color::White);
                burbuja.setOutlineColor(sf::Color(200, 200, 200));
                burbuja.setOutlineThickness(1.f);
                msg.setPosition({20.f + padding, y + padding - 4.f});
            }

            window.draw(burbuja);
            window.draw(msg);
            y += burbuja.getSize().y + 10.f;
        }

        // Auto-scroll logic
        maxScrollY = (y > 600.f) ? (y - 600.f) : 0;
        if (currentScrollY < maxScrollY && currentScrollY > maxScrollY - 60.f) {
            currentScrollY = maxScrollY;
        }

        // 2. --- DIBUJAR UI FIJA (Header y Footer) ---
        window.setView(window.getDefaultView()); // Restaurar vista estática

        // Header
        sf::RectangleShape header({450.f, 75.f});
        header.setFillColor(sf::Color(37, 211, 102)); // Verde principal
        window.draw(header);

        sf::Text titulo(font, "Soporte Tecnico", 20);
        titulo.setStyle(sf::Text::Bold);
        titulo.setPosition({20, 25});
        window.draw(titulo);

        // Footer (Barra de escritura)
        sf::RectangleShape footer({450.f, 80.f});
        footer.setPosition({0, 620});
        footer.setFillColor(sf::Color::White);
        window.draw(footer);

        if (!enEspera) {
            sf::Text actual(font, inputTexto.empty() ? "Escribe un mensaje..." : inputTexto + "|", 16);
            actual.setPosition({30, 650});
            actual.setFillColor(inputTexto.empty() ? sf::Color(150, 150, 150) : sf::Color::Black);
            window.draw(actual);
        }

        // 3. --- PANTALLA DE BLOQUEO (SI ESTÁ EN COLA) ---
        if (enEspera) {
            // Fondo oscuro semitransparente
            sf::RectangleShape overlay({450.f, 700.f});
            overlay.setFillColor(sf::Color(0, 0, 0, 200)); 
            window.draw(overlay);

            // Mensaje de espera
            sf::Text txtEspera(font, "Nuestros agentes estan ocupados", 20);
            sf::FloatRect bounds = txtEspera.getLocalBounds();
            txtEspera.setOrigin({bounds.size.x / 2.f, bounds.size.y / 2.f});
            txtEspera.setPosition({225.f, 300.f});
            txtEspera.setFillColor(sf::Color::White);
            window.draw(txtEspera);

            sf::Text subEspera(font, "Por favor espera tu turno...", 16);
            sf::FloatRect subBounds = subEspera.getLocalBounds();
            subEspera.setOrigin({subBounds.size.x / 2.f, subBounds.size.y / 2.f});
            subEspera.setPosition({225.f, 340.f});
            subEspera.setFillColor(sf::Color(200, 200, 200));
            window.draw(subEspera);
        }

        window.display();
    }

    return 0;
}