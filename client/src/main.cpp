#include "client_discovery_protocol.h"
#include "colors.h"
#include "data_transfer/client_request_transfer.h"
#include "date_time_utils.h"
#include <arpa/inet.h>
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <string>

int main(int argc, char *argv[]) {
#if _DEBUG
    std::cout << BOLD << YELLOW << "Running debug build!" << RESET << std::endl;
#endif
    // Validação dos argumentos da linha de comando
    if (argc < 2) {
        std::cerr << "Uso: ./cliente <porta>" << std::endl;
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

    // Broadcast para descobrir IP do servidor
    char return_server_ip[INET_ADDRSTRLEN];
    int return_server_port;
    if (client_discovery_protocol(return_server_ip, &return_server_port,
                                  (char *)"255.255.255.255", port, 5,
                                  500) == 0) {
#if _DEBUG
        std::cout << YELLOW << "Server found at IP: " << return_server_ip
                  << " Port: " << return_server_port << RESET << std::endl;
#endif
    } else {
        std::cerr << "Server not found" << std::endl;
        return 1;
    }

    // Formato da mensagem de inicio do cliente
    // DATE <YYYY-MM-DD> TIME <HH:MM:SS> server addr <IP_ADDRESS>:<PORT >
    // 2024-10-01 18:37:00 server_addr 10.1.1.20:4001


    std::cout << getCurrentDateString() << " " << getCurrentTimeString()
              << " server_addr " << return_server_ip << ":"
              << return_server_port << std::endl;

    client_request_transfer::RequestDispatcher *request_processor =
        client_request_transfer::RequestDispatcher::get_instance(
            return_server_ip, return_server_port, 500);

    // Process user text
    std::string input;
    while (std::getline(std::cin, input)) {

        if (input.substr(0, 4) == "TEST") {
            request_processor->queue_test(input);
            continue;
        }

        request_processor->queue_request(
            client_request_transfer::Request::from_string(input));
    }
    request_processor->queue_request(
        {0, 0, client_request_transfer::request_t::KILL});

    delete request_processor;
    return 0;
}