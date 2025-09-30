



// Client Discovery Protocol
// This function initiates the discovery protocol, sending a broadcasted UDP datagram with the DIS message type
// then it waits for a response from the server or a timeout
// if the timer runs out / message is invalid, try it again
// if a valid response is received, extract the server IP and port and return them
// if the maximum number of retries is reached, return an error
// return 0 on success, -1 on error

int client_discovery_protocol(char* return_server_ip, int* return_server_port, char* broadcast_ip, int broadcast_port, int max_retries, int initial_timeout_ms){
    int sockfd; 
    char buffer[MAXLINE]; 
    char send_buffer[MAXLINE]; 
    const char *discovery_message = "DIS"; 
    struct sockaddr_in     servaddr; 
  
    // Creating socket file descriptor 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 

    int broadcastEnable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)) < 0) {
        perror("setsockopt (SO_BROADCAST) failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
  
    memset(&servaddr, 0, sizeof(servaddr)); 

    // Filling server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(PORT); 
    //servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_addr.s_addr = inet_addr("192.168.0.255"); // broadcast address
      
    int n;
    socklen_t len; 
    std::string msg;


    // copy to send buffer
    strncpy(send_buffer, discovery_message, MAXLINE);
    sendto(sockfd, send_buffer, strlen(send_buffer), MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));

    std::cout<<" Sent Discovery Message " <<std::endl; 

    // espera receber mensagem

    n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *) &servaddr, &len);
    buffer[n] = '\0';
    std::cout<<"Server>> : "<<buffer<<std::endl;
        

    close(sockfd); 
    return 0; 

}
