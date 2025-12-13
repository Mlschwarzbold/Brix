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
#include "heartbeat.h"

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

    void RedundancyManager::stand_by() { 
        return; 
    }

    void RedundancyManager::init() {
        is_coordinator = false;
        in_standby_mode = false;

        port_n = 4000; // default requests port
        // create heartbeat receiver thread
        pthread_create(&heartbeat_thread, NULL, election::start_heartbeat_receiver, (void *)&port_n);
        pthread_detach(heartbeat_thread);

        // create heartbeat tester thread
        pthread_create(&heartbeat_tester_thread, NULL, election::start_heartbeat_tester_loop, (void *)&port_n);
        pthread_detach(heartbeat_tester_thread);    

        

        single_socket_election();
    }

    void RedundancyManager::promote() {
        is_coordinator = true;
        // Create heartbeat receiver thread
        //pthread_t heartbeat_thread;
        //pthread_create(&heartbeat_thread, NULL, election::start_heartbeat_receiver, (void *) NULL);
        //pthread_detach(heartbeat_thread);

    }

    void RedundancyManager::demote() {
        is_coordinator = false;
    }

    void RedundancyManager::enter_standby_mode() {
        in_standby_mode = true;
    }

    void RedundancyManager::exit_standby_mode() {
        in_standby_mode = false;
    }


    
}// namespace election



