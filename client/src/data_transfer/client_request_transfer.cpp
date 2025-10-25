#include "client_request_transfer.h"
#include "colors.h"
#include "data_transfer/socket_utils.h"
#include "date_time_utils.h"
#include "packets/string_packets.h"
#include <cstring>
#include <iostream>
#include <queue>
#include <unistd.h>

namespace client_request_transfer {

const int MAXLINE = 2048;
const int MAX_RETRIES = 5;

std::queue<std::string> RequestDispatcher::request_queue;
int RequestDispatcher::sockfd;
struct sockaddr_in RequestDispatcher::servaddr;
pthread_mutex_t RequestDispatcher::request_queue_lock;
bool RequestDispatcher::is_alive;
pthread_t RequestDispatcher::request_dispatcher_thread;

RequestDispatcher::RequestDispatcher(char request_server_ip[],
                                     int request_server_port,
                                     int initial_timeout_ms) {
    is_alive = true;
    request_queue = std::queue<std::string>();

    pthread_mutex_init(&request_queue_lock, NULL);

    // Creating socket file descriptor
    sockfd = create_udp_socket();

    // Add timeout
    set_timeout(sockfd, initial_timeout_ms);

    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr = create_sockaddr(request_server_ip, request_server_port);

    // std::thread request_dispatcher_thread(process_requests);

    pthread_create(&request_dispatcher_thread, NULL,
                   RequestDispatcher::process_requests, NULL);
};

RequestDispatcher::~RequestDispatcher() {
    is_alive = false;
    pthread_join(request_dispatcher_thread, NULL);
}

void RequestDispatcher::queue_request(std::string request) {
    pthread_mutex_lock(&request_queue_lock);
    request_queue.push(request);
    pthread_mutex_unlock(&request_queue_lock);
};

void RequestDispatcher::dispatch_request(std::string request) {
    std::cout << BLUE << "[REQUEST DISPATCHER] Dispatching: " << request
              << RESET << std::endl;
    int n = -1, retries = 0;
    socklen_t len;
    std::string msg;
    char buffer[MAXLINE + 1];

    while (n < 0 && retries < MAX_RETRIES) {
        sendto(sockfd, request.data(), request.length(), 0,
               (const struct sockaddr *)&servaddr, sizeof(servaddr));

        n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL,
                     (struct sockaddr *)&servaddr, &len);

        if (n >= 0) {
            buffer[n] = '\0';

            String_Packet response = String_Packet(buffer);

            try {
                ACK_Packet parsed_response = response.to_ACK_Packet();
                // 2024-10-01 18:37:01 server 10.1.1.20 id_req 1 dest 10.1.1.3
                // value 10 new balance
                std::cout << CYAN << getCurrentDateString() << " "
                          << getCurrentTimeString();
                std::cout << " server "
                          << addr_to_string(servaddr.sin_addr.s_addr);
                std::cout << " id_req " << parsed_response.seq_num;
                std::cout << " dest "
                          << addr_to_string(parsed_response.receiver_ip);
                std::cout << " value " << parsed_response.transfer_amount;
                std::cout << " new_balance " << parsed_response.new_balance;

                std::cout << " result ";
                if (parsed_response.result == "SUCCESS") {
                    std::cout << GREEN;
                } else {
                    std::cout << RED;
                }

                std::cout << parsed_response.result;
                std::cout << RESET << std::endl;
            } catch (std::exception const &) {
                std::cerr << RED
                          << "Failed to parse input into ACK : " << response
                          << RESET;

                // Keep trying to send
                continue;
            }

            return;
        }

        std::cerr << "Timeout or error receiving response, retrying..."
                  << std::endl;

        retries++;
    }

    std::cerr << "Too many retries, giving up." << std::endl;
}

void *RequestDispatcher::process_requests(void *arg) {
    while (is_alive) {
        if (request_queue.empty()) {
            // This could be done using a semaphore for signaling, but I
            // believe this to be somewhat more semantic and
            // straightforward.

            // pthread_yield() is deprecated!
            sched_yield();
        } else {
            pthread_mutex_lock(&request_queue_lock);
            auto request = request_queue.front();
            request_queue.pop();
            pthread_mutex_unlock(&request_queue_lock);
            dispatch_request(request);
        }
    }

    close(sockfd);
    pthread_mutex_destroy(&request_queue_lock);
    return nullptr;
};

RequestDispatcher *RequestDispatcher::get_instance(char *request_server_ip,
                                                   int request_server_port,
                                                   int initial_timeout_ms) {
    static RequestDispatcher *instance = new RequestDispatcher(
        request_server_ip, request_server_port, initial_timeout_ms);
    return instance;
}

} // namespace client_request_transfer