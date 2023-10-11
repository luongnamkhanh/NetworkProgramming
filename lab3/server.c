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
	//
	char alpha[100], numeric[100];
	//
	int server_sock; /* file descriptors */
	char buff[BUFF_SIZE];
	int bytes_sent, bytes_received;
	struct sockaddr_in server;			   /* server's address information */
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
	server.sin_port = htons(atoi(argv[1]));		 /* htons() chuyển đổi số nguyên sang định dạng mạng */
	server.sin_addr.s_addr = INADDR_ANY; /* INADDR_ANY đặt địa chỉ IP của server là địa chỉ IP của máy */
	bzero(&(server.sin_zero), 8);		 /* Thiết lập giá trị bằng 0 cho phần còn lại của struct */

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

				error = 0;
				separate_strings(buff, alpha, numeric);
				for (int i = 0; i < 2; i++)
				{
					if (client.sin_addr.s_addr == clients[i].sin_addr.s_addr && client.sin_port == clients[i].sin_port)
					{
						if (error == 1)
						{
							bytes_sent = sendto(server_sock, buff, strlen(buff), 0, (struct sockaddr *)&clients[1 - i], sin_size);
							if (bytes_sent < 0)
							{
								perror("\nError5: ");
								// return 0;
							}
						}
						else
						{
							if (strlen(numeric) != 0)
							{

								bytes_sent = sendto(server_sock, numeric, strlen(numeric), 0, (struct sockaddr *)&clients[1-i], sin_size);
								if (bytes_sent < 0)
								{
									perror("\nError6: ");
									// return 0;
								}
							}
							if (strlen(alpha) != 0)
							{

								bytes_sent = sendto(server_sock, alpha, strlen(alpha), 0, (struct sockaddr *)&clients[1-i], sin_size); /* Gửi lại tin nhắn cho client */
								if (bytes_sent < 0)
								{
									perror("\nError7: ");
									// return 0;
								}
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
