#include "redundancy_manager.h"
#include "colors.h"
#include "data_transfer/socket_utils.h"
#include "date_time_utils.h"
#include "ports.h"
#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <cstring>
#include <iostream>
#include <pthread.h>
#include <sstream>

namespace election {

RedundancyManager::RedundancyManager() {

    // ID is set based on IP
    std::string ip_str = get_self_ip();
    auto ip = inet_addr(ip_str.c_str());

    // convert to int
    id = ntohl(ip);
    std::cout << CYAN << "My ID is: " << id << RESET << std::endl;

    // precompute ELE, ANS and CRD messages
    snprintf(election_message, sizeof(election_message), "ELE %u END", id);
    snprintf(answer_message, sizeof(answer_message), "ANS %u END", id);
    snprintf(coord_announcement_message, sizeof(coord_announcement_message), "CRD %u END", id);


    is_coordinator = false;
    election_in_progress = false;

    single_socket_election();


    }

RedundancyManager::~RedundancyManager() {
#if _DEBUG
    std::cout << RED << "Destroying redundancy manager instance!" << RESET
              << std::endl;
#endif
}

RedundancyManager *RedundancyManager::get_instance() {
    static RedundancyManager *instance = new RedundancyManager();
    return instance;
}

    /*void* RedundancyManager::start_election(void *arg){

#if _DEBUG
    std::cout << BLUE << "Starting election " << RESET << std::endl;
#endif

    RedundancyManager::get_instance()->election();
    return 0;
}

void RedundancyManager::election() {

    char send_buffer[ELECTION_MAXLINE + 1];

    std::cout << YELLOW << "Starting election..." << RESET << std::endl;
    election_in_progress = true;

    // len = sizeof(cliaddr);
    std::string msg;

    // format: "ELE <ID> END"
    snprintf(send_buffer, sizeof(send_buffer), "ELE %u END", id);

    // Broadcast election message
    sendto(election_messages_socket, send_buffer, strlen(send_buffer),
           MSG_CONFIRM, (const struct sockaddr *)&messages_servaddr,
           sizeof(messages_servaddr));
    std::cout << YELLOW << "Election message broadcasted." << RESET
              << std::endl;
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

    last_received_id = 0;

    // 3 possible scenarios:
    // 1 -> received answer, not coordinator
    // 2 -> received coordinaor announcement, end election, not coordinator
    // 3 -> no answers received, no coordinator announcement, become coordinator
    while (elapsed_time < answer_wait_time) {
        // wait for responses
        n = recvfrom(election_responses_socket, (char *)buffer,
                     ELECTION_MAXLINE, MSG_WAITALL,
                     (struct sockaddr *)&responses_cliaddr, &len);
        buffer[n] = '\0';

        // ignore loopback (127.0.0.0/8)
        uint32_t src_h = ntohl(responses_cliaddr.sin_addr.s_addr);
        if ((src_h & 0xff000000) == 0x7f000000) {
            // received from loopback, ignore
            continue;
        }

        if (is_valid_coord_announcement(buffer, n)) {
            std::cout << YELLOW << "Coordinator announcement received from "
                      << inet_ntoa(responses_cliaddr.sin_addr) << RESET
                      << std::endl;
            is_coordinator = false;
            election_in_progress = false;
            return;
        }

        if (is_valid_answer(buffer, n)) {
            std::cout << YELLOW << "Answer received from "
                      << inet_ntoa(responses_cliaddr.sin_addr) << RESET
                      << std::endl;
            is_coordinator = false;
            election_in_progress = false;
        }

        elapsed_time = get_current_time_ms() - start_time;
    }

    /// if election has ended but no coordinator announcement received, wait for
    /// coordinator announcement
    if (election_in_progress) {
        while (elapsed_time < answer_wait_time * 5) {
            // wait for announcen]ment
            n = recvfrom(election_responses_socket, (char *)buffer,
                         ELECTION_MAXLINE, MSG_WAITALL,
                         (struct sockaddr *)&responses_cliaddr, &len);
            buffer[n] = '\0';

            // ignore loopback (127.0.0.0/8)
            uint32_t src_h = ntohl(responses_cliaddr.sin_addr.s_addr);
            if ((src_h & 0xff000000) == 0x7f000000) {
                // received from loopback, ignore
                continue;
            }

            if (is_valid_coord_announcement(buffer, n)) {
                std::cout << YELLOW << "Coordinator announcement received from "
                          << inet_ntoa(responses_cliaddr.sin_addr) << RESET
                          << std::endl;
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
    }*/

void RedundancyManager::stand_by() { return; }

    

    /*
    void* RedundancyManager::start_election_waiting_server(void *arg) {
        //delete (int*)arg; // free heap memory passed to the thread

    RedundancyManager::get_instance()->await_election();
    return 0;
}

void RedundancyManager::await_election() {

    char buffer[ELECTION_MAXLINE + 1];
    char send_buffer[ELECTION_MAXLINE + 1];

#if _DEBUG
    std::cout << CYAN << "Starting waiting election messages on port "
              << election_message_port << RESET << std::endl;
#endif

    socklen_t len;
    int n;
    len = sizeof(messages_cliaddr);
    std::string msg;

    // Generate response message
    snprintf(send_buffer, sizeof(send_buffer), "ANS %u END", id);

    while (true) {
        // receive incoming election messages
        n = recvfrom(election_messages_socket, (char *)buffer, ELECTION_MAXLINE,
                     MSG_WAITALL, (struct sockaddr *)&messages_cliaddr, &len);
        buffer[n] = '\0';

        // ignore loopback (127.0.0.0/8)
        uint32_t src_h = ntohl(responses_cliaddr.sin_addr.s_addr);
        if ((src_h & 0xff000000) == 0x7f000000) {
            // received from loopback, ignore
            continue;
        }

#if _DEBUG
        std::cout << CYAN << "Election message received from "
                  << inet_ntoa(messages_cliaddr.sin_addr) << RESET << std::endl;
#endif

        // answer with a response
        sendto(election_responses_socket, send_buffer, strlen(send_buffer),
               MSG_CONFIRM, (const struct sockaddr *)&responses_cliaddr, len);

        // start election, if not already in progress
        if (!election_in_progress) {
            election_in_progress = true;

#if _DEBUG
            std::cout << CYAN << "Starting election " << RESET << std::endl;
#endif

            // Send election broacast
            // format: "ELE <ID> END"
            snprintf(send_buffer, sizeof(send_buffer), "ELE %u END", id);

            start_election_thread();

            continue;
        }
    }
}

    void RedundancyManager::start_election_thread(){
        pthread_t election_thread;
        pthread_create(&election_thread, NULL, &RedundancyManager::start_election, (void *)NULL);
        pthread_detach(election_thread);
    }   
    
    */
}// namespace election



