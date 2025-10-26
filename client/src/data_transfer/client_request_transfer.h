#ifndef CLIENT_REQUEST_TRANSFER_H
#define CLIENT_REQUEST_TRANSFER_H
#include "packets/packets.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <queue>
#include <string>

namespace client_request_transfer {

typedef struct {
    in_addr_t dest_ip;
    unsigned long transfer_amount;
} Request;

class RequestDispatcher {
  public:
    static RequestDispatcher *get_instance(char *request_server_ip,
                                           int request_server_port,
                                           int initial_timeout_ms);

    ~RequestDispatcher();

    void queue_request(std::string request);

    void queue_test(std::string test_string);

  private:
    RequestDispatcher(char request_server_ip[], int request_server_port,
                      int initial_timeout_ms);

    static void *process_requests(void *arg);

    static void dispatch_request(Request request);

    static bool handle_response(ACK_Packet reponse);

    static bool is_alive;
    static int sockfd;
    static struct sockaddr_in servaddr;

    static pthread_mutex_t request_queue_lock;
    static pthread_t request_dispatcher_thread;

    static std::queue<Request> request_queue;
    static int current_request;
};

} // namespace client_request_transfer

#endif // CLIENT_REQUEST_TRANSFER_H