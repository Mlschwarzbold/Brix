// Server side implementation of UDP client-server model
#include "server_UDP_greeter.h"
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
        port, (char *)self_ip.c_str(), 4001);
    return nullptr;
}

// Stupid trick to get the local ip of server:
// Figure out what is the hostname of the computer.
// Use gethostbyname to get the ip of the given hostname (which in this case is
// just the ip of this computer).
// I wish there was a simpler way to do this (glances at getaddrinfo)
// based off:
// https://www.tutorialspoint.com/how-to-get-the-ip-address-of-local-computer-using-c-cplusplus
std::string get_self_ip() {
    char hostname[256];

    if (gethostname(hostname, sizeof(hostname)) == -1) {
        return 0;
    }

    hostent *host_entry = gethostbyname(hostname);
    if (host_entry == nullptr) {
        return 0;
    }

    in_addr *addr = (in_addr *)host_entry->h_addr_list[0];

    return inet_ntoa(*addr);
}

} // namespace udp_server_greeter