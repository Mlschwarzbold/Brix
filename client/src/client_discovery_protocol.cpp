#include "client_discovery_protocol.h"
#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAXLINE 1024

// Client Discovery Protocol
// This function initiates the discovery protocol, sending a broadcasted UDP
// datagram with the DIS message type then it waits for a response from the
// server or a timeout if the timer runs out / message is invalid, try it again
// if a valid response is received, extract the server IP and port and return
// them if the maximum number of retries is reached, return an error return 0 on
// success, -1 on error

int client_discovery_protocol(char *return_server_ip, int *return_server_port,
                              char *broadcast_ip, int broadcast_port,
                              int max_retries, int initial_timeout_ms) {
    int sockfd;
    char buffer[MAXLINE];
    char send_buffer[MAXLINE];
    const char *discovery_message = "DIS";
    struct sockaddr_in servaddr;
    int address_is_valid;

    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    int broadcastEnable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable,
                   sizeof(broadcastEnable)) < 0) {
        perror("setsockopt (SO_BROADCAST) failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // timeout
    // weird struct takes seconds and microseconds.
    // microseconds cant be greater than 1 million
    // so we gotta do weird math to convert milliseconds to both seconds and
    // microseconds
    struct timeval tv;
    tv.tv_sec = initial_timeout_ms / 1000;
    tv.tv_usec = (initial_timeout_ms % 1000) * 1000; // microseconds
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("setsockopt (SO_RCVTIMEO) failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(broadcast_port);
    servaddr.sin_addr.s_addr = inet_addr(broadcast_ip); // broadcast address

    int n;
    socklen_t len;
    std::string msg;
    int num_retries = 0;

    // copy to send buffer
    strncpy(send_buffer, discovery_message, MAXLINE);

    while (num_retries < max_retries) {

        // send discovery message
        sendto(sockfd, send_buffer, strlen(send_buffer), MSG_CONFIRM,
               (const struct sockaddr *)&servaddr, sizeof(servaddr));
        if (num_retries == 0)
            std::cout << "Sent Discovery Broadcast " << std::endl;

        // espera receber mensagem
        n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL,
                     (struct sockaddr *)&servaddr, &len);
        // std::cout<<" lenght: " <<n<<std::endl;

        if (n < 0) {
            std::cerr << "Timeout or error receiving response, retrying... ("
                      << num_retries + 1 << "/" << max_retries << ")"
                      << std::endl;
            num_retries++;
            continue; // timeout or error, attempt again
        }
        // message received
        buffer[n] = '\0';

        // Validate and extract IP and Port from response message
        // puts values direcly into return_server_ip and return_server_port
        address_is_valid = extract_ip_and_port_from_response(
            buffer, return_server_ip, return_server_port);

        if (address_is_valid != 0) {
            std::cerr << "Invalid response message received: " << buffer
                      << std::endl;
            num_retries++;
            continue; // invalid message, attempt again
        }

        // valid message received
        break;
    }

    if (num_retries >= max_retries) {
        std::cerr << "Could not find server" << std::endl;
        close(sockfd);
        return -1; // max retries reached
    }

    close(sockfd);
    return 0;
}

// Validates and extracts IP and Port number from discovery response message
// Expected format: "LOC <IP_ADDRESS> <PORT> END"
// returns 0 on success, -1 on NOT_LOC_MSG, -2 on INVALID_IP, -3 on
// INVALID_PORT, -4 on INVALID_FORMAT, -5 for NULL return pointers
int extract_ip_and_port_from_response(char *response, char *return_ip,
                                      int *return_port) {

    char *ip_str;
    char *port_str;
    char *strtok_ptr;
    struct sockaddr_in socket_address;

    // compare first 3 chars to "LOC"
    if (strncmp(response, "LOC", 3) != 0) {
        return -1; // NOT_LOC_MSG
    }

    // use strtok to split the string by spaces
    strtok_ptr = strtok(response, " ");
    strtok_ptr = strtok(NULL, " "); // get the IP part
    if (strtok_ptr == NULL) {
        return -4; // INVALID_FORMAT
    }
    ip_str = strtok_ptr;
    strtok_ptr = strtok(NULL, " "); // get the PORT part
    if (strtok_ptr == NULL) {
        return -4; // INVALID_FORMAT
    }
    port_str = strtok_ptr;

    // Check for END token
    strtok_ptr = strtok(NULL, " ");
    if (strtok_ptr == NULL || strcmp(strtok_ptr, "END") != 0) {
        return -4; // INVALID_FORMAT
    }

    // std::cout << "Extracted IP: " << ip_str << std::endl;
    // std::cout << "Extracted Port: " << port_str << std::endl;

    // validate IP address
    // this is done by trying to insert it into sockaddr_in struct
    // if inet_pton returns 1, it's valid
    if (inet_pton(AF_INET, ip_str, &(socket_address.sin_addr)) != 1) {
        return -2; // INVALID_IP
    }

    // validate port number
    int port = atoi(port_str); // if it's not a number, atoi returns 0
    if (port <= 0 ||
        port > 65535) { // server can be on well known ports, i guess
        return -3;      // INVALID_PORT
    }

    // check return pointers are not NULL
    if (return_ip == NULL || return_port == NULL) {
        return -5; // NULL return pointers
    }

    // ip and ports are valid, copy to return variables
    strcpy(return_ip, ip_str);
    *return_port = port;

    return 0;
}