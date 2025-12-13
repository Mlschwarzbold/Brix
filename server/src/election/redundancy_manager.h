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
        void start_election();
        void await_election(int port);
        void stand_by();

    private:
        RedundancyManager();
        unsigned int id;
        bool election_in_progress;
        bool is_coordinator;
        void election_result_switch(int sockfd, struct sockaddr_in cliaddr, socklen_t len, char *buffer, char *send_buffer);
        bool is_valid_answer(char *buffer, int n);
        bool is_valid_coord_announcement(char *buffer, int n);



    };

} // namespace election

#endif //REDUNDANCY_MANAGER_H