#ifndef SERVER_DISCOVERY_SERVICE_H
#define SERVER_DISCOVERY_SERVICE_H

namespace udp_server_greeter {

int server_discovery_service(
    int discovery_service_port, 
    char* requests_server_ip, 
    int requests_server_port
);

}// namespace udp_server_greeter
#endif // SERVER_DISCOVERY_SERVICE_H