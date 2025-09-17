// Server side implementation of UDP client-server model 
#include <bits/stdc++.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

  
#define PORT     8080 
#define MAXLINE 1024 

int server_UDP_greeter() { 
    int sockfd; 
    char buffer[MAXLINE]; 
    char send_buffer[MAXLINE];
    const char *hello = "Hello from server"; 
    struct sockaddr_in servaddr, cliaddr; 
      
    // Creating socket file descriptor 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
      

    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
      
    // Filling server information 
    servaddr.sin_family    = AF_INET; // IPv4 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(PORT); 
      
    // Bind the socket with the server address 
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,  
            sizeof(servaddr)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
      
    socklen_t len;
    int n; 
  
    len = sizeof(cliaddr);  //len is value/result 

    std::string msg;

    while (true)
    {
        n = recvfrom(sockfd, (char *)buffer, MAXLINE,  
                    MSG_WAITALL, ( struct sockaddr *) &cliaddr, 
                    &len); 
        buffer[n] = '\0'; 
        printf("Client>>  %s\n", buffer); 

        //get message from stdin
        std::cout << "Escreve uma resposta ";
        std::getline(std::cin, msg);

        // copy to send buffer
        strncpy(send_buffer, msg.c_str(), MAXLINE);


        sendto(sockfd, send_buffer, strlen(send_buffer), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len); 
        std::cout<<">> "<< send_buffer <<std::endl;  
    }

    /*
    n = recvfrom(sockfd, (char *)buffer, MAXLINE,  
                MSG_WAITALL, ( struct sockaddr *) &cliaddr, 
                &len); 
    buffer[n] = '\0'; 
    printf("Client : %s\n", buffer); 
    sendto(sockfd, (const char *)hello, strlen(hello),  
        MSG_CONFIRM, (const struct sockaddr *) &cliaddr, 
            len); 
    std::cout<<"Hello message sent."<<std::endl;  

    */
      
    return 0; 
}