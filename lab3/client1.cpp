#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080

int main() {
    int sockfd;
    struct sockaddr_in servaddr;
    char buffer[1024];

    // Create socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Set server address and port
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &servaddr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        return -1;
    }

    // Send message to server
    const char* message = "Hello from client1";
    sendto(sockfd, message, strlen(message), 0, (const struct sockaddr*) &servaddr, sizeof(servaddr));
    std::cout << "Message sent to server: " << message << std::endl;

    // Receive message from server
    socklen_t len = sizeof(servaddr);
    int n = recvfrom(sockfd, buffer, 1024, 0, (struct sockaddr*) &servaddr, &len);
    buffer[n] = '\0';
    std::cout << "Message received from server: " << buffer << std::endl;

    // Send message to client2 through server
    const char* message2 = "Hello from client1 to client2";
    sendto(sockfd, message2, strlen(message2), 0, (const struct sockaddr*) &servaddr, sizeof(servaddr));
    std::cout << "Message sent to server for client2: " << message2 << std::endl;

    // Receive message from client2 through server
    n = recvfrom(sockfd, buffer, 1024, 0, (struct sockaddr*) &servaddr, &len);
    buffer[n] = '\0';
    std::cout << "Message received from client2 via server: " << buffer << std::endl;

    // Close the socket
    close(sockfd);

    return 0;
}
