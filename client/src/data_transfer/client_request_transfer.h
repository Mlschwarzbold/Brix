#ifndef CLIENT_REQUEST_TRANSFER_H
#define CLIENT_REQUEST_TRANSFER_H
#include <netinet/in.h>
#include <queue>
#include <string>

namespace client_request_transfer {

class RequestDispatcher {
  public:
    static RequestDispatcher *get_instance(char *request_server_ip,
                                           int request_server_port,
                                           int initial_timeout_ms);

    ~RequestDispatcher();

    void queue_request(std::string request);

  private:
    RequestDispatcher(char request_server_ip[], int request_server_port,
                      int initial_timeout_ms);

    static void process_requests();

    static void dispatch_request(std::string request);

    static std::queue<std::string> request_queue;
    static int sockfd;
    static struct sockaddr_in servaddr;
    static pthread_mutex_t request_queue_lock;
    static bool is_alive;
};

} // namespace client_request_transfer

#endif // CLIENT_REQUEST_TRANSFER_H