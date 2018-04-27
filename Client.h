#include "ABKNetwork.h"
#include "ABKTorrent.h"
#include "ABKUnits.h"
#include <pthread.h>

#ifndef CLIENT_H
#define CLIENT_H

typedef struct {
    int chunk;
    char * torrent;
} DownloaderThreadArgs;

void * DownloaderThread(void * args)
{
    TorrentInfo * t_i = read_torrent(((DownloaderThreadArgs *)args)->torrent);
    DownloadedFileTable * dft = get_dft();
    int dft_entry = find_entry_dft(t_i->filename, dft);

    int seederfd = -1;
    printf("Filename: %s\n", t_i->filename);
    for (int i = 0; i < t_i->n_seeders; i++)
    {
        printf("%s:%s\n", t_i->seeders[i].ip, t_i->seeders[i].port);
        seederfd = ConnectTCPServer(t_i->seeders[i].ip, t_i->seeders[i].port);
        if (seederfd != -1)
            break;
    }
    if (seederfd == -1)
    {
        printf("(No seeder/peer || No seeder/peer with needed chunk) online at this moment.\n");
        return NULL;
    }

    char * filename = t_i->filename;
    DownloaderThreadArgs dtargs;
    dtargs.chunk = ((DownloaderThreadArgs *)args)->chunk;

    FILE * f = fopen(filename, "w+");
    char content[1024];
    printf("Socket fd: %d\n", seederfd);
    while (recv_weh(seederfd, content, 1024) > 0)
    {
        printf("%s\n", content);
    }
    printf("Filename: %s\n", filename);
}


//int UploadTorrent(int sockfd, char * filename)
//{
//    char * msg;
//    send_msg(sockfd, "RECV");
//
//    send_msg(sockfd, filename);
//
//    msg = recv_msg(sockfd);
//    if (strcmp(msg, PROCEED) == 0)
//    {
//        FILE * f = fopen(filename, "r");
//        int size_int = send_file_size(sockfd, f);
//
//        char * msg = recv_msg(sockfd);
//        if (strcmp(msg, PROCEED) == 0){
//            fread_send(sockfd, f, size_int);
//        }
//        else
//        {
//            free(msg);
//            printf("Error: %s\n", msg);
//            return -1;
//        }
//        free(msg);
//        msg = recv_msg(sockfd);
//        printf("Message from Server: %s\n", msg);
//        return 0;
//    }
//    else
//    {
//        msg = recv_msg(sockfd);
//        printf("Error: %s\n", msg);
//        return -1;
//    }
//
//}


int DownloadTorrent(int sockfd, char * filename)
{
    send_msg(sockfd, "SEND");
    send_msg(sockfd, filename);
    
    char * msg = recv_msg(sockfd);

    if (strcmp(msg, SUCCESS) == 0)
    {
        int size_int = recv_file_size(sockfd);
        if (size_int > 0 && size_int < 1 * MB)
        {
            printf("Size: %d\n", size_int);
            send_msg(sockfd, PROCEED);
            char * file_content = (char *)malloc(size_int);
            recv_weh(sockfd, file_content, size_int);
            buffer_to_file(file_content, size_int, filename);
        }
        else
        {
            send_msg(sockfd, TORRENT_BIG_UNREAL);
            free(msg);
            return -1;
        }
    }
    else
    {
        printf("Error: %s\n", msg);
        return -1;
    }
}


int DownloadFile(char * torrent, int chunk, struct addrinfo * hints)
{
    TorrentInfo * t_i = read_torrent(torrent);

    DownloadedFileTable * dft = get_dft();
    int dft_entry = find_entry_dft(t_i->filename, dft);
    if (dft_entry == -1)
    {
        set_progress(torrent, chunk, 0);
    }
    if (dft->progresses[dft_entry].progress[chunk] == 100)
    {
        printf("This chunk is already downloaded\n.");
        return 0;
    }

    if (t_i != NULL)
    {
        pthread_t t;
        DownloaderThreadArgs * dtargs = malloc(sizeof(DownloaderThreadArgs));
        dtargs->chunk = chunk;
        dtargs->torrent = malloc(strlen(torrent));
        strcpy(dtargs->torrent, torrent);
        pthread_create(&t, NULL, DownloaderThread, (void *)&dtargs);
            
        return 0;
    }
    printf("Such torrent does not exist.\n");
    return -1;
}


int DownloadCompleteFile(char * torrent, struct addrinfo * hints)
{

}

#endif