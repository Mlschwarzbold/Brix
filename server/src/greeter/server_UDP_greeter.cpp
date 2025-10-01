// Server side implementation of UDP client-server model
#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "server_UDP_greeter.h"
#include "server_discovery_service.h"

#define PORT 8080
#define MAXLINE 1024

namespace udp_server_greeter {

int start_server() {

    udp_server_greeter::server_discovery_service(4000, "217.0.0.1", 4001);

    return 0;
}
} // namespace udp_server_greeter