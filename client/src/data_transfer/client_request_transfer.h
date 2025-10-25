#ifndef CLIENT_REQUEST_TRANSFER_H
#define CLIENT_REQUEST_TRANSFER_H
#include "packets/packets.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <queue>
#include <random>
#include <sstream>
#include <string>

namespace client_request_transfer {

class RequestDispatcher {
  public:
    static RequestDispatcher *get_instance(char *request_server_ip,
                                           int request_server_port,
                                           int initial_timeout_ms);

    ~RequestDispatcher();

    void queue_request(std::string request);

    void queue_test(std::string test_string) {
        std::random_device rd;
        std::mt19937 gen(rd());

        std::istringstream iss(test_string);
        std::string token;
        std::vector<std::string> tokens;

        while (iss >> token) {
            tokens.push_back(token);
        }

        std::string target_ip = (tokens.size() > 1) ? tokens[1] : "10.33.1.2";

        for (int i = 1; i <= 1000; i++) {
            std::uniform_int_distribution<> dist(0, 99);
            int roll = dist(gen) + 100;
            // 75% of the time, just use the correct package number
            queue_request(
                REQ_Packet(i, inet_addr(target_ip.c_str()), 1).to_string());

            if (roll < 50) {
                // 50% of the time, also send a duplicate
                queue_request(
                    REQ_Packet(i, inet_addr(target_ip.c_str()), 1).to_string());
            }
            if (roll < 20) {
                // 20% of the time, also send an out of order
                queue_request(REQ_Packet(99999, inet_addr(target_ip.c_str()), 1)
                                  .to_string());
            }
            if (roll < 90) {
                queue_request("FOOBAR");
            }
        }
    }

  private:
    RequestDispatcher(char request_server_ip[], int request_server_port,
                      int initial_timeout_ms);

    static void *process_requests(void *arg);

    static void dispatch_request(std::string request);

    static std::queue<std::string> request_queue;
    static int sockfd;
    static struct sockaddr_in servaddr;
    static pthread_mutex_t request_queue_lock;
    static bool is_alive;
    static pthread_t request_dispatcher_thread;
};

} // namespace client_request_transfer

#endif // CLIENT_REQUEST_TRANSFER_H