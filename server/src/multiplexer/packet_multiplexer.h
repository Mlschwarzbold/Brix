#ifndef PACKET_MULTIPLEXER_H
#define PACKET_MULTIPLEXER_H
namespace multiplexer {

void *start_multiplexer_server(void *arg);

int packet_multiplexer(int port);

} // namespace multiplexer
#endif // PACKET_MULTIPLEXER_H