#include "heartbeat.h"
#include "colors.h"
#include <iostream>
#include <pthread.h>
#include <arpa/inet.h>
#include <cstring>
#include "data_transfer/socket_utils.h"
#include <bits/stdc++.h>
#include "ports.h"
#include "redundancy_manager.h"

#define MAXLINE_HEARTBEAT 32

namespace election {

    void heartbeat_receiver(int port){
        int sockfd;
        char buffer[MAXLINE_HEARTBEAT + 1];
        char send_buffer[MAXLINE_HEARTBEAT + 1];
        struct sockaddr_in servaddr, cliaddr;

        // Creating socket file descriptor
        sockfd = create_udp_socket();

        // memset(&servaddr, 0, sizeof(servaddr));
        memset(&cliaddr, 0, sizeof(cliaddr));

        // Filling server information
        servaddr = create_sockaddr("0.0.0.0", port);

        // Bind the socket with the server address
        bind_to_sockaddr(sockfd, &servaddr);

        socklen_t len;
        int n;

        len = sizeof(cliaddr);

        std::string msg;

        // Generate response message
        snprintf(send_buffer, sizeof(send_buffer), "ALV");

        while (true) {
            // receive incoming heartbeat requests
            n = recvfrom(sockfd, (char *)buffer, MAXLINE_HEARTBEAT, MSG_WAITALL,
                        (struct sockaddr *)&cliaddr, &len);
            buffer[n] = '\0';

            // answer with a heartbeat response
            sendto(sockfd, send_buffer, strlen(send_buffer), MSG_CONFIRM,
                (const struct sockaddr *)&cliaddr, len);

    #if _DEBUG
            std::cout << RED
                    << "Heartbeat request from: " << inet_ntoa(cliaddr.sin_addr)
                    << ":" << ntohs(cliaddr.sin_port) << RESET << std::endl;
    #endif
        }
    }



    void *start_heartbeat_receiver(void *arg) {
        int port = *(int *)arg + HEARTBEAT_PORT_DELTA;

    #if _DEBUG
        std::cout << RED << "Starting Heartbeat Receiver on port " << port << RESET
                << std::endl;
    #endif
        (void)heartbeat_receiver(port);
        return 0;
}


    bool heartbeat_test(const char *target_ip, int target_port, int timeout_ms, int tries){
        int sockfd;
        char buffer[MAXLINE_HEARTBEAT + 1];
        char send_buffer[MAXLINE_HEARTBEAT + 1];
        const char *discovery_message = "TST";
        struct sockaddr_in servaddr;

        // Creating socket file descriptor
        sockfd = create_udp_socket();

        // Add timeout
        set_timeout(sockfd, timeout_ms);

        memset(&servaddr, 0, sizeof(servaddr));

        // Filling server information
        servaddr = create_sockaddr(target_ip, target_port);

        int n;
        socklen_t len;
        std::string msg;
        int num_retries = 0;

        // copy to send buffer
        strncpy(send_buffer, discovery_message, MAXLINE_HEARTBEAT);

        while (num_retries < tries) {

            // send discovery message
            sendto(sockfd, send_buffer, strlen(send_buffer), MSG_CONFIRM,
                (const struct sockaddr *)&servaddr, sizeof(servaddr));

    #if _DEBUG
            if (num_retries == 0)
                std::cout << MAGENTA << "Sent Heartbeat test " << RESET << std::endl;
    #endif

            // espera receber mensagem
            n = recvfrom(sockfd, (char *)buffer, MAXLINE_HEARTBEAT, MSG_WAITALL,
                        (struct sockaddr *)&servaddr, &len);

            if (n < 0) {
                std::cout << MAGENTA << "Timeout or error receiving answer, retrying... ("
                        << num_retries + 1 << "/" << tries << ")"
                        << RESET << std::endl;
                num_retries++;
                continue; // timeout or error, attempt again
            }
            // message received
            buffer[n] = '\0';
            return true;
        }

        if (num_retries >= tries) {
            std::cout << MAGENTA << "Could not get a response, main server down" << RESET <<std::endl;
            close(sockfd);
            return false; // max retries reached
        }

        close(sockfd);
        return false;


    }


    void heartbeat_tester_loop(int target_port, int timeout_ms, int tries){

        auto RM = RedundancyManager::get_instance();
        const char *ip_str;
        while (true) {
            // wait before next test
            std::this_thread::sleep_for(std::chrono::seconds(2));
            if(RM->is_coordinator) continue; // skip if we are coordinator

            try {
                //if(RM->coordinator_ip == 0) continue; // skip if we don't know coordinator ip
                
                ip_str = inet_ntoa(*(in_addr *)&RM->coordinator_ip);

                std::cout << CYAN << ip_str << RESET << std::endl;
                

                bool alive = heartbeat_test( ip_str, target_port, timeout_ms, tries);
                if (!alive) {
                    std::cout << RED << "No heartbeat response from main server. Initiating election..." << RESET << std::endl;
                    // initiate election
                    RM->backup_election();
            }
            } catch (const std::exception &e) {
                std::cerr << RED << "Error in heartbeat tester loop: " << e.what() << RESET << std::endl;

            }
            

            
        }

        std::cout << RED << "Exiting heartbeat tester loop." << RESET << std::endl;
        return;
    }

    void *start_heartbeat_tester_loop(void *arg){
        int port = *(int *)arg + HEARTBEAT_PORT_DELTA;

    #if _DEBUG
        std::cout << RED << "Starting Heartbeat Tester Loop targeting port " << port << RESET
                << std::endl;
    #endif
        (void)heartbeat_tester_loop(port, 10, 3);
        return 0;
    }

} // namespace election