// Server side implementation of UDP client-server model
#include "server_UDP_greeter.h"
#include "data_transfer/socket_utils.h"
#include "server_discovery_service.h"
#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace udp_server_greeter {

void *start_server(void *arg) {
    int port = *(int *)arg;

    const auto self_ip = get_self_ip();

    (void)udp_server_greeter::server_discovery_service(
        port, (char *)self_ip.c_str(), port + 1);
    return nullptr;
}

void test_print() { std::cout << "TEST PRINT" << std::endl; }

} // namespace udp_server_greeter