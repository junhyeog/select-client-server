#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

/** Constants
 * BUF_SIZE : TCP payload size
*/
#define BUF_SIZE 1 << 16

/** Global variables
 * char user_mes[BUF_SIZE]: buffer for user message
 * char echoed_mes[BUF_SIZE] : buffer for echoed message
*/
char user_mes[BUF_SIZE];
char echoed_mes[BUF_SIZE];

void handle_error(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(EXIT_FAILURE);
}

void usage()
{
    printf("syntax : ./select-client <ip address> <port number>\n");
    printf("sample : ./select-client 112.113.114.115 8888\n");
    handle_error("[-] Usage error");
}

int main(int argc, char *argv[])
{
    // Result value
    int res;

    // Check command-line arguments
    if (argc != 3)
        usage();

    // Parse command-line arguments
    int port_num = atoi(argv[2]);
    struct in_addr ip;
    res = inet_pton(AF_INET, argv[1], &ip);
    if (res < 0)
        handle_error("[-] Fail to get a IP address");
    else if (!res)
        handle_error("[-] Invalid IP address");
    if (port_num < 0 || (int)(1 << 16) <= port_num)
        handle_error("[-] Invalid port number");

    // Create a socket
    // IPv4 Internet protocols and TCP
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        handle_error("[-] Fail to create a socket");

    // Build a socket
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_num);
    addr.sin_addr.s_addr = ip.s_addr;
    memset(addr.sin_zero, 0, sizeof(addr.sin_zero));

    // Connect
    res = connect(sockfd, (struct sockaddr *)(&addr), sizeof(addr));
    if (res < 0)
    {
        close(sockfd);
        handle_error("[-] Fail to connect");
    }
    printf("[+] Connected\n");

    // Set fd_set struct
    fd_set init_read_set, read_set;
    FD_ZERO(&init_read_set);
    FD_SET(0, &init_read_set);      // for stdin
    FD_SET(sockfd, &init_read_set); // for socket
    int max_fd = sockfd;

    while (1)
    {
        read_set = init_read_set;

        // Check
        res = select(max_fd + 1, &read_set, NULL, NULL, NULL);
        if (res < 0)
        {
            close(sockfd);
            handle_error("[-] Fail to select");
        }
        else if (!res)
        {
            continue;
        }
        else
        {
            // If stdin ready
            if (FD_ISSET(0, &read_set))
            {
                // Get a user's message
                fgets(user_mes, BUF_SIZE, stdin);
                // Send a user's message
                ssize_t sent_len;
                sent_len = send(sockfd, user_mes, strlen(user_mes) - 1, 0);
                if (sent_len < 0)
                {
                    close(sockfd);
                    handle_error("[-] Fail to send a user's message");
                }
            }
            // If socket ready
            if (FD_ISSET(sockfd, &read_set))
            {
                // Read echoed message
                ssize_t received_len = recv(sockfd, echoed_mes, BUF_SIZE - 1, 0);
                echoed_mes[received_len] = '\0';
                if (received_len < 0)
                {
                    close(sockfd);
                    handle_error("[-] Fail to recieve a echoed message");
                }
                if (!received_len)
                {
                    close(sockfd);
                    handle_error("[-] Closed");
                }
                printf("%s\n", echoed_mes);
            }
        }
    }
}