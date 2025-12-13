#include "redundancy_manager.h"
#include "colors.h"
#include <iostream>
#include <pthread.h>
#include <arpa/inet.h>
#include <cstring>
#include "data_transfer/socket_utils.h"
#include <bits/stdc++.h>
#include "date_time_utils.h"


#define ELECTION_MAXLINE 128

namespace election {


    RedundancyManager::RedundancyManager() {
    #if _DEBUG
        std::cout << GREEN << "Starting redundancy manager instance!" << RESET << std::endl;
    #endif

    // ID is set based on IP
    std::string ip_str = get_self_ip();
    auto ip = inet_addr(ip_str.c_str());

    // convert to int
    id = ntohl(ip);
    std::cout << GREEN << "My ID is: " << id << RESET << std::endl;


    is_coordinator = false;
    election_in_progress = false;

    start_election();

    }

    RedundancyManager::~RedundancyManager() {
    #if _DEBUG
        std::cout << RED << "Destroying redundancy manager instance!" << RESET << std::endl;
    #endif
    } 


    RedundancyManager *RedundancyManager::get_instance() {
        static RedundancyManager *instance = new RedundancyManager();
        return instance;
    }

    void RedundancyManager::start_election() {

        int sockfd;
        int election_broadcast_port = 5000; // Example port for election messages
        int answer_timeout_ms = 100; // time to wait for answers
        char buffer[ELECTION_MAXLINE + 1];
        char send_buffer[ELECTION_MAXLINE + 1];
        struct sockaddr_in servaddr, cliaddr;
        socklen_t len;
        
        std::cout << YELLOW << "Starting election..." << RESET << std::endl;
        election_in_progress = true;

        // Broadcast election message

        // Creating socket file descriptor
        sockfd = create_udp_socket();

        // Enable broadcast
        enable_broadcast(sockfd);

        // Add timeout
        set_timeout(sockfd, answer_timeout_ms);

        // memset(&servaddr, 0, sizeof(servaddr));
        memset(&cliaddr, 0, sizeof(cliaddr));

        // Filling server information
        servaddr = create_sockaddr("0.0.0.0", election_broadcast_port);

        // Bind the socket with the server address
        bind_to_sockaddr(sockfd, &servaddr);

        len = sizeof(cliaddr);
        std::string msg;

        // format: "ELE <ID> END"
        snprintf(send_buffer, sizeof(send_buffer), "ELE %u END", id);



        // Broadcast election message
        sendto(sockfd, send_buffer, strlen(send_buffer), MSG_CONFIRM,
            (const struct sockaddr *)&servaddr, sizeof(servaddr));
        std::cout << YELLOW << "Election message broadcasted." << RESET << std::endl;
        // Wait for answers or coordinator announcement

        election_result_switch(sockfd, cliaddr, len, buffer, send_buffer);

        close(sockfd);
        
        return;
    }

    void RedundancyManager::election_result_switch(int sockfd, struct sockaddr_in cliaddr, socklen_t len, char *buffer, char *send_buffer) {
        int n;
        long elapsed_time = 0;
        long start_time = get_current_time_ms();
        long answer_wait_time = 500; // time to wait for coordinator announcement

        // 3 possible scenarios:
        // 1 -> received answer, not coordinator
        // 2 -> received coordinaor announcement, end election, not coordinator
        // 3 -> no answers received, no coordinator announcement, become coordinator
        while (elapsed_time < answer_wait_time) {
             // wait for responses
            n = recvfrom(sockfd, (char *)buffer, ELECTION_MAXLINE, MSG_WAITALL,
                        (struct sockaddr *)&cliaddr, &len);
            buffer[n] = '\0';

            if (is_valid_coord_announcement(buffer, n)) {
                std::cout << YELLOW << "Coordinator announcement received from " << inet_ntoa(cliaddr.sin_addr) << RESET << std::endl;
                is_coordinator = false;
                election_in_progress = false;
                return;
            }

            if (is_valid_answer(buffer, n)) {
                std::cout << YELLOW << "Answer received from " << inet_ntoa(cliaddr.sin_addr) << RESET << std::endl;
                is_coordinator = false;
                election_in_progress = false;
            }


            elapsed_time = get_current_time_ms() - start_time;
        }

        /// if election has ended but no coordinator announcement received, wait for coordinator announcement
        if (election_in_progress) {
            while (elapsed_time < answer_wait_time * 5) {
                // wait for announcen]ment
                n = recvfrom(sockfd, (char *)buffer, ELECTION_MAXLINE, MSG_WAITALL,
                            (struct sockaddr *)&cliaddr, &len);
                buffer[n] = '\0';

                if (is_valid_coord_announcement(buffer, n)) {
                    std::cout << YELLOW << "Coordinator announcement received from " << inet_ntoa(cliaddr.sin_addr) << RESET << std::endl;
                    is_coordinator = false;
                    election_in_progress = false;
                    return;
                }

                elapsed_time = get_current_time_ms() - start_time;
            }
        }

        // If we got here, no answers or announcements were received, we become coordinator
        std::cout << YELLOW << "I am the new coordinator!" << RESET << std::endl;
        is_coordinator = true;
        election_in_progress = false;
        
         // Broadcast coordinator announcement
        snprintf(send_buffer, ELECTION_MAXLINE + 1, "CRD %u END", id);
        sendto(sockfd, send_buffer, strlen(send_buffer), MSG_CONFIRM,
            (const struct sockaddr *)&cliaddr, len);

        
        
        return;
    }

    void RedundancyManager::stand_by() {

        return;
    }


    bool RedundancyManager::is_valid_answer(char *buffer, int n) {
        unsigned int responder_id;
        if (n <= 0) {
            return false;
        }
        std::string response(buffer);
        if (response.find("ANS") == 0) {
            //check higher id
            sscanf(buffer, "ANS %u END", &responder_id);
            if (responder_id > id) {
                return true;
            }
        }
        return false;
    }

    bool RedundancyManager::is_valid_coord_announcement(char *buffer, int n) {
        unsigned int coordinator_id;
        if (n <= 0) {
            return false;
        }
        std::string response(buffer);
        if (response.find("CRD") == 0) {
            //check higher id
            sscanf(buffer, "CRD %u END", &coordinator_id);
            if (coordinator_id > id) {
                return true;
            }
        }
        return false;
    }


    void RedundancyManager::await_election(int port) {
        int sockfd;
        char buffer[ELECTION_MAXLINE + 1];
        char send_buffer[ELECTION_MAXLINE + 1];
        struct sockaddr_in servaddr, cliaddr;

        // Creating socket file descriptor
        sockfd = create_udp_socket();

        // memset(&servaddr, 0, sizeof(servaddr));
        memset(&cliaddr, 0, sizeof(cliaddr));

        // Filling server information
        servaddr = create_sockaddr("0.0.0.0", port);

        //enable broadcast
        enable_broadcast(sockfd);

        // Bind the socket with the server address
        bind_to_sockaddr(sockfd, &servaddr);

        socklen_t len;
        int n;

        len = sizeof(cliaddr);

        std::string msg;

        // Generate response message
        snprintf(send_buffer, sizeof(send_buffer), "ANS %u END", id);

        while (true) {
            // receive incoming election messages
            n = recvfrom(sockfd, (char *)buffer, ELECTION_MAXLINE, MSG_WAITALL,
                        (struct sockaddr *)&cliaddr, &len);
            buffer[n] = '\0';

            // answer with a response
            sendto(sockfd, send_buffer, strlen(send_buffer), MSG_CONFIRM,
                (const struct sockaddr *)&cliaddr, len);

            // start election, if not already in progress
            if (!election_in_progress) {
                start_election();
            }

    #if _DEBUG
            std::cout << GREEN
                    << "Election message from: " << inet_ntoa(cliaddr.sin_addr)
                    << ":" << ntohs(cliaddr.sin_port) << RESET << std::endl;
    #endif
        }

    }
    

}// namespace election



