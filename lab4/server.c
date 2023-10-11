/*UDP Server*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>

// #define PORT 5550 /* Cổng mà server mở */
#define BUFF_SIZE 1024
int error = 0;
int state[2] = {0, 0};
// initailize struct containing user information (username, password, status, count)
typedef struct data
{
    char username[20];
    char password[20];
    int status;  // 1:active, 0:blocked
    int count;   // number of failed attempts
    int sign_in; // 1: signed in, 0: signed out
} account;
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
            strcpy(input, "Error");
            error = 1;
            break;
        }
    }

    alpha[j] = '\0';
    numeric[k] = '\0';
}
int isClientNew(struct sockaddr_in client, struct sockaddr_in clients[2])
{
    int i;
    for (i = 0; i < 2; i++)
    {
        if (client.sin_addr.s_addr == clients[i].sin_addr.s_addr && client.sin_port == clients[i].sin_port)
        {
            return 0;
        }
    }
    return 1;
}
int main(int argc, char *argv[])
{
    char username_signed_in[2][20];
    int sign_in = 0;
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
    int j[2] = {0, 0};
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
    char username[2][20];
    char password[2][20];
    int flag;
    //
    char alpha[100], numeric[100];
    //
    int server_sock; /* file descriptors */
    char buff[BUFF_SIZE];
    int bytes_sent, bytes_received;
    struct sockaddr_in server;             /* server's address information */
    struct sockaddr_in client, clients[2]; /* client's address information */
    int sin_size;
    int client_register = 0;

    // Bước 1: Khởi tạo socket UDP
    if ((server_sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    { /* Gọi hàm socket() */
        perror("\nError1: ");
        return 0;
    }

    // Bước 2: Bind địa chỉ vào socket
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[1])); /* htons() chuyển đổi số nguyên sang định dạng mạng */
    server.sin_addr.s_addr = INADDR_ANY;    /* INADDR_ANY đặt địa chỉ IP của server là địa chỉ IP của máy */
    bzero(&(server.sin_zero), 8);           /* Thiết lập giá trị bằng 0 cho phần còn lại của struct */

    if (bind(server_sock, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    { /* Gọi hàm bind() */
        perror("\nError2: ");
        return 0;
    }

    // Bước 3: Giao tiếp với clients
    while (1)
    {
        sin_size = sizeof(struct sockaddr_in);

        bytes_received = recvfrom(server_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *)&client, &sin_size);

        if (bytes_received < 0)
        {
            perror("\nError3: ");
            return 0;
        }
        else
        {
            if (isClientNew(client, clients) && client_register == 0)
            {
                client_register++;
                clients[0] = client;
                printf("Client 1: %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
            }
            else if (isClientNew(client, clients) && client_register == 1)
            {
                client_register++;
                clients[1] = client;
                printf("Client 2: %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
            }

            if (strlen(buff) != 0)
            {
                buff[bytes_received - 1] = '\0';
                printf("[%s:%d]: %s\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port), buff);
                for (int k = 0; k < 2; k++)
                {
                    if (state[k] == 0 && client.sin_addr.s_addr == clients[k].sin_addr.s_addr && client.sin_port == clients[k].sin_port)
                    {

                        strcpy(username[k], buff);
                        flag = 0;
                        for (i = 0; i < n; i++)
                        {
                            if (strcmp(username[k], arr[i].username) == 0)
                            {
                                flag = 1;
                                break;
                            }
                        }
                        j[k] = i;
                        if (flag == 0)
                        {
                            state[k] = 0;
                            continue;
                        }

                        strcpy(buff, "Insert password");
                        bytes_sent = sendto(server_sock, buff, strlen(buff), 0, (struct sockaddr *)&clients[k], sin_size);
                        if (bytes_sent < 0)
                        {
                            perror("\nError4: ");
                            // return 0;
                        }
                        state[k] = 1;
                    }
                    else if (state[k] == 1 && client.sin_addr.s_addr == clients[k].sin_addr.s_addr && client.sin_port == clients[k].sin_port)
                    {
                        strcpy(password[k], buff);
                        flag = 0;
                        for (i = 0; i < n; i++)
                        {
                            if (strcmp(username[k], arr[i].username) == 0 && strcmp(password[k], arr[i].password) == 0)
                            {
                                flag = 1;
                                break;
                            }
                        }
                        if (flag == 1 && arr[i].sign_in == 1)
                        {
                            strcpy(buff, "Account is already signed in in another device");
                            bytes_sent = sendto(server_sock, buff, strlen(buff), 0, (struct sockaddr *)&clients[k], sin_size);
                            if (bytes_sent < 0)
                            {
                                perror("\nError5: ");
                                // return 0;
                            }
                            state[k] = 0;
                        }
                        else if (flag == 1 && arr[i].status == 1)
                        {
                            strcpy(buff, "OK");
                            arr[i].sign_in = 1;
                            bytes_sent = sendto(server_sock, buff, strlen(buff), 0, (struct sockaddr *)&clients[k], sin_size);
                            if (bytes_sent < 0)
                            {
                                perror("\nError5: ");
                                // return 0;
                            }
                            strcpy(username_signed_in[k], username[k]);
                            arr[i].count = 0;
                            state[k] = 2;
                        }
                        else if (flag == 1 && arr[i].status == 0)
                        {
                            strcpy(buff, "Account not ready");
                            bytes_sent = sendto(server_sock, buff, strlen(buff), 0, (struct sockaddr *)&clients[k], sin_size);
                            if (bytes_sent < 0)
                            {
                                perror("\nError6: ");
                                // return 0;
                            }
                            state[k] = 0;
                        }
                        else if (flag == 0)
                        {
                            arr[j[k]].count++; // increase the number of wrong password
                            if (arr[j[k]].count != 4)
                            {
                                strcpy(buff, "Not OK");
                                bytes_sent = sendto(server_sock, buff, strlen(buff), 0, (struct sockaddr *)&clients[k], sin_size);
                                if (bytes_sent < 0)
                                {
                                    perror("\nError7: ");
                                    // return 0;
                                }
                            }

                            if (arr[j[k]].count == 4)
                            {
                                arr[j[k]].status = 0;
                                strcpy(buff, "Account is blocked");
                                bytes_sent = sendto(server_sock, buff, strlen(buff), 0, (struct sockaddr *)&clients[k], sin_size);
                                if (bytes_sent < 0)
                                {
                                    perror("\nError8: ");
                                    // return 0;
                                }
                                // replace the old data in file account.txt
                                f = fopen(filename, "w");
                                for (i = 0; i < n; i++)
                                {
                                    fprintf(f, "%s %s %d\n", arr[i].username, arr[i].password, arr[i].status);
                                }
                                fclose(f);
                            }
                            state[k] = 0;
                        }
                    }
                    else if (state[k] == 2 && client.sin_addr.s_addr == clients[k].sin_addr.s_addr && client.sin_port == clients[k].sin_port)
                    {
                        if (strcmp(buff, "bye") == 0)
                        {
                            strcpy(buff, "Good bye ");
                            strcat(buff, username_signed_in[k]);
                            bytes_sent = sendto(server_sock, buff, strlen(buff), 0, (struct sockaddr *)&clients[k], sin_size);
                            if (bytes_sent < 0)
                            {
                                perror("\nError12: ");
                                // return 0;
                            }
                            arr[j[k]].sign_in = 0;
                            state[k] = 0;
                        }
                        else if (strcmp(buff, "change password") == 0)
                        {
                            bytes_received = recvfrom(server_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *)&client, &sin_size);
                            buff[bytes_received - 1] = '\0';
                            state[k] = 2;
                            error = 0;
                            separate_strings(buff, alpha, numeric);

                            if (error == 1)
                            {
                                bytes_sent = sendto(server_sock, buff, strlen(buff), 0, (struct sockaddr *)&clients[k], sin_size);
                                if (bytes_sent < 0)
                                {
                                    perror("\nError9: ");
                                    // return 0;
                                }
                            }
                            else
                            {
                                // replace the password by buff in file account.txt
                                f = fopen(filename, "w");
                                for (i = 0; i < n; i++)
                                {
                                    if (strcmp(username_signed_in[k], arr[i].username) == 0)
                                    {
                                        strcpy(arr[i].password, buff);
                                    }
                                    fprintf(f, "%s %s %d\n", arr[i].username, arr[i].password, arr[i].status);
                                }
                                fclose(f);
                                //
                                if (strlen(numeric) != 0)
                                {

                                    bytes_sent = sendto(server_sock, numeric, strlen(numeric), 0, (struct sockaddr *)&clients[k], sin_size);
                                    if (bytes_sent < 0)
                                    {
                                        perror("\nError10: ");
                                        // return 0;
                                    }
                                }
                                if (strlen(alpha) != 0)
                                {

                                    bytes_sent = sendto(server_sock, alpha, strlen(alpha), 0, (struct sockaddr *)&clients[k], sin_size); /* Gửi lại tin nhắn cho client */
                                    if (bytes_sent < 0)
                                    {
                                        perror("\nError11: ");
                                        // return 0;
                                    }
                                }
                            }
                        }
                        else
                        {
                            int count = 0;
                            for (i = 0; i < n; i++)
                            {
                                count += arr[i].sign_in;
                            }
                            if (count == 2)
                            {
                                bytes_sent = sendto(server_sock, buff, strlen(buff), 0, (struct sockaddr *)&clients[1 - k], sin_size);
                                if (bytes_sent < 0)
                                {
                                    perror("\nError5: ");
                                    // return 0;
                                }

                                state[k] = 2;
                            }
                        }
                    }
                }
            }
        }
    }

    close(server_sock);
    return 0;
}
