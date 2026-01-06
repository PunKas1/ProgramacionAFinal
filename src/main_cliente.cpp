/**
 * @file main_cliente.cpp
 * @author Valencia Cedeño Marcos Gael
 * @author Peralta Ordóñez Jesús
 * @author Esteves Flores Andrés
 * @brief Punto de entrada de la aplicación Cliente (Usuario).
 * @version 1.0
 * @date 06/01/2026
 * * Este archivo maneja la Interfaz Gráfica (SFML) para el usuario final.
 * * Implementa el protocolo de comunicación (/WAIT, /START) para bloquear
 * o desbloquear la interacción según la disponibilidad del agente.
 */

#include "../include/clienteSocket.h"
#include "../include/chat.h"
#include <SFML/Graphics.hpp>
#include <thread>
#include <atomic> // Para manejar variables entre hilos de forma segura
#include <iostream>
#include <optional>
#include <cstdint>

/**
 * @brief Bandera de estado compartida entre hilos (Thread-Safe).
 * * Se usa std::atomic<bool> en lugar de un bool normal para evitar "Condiciones de Carrera".
 * * True: El cliente está en la cola de espera (Pantalla Bloqueada).
 * * False: El cliente está siendo atendido (Pantalla Libre).
 */
std::atomic<bool> enEspera(true); 

// ================= HILO DE RED =================
/**
 * @brief Función del Hilo Secundario: Escucha al servidor.
 * * Se encarga de recibir mensajes y decidir si son TEXTO para el chat
 * o COMANDOS para cambiar el estado de la aplicación.
 */
void hiloRedCliente(ClienteSocket* cliente, Chat* manager) {
    while (true) {
        // Bloqueante: Espera a que llegue algo del servidor
        std::string mensaje = cliente->recibir();
        
        if (mensaje.empty()) continue; // Si hay error o vacío, reintentamos

        // --- DETECTAR COMANDOS DEL PROTOCOLO ---
        if (mensaje == "/WAIT") {
            // El servidor nos dice que esperemos. Bloqueamos la UI.
            enEspera = true; 
            std::cout << "[SISTEMA] Puesto en cola de espera.\n";
        }
        else if (mensaje == "/START") {
            // El servidor nos dice que es nuestro turno. Desbloqueamos la UI.
            enEspera = false;
            std::cout << "[SISTEMA] Agente conectado. Iniciando chat.\n";
        }
        else {
            // Si no es comando, es un mensaje de texto normal del Agente.
            manager->agregarMensaje("Soporte", mensaje, false);
        }
    }
}

// ================= MAIN =================
/**
 * @brief Hilo Principal (UI Thread).
 * * Dibuja la ventana, maneja el input del usuario y renderiza el overlay de bloqueo.
 */
int main() {
    ClienteSocket cliente;
    Chat miChat;

    // 1. Intentar conectar al servidor (Handshake TCP)
    // CAMBIAR "127.0.0.1" por la IP del servidor si es remoto.
    if (!cliente.crear() || !cliente.conectar("127.0.0.1", 8080)) {
        std::cerr << "Error: No se pudo conectar al servidor.\n";
        return -1;
    }

    // 2. Iniciar el hilo de escucha en background
    // Usamos detach() para que corra libremente sin bloquear la ventana.
    std::thread read(hiloRedCliente, &cliente, &miChat);
    read.detach();

    // ================= CONFIGURACIÓN SFML 3.0 =================
    sf::ContextSettings settings;
    settings.antiAliasingLevel = 8;

    sf::RenderWindow window(
        sf::VideoMode({450, 700}),
        "Centro de Ayuda - Chat",
        sf::State::Windowed,
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

    // ================= BUCLE PRINCIPAL (Game Loop) =================
    while (window.isOpen()) {
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();

            // Scroll manual (Siempre permitido para leer historial)
            if (const auto* mouseWheel = event->getIf<sf::Event::MouseWheelScrolled>()) {
                if (mouseWheel->wheel == sf::Mouse::Wheel::Vertical) {
                    currentScrollY -= mouseWheel->delta * 25.f;
                    if (currentScrollY < 0) currentScrollY = 0;
                }
            }

            // Entrada de texto (CRÍTICO: SOLO SI NO ESTÁ EN ESPERA)
            // Si enEspera es true, ignoramos lo que escriba el usuario.
            if (!enEspera) {
                if (const auto* texto = event->getIf<sf::Event::TextEntered>()) {
                    std::uint32_t unicode = texto->unicode;
                    
                    if (unicode == '\n' || unicode == '\r') {
                        if (!inputTexto.empty()) {
                            // Enviar al servidor y mostrar en pantalla propia
                            cliente.enviar(inputTexto.c_str());
                            miChat.agregarMensaje("Yo", inputTexto, true);
                            inputTexto.clear();
                            currentScrollY = maxScrollY; // Auto-scroll al fondo
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

        window.clear(sf::Color(240, 242, 245)); // Fondo gris suave (Estilo App Moderna)

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

        // Lógica de límite de scroll
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

        // Solo dibujamos el texto de entrada si NO estamos bloqueados
        if (!enEspera) {
            sf::Text actual(font, inputTexto.empty() ? "Escribe un mensaje..." : inputTexto + "|", 16);
            actual.setPosition({30, 650});
            actual.setFillColor(inputTexto.empty() ? sf::Color(150, 150, 150) : sf::Color::Black);
            window.draw(actual);
        }

        // 3. --- PANTALLA DE BLOQUEO (OVERLAY) ---
        // Si el servidor nos mandó /WAIT, dibujamos esto encima de todo
        if (enEspera) {
            // Fondo oscuro semitransparente (Efecto "Dimming")
            sf::RectangleShape overlay({450.f, 700.f});
            overlay.setFillColor(sf::Color(0, 0, 0, 200)); // Negro con Alpha 200
            window.draw(overlay);

            // Mensaje de espera
            sf::Text txtEspera(font, "Nuestros agentes estan ocupados", 20);
            sf::FloatRect bounds = txtEspera.getLocalBounds();
            txtEspera.setOrigin({bounds.size.x / 2.f, bounds.size.y / 2.f}); // Centrado
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