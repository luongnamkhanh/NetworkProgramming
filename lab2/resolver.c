#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>

struct sockaddr_in sa;
struct hostent *hp;
int is_valid_ip(const char *ip)
{
    return inet_pton(AF_INET, ip, &(sa.sin_addr)) != 0;
}
int is_valid_domain(const char *domain)
{
    char *p;
    int i, len = strlen(domain);

    if (len < 1 || len > 255)
        return 0;

    if (domain[len - 1] == '.')
        return 0;

    for (i = 0; i < len; i++)
    {
        if (domain[i] == '.')
        {
            if (i == 0 || i == len - 1 || domain[i + 1] == '.')
                return 0;
        }
        else if (!(isalnum(domain[i]) || domain[i] == '-'))
            return 0;
    }

    return 1;
}
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Invalid number of arguments\n");
        exit(1);
    }

    if (is_valid_ip(argv[1]))
    {
        socklen_t len; /* input */
        char hbuf[NI_MAXHOST];

        memset(&sa, 0, sizeof(struct sockaddr_in));

        /* For IPv4*/
        sa.sin_family = AF_INET;
        sa.sin_addr.s_addr = inet_addr(argv[1]);
        len = sizeof(struct sockaddr_in);

        if (getnameinfo((struct sockaddr *)&sa, len, hbuf, sizeof(hbuf), NULL, 0, NI_NAMEREQD))
        {
            printf("Not found information\n");
        }
        else
        {
            printf("Official name: %s\n", hbuf);
        }
    }
    else if (is_valid_domain(argv[1]))
    {
        hp = gethostbyname(argv[1]);

        if (hp == 0)
        {
            printf("Not found information\n");
        }
        else
        {
            printf("Official IP: ");
            if (hp->h_addr_list[0] != 0)
                printf("%s", inet_ntoa(*(struct in_addr *)(hp->h_addr_list[0])));
            int i = 1;
            if (hp->h_addr_list[i] != 0)
                printf("\nAlias IP: \n");
            while (hp->h_addr_list[i] != 0)
            {
                printf("%s\n", inet_ntoa(*(struct in_addr *)(hp->h_addr_list[i])));
                i++;
            }
            printf("\n");
        }
    }
    else
        printf("Not found information\n");
    return 0;
}
