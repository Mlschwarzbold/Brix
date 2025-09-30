#ifndef CLIENT_DISCOVERY_PROTOCOL_H
#define CLIENT_DISCOVERY_PROTOCOL_H

// Your code here
int client_discovery_protocol(char* return_server_ip, int* return_server_port, char* broadcast_ip, int broadcast_port, int max_retries, int initial_timeout_ms);

int extract_ip_and_port_from_response(char* response, char* return_ip, int* return_port);

#endif // CLIENT_DISCOVERY_PROTOCOL_H