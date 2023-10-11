#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <unistd.h>
const int BUFF_SIZE = 8192;



int main(int argc, char *argv[])
{
	
	int client_sock;
	struct sockaddr_in server_addr; /* server's address information */
	int bytes_sent;

	// Step 1: Construct socket
	client_sock = socket(AF_INET, SOCK_STREAM, 0);

	// Step 2: Specify server address
	server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	
	// Step 3: Request to connect server
	if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0)
	{
		printf("\nError!Can not connect to sever! Client exit immediately! ");
		return 0;
	}

	// Step 4: Use uname() to get system information

	struct utsname uname_pointer;
	uname(&uname_pointer);
	bytes_sent = send(client_sock, &uname_pointer, sizeof(struct utsname), 0);
	if (bytes_sent <= 0)
	{
		printf("\nConnection closed!\n");
	}
	// Step 5: Close socket
	close(client_sock);
	return 0;
}
