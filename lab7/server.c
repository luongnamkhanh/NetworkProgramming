#include <stdio.h> /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_CLIENTS 2 // Maximum number of clients that can be connected at a time
#define BUFF_SIZE 8192
int error = 0;
int state[MAX_CLIENTS] = {0, 0};
// initailize struct containing user information (username, password, status, count)
typedef struct data
{
    char username[20];
    char password[20];
    int status;  // 1:active, 0:blocked
    int count;   // number of failed attempts
    int sign_in; // 1: signed in, 0: signed out
} account;

int main(int argc, char *argv[])
{
    //
    account *arr;
    arr = (account *)malloc(1000 * sizeof(account));
    char username_signed_in[MAX_CLIENTS][20];
    int sign_in = 0;
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
    int j[MAX_CLIENTS] = {0, 0};
    int k;
    while (!feof(f))
    {
        fscanf(f, "%s %s %d", arr[i].username, arr[i].password, &arr[i].status);
        i++;
    }
    n = i - 1;
    for (i = 0; i < n; i++)
    {
        arr[i].count = 0;
        arr[i].sign_in = 0;
    }
    fclose(f);
    //
    char username[MAX_CLIENTS][20];
    char password[MAX_CLIENTS][20];
    int flag;
    //
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    int client_fds[MAX_CLIENTS]; // Array to store client socket descriptors
    fd_set read_fds;             // Set of socket descriptors to monitor for read operations
    int max_fd;                  // Maximum file descriptor value
    int bytes_received, bytes_sent;
    char recv_data[BUFF_SIZE];

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

    bind(server_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));

    // Step 3: Listen request from client
    if (listen(server_sock, MAX_CLIENTS) == -1)
    {
        perror("Listen");
        return -1;
    }

    printf("\nServer started on port %d\n", atoi(argv[1]));

    // Step 4: Initialize client_fds and max_fd
    for (i = 0; i < MAX_CLIENTS; i++)
    {
        client_fds[i] = -1;
    }
    max_fd = server_sock;

    // Step 5: Main server loop
    while (1)
    {
        FD_ZERO(&read_fds);
        FD_SET(server_sock, &read_fds);

        for (i = 0; i < MAX_CLIENTS; i++)
        {
            if (client_fds[i] != -1)
            {
                FD_SET(client_fds[i], &read_fds);
                if (client_fds[i] > max_fd)
                    max_fd = client_fds[i];
            }
        }

        // Use select to monitor file descriptors
        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1)
        {
            perror("Select");
            return -1;
        }

        // Check if new connection is available
        if (FD_ISSET(server_sock, &read_fds))
        {
            // Accept new connection
            client_sock = accept(server_sock, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr);

            for (i = 0; i < MAX_CLIENTS; i++)
            {
                if (client_fds[i] == -1)
                {
                    client_fds[i] = client_sock;
                    printf("New connection accepted. Client socket descriptor: %d\n", client_sock);
                    break;
                }
            }
            if (i == MAX_CLIENTS)
            {
                printf("Max clients reached. Rejecting connection. Client socket descriptor: %d\n", client_sock);
                close(client_sock);
            }
        }

        // Check if data is available on existing connections
        for (i = 0; i < MAX_CLIENTS; i++)
        {
            if (client_fds[i] != -1 && FD_ISSET(client_fds[i], &read_fds))
            {
                bytes_received = recv(client_fds[i], recv_data, BUFF_SIZE - 1, 0);
                if (bytes_received <= 0)
                {
                    printf("Connection closed. Client socket descriptor: %d\n", client_fds[i]);
                    close(client_fds[i]);
                    client_fds[i] = -1;
                }
                else
                {
                    recv_data[bytes_received-1] = '\0';
                    printf("Client %d: %s\n", client_fds[i], recv_data);
                }
                if (state[i] == 0)
                {
                    strcpy(username[i], recv_data);
                    flag = 0;
                    for (k = 0; k < n; k++)
                    {
                        if (strcmp(username[i], arr[k].username) == 0)
                        {
                            flag = 1;
                            break;
                        }
                    }
                    j[i] = k;
                    if (flag == 0)
                    {
                        state[i] = 0;
                        continue;
                    }
                    strcpy(recv_data, "Insert Password\n");
                    bytes_sent = send(client_fds[i], recv_data, strlen(recv_data), 0);
                    if (bytes_sent < 0)
                    {
                        perror("\nError2: ");
                        return 0;
                    }
                    state[i] = 1;
                }
                else if (state[i] == 1)
                {
                    strcpy(password[i], recv_data);
                    flag = 0;
                    for (k = 0; k < n; k++)
                    {
                        if (strcmp(username[i], arr[k].username) == 0 && strcmp(password[i], arr[k].password) == 0)
                        {
                            flag = 1;
                            break;
                        }
                    }
                    if (flag == 1 && arr[k].sign_in == 1)
                    {
                        strcpy(recv_data, "Account is already signed in in another device\n");
                        bytes_sent = send(client_fds[i], recv_data, strlen(recv_data), 0);
                        if (bytes_sent < 0)
                        {
                            perror("\nError3: ");
                            return 0;
                        }
                        state[i] = 0;
                    }
                    else if (flag == 1 && arr[k].status == 1)
                    {
                        strcpy(recv_data, "Hello ");
                        strcat(recv_data, username[i]);
                        strcat(recv_data, ". Login successfully.\n");
                        bytes_sent = send(client_fds[i], recv_data, strlen(recv_data), 0);
                        if (bytes_sent < 0)
                        {
                            perror("\nError4: ");
                            return 0;
                        }
                        strcpy(username_signed_in[i], username[i]);
                        arr[k].sign_in = 1;
                        arr[k].count = 0;
                        state[i] = 2;
                    }
                    else if (flag == 1 && arr[k].status == 0)
                    {
                        strcpy(recv_data, "Account is blocked.\n");
                        bytes_sent = send(client_fds[i], recv_data, strlen(recv_data), 0);
                        if (bytes_sent < 0)
                        {
                            perror("\nError5: ");
                            return 0;
                        }
                        state[i] = 0;
                    }
                    else if (flag == 0)
                    {
                        arr[j[i]].count++; // increase the number of wrong password
                        if (arr[j[i]].count != 3)
                        {
                            strcpy(recv_data, "Password is incorrect.\n");
                            bytes_sent = send(client_fds[i], recv_data, strlen(recv_data), 0);
                            if (bytes_sent < 0)
                            {
                                perror("\nError6: ");
                                return 0;
                            }
                        }
                        if (arr[j[i]].count == 3)
                        {
                            arr[j[i]].status = 0;
                            strcpy(recv_data, "Password is incorrect. Account is blocked.\n");
                            bytes_sent = send(client_fds[i], recv_data, strlen(recv_data), 0);
                            if (bytes_sent < 0)
                            {
                                perror("\nError7: ");
                                return 0;
                            }
                            // replace the old data in file account.txt
                            f = fopen("account.txt", "w");
                            for (k = 0; k < n; k++)
                            {
                                fprintf(f, "%s %s %d\n", arr[k].username, arr[k].password, arr[k].status);
                            }
                            fclose(f);
                        }
                        state[i] = 0;
                    }
                }
                else if (state[i] == 2)
                {
                    if (strcmp(recv_data, "logout") == 0)
                    {
                        strcpy(recv_data, "Good bye ");
                        strcat(recv_data, username_signed_in[i]);
                        strcat(recv_data, ".\n");
                        bytes_sent = send(client_fds[i], recv_data, strlen(recv_data), 0);
                        if (bytes_sent < 0)
                        {
                            perror("\nError8: ");
                            return 0;
                        }
                        arr[j[i]].sign_in = 0;
                        state[i] = 0;
                    }
                }
            }
        }
    }

    return 0;
}
