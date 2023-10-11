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
int isLogin = 0;
pthread_mutex_t login_mutex;

void menu()
{
    printf("\n-----------------------------------\n");
    printf("1. Gửi xâu bất kỳ\n");
    printf("2. Gửi nội dung một file\n");
    printf("3. Thoát\n");
}

struct arg
{
    int sock;
};

int main(int argc, char *argv[])
{
    int client_sock;                /* file descriptors */
    struct sockaddr_in server_addr; /* server's address information */
    pthread_t thread_send, thread_recv;
    // Initialize the login mutex
    if (pthread_mutex_init(&login_mutex, NULL) != 0)
    {
        perror("pthread_mutex_init");
        exit(1);
    }

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

    // Destroy the login mutex
    pthread_mutex_destroy(&login_mutex);
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
    while (isLogin == 0)
    {
        memset(buff, '\0', (strlen(buff) + 1));
        fgets(buff, BUFF_SIZE, stdin);
        msg_len = strlen(buff);
        bytes_sent = send(sock, buff, msg_len, 0);
        if (bytes_sent <= 0)
        {
            printf("\nConnection closed!\n");
            exit(1);
        }
    }
    while (isLogin == 1)
    {
        menu();
        memset(buff, '\0', (strlen(buff) + 1));
        fgets(buff, BUFF_SIZE, stdin);
        int choice = atoi(buff);
        switch (choice)
        {
        case 1:
            // Handle echo message mode
            printf("Enter message to echo: ");
            fgets(buff, BUFF_SIZE, stdin);
            msg_len = strlen(buff);
            if (msg_len == 0)
                break;
            bytes_sent = send(sock, buff, msg_len, 0);
            if (bytes_sent <= 0)
            {
                printf("\nConnection closed!\n");
                exit(1);
            }
            break;

        case 2:
            // Handle send image file mode
            printf("Enter the path of the image file: ");
            fgets(filename, BUFF_SIZE, stdin);
            // Remove the trailing newline character
            filename[strcspn(filename, "\n")] = 0;
            fp = fopen(filename, "rb");
            if (fp == NULL)
            {
                printf("Error opening file: %s\n", filename);
                continue;
            }
            while ((bytes_sent = fread(buff, 1, BUFF_SIZE, fp)) > 0)
            {
                if (send(sock, buff, bytes_sent, 0) < 0)
                {
                    printf("\nError sending file\n");
                    break;
                }
            }
            fclose(fp);
            printf("File sent successfully\n");
            break;
        case 3:
            pthread_mutex_lock(&login_mutex); // Acquire the lock
            strcpy(buff, "QUIT\n");
            bytes_sent = send(sock, buff, strlen(buff), 0);
            if (bytes_sent <= 0)
            {
                printf("\nConnection closed!\n");
                exit(1);
            }
            pthread_mutex_unlock(&login_mutex); // Release the lock
            break;
        default:
            printf("Invalid choice. Please try again.\n");
            break;
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
        if (strcmp(buff, "LOGIN_SUCCESS\n") == 0)
        {
            pthread_mutex_lock(&login_mutex); // Acquire the lock
            isLogin = 1;
            printf("Press Enter to continue\n");
            pthread_mutex_unlock(&login_mutex); // Release the lock
        }
        if (strcmp(buff, "LOGOUT_SUCCESS\n") == 0)
        {
            pthread_mutex_lock(&login_mutex); // Acquire the lock
            isLogin = 0;
            pthread_mutex_unlock(&login_mutex); // Release the lock
        }
    }
    pthread_exit(NULL);
}
