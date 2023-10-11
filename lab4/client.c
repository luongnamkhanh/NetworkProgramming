/*UDP Client*/
#include <stdio.h> /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

void *recv_thread(void *arg);
void *send_thread(void *arg);

// #define SERV_PORT 5550
// #define SERV_IP "127.0.0.1"
#define BUFF_SIZE 1024

struct arg{
	int sock;
	struct sockaddr_in server_addr;
};


int main(int argc, char *argv[])
{
	int client_sock;
	struct sockaddr_in server_addr;
	int bytes_sent, bytes_received, sin_size;
	pthread_t thread_send, thread_recv;

	// Step 1: Construct a UDP socket
	if ((client_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{ /* calls socket() */
		perror("\nError: ");
		exit(0);
	}
	// Step 2: Define the address of the server
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	// mo phong tcp
	char *buff = (char *)malloc(1024);
	strcpy(buff, "");

	bytes_sent = sendto(client_sock, buff, strlen(buff), 0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
	if (bytes_sent < 0)
	{
		perror("\nError: ");
		close(client_sock);
		exit(0);
	}
	struct arg data;
	data.sock = client_sock;
	data.server_addr = server_addr;
	// Create threads for sending and receiving data
    if (pthread_create(&thread_send, NULL, send_thread, (void *)&data) != 0) {
        perror("pthread_create for send_thread");
        exit(1);
	}
	if (pthread_create(&thread_recv, NULL, recv_thread, (void *)&data) != 0) {
        perror("pthread_create for recv_thread");
        exit(1);
    }
	// Wait for threads to finish
    pthread_join(thread_send, NULL);
    pthread_join(thread_recv, NULL);
	// Close socket
    close(client_sock);

    return 0;
}
void *recv_thread(void *data) {
    int sock = ((struct arg *)data)->sock;
    char buffer[1024];
    ssize_t nbytes;
    struct sockaddr_in server = ((struct arg *)data)->server_addr;
    socklen_t addrlen = sizeof(struct sockaddr);

    while (1) {
        // Receive data from server
        nbytes = recvfrom(sock, buffer, sizeof(buffer), 0,(struct sockaddr *)&server, &addrlen);
        if (nbytes == -1) {
            perror("recvfrom");
            exit(1);
        }
        // Process received data
        buffer[nbytes] = '\0';
        printf("%s\n", buffer);
    }

    pthread_exit(NULL);
}
void *send_thread(void *data) {
    int sock = ((struct arg *)data)->sock;
    char buffer[1024];
    struct sockaddr_in server_addr = ((struct arg *)data)->server_addr;
    socklen_t addrlen = sizeof(server_addr);

    // Initialize server address

    while (1) {
        // Read input from user
        memset(buffer, '\0', (strlen(buffer) + 1));
		fgets(buffer, BUFF_SIZE, stdin);
		if (strlen(buffer)-1 == 0)
		{
            exit(0);
		}
        // Send data to server
        if (sendto(sock, buffer, strlen(buffer), 0,
                   (struct sockaddr *)&server_addr, addrlen) == -1) {
            perror("sendto");
            exit(1);
        }
    }
    pthread_exit(NULL);
}

	