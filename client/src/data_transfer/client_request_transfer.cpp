#include "client_request_transfer.h"
#include "client_discovery_protocol.h"
#include "colors.h"
#include "data_transfer/socket_utils.h"
#include "date_time_utils.h"
#include "packets/string_packets.h"
#include <cstring>
#include <iostream>
#include <queue>
#include <sstream>
#include <unistd.h>

namespace client_request_transfer {

const int MAXLINE = 2048;
const int MAX_RETRIES = 5;
const int INITIAL_TIMEOUT = 200;

bool RequestDispatcher::is_alive;
int RequestDispatcher::sockfd;
struct sockaddr_in RequestDispatcher::servaddr;
int RequestDispatcher::location_service_port;

pthread_mutex_t RequestDispatcher::request_queue_lock;
pthread_t RequestDispatcher::request_dispatcher_thread;

std::queue<Request> RequestDispatcher::request_queue;
int RequestDispatcher::current_request;

RequestDispatcher::RequestDispatcher(int port) {
    is_alive = true;
    current_request = 1;
    request_queue = std::queue<Request>();
    location_service_port = port;

    pthread_mutex_init(&request_queue_lock, NULL);

    // Creating socket file descriptor
    sockfd = create_udp_socket();

    // Add timeout
    set_timeout(sockfd, INITIAL_TIMEOUT);

    memset(&servaddr, 0, sizeof(servaddr));

    locate_request_server();

    pthread_create(&request_dispatcher_thread, NULL,
                   RequestDispatcher::process_requests, NULL);
};

RequestDispatcher::~RequestDispatcher() {
    is_alive = false;
    pthread_join(request_dispatcher_thread, NULL);
}

void RequestDispatcher::queue_request(Request request) {
    if (request.type == request_t::ERROR)
        return;

    pthread_mutex_lock(&request_queue_lock);
    request_queue.push(request);
    pthread_mutex_unlock(&request_queue_lock);
};

void RequestDispatcher::dispatch_request(Request request) {
    int n = -1, retries = 0;
    socklen_t len;
    std::string msg;
    char buffer[MAXLINE + 1];

    while (retries < MAX_RETRIES) {
        retries++;

        std::string serialized_request;

        if (request.type == request_t::REQ) {
            serialized_request = REQ_Packet(current_request, request.dest_ip,
                                            request.transfer_amount)
                                     .to_string();

        } else if (request.type == request_t::KILL) {
            serialized_request = "KIL END";
        }
#if _DEBUG
        std::cout << BLUE
                  << "[REQUEST DISPATCHER] Dispatching: " << serialized_request
                  << " to " << addr_to_string(servaddr.sin_addr.s_addr) << ":"
                  << servaddr.sin_port << RESET << std::endl;
#endif

        sendto(sockfd, serialized_request.data(), serialized_request.length(),
               0, (const struct sockaddr *)&servaddr, sizeof(servaddr));

        n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL,
                     (struct sockaddr *)&servaddr, &len);
        if (n < 0) {
#if _DEBUG
            std::cerr << BLUE
                      << "[REQUEST DISPATCHER] Timeout or error receiving "
                         "response, retrying..."
                      << RESET << std::endl;
#endif

            // Figure out if server is not down
            locate_request_server();
            continue;
        }

        buffer[n] = '\0';

        String_Packet response = String_Packet(buffer);

#if _DEBUG
        std::cout << YELLOW << "Server answer: " << buffer << RESET
                  << std::endl;
#endif

        try {
            ACK_Packet parsed_response = response.to_ACK_Packet();

            // Check response for sucess and update current_request in case of
            // failure.
            bool success = handle_response(parsed_response);

            // Managed to send
            if (success) {
                return;
            }

        } catch (std::exception const &) {
#if _DEBUG
            std::cerr << BLUE
                      << "[REQUEST DISPATCHER] Failed to parse response "
                         "into ACK : "
                      << response << RESET;
#endif
        }
    }

#if _DEBUG
    std::cerr << RED << "[REQUEST DISPATCHER] Too many retries, giving up."
              << RESET << std::endl;
#endif
}

void *RequestDispatcher::process_requests(void *arg) {
    // Prevent thread from dying while there are still pending requests
    while (is_alive || !request_queue.empty()) {
        if (request_queue.empty()) {
            // This could be done using a semaphore for signaling, but I
            // believe this to be somewhat more semantic and
            // straightforward.

            // pthread_yield() is deprecated!
            sched_yield();
        } else {
            pthread_mutex_lock(&request_queue_lock);
            Request request = request_queue.front();
            request_queue.pop();
            pthread_mutex_unlock(&request_queue_lock);
            dispatch_request(request);
        }
    }

    close(sockfd);
    pthread_mutex_destroy(&request_queue_lock);
    return nullptr;
};

RequestDispatcher *RequestDispatcher::get_instance(int location_service_port) {
    static RequestDispatcher *instance =
        new RequestDispatcher(location_service_port);
    return instance;
}

void RequestDispatcher::queue_test(std::string test_string) {
    std::istringstream iss(test_string);
    std::string token;
    std::vector<std::string> tokens;

    while (iss >> token) {
        tokens.push_back(token);
    }

    std::string target_ip = (tokens.size() > 1) ? tokens[1] : "10.33.1.2";

    std::string request = target_ip + " 1";

    for (int i = 1; i <= 100000; i++) {
        queue_request(Request::from_string(request));
    }
}

bool RequestDispatcher::handle_response(ACK_Packet response) {
    current_request = response.seq_num + 1;

    if (response.result == "SUCCESS") {
        // 2024-10-01 18:37:01 server 10.1.1.20 id_req 1
        // dest 10.1.1.3 value 10 new balance
        std::cout << CYAN << getCurrentDateString() << " "
                  << getCurrentTimeString();
        std::cout << " server " << addr_to_string(servaddr.sin_addr.s_addr);
        std::cout << " id_req " << response.seq_num;
        std::cout << " dest " << addr_to_string(response.receiver_ip);
        std::cout << " value " << response.transfer_amount;
        std::cout << " new_balance " << response.new_balance;
        std::cout << " " << GREEN << response.result;
        std::cout << RESET << std::endl;
        return true;
    }

    if (response.result == "BALANCE_CHECK") {
        // 2024-10-01 18:37:01 server 10.1.1.20 id_req 1
        // dest 10.1.1.3 value 10 new balance
        std::cout << CYAN << getCurrentDateString() << " "
                  << getCurrentTimeString();
        std::cout << " server " << addr_to_string(servaddr.sin_addr.s_addr);
        std::cout << " id_req " << response.seq_num;
        std::cout << " new_balance " << response.new_balance;
        std::cout << " " << GREEN << response.result;
        std::cout << RESET << std::endl;
        return true;
    }

    if (response.result == "KILL") {
        // 2024-10-01 18:37:01 server 10.1.1.20 id_req 1
        // dest 10.1.1.3 value 10 new balance
        std::cout << CYAN << getCurrentDateString() << " "
                  << getCurrentTimeString();
        std::cout << " server " << addr_to_string(servaddr.sin_addr.s_addr);
        std::cout << " id_req " << response.seq_num;
        std::cout << " removed_balance " << response.new_balance;
        std::cout << " " << GREEN << response.result;
        std::cout << RESET << std::endl;
        return true;
    }

#if _DEBUG
    std::cout << YELLOW << getCurrentDateString() << " "
              << getCurrentTimeString();
    std::cout << " server " << addr_to_string(servaddr.sin_addr.s_addr);
    std::cout << " id_req " << response.seq_num;
    std::cout << " dest " << addr_to_string(response.receiver_ip);
    std::cout << " value " << response.transfer_amount;
    std::cout << " new_balance " << response.new_balance;
    std::cout << " " << RED << response.result;
    std::cout << RESET << std::endl;
#endif

    return false;
}
request_t request_t::from_string(std::string input) {
    // Transform user input into dispatchable request
    std::istringstream iss(input);
    std::string token;
    std::vector<std::string> tokens;

    while (iss >> token) {
        tokens.push_back(token);
    }

    if (tokens.size() != 2) {
        std::cout << RED << "Bad input!\n"
                  << BOLD << "Usage: <IP> <AMOUNT>" << RESET << std::endl;
        return {0, 0, ERROR};
    }

    try {
        in_addr_t dest_ip = inet_addr(tokens[0].c_str());
        unsigned long transfer_amount = std::stoul(tokens[1]);

        return {dest_ip, transfer_amount, REQ};

    } catch (std::invalid_argument &) {
        std::cout << RED << "Bad input!\n"
                  << BOLD << "Usage: <IP> <AMOUNT>" << RESET << std::endl;
        return {0, 0, ERROR};
    }
};
void RequestDispatcher::locate_request_server() {
    std::cout << location_service_port << std::endl;
    // Broadcast para descobrir IP do servidor
    char request_server_ip[INET_ADDRSTRLEN];
    int request_server_port;
    if (client_discovery_protocol(request_server_ip, &request_server_port,
                                  (char *)"255.255.255.255",
                                  location_service_port, 5, 500) == 0) {
#if _DEBUG
        std::cout << YELLOW << "Server found at IP: " << request_server_ip
                  << " Port: " << request_server_port << RESET << std::endl;
#endif
    } else {
        std::cerr << "Server not found" << std::endl;
    }

    // Formato da mensagem de inicio do cliente
    // DATE <YYYY-MM-DD> TIME <HH:MM:SS> server addr <IP_ADDRESS>:<PORT >
    // 2024-10-01 18:37:00 server_addr 10.1.1.20:4001

    std::cout << getCurrentDateString() << " " << getCurrentTimeString()
              << " server_addr " << request_server_ip << ":"
              << request_server_port << std::endl;

    servaddr = create_sockaddr(request_server_ip, request_server_port);
};
} // namespace client_request_transfer