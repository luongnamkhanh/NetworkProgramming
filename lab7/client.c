#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
void *recv_thread(void *arg);
void *send_thread(void *arg);

// #define SERVER_ADDR "127.0.0.1"
// #define SERVER_PORT 5550
#define BUFF_SIZE 8192

struct arg
{
    int sock;
};


int main(int argc, char *argv[])
{
    int client_sock;                /* file descriptors */
    struct sockaddr_in server_addr; /* server's address information */
    pthread_t thread_send, thread_recv;

    // Step 1: Construct socket
    client_sock = socket(AF_INET, SOCK_STREAM, 0);

    // Step 2: Specify server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    // Step 3: Request to connect server
    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0)
    {
        printf("\nError!Can not connect to sever! Client exit imediately! ");
        return 0;
    }
    struct arg data;
    data.sock = client_sock;
    // Step 4: Create threads for sending and receiving data
    if (pthread_create(&thread_send, NULL, send_thread, (void *)&data) != 0)
    {
        perror("pthread_create for send_thread");
        exit(1);
    }
    if (pthread_create(&thread_recv, NULL, recv_thread, (void *)&data) != 0)
    {
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
void *send_thread(void *data)
{
    FILE *fp;
    char filename[BUFF_SIZE];
    char buff[BUFF_SIZE];
    int sock = ((struct arg *)data)->sock;
    int bytes_sent, bytes_received;
    int msg_len;
    while (1)
    {
        memset(buff, '\0', (strlen(buff) + 1));
        fgets(buff, BUFF_SIZE, stdin);
        msg_len = strlen(buff);
        if (msg_len -1 == 0) // exit when user only press enter
            exit(0);
        bytes_sent = send(sock, buff, msg_len, 0);
        if (bytes_sent <= 0)
        {
            printf("\nConnection closed!\n");
            exit(1);
        }
    }
    pthread_exit(NULL);
}

void *recv_thread(void *data)
{
    int client_sock = ((struct arg *)data)->sock;
    int bytes_received;
    char buff[BUFF_SIZE];
    while (1)
    {
        bytes_received = recv(client_sock, buff, BUFF_SIZE - 1, 0);
        if (bytes_received <= 0)
        {
            printf("\nError!Cannot receive data from sever!\n");
            break;
        }
        buff[bytes_received] = '\0';
        printf("%s", buff);
    }
    pthread_exit(NULL);
}
