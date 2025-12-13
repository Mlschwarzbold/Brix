#ifndef REDUNDANCY_MANAGER_H
#define REDUNDANCY_MANAGER_H

#include <iostream>
#include <pthread.h>
#include <arpa/inet.h>

namespace election {

    class RedundancyManager {
    public:
        ~RedundancyManager();
        static RedundancyManager *get_instance();
        static void* start_election(void *arg);
        void election();
        static void* start_election_waiting_server(void *arg);
        void await_election();
        void stand_by();

    private:
        RedundancyManager();
        unsigned int id;
        bool election_in_progress;
        bool is_coordinator;
        void election_result_switch();
        bool is_valid_answer(char *buffer, int n);
        bool is_valid_coord_announcement(char *buffer, int n);
        int election_message_port, election_response_port;
        int election_messages_socket, election_responses_socket;
        struct sockaddr_in messages_servaddr, responses_servaddr, messages_cliaddr, responses_cliaddr;




    };

} // namespace election

#endif //REDUNDANCY_MANAGER_H