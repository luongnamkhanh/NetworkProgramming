#include <stdio.h> /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

// #define PORT 5550 /* Port that will be opened */
#define BACKLOG 2 /* Number of allowed connections */
#define BUFF_SIZE 1024
int error = 0;
void separate_strings(char *input, char *alpha, char *numeric)
{
    int len = strlen(input);
    int i, j, k;
    j = k = 0;

    for (i = 0; i < len; i++)
    {
        if (isalpha(input[i]))
        {
            alpha[j] = input[i];
            j++;
        }
        else if (isdigit(input[i]))
        {
            numeric[k] = input[i];
            k++;
        }
        else
        {
            printf("Error: String contains non-alphanumeric characters.\n");
            error = 1;
            break;
        }
    }

    alpha[j] = '\n';
    alpha[j + 1] = '\0';
    numeric[k] = '\n';
    numeric[k + 1] = '\0';
}
int main(int argc, char *argv[])
{
    //
    FILE *fp;
    char alpha[100], numeric[100];
    //
    int listen_sock, conn_sock; /* file descriptors */
    char recv_data[BUFF_SIZE];
    int bytes_sent, bytes_received;
    struct sockaddr_in server; /* server's address information */
    struct sockaddr_in client; /* client's address information */
    int sin_size;

    // Step 1: Construct a TCP socket to listen connection request
    if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    { /* calls socket() */
        perror("\nError: ");
        return 0;
    }
    int optval = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    // Step 2: Bind address to socket
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[1]));              /* Remember htons() from "Conversions" section? =) */
    server.sin_addr.s_addr = htonl(INADDR_ANY); /* INADDR_ANY puts your IP address automatically */
    if (bind(listen_sock, (struct sockaddr *)&server, sizeof(server)) == -1)
    { /* calls bind() */
        perror("\nError: ");
        return 0;
    }

    // Step 3: Listen request from client
    if (listen(listen_sock, BACKLOG) == -1)
    { /* calls listen() */
        perror("\nError: ");
        return 0;
    }

    // Step 4: Communicate with client
    while (1)
    {
        // accept request
        sin_size = sizeof(struct sockaddr_in);
        if ((conn_sock = accept(listen_sock, (struct sockaddr *)&client, &sin_size)) == -1)
            perror("\nError: ");

        printf("You got a connection from %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port)); /* prints client's IP */

        // start conversation
        while (1)
        {
            // receives message from client
            bytes_received = recv(conn_sock, recv_data, BUFF_SIZE - 1, 0); // blocking
            if (bytes_received <= 0)
            {
                printf("\nConnection closed");
                break;
            }
            else
            {
                recv_data[bytes_received - 1] = '\0';
                printf("Client: %s \n", recv_data);
            }
            if (strcmp(recv_data, "1") == 0 )
            {
                while (1)
                {
                    bytes_received = recv(conn_sock, recv_data, BUFF_SIZE - 1, 0); // blocking
                    if (bytes_received <= 0)
                    {
                        printf("\nConnection closed");
                        break;
                    }
                    else
                    {
                        recv_data[bytes_received - 1] = '\0';
                        printf("Client: %s \n", recv_data);
                        if (strlen(recv_data)-1 == 0)
                        {
                            break;
                        }
                    }
                    error = 0;
                    separate_strings(recv_data, alpha, numeric);
                    if (error == 1)
                    {
                        // echo to client
                        strcpy(recv_data, "Error\n");
                        bytes_sent = send(conn_sock, recv_data, strlen(recv_data), 0); /* send to the client welcome message */
                        if (bytes_sent <= 0)
                        {
                            printf("\nConnection closed");
                        }
                    }
                    else
                    {
                        if (strlen(numeric) - 1 != 0)
                        {
                            bytes_sent = send(conn_sock, numeric, strlen(numeric), 0); /* send to the client welcome message */
                            if (bytes_sent <= 0)
                            {
                                printf("\nConnection closed");
                            }
                        }
                        if (strlen(alpha) - 1 != 0)
                        {
                            bytes_sent = send(conn_sock, alpha, strlen(alpha), 0); /* send to the client welcome message */
                            if (bytes_sent <= 0)
                            {
                                printf("\nConnection closed");
                            }
                        }
                    }
                }
            }
            if (strcmp(recv_data, "2") == 0)
            {
                fp = fopen("received_file.txt", "wb");
                long file_size;
                long bytes_total = 0;
                recv(conn_sock, &file_size, sizeof(file_size), 0);
                while (bytes_total < file_size && (bytes_received = recv(conn_sock, recv_data, BUFF_SIZE - 1, 0)) > 0)
                {
                    fwrite(recv_data, 1, bytes_received, fp);
                    bytes_total += bytes_received;
                }
                fclose(fp);
                //Step 7: Print file contents to console
                printf("\nFile contents:\n");
                fp = fopen("received_file.txt", "r");
                while (fgets(recv_data, BUFF_SIZE, fp) != NULL)
                {
                    printf("%s", recv_data);
                }
                printf("\n");
                fclose(fp);
            }
        }

    } // end conversation
    close(conn_sock);

    close(listen_sock);
    return 0;
}
