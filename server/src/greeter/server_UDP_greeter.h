#ifndef SERVER_UDP_GREETER_H
#define SERVER_UDP_GREETER_H

#include <string>
namespace udp_server_greeter {
std::string get_self_ip();
void *start_server(void *arg);
} // namespace udp_server_greeter

#endif // SERVER_UDP_GREETER_H