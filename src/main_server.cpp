/**
 * @file main_server.cpp
 * @author Valencia Cedeño Marcos Gael
 * @author Peralta Ordóñez Jesús
 * @author Esteves Flores Andrés
 * @brief Punto de entrada de la aplicación Servidor (Agente de Soporte).
 * @version 1.0
 * @date 06/01/2026
 * * Este archivo orquesta los tres componentes principales:
 * 1. La Interfaz Gráfica (GUI) con SFML.
 * 2. La lógica de red en segundo plano (Hilos).
 * 3. La gestión de archivos para generar los Tickets de reporte.
 */

#include "../include/socket.h"
#include "../include/chat.h"
#include <SFML/Graphics.hpp>
#include <thread>
#include <iostream>
#include <optional>
#include <cstdint>
#include <chrono> // Para pausas pequeñas
#include <fstream>  // Para crear y escribir archivos .txt
#include <ctime>    // Para obtener la fecha y hora actual
#include <iomanip>  // Para dar formato a la fecha
#include <sstream>  // Para construir nombres de string

/**
 * @brief Genera una marca de tiempo actual (Timestamp).
 * @return std::string Fecha y hora en formato "YYYY-MM-DD HH:MM:SS".
 */
std::string obtenerTimestamp() {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

/**
 * @brief Crea un archivo de tipo ticket (.txt) al finalizar la sesión.
 * * Esta función garantiza la PERSISTENCIA de los datos. Si el programa se cierra,
 * la conversación queda guardada en disco.
 * @param id ID numérico del cliente.
 * @param nombre Nombre legible (ej. "Cliente 5").
 * @param historial Vector con todos los mensajes de la sesión.
 */
void generarTicket(int id, std::string nombre, const std::vector<Mensaje>& historial) {
    // 1. Crear un nombre de archivo único para evitar sobrescribir tickets anteriores.
    std::string filename = "Ticket_" + nombre + "_" + std::to_string(std::time(nullptr)) + ".txt";
    
    // 2. Abrir archivo para escritura (std::ofstream)
    std::ofstream archivo(filename);
    
    if (archivo.is_open()) {
        archivo << "========================================\n";
        archivo << "          TICKET DE SOPORTE             \n";
        archivo << "========================================\n";
        archivo << "ID Cliente:   " << id << "\n";
        archivo << "Nombre:       " << nombre << "\n";
        archivo << "Fecha y Hora: " << obtenerTimestamp() << "\n";
        archivo << "Total Msjs:   " << historial.size() << "\n";
        archivo << "========================================\n\n";
        archivo << "--- HISTORIAL DE CONVERSACION ---\n";

        // 3. Escribir cada mensaje del chat
        for (const auto& msg : historial) {
            std::string autor = msg.esMio ? "AGENTE" : nombre;
            archivo << "[" << autor << "]: " << msg.texto << "\n";
        }
        
        archivo << "\n========================================\n";
        archivo << "          FIN DEL REPORTE               \n";
        archivo.close();
        
        std::cout << "[SISTEMA] Ticket generado exitosamente: " << filename << "\n";
    } else {
        std::cerr << "[ERROR] No se pudo crear el archivo del ticket.\n";
    }
}

// ================= HILO DE RED (ESCUCHA) =================
/**
 * @brief Función ejecutada por el Hilo Lector (Reader Thread).
 * * Se mantiene en un bucle infinito escuchando al cliente ACTIVO.
 * * Detecta desconexiones y dispara la generación automática del ticket.
 * @param servidor Puntero a la instancia del socket (para recibir datos).
 * @param manager Puntero al gestor del chat (para guardar mensajes).
 */
void hiloRedServidor(ServerSocket* servidor, Chat* manager) {
    while (true) {
        // Bloqueante: Espera aquí hasta que llegue algo
        std::string mensaje = servidor->recibir();

        // Si el mensaje está vacío, significa que el cliente cortó la conexión (FIN packet)
        if (mensaje.empty()) {
            if (servidor->estoyAtendiendo()) {
                std::cout << "[RED] El cliente actual se ha desconectado.\n";
                
                // --- GENERAR EL TICKET ---
                int id = servidor->getClienteActual();
                std::string nombre = servidor->obtenerNombrePorSocket(id);
                
                generarTicket(id, nombre, manager->obtenerHistorial());
                
                manager->agregarMensaje("Sistema", "Ticket guardado. Sesion finalizada.", false);
                
                // Liberamos el puesto para que "El Portero" (Main) deje pasar al siguiente
                servidor->liberarClienteActual();
            }
            
            // Pausa para evitar uso excesivo de CPU cuando no hay nadie
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            continue;
        }

        // Si llega mensaje real, buscamos quién lo envía
        int idSocket = servidor->getClienteActual();
        std::string nombreCliente = servidor->obtenerNombrePorSocket(idSocket);

        // Lo agregamos al historial compartido (Thread-Safe gracias al Mutex en Chat)
        manager->agregarMensaje(nombreCliente, mensaje, false);
    }
}

// ================= MAIN =================
/**
 * @brief Hilo Principal (UI Thread).
 * * Maneja la ventana de SFML, los eventos de entrada (teclado/mouse)
 * y la lógica de asignación de turnos ("El Portero").
 */
int main() {
    ServerSocket servidor;
    Chat miChat;

    std::cout << "Iniciando servidor...\n";

    // 1. Configuración de Red
    // NOTA: Usar "0.0.0.0" para aceptar conexiones externas (LAN), "127.0.0.1" solo local.
    if (!servidor.crear() || !servidor.configurar("127.0.0.1", 8080) ||
        !servidor.bindear() || !servidor.escuchar()) {
        std::cerr << "[ERROR] No se pudo iniciar el servidor.\n";
        return -1;
    }

    // 2. Lanzamiento de Hilos
    // Hilo Aceptador: Mete clientes a la cola en background.
    std::thread tAceptar(&ServerSocket::aceptarClientes, &servidor);
    tAceptar.detach();

    // Hilo Lector: Escucha mensajes del cliente activo.
    std::thread tLeer(hiloRedServidor, &servidor, &miChat);
    tLeer.detach();

    std::cout << "Servidor listo y esperando clientes.\n";

    // ================= CONFIGURACIÓN SFML 3.0 =================
    sf::ContextSettings settings;
    settings.antiAliasingLevel = 8; // Suavizado de bordes

    sf::RenderWindow window(
        sf::VideoMode({450, 700}),
        "Panel de Agente - Soporte Tecnico",
        sf::State::Windowed,
        settings
    );
    window.setFramerateLimit(60);

    // Sistema de Scroll (Cámara virtual)
    sf::View viewChat(sf::FloatRect({0, 0}, {450, 700}));
    float currentScrollY = 0;
    float maxScrollY = 0;

    sf::Font font;
    if (!font.openFromFile("arial.ttf")) {
        std::cerr << "No se encontro arial.ttf\n";
        return -1;
    }

    std::string inputTexto;

    // ================= BUCLE PRINCIPAL =================
    while (window.isOpen()) {
        
        // --- LOGICA AUTOMATICA (EL PORTERO) ---
        // Revisa constantemente si el puesto está libre y si hay alguien esperando.
        if (!servidor.estoyAtendiendo() && servidor.hayClientesEnCola()) {
            std::cout << "[SISTEMA] Pasando al siguiente cliente...\n";
            
            if (servidor.tomarSiguienteCliente()) {
                // Limpiamos la pantalla para la nueva sesión
                miChat.limpiarHistorial(); 
                
                std::string nombre = servidor.obtenerNombrePorSocket(servidor.getClienteActual());
                miChat.agregarMensaje("Sistema", "Conectado con: " + nombre, false);
            }
        }

        // --- PROCESAR EVENTOS (Inputs) ---
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();

            // Scroll con la rueda del ratón
            if (const auto* mouseWheel = event->getIf<sf::Event::MouseWheelScrolled>()) {
                if (mouseWheel->wheel == sf::Mouse::Wheel::Vertical) {
                    currentScrollY -= mouseWheel->delta * 25.f;
                    if (currentScrollY < 0) currentScrollY = 0;
                }
            }

            // Escritura de texto (Solo permitida si hay un cliente activo)
            if (servidor.estoyAtendiendo()) {
                if (const auto* texto = event->getIf<sf::Event::TextEntered>()) {
                    std::uint32_t unicode = texto->unicode;
                    // Manejo de Enter (Enviar) y Backspace (Borrar)
                    if (unicode == '\n' || unicode == '\r') {
                        if (!inputTexto.empty()) {
                            servidor.enviar(inputTexto);
                            miChat.agregarMensaje("Yo", inputTexto, true); // True = Es Mío (Color Azul)
                            inputTexto.clear();
                            currentScrollY = maxScrollY; // Auto-scroll al fondo
                        }
                    } else if (unicode == 8 && !inputTexto.empty()) {
                        inputTexto.pop_back();
                    } else if (unicode >= 32 && unicode < 128) {
                        inputTexto += static_cast<char>(unicode);
                    }
                }
            }
        }

        window.clear(sf::Color(240, 240, 245)); // Fondo gris claro (Estilo WhatsApp)

        // 1. --- DIBUJAR CHAT (Mundo Dinámico) ---
        viewChat.setCenter({225, 350 + currentScrollY});
        window.setView(viewChat);

        float y = 90.f; // Margen superior inicial
        for (const auto& m : miChat.obtenerHistorial()) {
            // Renderizado de burbujas de chat...
            sf::Text msg(font, m.texto, 16);
            msg.setFillColor(m.esMio ? sf::Color::White : sf::Color(30, 30, 30));

            sf::FloatRect bounds = msg.getLocalBounds();
            sf::RectangleShape burbuja;
            float padding = 15.f;
            burbuja.setSize({bounds.size.x + (padding * 2), bounds.size.y + (padding * 2)});
            
            // Diferenciación visual: Agente (Der/Azul) vs Cliente (Izq/Blanco)
            if (m.esMio) { 
                float xPos = window.getSize().x - burbuja.getSize().x - 20.f;
                burbuja.setPosition({xPos, y});
                burbuja.setFillColor(sf::Color(0, 102, 255));
                msg.setPosition({xPos + padding, y + padding - 4.f});
            } else { 
                burbuja.setPosition({20.f, y});
                burbuja.setFillColor(sf::Color::White);
                burbuja.setOutlineThickness(1.f);
                burbuja.setOutlineColor(sf::Color(200, 200, 200));
                msg.setPosition({20.f + padding, y + padding - 4.f});
            }

            window.draw(burbuja);
            window.draw(msg);
            y += burbuja.getSize().y + 12.f;
        }

        // Cálculo del límite de scroll
        maxScrollY = (y > 600.f) ? (y - 600.f) : 0;
        if (currentScrollY < maxScrollY && currentScrollY > maxScrollY - 60.f) currentScrollY = maxScrollY;

        // 2. --- DIBUJAR UI ESTÁTICA (HUD) ---
        window.setView(window.getDefaultView()); // Reseteamos la vista para que el header no se mueva

        // Header (Barra Superior)
        sf::RectangleShape header({450.f, 75.f});
        header.setFillColor(sf::Color(0, 51, 102));
        window.draw(header);

        // Texto Dinámico del Header
        std::string tituloStr = "Esperando clientes...";
        if (servidor.estoyAtendiendo()) {
            tituloStr = "Chat con: " + servidor.obtenerNombrePorSocket(servidor.getClienteActual());
        }
        
        sf::Text titulo(font, tituloStr, 18);
        titulo.setPosition({20, 25});
        window.draw(titulo);

        // Footer (Caja de Texto)
        sf::RectangleShape footer({450.f, 90.f});
        footer.setPosition({0, 610});
        footer.setFillColor(sf::Color::White);
        window.draw(footer);

        std::string placeholderStr = servidor.estoyAtendiendo() ? "Responder..." : "(Sin cliente activo)";
        sf::Text actual(font, inputTexto.empty() ? placeholderStr : inputTexto + "|", 16);
        actual.setPosition({30, 640});
        actual.setFillColor(inputTexto.empty() ? sf::Color(150, 150, 150) : sf::Color::Black);
        window.draw(actual);

        window.display();
    }

    return 0;
}