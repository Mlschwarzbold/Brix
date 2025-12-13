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

typedef enum {
    NONE,
    ELECTION,
    ANSWER,
    COORD_ANNOUNCEMENT,
    TIMEOUT
} ElectionSwitchResult;

typedef enum {
    INIT,
    NOT_IN_PROGRESS,
    IN_PROGRESS,
    WAITING_FOR_COORD
} ElectionState;

namespace election{

    void RedundancyManager::single_socket_election() {
        // Implementation of single socket election logic goes here
        std::cout << BOLD << "Starting single socket election on port "<< 4000 + SS_PORT_DELTA << RESET << std::endl;


        // socket creation and setup
        ss_sockfd = create_udp_socket();
        enable_broadcast(ss_sockfd);
        set_timeout(ss_sockfd, 900);
        memset(&ss_cliaddr, 0, sizeof(ss_cliaddr));
        ss_servaddr = create_sockaddr("0.0.0.0", 4000 + SS_PORT_DELTA); // use messages port for single socket
        bind_to_sockaddr(ss_sockfd, &ss_servaddr);
        ss_len = sizeof(ss_cliaddr);


        broadcastcast_addr = ss_servaddr;               // reuse bound port/interface
        broadcastcast_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);     // 255.255.255.255 (or use subnet broadcast)
        broadcastcast_len = sizeof(broadcastcast_addr);

        state = INIT;
        //int response_wait_time_ms = 0;

        
        current_wait_time = 0;
        start_time = get_current_time_ms();
        answer_max_wait_time = 800; // time to wait for coordinator announcement
        std::cout << "starting Finite State machine" << std::endl;
        while(true){
            // clear buffer
            memset(ss_buffer, 0, sizeof(ss_buffer));
            // read from socket
            ss_n = recvfrom(ss_sockfd, (char *)ss_buffer, ELECTION_MAXLINE, MSG_WAITALL,
                            (struct sockaddr *)&ss_cliaddr, &ss_len);
                ss_buffer[ss_n] = '\0';

            if(ss_n > 0) std::cout << BOLD << "Message received: " << ss_buffer << " from " << inet_ntoa(ss_cliaddr.sin_addr) << " port " << ntohs(ss_cliaddr.sin_port) << RESET << std::endl;
            
            // ignore loopback (127.0.0.0/8)
            uint32_t src_h = ntohl(ss_cliaddr.sin_addr.s_addr);
            if ((src_h & 0xff000000) == 0x7f000000) {
                // received from loopback, ignore
                continue;
            }

            // ================ Election Finite State Machine =================


            

            switch(state){
                case INIT:
                    state = IN_PROGRESS;
                    is_coordinator = false;
                    // Send election message with own id
                    broadcast_election_message();
                    break;
                // --------- NOT IN PROGRESS ---------
                case NOT_IN_PROGRESS:
                    current_wait_time = 0;
                    std::cout << "NOT IN PROGRESS STATE" << std::endl;
                    switch (ss_election_result_switch()) {
                        case COORD_ANNOUNCEMENT:
                            handle_coordinator_announcement();
                            break;
                        case ANSWER:
                            break;
                        case ELECTION:
                            // start election process
                            std::cout << GREEN << "ELECTION MESSAGE RECEIVED" << RESET << std::endl;
                            if(last_received_id < id) start_election_procedure(); //if we have higher id, start election
                            else state = WAITING_FOR_COORD; // if we have lower id, do nothing and wait for coordinator announcement  

                            break;
                        case TIMEOUT:   
                            // remain in NOT IN PROGRESS, just wait
                            break;
                        default:
                            std::cout << YELLOW << "INVALID MESSAGE RECEIVED - " << ss_buffer << RESET << std::endl;
                            break;
                        }
                    break;
                // --------- ELECTION IN PROGRESS ---------
                case IN_PROGRESS:
                    std::cout << "IN PROGRESS STATE" << std::endl;
                    switch (ss_election_result_switch()) {
                        case COORD_ANNOUNCEMENT:
                            handle_coordinator_announcement();
                            break;
                        case ANSWER:
                            break;
                        case ELECTION:
                            handle_election_message();
                            break;
                        case TIMEOUT:
                            handle_in_progress_timeout();
                            break;
                        }
                    break;
                case WAITING_FOR_COORD:
                current_wait_time = 0;
                std::cout << "WAITING FOR COORD" << std::endl;
                    switch (ss_election_result_switch()) {
                        case COORD_ANNOUNCEMENT:
                            handle_coordinator_announcement();
                            break;
                        case ANSWER:
                            handle_answer_message();
                            break;
                        case ELECTION:
                            break;
                        case TIMEOUT:
                            break;
                        }
                    break;
            }




            current_wait_time = get_current_time_ms() - start_time;
        }
    }

    int RedundancyManager::ss_election_result_switch() {  
 
            bool socket_timeout = true;

            if (is_valid_coord_announcement(ss_buffer, ss_n)) {
                std::cout << BOLD << "Coordinator announcement received from " << inet_ntoa(ss_cliaddr.sin_addr) << RESET << std::endl;
                socket_timeout = false;
                return COORD_ANNOUNCEMENT;
            }

            if (is_valid_answer(ss_buffer, ss_n)) {
                std::cout << BOLD << "Answer received from " << inet_ntoa(ss_cliaddr.sin_addr) << RESET << std::endl;
                socket_timeout = false;
                return ANSWER;
            }

            if(is_valid_election_message(ss_buffer, ss_n)){
                std::cout << BOLD << "Election message received from " << inet_ntoa(ss_cliaddr.sin_addr) << RESET << std::endl;
                socket_timeout = false;
                return ELECTION;
            }

            if(socket_timeout && ss_n < 0){
                return TIMEOUT;
            }
        return NONE;
    }


    void RedundancyManager::start_election_procedure(){

        state = IN_PROGRESS;
        is_coordinator = false;
        // Answer election messages with own id
        send_answer_back(); 

        // Send election message with own id
        broadcast_election_message();
        return;
    }

    void RedundancyManager::handle_election_message(){
        if (last_received_id < id) {
            send_answer_back();
        } else {
            state = WAITING_FOR_COORD;
        }

    }

    void RedundancyManager:: handle_answer_message(){
        if (last_received_id < id) {
            // nao deveria acontecer, ignora
        } else {
            state = WAITING_FOR_COORD;
        }
    }

    void RedundancyManager::handle_coordinator_announcement(){
        is_coordinator = false;
        state = NOT_IN_PROGRESS;
        return;
    }

    void RedundancyManager::handle_in_progress_timeout(){
        if(current_wait_time >= answer_max_wait_time){
            // no answers or coordinator announcement received, become coordinator
            std::cout << YELLOW << "I am the new coordinator!" << RESET << std::endl;
            is_coordinator = true;
            state = NOT_IN_PROGRESS;
        } // else just wait
    }

    
    void RedundancyManager::send_answer_back(){

        sendto(ss_sockfd, answer_message, strlen(answer_message), MSG_CONFIRM,
                (const struct sockaddr *)&ss_cliaddr, ss_len);

        std::cout << BOLD << "Answer message sent to " << inet_ntoa(ss_cliaddr.sin_addr) << RESET << std::endl;

    }


    void RedundancyManager::broadcast_election_message(){

        // broadcast election message

        sendto(ss_sockfd, election_message, strlen(election_message), MSG_CONFIRM,
                (const struct sockaddr *)&broadcastcast_addr, broadcastcast_len);
        std::cout << BOLD << "Election message broadcasted." << RESET << std::endl;

    }


} // namespace election