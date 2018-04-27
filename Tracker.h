#include "ABKNetwork.h"
#include "ABKUnits.h"
#include "ABKTorrent.h"

#ifndef TRACKER_H
#define TRACKER_H

void recv_torrent(int clientfd)
{
    char * filename = recv_msg(clientfd);
    if (filename != NULL)
    {
        send_msg(clientfd, PROCEED);
        int size_int = recv_file_size(clientfd);
        
        printf("File Size: %d\n", size_int);
        if (size_int <= 0 || size_int > 1 * MB) // If File too big or too small throw error
        {
            send_msg(clientfd, TORRENT_BIG_UNREAL);
            return;
        }
        else // otherwise get file content
        {
            send_msg(clientfd, PROCEED);
            char * file_content = (char *)malloc(size_int);
            if (recv_weh(clientfd, file_content, size_int) == 0) // if there is nothing in file throw error
            {
                send_msg(clientfd, EMPTY_TORRENT_FILE);
                return;
            }
            printf("Filename: %s\n", filename);
            buffer_to_file(file_content, size_int, filename);
            send_msg(clientfd, "SUCCESS");
            free(file_content);
            free(filename);
        }
    } // end of filename != NULL
    else
    {
        send_msg(clientfd, ERROR_RECV_FILENAME);
    }
}

void send_torrent(int clientfd)
{
    char * filename = recv_msg(clientfd);
    if (filename != NULL) // got file
    {
        printf("Filename: %s\n", filename);
        FILE * f = fopen(filename, "r");
        if (f != NULL)
        {
            send_msg(clientfd, SUCCESS);

            int size_int = send_file_size(clientfd, f);
            
            char * msg = recv_msg(clientfd);
            if (strcmp(msg, PROCEED) == 0)
                fread_send(clientfd, f, size_int);
            else
                printf("Error: %s\n", msg);
            free(msg);
        }
        else
        {
            send_msg(clientfd, FILE_NOT_FOUND);
        }
    }
    else
    {
        send_msg(clientfd, ERROR_RECV_FILENAME);
        return;
    }
    free(filename);
}

void updt_torrent()
{
    not_implemented();
}

// Recv and process request
void recv_proccess(int clientfd)
{
    char * msg = recv_msg(clientfd);
    if (msg == NULL)
    {
        printf("Client Crashed\n");
        return;
    }
    if (strcmp(msg, "RECV") == 0)
    {
        free(msg);
        msg = recv_msg(clientfd); // Get File Name
        recv_write_file2(clientfd, msg, O_CREAT | O_RDONLY);
        printf("File recv successfully\n");
    }
    else if (strcmp(msg, "SEND") == 0)
    {
        free(msg);
        msg = recv_msg(clientfd); // Get File Name
        if (access(msg, F_OK) == 0) // file exist
        {
            send_msg(clientfd, SUCCESS);
            send_file2(clientfd, msg);
        }
        else
        {
            send_msg(clientfd, ERROR);
        }
    }
    else if (strcmp(msg, "UPDT") == 0)
    {
        updt_torrent();
    }
    else
    {
        send_msg(clientfd, UNKNOWN_OPTION);
    }
}


#endif
