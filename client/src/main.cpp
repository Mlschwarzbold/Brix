#include <iostream>
#include <string>
#include <stdexcept>
#include "date_time_utils.h"
#include "client_UDP_broadcast.h"

int main(int argc, char* argv[]) {
    // Validação dos argumentos da linha de comando
    if(argc < 2) {
        std::cerr << "Uso: ./cliente <porta>" << std::endl;
        return 1;
    }
    // recebe e valida porta do servidor
    int port;
    try{
        port = std::stoi(argv[1]);
    } catch(const std::invalid_argument& e) {
        std::cerr << "Porta inválida: " << argv[1] << std::endl;
        return 1;
    }
    if(port < 0 || port > 65535) {
        std::cerr << "Valores válidos: 0 e 65535" << std::endl;
        return 1;
    }
    // Porta válida, iniciar cliente

    std::cout << "Cliente iniciado na porta: " << port << std::endl;
    std::cout << "Data atual: " << getCurrentDateString() << " " << getCurrentTimeString() << std::endl;


    // Broadcast para descobrir IP do servidor

    client_UDP_broadcast();
    

    return 0;
}