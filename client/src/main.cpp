#include "client_UDP_broadcast.h"
#include "client_discovery_protocol.h"
#include "date_time_utils.h"
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <string>

#define INET_ADDRSTRLEN 16 // xxx.xxx.xxx.xxx\0

int main(int argc, char *argv[]) {
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

  // Porta válida, iniciar cliente

  std::cout << "Cliente iniciado na porta: " << port << std::endl;
  std::cout << "Data atual: " << getCurrentDateString() << " "
            << getCurrentTimeString() << std::endl;

  // Broadcast para descobrir IP do servidor
  char return_server_ip[INET_ADDRSTRLEN];
  int return_server_port;
  client_discovery_protocol(return_server_ip, &return_server_port, "192.168.0.255", port, -1, -1);

  std::cout<< "port: " << return_server_port << " ip: " << return_server_ip << std::endl;

  return 0;
}