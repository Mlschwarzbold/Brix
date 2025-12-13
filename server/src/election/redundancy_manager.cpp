#include "redundancy_manager.h"
#include "colors.h"
#include <iostream>
#include <pthread.h>
#include <arpa/inet.h>
#include <cstring>
#include "data_transfer/socket_utils.h"
#include <bits/stdc++.h>
#include "date_time_utils.h"
#include "ports.h"

#define ELECTION_MAXLINE 128

namespace election {


    RedundancyManager::RedundancyManager() {

    // ID is set based on IP
    std::string ip_str = get_self_ip();
    auto ip = inet_addr(ip_str.c_str());

    // convert to int
    id = ntohl(ip);
    std::cout << GREEN << "My ID is: " << id << RESET << std::endl;

    // Create election sockets
    election_message_port = 4000 + ELECTION_MESSAGES_PORT_DELTA;
    election_response_port = 4000 + ELECTION_RESPONSES_PORT_DELTA;
    election_messages_socket = create_udp_socket();
    election_responses_socket = create_udp_socket();
        
    // Enable broadcast
    enable_broadcast(election_messages_socket);
    //enable_broadcast(election_responses_socket);

    // Add timeout
    //set_timeout(election_messages_socket, 10);
    set_timeout(election_responses_socket, 10);

    memset(&responses_servaddr, 0, sizeof(responses_servaddr));
    memset(&messages_servaddr, 0, sizeof(messages_servaddr));

    memset(&messages_cliaddr, 0, sizeof(messages_cliaddr));
    memset(&responses_cliaddr, 0, sizeof(responses_cliaddr));

     // Filling server information
    responses_servaddr = create_sockaddr("0.0.0.0", election_response_port);
    messages_servaddr = create_sockaddr("0.0.0.0", election_message_port);

    // Bind the socket with the server address
    bind_to_sockaddr(election_messages_socket, &messages_servaddr);
    bind_to_sockaddr(election_responses_socket, &responses_servaddr);




    is_coordinator = false;
    election_in_progress = false;


    pthread_t election_listener_thread, election_thread;

    // start listening for election messages
    int *port_ptr = new int(4000);
    pthread_create(&election_listener_thread, NULL, &RedundancyManager::start_election_waiting_server, (void *)port_ptr);
    pthread_detach(election_listener_thread);


    // start election
    int *election_port_ptr = new int(4000);
    pthread_create(&election_thread, NULL, &RedundancyManager::start_election, (void *)election_port_ptr);
    pthread_detach(election_thread);

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

    void* RedundancyManager::start_election(void *arg){
        int port = *(int *)arg;
        delete (int*)arg; // free heap memory passed to the thread

    #if _DEBUG
        std::cout << BLUE << "Starting election on port " << port + ELECTION_MESSAGES_PORT_DELTA << RESET
                << std::endl;
    #endif

        RedundancyManager::get_instance()->election();
        return 0;
    }

    void RedundancyManager::election() {

        char send_buffer[ELECTION_MAXLINE + 1];

        
        std::cout << YELLOW << "Starting election..." << RESET << std::endl;
        election_in_progress = true;

        //len = sizeof(cliaddr);
        std::string msg;

        // format: "ELE <ID> END"
        snprintf(send_buffer, sizeof(send_buffer), "ELE %u END", id);


        // Broadcast election message
        sendto(election_messages_socket, send_buffer, strlen(send_buffer), MSG_CONFIRM,
            (const struct sockaddr *)&messages_servaddr, sizeof(messages_servaddr));
        std::cout << YELLOW << "Election message broadcasted." << RESET << std::endl;
        // Wait for answers or coordinator announcement

        election_result_switch();
        
        return;
    }

    void RedundancyManager::election_result_switch() {  
        char buffer[ELECTION_MAXLINE + 1];
        char send_buffer[ELECTION_MAXLINE + 1];

        long elapsed_time = 0;
        long start_time = get_current_time_ms();
        long answer_wait_time = 500; // time to wait for coordinator announcement


        socklen_t len;
        len = sizeof(messages_cliaddr);
        std::string msg;
        int n;

        // 3 possible scenarios:
        // 1 -> received answer, not coordinator
        // 2 -> received coordinaor announcement, end election, not coordinator
        // 3 -> no answers received, no coordinator announcement, become coordinator
        while (elapsed_time < answer_wait_time) {
             // wait for responses
            n = recvfrom(election_responses_socket, (char *)buffer, ELECTION_MAXLINE, MSG_WAITALL,
                        (struct sockaddr *)&responses_cliaddr, &len);
            buffer[n] = '\0';

            if (is_valid_coord_announcement(buffer, n)) {
                std::cout << YELLOW << "Coordinator announcement received from " << inet_ntoa(responses_cliaddr.sin_addr) << RESET << std::endl;
                is_coordinator = false;
                election_in_progress = false;
                return;
            }

            if (is_valid_answer(buffer, n)) {
                std::cout << YELLOW << "Answer received from " << inet_ntoa(responses_cliaddr.sin_addr) << RESET << std::endl;
                is_coordinator = false;
                election_in_progress = false;
            }


            elapsed_time = get_current_time_ms() - start_time;
        }

        /// if election has ended but no coordinator announcement received, wait for coordinator announcement
        if (election_in_progress) {
            while (elapsed_time < answer_wait_time * 5) {
                // wait for announcen]ment
                n = recvfrom(election_responses_socket, (char *)buffer, ELECTION_MAXLINE, MSG_WAITALL,
                            (struct sockaddr *)&responses_cliaddr, &len);
                buffer[n] = '\0';

                if (is_valid_coord_announcement(buffer, n)) {
                    std::cout << YELLOW << "Coordinator announcement received from " << inet_ntoa(responses_cliaddr.sin_addr) << RESET << std::endl;
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
        sendto(election_responses_socket, send_buffer, strlen(send_buffer), MSG_CONFIRM,
            (const struct sockaddr *)&responses_cliaddr, len);

        
        
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

    void* RedundancyManager::start_election_waiting_server(void *arg) {
        delete (int*)arg; // free heap memory passed to the thread

        RedundancyManager::get_instance()->await_election();
        return 0;
    }

    void RedundancyManager::await_election() {

        char buffer[ELECTION_MAXLINE + 1];
        char send_buffer[ELECTION_MAXLINE + 1];

    #if _DEBUG
        std::cout << BLUE << "Starting waiting election messages on port " << election_message_port << RESET
                << std::endl;
    #endif


        socklen_t len;
        int n;
        len = sizeof(messages_cliaddr);
        std::string msg;

        // Generate response message
        snprintf(send_buffer, sizeof(send_buffer), "ANS %u END", id);

        while (true) {
            // receive incoming election messages
            n = recvfrom(election_messages_socket, (char *)buffer, ELECTION_MAXLINE, MSG_WAITALL,
                        (struct sockaddr *)&messages_cliaddr, &len);
            buffer[n] = '\0';

            std::cout << YELLOW << "Election message received from " << inet_ntoa(messages_cliaddr.sin_addr) << RESET << std::endl;
            // answer with a response
            sendto(election_responses_socket, send_buffer, strlen(send_buffer), MSG_CONFIRM,
                (const struct sockaddr *)&messages_cliaddr, len);

            // start election, if not already in progress
            if (!election_in_progress) {
                //start_election();
                continue;
            }

    #if _DEBUG
            std::cout << GREEN
                    << "Election message from: " << inet_ntoa(messages_cliaddr.sin_addr)
                    << ":" << ntohs(messages_cliaddr.sin_port) << RESET << std::endl;
    #endif
        }

    }
    

}// namespace election



