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

        void init();
        void stand_by();
        void single_socket_election();
        void backup_election();

        void promote();
        void demote();

        void enter_standby_mode();
        void exit_standby_mode();

        void start_heartbeat_tester_thread();
        void start_heartbeat_receiver_thread();
        

        bool is_coordinator;
        in_addr_t coordinator_ip;
        bool in_standby_mode;

        int port_n;

    private:
        RedundancyManager();
        int ss_election_result_switch();
        bool is_valid_answer(char *buffer, int n);
        bool is_valid_coord_announcement(char *buffer, int n);
        bool is_valid_election_message(char *buffer, int n);

        unsigned int id;
        bool election_in_progress;
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

        int state;
        unsigned int last_received_id;

        void start_election_procedure();
        void broadcast_election_message();
        void send_answer_back();
        void send_coordinator_announcement();
        void handle_election_message();
        void handle_answer_message();
        void handle_in_progress_timeout();
        void handle_coordinator_announcement();

        //bool possible_coordinator;
        long answer_max_wait_time;
        long current_wait_time;
        long start_time;
        long elapsed_time;


        // ELE, ANS and CRD messages (fixed as TYPE <id> END)
        char election_message[ELECTION_MAXLINE + 1];
        char answer_message[ELECTION_MAXLINE + 1];
        char coord_announcement_message[ELECTION_MAXLINE + 1];


        pthread_t heartbeat_thread, heartbeat_tester_thread;





    };

} // namespace election

#endif //REDUNDANCY_MANAGER_H