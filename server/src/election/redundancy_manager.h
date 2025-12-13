#ifndef REDUNDANCY_MANAGER_H
#define REDUNDANCY_MANAGER_H

#include <iostream>
#include <pthread.h>
#include <arpa/inet.h>

#define ELECTION_MAXLINE 128

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

        void start_election_thread();

        void single_socket_election();
        int ss_election_result_switch();

    private:
        RedundancyManager();
        void election_result_switch();
        bool is_valid_answer(char *buffer, int n);
        bool is_valid_coord_announcement(char *buffer, int n);
        bool is_valid_election_message(char *buffer, int n);

        unsigned int id;
        bool election_in_progress;
        bool is_coordinator;
        int election_message_port, election_response_port;
        int election_messages_socket, election_responses_socket;
        struct sockaddr_in messages_servaddr, responses_servaddr, messages_cliaddr, responses_cliaddr;

        int ss_n;
        int ss_sockfd;
        char ss_buffer[ELECTION_MAXLINE + 1];
        char ss_send_buffer[ELECTION_MAXLINE + 1];
        struct sockaddr_in ss_servaddr, ss_cliaddr;
        socklen_t ss_len;

        struct sockaddr_in broadcastcast_addr;
        socklen_t broadcastcast_len;

        void start_election_procedure();
        void broadcast_election_message();
        void send_answer_back();

        // ELE, ANS and CRD messages (fixed as TYPE <id> END)
        char election_message[ELECTION_MAXLINE + 1];
        char answer_message[ELECTION_MAXLINE + 1];
        char coord_announcement_message[ELECTION_MAXLINE + 1];





    };

} // namespace election

#endif //REDUNDANCY_MANAGER_H