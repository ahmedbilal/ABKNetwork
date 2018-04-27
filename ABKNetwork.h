

#ifndef ABKNETWORK_H
#define ABKNETWORK_H

#include <sys/types.h> // socket(), bind(), listen(), accept(), send(), recv(), getaddrinfo(), open()
#include <sys/socket.h> // socket(), bind(), listen(), accept(), send(), recv(), getaddrinfo()
#include <netdb.h> // getaddrinfo()
#include <stdio.h> // printf(), fopen(), fread()
#include <string.h> // memset()
#include <sys/sendfile.h> // sendfile()
#include <sys/stat.h> // open()
#include <fcntl.h> // open()
#include <unistd.h>
#include "ABKError.h"
#include "ABKUtility.h"

int CreateTCPServer(const char * ip, const char * port)
{
    struct addrinfo *res;

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(ip, port, &hints, &res) == -1)
        dwerror();
    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd == -1)
        dwerror();

    if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1)
        dwerror();

    if (listen(sockfd, 1) == -1)
        dwerror();

    freeaddrinfo(res);

    return sockfd;
}

int ConnectTCPServer(const char * ip, const char * port)
{
    struct addrinfo *res;

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(ip, port, &hints, &res) == -1){
        printf("Problem in getaddrinfo()\n");
        dwerror();
    }

    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd == -1){
        printf("Problem in socket()\n");
        dwerror();
    }

    if (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1)
        dwerror();

    freeaddrinfo(res);

    return sockfd;
}

int ConnectTCPServerWoExit(const char * ip, const char * port)
{
    struct addrinfo *res;

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(ip, port, &hints, &res) == -1){
        ddwerror("getaddrinfo()");
        return -1;
    }

    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd == -1){
        ddwerror("Socket()");
        return -1;
    }

    if (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1)
    {
        ddwerror("connect()");
        return -1;
    }
    freeaddrinfo(res);
    return sockfd;
}

int ConnectTCPWoExitWoError(const char * ip, const char * port)
{
    struct addrinfo *res;

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(ip, port, &hints, &res) == -1)
        return -1;

    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd == -1)
        return -1;
    
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1)
        return -1;

    freeaddrinfo(res);

    return sockfd;
}

// recv() with error handling
int recv_weh(int clientfd, char * msg, int s)
{
    int rb = 0;
    if ((rb = recv(clientfd, msg, s, 0)) == -1)
        ddwerror("recv()");
    return rb;
}

// send() with error handling
int send_weh(int clientfd, char * msg, int s)
{
    int sb = 0;
    if ((sb = send(clientfd, msg, s, 0)) == -1)
        ddwerror("send()");
    return sb;
}

void send_msg(int clientfd, char * m)
{
    char msg[256] = { 0 };

    for (int i = 0; i < 256 - 1; i++)
        msg[i] = m[i];
    msg[sizeof(msg) - 1] = '\0';

    send_weh(clientfd, msg, sizeof(msg));
}

// send n byte message
void send_nb_msg(int clientfd, char * m, int nb)
{
    char * msg = (char *) calloc(nb, 1);

    for (int i = 0; i < nb - 1; i++)
        msg[i] = m[i];
    msg[sizeof(msg) - 1] = '\0';

    send_weh(clientfd, msg, sizeof(msg));
}

char * recv_nb_msg(int clientfd, char * m, int nb)
{
    char * msg = (char *) calloc(nb, 1);
    recv_weh(clientfd, msg, sizeof(msg));
    return msg;
}
char * recv_msg(int clientfd)
{
    char * msg = (char *)malloc(256);
    memset(msg, 0, 256);
    if (recv_weh(clientfd, msg, 256) == 0)
        return NULL;
    return msg;
}

int fread_send(int sockfd, FILE * f, int size)
{
    char * file_content = (char*)malloc(size);
    fread(file_content, 1, size, f);

    int rb = send_weh(sockfd, file_content, size);
    free(file_content);
    
    return rb;
}
long send_file_size(int clientfd, FILE * f)
{
    long size_long = get_file_size_f(f);
    char size_char[256] = { 0 };
    sprintf(size_char, "%ld", size_long);
    send_msg(clientfd, size_char);
    return size_long;
}

long recv_file_size(int clientfd)
{
    char * size_char = recv_msg(clientfd);
    return atol(size_char);
}

// send file to sockfd
// better implementation than sendfile()
void send_file(int sockfd, char * file)
{
    char msg[256] = { 0 };
    FILE * f = fopen(file, "r");

    long size = send_file_size(sockfd, f); // send file size

    while (fread(msg, 1, sizeof(msg), f) != 0){
        send_weh(sockfd, msg, 256);
        memset(msg, 0, sizeof(msg));
    }
}

void send_file2(int sockfd, char * file)
{
    FILE * f = fopen(file, "r");
    long size = send_file_size(sockfd, f); // send file size
    fclose(f);
    f = fopen(file, "r");
    sendfile(sockfd, fileno(f), 0, size);
}
void recv_write_file2(int inputfd, char * filename, int flags)
{
    int outputfd = open(filename, flags);
    char msg[256] = { 0 };
    char * size_c = recv_msg(inputfd);
    int size = atoi(size_c);
    while (size > 0)
    {
        memset(msg, 0, sizeof(msg));
        if (size - 256 > 0)
        {
            recv(inputfd, msg, sizeof(msg), 0);
            write(outputfd, msg, 256);

            size = size - 256;
        }
        else
        {
            //recv(inputfd, msg, size, 0);
            recv_weh(inputfd, msg, size);
            printf("MSG %s\n", msg);
            int wb = write(outputfd, msg, size);
            size = 0;
            close(outputfd);
            printf("File successfully downloaded\n");
            return;
        }

    }
}

// recv file and write it to disk
//void recv_write_file(int inputfd, int outputfd, int is_client)
//{
//    char msg[256];
//    char * size_char;
//
//    size_char = recv_msg(inputfd);
//    send_msg(outputfd, size_char);
//
//    long size = atol(size_char);
//    if (size < 0)   return;
//
//    while (recv_weh(inputfd, msg, 256) > 0)
//    {
//        if (size - 256 >= 0){
//            write(outputfd, msg, 256);
//        }
//        else if (is_client)
//        {
//            write(outputfd, msg, size);
//        }
//        else{
//            write(outputfd, msg, size);
//        }
//        size = size - 256;
//
//        memset(msg, 0, sizeof(msg));
//    }
//}

// recv file from sender and send it to receiver
//void recv_and_send_file(int senderfd, int receiverfd)
//{
//    char msg[256] = { 0 };
//
//    char * size_char = recv_msg(senderfd);
//    send_msg(receiverfd, size_char);
//
//    long size = atol(size_char);
//
//    while (recv_weh(senderfd, msg, 256) > 0)
//    {
//        send_weh(receiverfd, msg, 256);
//
//        if (strcmp(msg, __ABK_TRANSACTION_COMPLETE__) == 0) return;
//
//        memset(msg, 0, sizeof(msg));
//    }
//}

#endif