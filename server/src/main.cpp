#include "colors.h"
#include "date_time_utils.h"
#include "db_manager/db_manager.h"
#include "greeter/server_UDP_greeter.h"
#include "multiplexer/packet_multiplexer.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

int main(int argc, char *argv[]) {
    // Validação dos argumentos da linha de comando
    if (argc < 2) {
        std::cerr << "Uso: ./servidor <porta>" << std::endl;
        return 1;
    }
    // recebe e valida porta do servidor
    int port;
    try {
        port = std::stoi(argv[1]);
    } catch (const std::invalid_argument &e) {
        std::cerr << "Porta inválida: " << argv[1] << std::endl;
        return 1;
    }
    if (port < 0 || port > 65535) {
        std::cerr << "Valores válidos: 0 e 65535" << std::endl;
        return 1;
    }
    // Porta válida, iniciar servidor

    std::cout << YELLOW << "Servidor iniciado na porta: " << port << RESET
              << std::endl;
    std::cout << YELLOW << "Data atual: " << getCurrentDateString() << " "
              << getCurrentTimeString() << RESET << std::endl;

    // db_manager testing
    db_manager::DbManager *db = db_manager::DbManager::get_instace();

    // db->register_client(inet_addr("10.0.0.1"));
    // db->register_client(inet_addr("10.0.0.2"));

    // Initiate greeter service
    // udp_server_greeter::start_server();
    // udp_server_greeter::server_discovery_service(4000, "217.0.0.1", 4001);

    std::thread greeter_thread(udp_server_greeter::start_server, port);

    // Initiate packet multiplexer thread
    std::thread multiplexer_thread(multiplexer::start_multiplexer_server,
                                   port + 1);

    greeter_thread.join();
    multiplexer_thread.join();

    delete db;
    return 0;
}