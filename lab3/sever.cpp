#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int server_fd, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
        std::cerr << "Socket creation failed" << std::endl;
        return -1;
    }

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        std::cerr << "setsockopt failed" << std::endl;
        return -1;
    }

    // Bind socket to address and port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        std::cerr << "Bind failed" << std::endl;
        return -1;
    }

    while (true) {
        // Receive message from client
        memset(buffer, 0, BUFFER_SIZE);
        int num_bytes = recvfrom(server_fd, buffer, BUFFER_SIZE, MSG_WAITALL, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        std::cout << "Received message: " << buffer << std::endl;

        // Send message to other client
        int sendto_fd;
        if ((sendto_fd = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
            std::cerr << "Socket creation failed" << std::endl;
            return -1;
        }

        struct sockaddr_in other_address;
        other_address.sin_family = AF_INET;
        other_address.sin_port = htons(PORT);
        other_address.sin_addr.s_addr = inet_addr("127.0.0.1");

        sendto(sendto_fd, buffer, num_bytes, MSG_CONFIRM, (const struct sockaddr *)&other_address, sizeof(other_address));
        std::cout << "Message forwarded to other client" << std::endl;
    }

    return 0;
}
