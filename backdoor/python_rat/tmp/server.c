#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>

void handle_ftp(int connfd)
{
    printf("ftp\n");
    char sendBuff[32768];
    char readBuff[32768];
    read(connfd, readBuff, 32768);
    printf("%s\n", readBuff);
    close(connfd);
}

void handle_ssh(int connfd)
{
    printf("ssh\n");
    char sendBuff[32768];
    char readBuff[32768];
    read(connfd, readBuff, 32768);
    printf("%s\n", readBuff);
    close(connfd);
}

void handle_http(int connfd)
{
    printf("http\n");
    char sendBuff[32768];
    char readBuff[32768];
    read(connfd, readBuff, 32768);
    printf("%s\n", readBuff);
    close(connfd);
}

int run_server(char* service)
{
    int result = 0;
    int ftp_fd = 0;
    int ssh_fd = 0;
    int http_fd = 0;
    int connfd = 0;
    struct sockaddr_in serv_ftp;
    struct sockaddr_in serv_ssh;
    struct sockaddr_in serv_http;
    fd_set socks;
    ftp_fd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_ftp, '0', sizeof(serv_ftp));
    serv_ftp.sin_family = AF_INET;
    serv_ftp.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_ftp.sin_port = htons(21);
    bind(ftp_fd, (struct sockaddr*)&serv_ftp, sizeof(serv_ftp));
    listen(ftp_fd, 10);
    ssh_fd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_ssh, '0', sizeof(serv_ssh));
    serv_ssh.sin_family = AF_INET;
    serv_ssh.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_ssh.sin_port = htons(22);
    bind(ssh_fd, (struct sockaddr*)&serv_ssh, sizeof(serv_ssh));
    listen(ssh_fd, 10);
    http_fd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_http, '0', sizeof(serv_http));
    serv_http.sin_family = AF_INET;
    serv_http.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_http.sin_port = htons(80);
    bind(http_fd, (struct sockaddr*)&serv_http, sizeof(serv_http));
    listen(http_fd, 10);
    while(1)
    {
        FD_ZERO(&socks);
        FD_SET(ftp_fd, &socks);
        FD_SET(ssh_fd, &socks);
        FD_SET(http_fd, &socks);
        printf("listening ...\n");
        result = select(http_fd+1, &socks, NULL, NULL, NULL);
        if (result > 0);
        {
            if FD_ISSET(ftp_fd, &socks)
            {
                connfd = accept(ftp_fd, (struct sockaddr*)NULL, NULL);
                handle_ftp(connfd);
            }
            if FD_ISSET(ssh_fd, &socks)
            {
                connfd = accept(ssh_fd, (struct sockaddr*)NULL, NULL);
                handle_ssh(connfd);
            }
            if FD_ISSET(http_fd, &socks)
            {
                connfd = accept(http_fd, (struct sockaddr*)NULL, NULL);
                handle_http(connfd);
            }
        }
    }
    return 0;
}
