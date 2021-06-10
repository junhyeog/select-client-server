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
 * char mes[BUF_SIZE] : buffer for message
*/
char mes[BUF_SIZE];

void handle_error(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(EXIT_FAILURE);
}

void usage()
{
    printf("syntax : ./select-server <port number>\n");
    printf("sample : ./select-server 8888\n");
    handle_error("[-] Usage error");
}

int main(int argc, char *argv[])
{
    // Result value
    int res;

    // Check command-line argument
    if (argc != 2)
        usage();

    // Parse command-line argument
    int port_num = atoi(argv[1]);
    if (port_num < 0 || (int)(1 << 16) <= port_num)
        handle_error("[-] Invalid port number");

    // For client
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_sockfd;

    // Create a welcome socket
    // IPv4 Internet protocols and TCP
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        handle_error("[-] Fail to create a socket");

    // Build a socket
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_num);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(addr.sin_zero, 0, sizeof(addr.sin_zero));

    // Bind
    res = bind(sockfd, (struct sockaddr *)(&addr), sizeof(struct sockaddr));
    if (res < 0)
    {
        close(sockfd);
        handle_error("[-] Fail to bind");
    }

    // Listen
    res = listen(sockfd, SOMAXCONN);
    if (res < 0)
    {
        close(sockfd);
        handle_error("[-] Fail to listen");
    }

    // Set fd_set struct
    fd_set init_read_set, read_set;
    FD_ZERO(&init_read_set);
    FD_SET(sockfd, &init_read_set); // for welcome socket
    int max_fd = sockfd;

    while (1)
    {
        read_set = init_read_set;

        // Check
        res = select(max_fd + 1, &read_set, NULL, NULL, NULL);
        // If error,
        if (res < 0)
        {
            perror("[-] Fail to select\n");
            break;
        }
        for (int i = 0; i < max_fd + 1; i++)
        {
            if (!FD_ISSET(i, &read_set))
                continue;
            // If connection request,
            if (i == sockfd)
            {
                client_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_len);
                // Update init_read_set and max_fd
                FD_SET(client_sockfd, &init_read_set);
                max_fd = max_fd > client_sockfd ? max_fd : client_sockfd;
                printf("[+] Client(%d) connected\n", client_sockfd);
            }
            else
            {
                memset(mes, 0, sizeof(mes));
                ssize_t received_len = recv(i, mes, BUF_SIZE - 1, 0);
                mes[received_len] = '\0';
                if (received_len < 0)
                {
                    fprintf(stderr, "[-] Fail to recieve a message from client(%d)\n", i);
                    continue;
                }
                // Echo message
                ssize_t sent_len;
                sent_len = send(i, mes, strlen(mes), 0);
                if (sent_len < 0)
                {
                    fprintf(stderr, "[-] Fail to echo a message to client(%d)\n", i);
                    continue;
                }
                if (!sent_len)
                {
                    FD_CLR(i, &init_read_set);
                    close(i);
                    printf("[+] Client(%d) closed\n", i);
                    continue;
                }
                printf("[+] Echo to client(%d) : %s\n", i, mes);
            }
        }
    }
    for (int i = 3; i < max_fd + 1; i++)
    {
        if (!FD_ISSET(i, &init_read_set))
            continue;
        close(i);
    }
    exit(EXIT_FAILURE);
}