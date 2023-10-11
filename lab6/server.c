#include <stdio.h> /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <pthread.h>

// #define PORT 5550 /* Port that will be opened */
#define BACKLOG 2 /* Number of allowed connections */
#define BUFF_SIZE 1024
int error = 0;
// initailize struct containing user information (username, password, status, count)
typedef struct data
{
    char username[20];
    char password[20];
    int status; // 1:active, 0:blocked
    int count;  // number of failed attempts
} account;

struct arg
{
    int client_sock;
    struct sockaddr_in client_addr;
    account *arr;
    int n;
};

void *handle_client(void *arg);

int main(int argc, char *argv[])
{
    //
    account *arr;
    arr = (account *)malloc(1000 * sizeof(account));
    // open file account.txt
    FILE *f;
    char filename[] = "account.txt";
    if ((f = fopen(filename, "r")) == NULL)
    {
        printf("Cannot open file %s\n", filename);
        return 1;
    }
    // read data from file to store in array
    int i = 0, n;
    while (!feof(f))
    {
        fscanf(f, "%s %s %d", arr[i].username, arr[i].password, &arr[i].status);
        i++;
    }
    n = i - 1;
    for (i = 0; i < n; i++)
    {
        arr[i].count = 0;
    }
    fclose(f);
    //
    int server_sock, client_sock; /* file descriptors */
    char recv_data[BUFF_SIZE];
    int bytes_sent, bytes_received;
    struct sockaddr_in server_addr; /* server's address information */
    struct sockaddr_in client_addr; /* client's address information */
    pthread_t tid[2];               // for two clients
    int sin_size;
    int client = 0;

    // Step 1: Construct a TCP socket to listen connection request
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    { /* calls socket() */
        perror("\nError1: ");
        return 0;
    }
    int optval = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    // Step 2: Bind address to socket
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));     /* Remember htons() from "Conversions" section? =) */
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); /* INADDR_ANY puts your IP address automatically */
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    { /* calls bind() */
        perror("\nError2: ");
        return 0;
    }

    // Step 3: Listen request from client
    if (listen(server_sock, BACKLOG) == -1)
    { /* calls listen() */
        perror("\nError3: ");
        return 0;
    }

    // Step 4: Communicate with client
    while (1)
    {
        // Accept request from client
        sin_size = sizeof(struct sockaddr_in);
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &sin_size);
        //
        struct arg data;
        data.client_sock = client_sock;
        data.client_addr = client_addr;
        data.arr = arr;
        data.n = n;
        //
        if (client < BACKLOG)
        {
            // Create a thread to handle the client
            printf("Client %d: %s:%d\n", client + 1, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            pthread_create(&tid[client], NULL, handle_client, (void *)&data);
            client++;
        }
        else
        {
            printf("Connection limit reached. Rejecting new connection.\n");
            close(client_sock);
        }
    }

    return 0;
}

void *handle_client(void *data)
{
    //
    int client_sock = ((struct arg *)data)->client_sock;
    struct sockaddr_in client_addr = ((struct arg *)data)->client_addr;
    int bytes_received, bytes_sent;
    char recv_data[BUFF_SIZE];
    //
    account *arr = ((struct arg *)data)->arr;
    int n = ((struct arg *)data)->n;
    //
    FILE *f;
    char filename[] = "account.txt";
    //
    char username[20], password[20];
    char username_signed_in[20];
    int flag = 0;
    int state = 0;
    int i, j;

    // start conversation
    while (1)
    {
        // receives message from client
        bytes_received = recv(client_sock, recv_data, BUFF_SIZE - 1, 0); // blocking
        if (bytes_received <= 0)
        {
            printf("\nConnection closed\n");
            break;
        }
        else
        {
            if (strlen(recv_data) != 0)
            {
                recv_data[bytes_received - 1] = '\0';
                printf("[%s:%d]: %s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), recv_data);
                if (state == 0)
                {
                    strcpy(username, recv_data);
                    flag = 0;
                    for (i = 0; i < n; i++)
                    {
                        if (strcmp(username, arr[i].username) == 0)
                        {
                            flag = 1;
                            break;
                        }
                    }
                    j = i;
                    if (flag == 0)
                    {
                        strcpy(recv_data, "Not exist account. Please try again.\n");
                        bytes_sent = send(client_sock, recv_data, strlen(recv_data), 0);
                        if (bytes_sent <= 0)
                        {
                            printf("\nError3.5: ");
                            break;
                        }
                        state = 0;
                        continue;
                    }
                    strcpy(recv_data, "Input password: \n");
                    bytes_sent = send(client_sock, recv_data, strlen(recv_data), 0);
                    if (bytes_sent <= 0)
                    {
                        printf("\nError4: ");
                        break;
                    }
                    state = 1;
                }
                else if (state == 1)
                {
                    strcpy(password, recv_data);
                    flag = 0;
                    for (i = 0; i < n; i++)
                    {
                        if (strcmp(username, arr[i].username) == 0 && strcmp(password, arr[i].password) == 0)
                        {
                            flag = 1;
                            break;
                        }
                    }
                    if (flag == 1 && arr[i].status == 1)
                    {
                        strcpy(recv_data, "Hello ");
                        strcat(recv_data, username);
                        strcat(recv_data, ". Login successfully.\n");
                        bytes_sent = send(client_sock, recv_data, strlen(recv_data), 0);
                        if (bytes_sent <= 0)
                        {
                            printf("\nError5: ");
                            break;
                        }
                        strcpy(username_signed_in, username);
                        arr[i].count = 0;
                        state = 2;
                    }
                    else if (flag == 1 && arr[i].status == 0)
                    {
                        strcpy(recv_data, "Account is blocked.\n");
                        bytes_sent = send(client_sock, recv_data, strlen(recv_data), 0);
                        if (bytes_sent <= 0)
                        {
                            printf("\nError6: ");
                            break;
                        }
                        state = 0;
                    }
                    else
                    {
                        arr[j].count++;
                        if (arr[j].count != 3)
                        {
                            strcpy(recv_data, "Password is incorrect.\n");
                            bytes_sent = send(client_sock, recv_data, strlen(recv_data), 0);
                            if (bytes_sent <= 0)
                            {
                                printf("\nError7: ");
                                break;
                            }
                        }
                        if (arr[j].count == 3)
                        {
                            arr[j].status = 0;
                            strcpy(recv_data, "Password is incorrect. Account is blocked.\n");
                            bytes_sent = send(client_sock, recv_data, strlen(recv_data), 0);
                            if (bytes_sent <= 0)
                            {
                                printf("\nError8: ");
                                break;
                            }
                            // replace the old data in file account.txt
                            f = fopen(filename, "w");
                            for (i = 0; i < n; i++)
                            {
                                fprintf(f, "%s %s %d\n", arr[i].username, arr[i].password, arr[i].status);
                            }
                            fclose(f);
                        }
                        state = 0;
                    }
                }
                else if (state == 2)
                {
                    if (strcmp(recv_data, "logout") == 0)
                    {
                        strcpy(recv_data, "Good bye ");
                        strcat(recv_data, username_signed_in);
                        strcat(recv_data, ".\n");
                        bytes_sent = send(client_sock, recv_data, strlen(recv_data), 0);
                        if (bytes_sent <= 0)
                        {
                            printf("\nError9: ");
                            break;
                        }
                        state = 0;
                    }
                }
            }
        }
    }

    close(client_sock);
    pthread_exit(NULL);
}