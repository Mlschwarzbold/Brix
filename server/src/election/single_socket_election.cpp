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
    NOT_IN_PROGRESS,
    IN_PROGRESS
} ElectionState;

namespace election{

    void RedundancyManager::single_socket_election() {
        // Implementation of single socket election logic goes here
        std::cout << BOLD << "Starting single socket election..." << RESET << std::endl;


        // socket creation and setup
        ss_sockfd = create_udp_socket();
        enable_broadcast(ss_sockfd);
        set_timeout(ss_sockfd, 10);
        memset(&ss_cliaddr, 0, sizeof(ss_cliaddr));
        ss_servaddr = create_sockaddr("0.0.0.0", 4000 + SS_PORT_DELTA); // use messages port for single socket
        bind_to_sockaddr(ss_sockfd, &ss_servaddr);

        int state = NOT_IN_PROGRESS;
        //int response_wait_time_ms = 0;

        
        //long elapsed_time = 0;
        //long start_time = get_current_time_ms();
        //long answer_wait_time = 500; // time to wait for coordinator announcement
        while(true){
            
            // read from socket
            ss_n = recvfrom(ss_sockfd, (char *)ss_buffer, ELECTION_MAXLINE, MSG_WAITALL,
                            (struct sockaddr *)&ss_cliaddr, &ss_len);
                ss_buffer[ss_n] = '\0';
            
            // ignore loopback (127.0.0.0/8)
            uint32_t src_h = ntohl(ss_cliaddr.sin_addr.s_addr);
            if ((src_h & 0xff000000) == 0x7f000000) {
                // received from loopback, ignore
                continue;
            }

            // ================ Election Finite State Machine =================


            switch(state){
                // --------- NOT IN PROGRESS ---------
                case NOT_IN_PROGRESS:
                    switch (ss_election_result_switch()) {
                        case COORD_ANNOUNCEMENT:
                            break;
                        case ANSWER:
                            break;
                        case ELECTION:
                            break;
                        case TIMEOUT:
                            break;
                        }
                    break;
                // --------- ELECTION IN PROGRESS ---------
                case IN_PROGRESS:
                    switch (ss_election_result_switch()) {
                        case COORD_ANNOUNCEMENT:
                            break;
                        case ANSWER:
                            break;
                        case ELECTION:
                            break;
                        case TIMEOUT:
                            break;
                        }
                    break;
            }




            //elapsed_time = get_current_time_ms() - start_time;
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

            if(socket_timeout){
                return TIMEOUT;
            }
        return NONE;
    }


} // namespace election