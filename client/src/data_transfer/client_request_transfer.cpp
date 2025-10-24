#include "client_request_transfer.h"
#include "data_transfer/socket_utils.h"
#include <cstring>
#include <iostream>
#include <queue>
#include <thread>
#include <unistd.h>

namespace client_request_transfer {

const int MAXLINE = 1024;

std::queue<std::string> RequestDispatcher::request_queue;
int RequestDispatcher::sockfd;
struct sockaddr_in RequestDispatcher::servaddr;
pthread_mutex_t RequestDispatcher::request_queue_lock;
bool RequestDispatcher::is_alive;

RequestDispatcher::RequestDispatcher(char request_server_ip[],
                                     int request_server_port,
                                     int initial_timeout_ms) {
    is_alive = true;
    request_queue = std::queue<std::string>();

    pthread_mutex_init(&request_queue_lock, NULL);

    // Creating socket file descriptor
    sockfd = create_udp_socket();

    // Add timeout
    // set_timeout(sockfd, initial_timeout_ms);

    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr = create_sockaddr(request_server_ip, request_server_port);

    std::thread request_dispatcher_thread(process_requests);

    request_dispatcher_thread.detach();
};

RequestDispatcher::~RequestDispatcher() {
    close(sockfd);
    pthread_mutex_destroy(&request_queue_lock);
    is_alive = false;
}

void RequestDispatcher::queue_request(std::string request) {
    pthread_mutex_lock(&request_queue_lock);
    request_queue.push(request);
    pthread_mutex_unlock(&request_queue_lock);
};

void RequestDispatcher::dispatch_request(std::string request) {
    int n = -1;
    socklen_t len;
    std::string msg;
    char buffer[MAXLINE + 1];

    while (n < 0) {
        // send discovery message
        sendto(sockfd, request.data(), request.length(), 0,
               (const struct sockaddr *)&servaddr, sizeof(servaddr));

        // espera receber mensagem
        n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL,
                     (struct sockaddr *)&servaddr, &len);

        if (n < 0) {
            std::cerr << "Timeout or error receiving response, retrying..."
                      << std::endl;

            continue; // timeout or error, attempt again
        }
        // message received
        buffer[n] = '\0';

        // Validate and extract IP and Port from response message
        // puts values direcly into return_server_ip and return_server_port

        std::cout << buffer << std::endl;

        // valid message received
        break;
    }
}

void RequestDispatcher::process_requests() {
    while (is_alive) {
        if (request_queue.empty()) {
            // This could be done using a semaphore for signaling, but I
            // believe this to be somewhat more semantic and
            // straightforward.
            std::this_thread::yield();
        } else {
            pthread_mutex_lock(&request_queue_lock);
            auto request = request_queue.front();
            request_queue.pop();
            pthread_mutex_unlock(&request_queue_lock);
            dispatch_request(request);
        }
    }
};

RequestDispatcher *RequestDispatcher::get_instance(char *request_server_ip,
                                                   int request_server_port,
                                                   int initial_timeout_ms) {
    static RequestDispatcher *instance = new RequestDispatcher(
        request_server_ip, request_server_port, initial_timeout_ms);
    return instance;
}

} // namespace client_request_transfer