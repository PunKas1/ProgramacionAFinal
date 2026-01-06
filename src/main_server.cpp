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

std::string obtenerTimestamp() {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

// Función para guardar el ticket en disco
void generarTicket(int id, std::string nombre, const std::vector<Mensaje>& historial) {
    // 1. Crear un nombre de archivo único (ej: Ticket_Cliente1_1583.txt)
    // Usamos time(nullptr) para que el nombre cambie si se conecta varias veces
    std::string filename = "Ticket_" + nombre + "_" + std::to_string(std::time(nullptr)) + ".txt";
    
    // 2. Abrir archivo para escritura
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
// Este hilo lee lo que envía el cliente que está siendo atendido actualmente
void hiloRedServidor(ServerSocket* servidor, Chat* manager) {
    while (true) {
        std::string mensaje = servidor->recibir();

        // Si el mensaje está vacío, significa DESCONEXIÓN
        if (mensaje.empty()) {
            if (servidor->estoyAtendiendo()) {
                std::cout << "[RED] El cliente actual se ha desconectado.\n";
                
                // --- PASO NUEVO: GENERAR EL TICKET ---
                // 1. Obtenemos los datos ANTES de liberar al cliente
                int id = servidor->getClienteActual();
                std::string nombre = servidor->obtenerNombrePorSocket(id);
                
                // 2. Llamamos a nuestra función mágica
                generarTicket(id, nombre, manager->obtenerHistorial());
                
                // 3. Agregamos mensaje al chat visual del agente
                manager->agregarMensaje("Sistema", "Ticket guardado. Sesion finalizada.", false);
                
                // 4. Liberamos el puesto (Como ya tenías)
                servidor->liberarClienteActual();
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            continue;
        }

        // 3. Si llega mensaje real, buscamos el nombre
        // Nota: Aquí usamos '->' porque 'servidor' ES un puntero en esta función
        int idSocket = servidor->getClienteActual();
        std::string nombreCliente = servidor->obtenerNombrePorSocket(idSocket);

        // 4. Lo agregamos al chat
        manager->agregarMensaje(nombreCliente, mensaje, false);
    }
}

// ================= MAIN =================
int main() {
    ServerSocket servidor;
    Chat miChat;

    std::cout << "Iniciando servidor...\n";

    // 1. Configuración de Red
    if (!servidor.crear() || !servidor.configurar("127.0.0.1", 8080) ||
        !servidor.bindear() || !servidor.escuchar()) {
        std::cerr << "[ERROR] No se pudo iniciar el servidor.\n";
        return -1;
    }

    // Hilo para aceptar conexiones nuevas (background)
    std::thread tAceptar(&ServerSocket::aceptarClientes, &servidor);
    tAceptar.detach();

    // Hilo para leer mensajes
    std::thread tLeer(hiloRedServidor, &servidor, &miChat);
    tLeer.detach();

    std::cout << "Servidor listo y esperando clientes.\n";

    // ================= CONFIGURACIÓN SFML 3.0 =================
    sf::ContextSettings settings;
    settings.antiAliasingLevel = 8;

    sf::RenderWindow window(
        sf::VideoMode({450, 700}),
        "Panel de Agente - Soporte Tecnico",
        sf::State::Windowed, // SFML 3.0
        settings
    );
    window.setFramerateLimit(60);

    // Sistema de Scroll
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
        // Si no atiendo a nadie Y hay gente en cola...
        if (!servidor.estoyAtendiendo() && servidor.hayClientesEnCola()) {
            std::cout << "[SISTEMA] Pasando al siguiente cliente...\n";
            
            if (servidor.tomarSiguienteCliente()) {
                // CORRECCION 1: Usar metodo limpiar en vez de asignar objeto nuevo
                miChat.limpiarHistorial(); 
                
                // CORRECCION 2: Usar punto (.) porque 'servidor' no es puntero aquí
                std::string nombre = servidor.obtenerNombrePorSocket(servidor.getClienteActual());
                miChat.agregarMensaje("Sistema", "Conectado con: " + nombre, false);
            }
        }

        // --- PROCESAR EVENTOS ---
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();

            // Scroll manual
            if (const auto* mouseWheel = event->getIf<sf::Event::MouseWheelScrolled>()) {
                if (mouseWheel->wheel == sf::Mouse::Wheel::Vertical) {
                    currentScrollY -= mouseWheel->delta * 25.f;
                    if (currentScrollY < 0) currentScrollY = 0;
                }
            }

            // Escritura (Solo si hay cliente activo)
            if (servidor.estoyAtendiendo()) {
                if (const auto* texto = event->getIf<sf::Event::TextEntered>()) {
                    std::uint32_t unicode = texto->unicode;
                    if (unicode == '\n' || unicode == '\r') {
                        if (!inputTexto.empty()) {
                            servidor.enviar(inputTexto);
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
        }

        window.clear(sf::Color(240, 240, 245));

        // 1. --- DIBUJAR CHAT ---
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
            
            if (m.esMio) { // Agente (Azul)
                float xPos = window.getSize().x - burbuja.getSize().x - 20.f;
                burbuja.setPosition({xPos, y});
                burbuja.setFillColor(sf::Color(0, 102, 255));
                msg.setPosition({xPos + padding, y + padding - 4.f});
            } else { // Cliente (Blanco)
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

        // Auto-Scroll
        maxScrollY = (y > 600.f) ? (y - 600.f) : 0;
        if (currentScrollY < maxScrollY && currentScrollY > maxScrollY - 60.f) currentScrollY = maxScrollY;

        // 2. --- UI ESTÁTICA ---
        window.setView(window.getDefaultView());

        // Header
        sf::RectangleShape header({450.f, 75.f});
        header.setFillColor(sf::Color(0, 51, 102));
        window.draw(header);

        // Texto Header
        std::string tituloStr = "Esperando clientes...";
        if (servidor.estoyAtendiendo()) {
            tituloStr = "Chat con: " + servidor.obtenerNombrePorSocket(servidor.getClienteActual());
        }
        
        sf::Text titulo(font, tituloStr, 18);
        titulo.setPosition({20, 25});
        window.draw(titulo);

        // Footer
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