#include "date_time_utils.h"
#include "db_manager/db_manager.h"
#include "greeter/server_UDP_greeter.h"
#include <iostream>
#include <stdexcept>
#include <string>

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

    std::cout << "Servidor iniciado na porta: " << port << std::endl;
    std::cout << "Data atual: " << getCurrentDateString() << " "
              << getCurrentTimeString() << std::endl;

    // db_manager testing
    db_manager::demo();

    // Initiate greeter service
    udp_server_greeter::start_server();

    return 0;
}