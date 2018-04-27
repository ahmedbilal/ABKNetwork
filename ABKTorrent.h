#ifndef ABKTORRENT_H
#define ABKTORRENT_H

#include "ABKUtility.h"
#include "ABKUnits.h"


#define PROCEED "PROCEED"
#define TORRENT_BIG_UNREAL "TORRENT_BIG_UNREAL"
#define EMPTY_TORRENT_FILE "EMPTY_TORRENT_FILE"
#define SUCCESS "SUCCESS"
#define ERROR_RECV_TORRENT "ERROR_RECV_TORRENT"
#define ERROR_RECV_FILENAME "ERROR_RECV_FILENAME"
#define FILE_NOT_FOUND "FILE_NOT_FOUND"
#define UNKNOWN_OPTION "UNKNOWN_OPTION"

int return_multiplier(long s)
{
    int multiplier;
    if (s <= 1 * MB)
    {
        multiplier = 1;
    }
    else if (s <= 512 * MB)
    {
        multiplier = 5;
    }
    else
    {
        multiplier = 10;
    }
    return multiplier;
}

typedef struct {
    char ip[32 + 1];
    char port[5 + 1];
    int progress[10]; 
} SeederInfo;

typedef struct {
    char filename[255 + 1];
    long size;
    char type[32];
    int n_seeders;
    int portions;
    SeederInfo * seeders;
} TorrentInfo;


int make_torrent(char * filename, char * type, char * ip, char * port, int progress[])
{
    TorrentInfo t_i;
    strncpy(t_i.filename, filename, sizeof(t_i.filename));
    t_i.size = get_file_size_s(filename);
    if (t_i.size == -1){
        printf("File \"%s\" does not exist.\n", filename);
        return -1;
    }
    strncpy(t_i.type, type, sizeof(t_i.type));
    t_i.n_seeders = 1;
    t_i.seeders = (SeederInfo *)malloc(sizeof(SeederInfo));
    strncpy(t_i.seeders[0].ip, ip, sizeof(t_i.seeders[0].ip));
    strncpy(t_i.seeders[0].port, port, sizeof(t_i.seeders[0].port));

    char torrent_filename[strlen(filename) + strlen(".torrent")];
    strcpy(torrent_filename, filename);
    strcat(torrent_filename, ".torrent");

    FILE * f = fopen(torrent_filename, "wb");
    fprintf(f, "%s\n%ld\n%s\n%d\n%s %s ", t_i.filename, t_i.size, t_i.type, t_i.n_seeders,
            ip, port);

    for (int i = 0; i < 10; i++)
    {
        if (i != 9)
            fprintf(f, "%d ", progress[i]);
        else
            fprintf(f, "%d", progress[i]);
    }
    fprintf(f, "\n");
    fclose(f);

    return 0;
}

TorrentInfo * read_torrent(char * filename)
{
    TorrentInfo * t_i = (TorrentInfo *)malloc(sizeof(TorrentInfo));
    FILE * f = fopen(filename, "rb");
    if (f == NULL)
    {
        printf("File %s does not exit.\n", filename);
        return NULL;
    }

    fscanf(f, "%s\n%ld\n%s\n%d\n", t_i->filename, &t_i->size, t_i->type, &t_i->n_seeders);
    t_i->seeders = (SeederInfo *)malloc(sizeof(SeederInfo) * t_i->n_seeders);

    for (int i = 0; i < t_i->n_seeders; i++)
    {
        fscanf(f, "%s %s %d", t_i->seeders[i].ip, t_i->seeders[i].port, &t_i->portions);
        for (int j = 0; j < 10; j++)
            fscanf(f, " %d ", &t_i->seeders[i].progress[j]);
        fscanf(f, "\n");
    }
    
    fread(&t_i, sizeof(TorrentInfo) + sizeof(SeederInfo) - sizeof(void *), 1, f);
    fclose(f);
    return t_i;
}

typedef struct {
    char filename [255 + 1];
    int progress[10];
    long size;
    int seeding;
} FileProgress;

typedef struct {
    int files_downloaded;
    FileProgress * progresses;
} DownloadedFileTable;




int write_dft(DownloadedFileTable dft)
{
    FILE * f = fopen("progress.txt", "wb");
    if (f == NULL) return -1;

    fprintf(f, "%d\n", dft.files_downloaded);
    for (int i = 0; i < dft.files_downloaded; i++)
    {
        fprintf(f, "%s", dft.progresses[i].filename);
        fprintf(f, " %d", dft.progresses[i].seeding);
        fprintf(f, " %ld", dft.progresses[i].size);
        for (int j = 0; j < 10; j++)
            fprintf(f, " %d", dft.progresses[i].progress[j]);
        fprintf(f, "\n");
    }
    fclose(f);
    return 0;
}

DownloadedFileTable * get_dft()
{
    FILE * f = fopen("progress.txt", "rb");
    int size = get_file_size_f(f);

    if (size == -1) return NULL;

    DownloadedFileTable * dft = NULL;

    if (size != 0)
    {
        dft = (DownloadedFileTable *)malloc(sizeof(DownloadedFileTable));
        fscanf(f, "%d\n", &dft->files_downloaded);
        dft->progresses = (FileProgress *)malloc(dft->files_downloaded * sizeof(FileProgress));

        for (int i = 0; i < dft->files_downloaded; i++)
        {
            fscanf(f, "%s", &dft->progresses[i].filename);
            fscanf(f, " %d", &dft->progresses[i].seeding);
            fscanf(f, " %ld", &dft->progresses[i].size);
            for (int j = 0; j < 10; j++)
                fscanf(f, " %d", &dft->progresses[i].progress[j]);
            fscanf(f, "\n");
        }
    }
    return dft;
}

int find_entry_dft(char * filename, DownloadedFileTable * dft)
{
    if (dft == NULL) return -1;
    for (int i = 0; i < dft->files_downloaded; i++)
        if (strcmp(dft->progresses[i].filename, filename) == 0)
            return i;
    return -1;
}

int set_progress(char * torrent, int chunk, int p)
{
    TorrentInfo * t_i = read_torrent(torrent);
    if (t_i == NULL)
    {
        printf("Torrent File could not be read.\n");
        return -1;
    }
    // printf("Torrent: %s\n", torrent);
    FILE * f = fopen("progress.txt", "rb");
    DownloadedFileTable * dft = get_dft();

    if (f == NULL || dft == NULL)
    {
        dft = (DownloadedFileTable *) malloc(sizeof(DownloadedFileTable));
        f = fopen("progress.txt", "wb");
        printf("Creating progress file.\n");
        dft->files_downloaded = 1;
        dft->progresses = (FileProgress *)malloc(sizeof(FileProgress));
        strcpy(dft->progresses[0].filename, t_i->filename);
        memset(dft->progresses[0].progress, 0, sizeof(dft->progresses[0].progress));
        dft->progresses[0].progress[chunk] = p;
        dft->progresses[0].seeding = 1;
        dft->progresses[0].size = t_i->size;
        write_dft(*dft);
        return 0;
    }

    for (int i = 0; i < dft->files_downloaded; i++)
    {
        if (strcmp(dft->progresses[i].filename, t_i->filename) == 0)
        {
            dft->progresses[i].progress[chunk] = p;
            write_dft(*dft);
            return 0;
        }
    }
    dft->progresses = (FileProgress *)realloc(dft->progresses, sizeof(FileProgress) * (dft->files_downloaded + 1));
    strcpy(dft->progresses[dft->files_downloaded].filename, t_i->filename);
    memset(dft->progresses[dft->files_downloaded].progress, 0, sizeof(dft->progresses[dft->files_downloaded].progress));
    dft->progresses[dft->files_downloaded].progress[chunk] = p;
    dft->progresses[dft->files_downloaded].seeding = 1;
    dft->files_downloaded++;
    write_dft(*dft);
    return 0;
}

//void seed_torrent(int port)
//{
//    int serverfd = CreateTCPServer("127.0.0.1", port);
//
//    while (serverfd > 0)
//    {
//        int peerfd = accept(serverfd, NULL, NULL);
//        char * torrent = recv_msg(peerfd);
//        TorrentInfo * ti = read_torrent(torrent);
//
//        if (access(torrent, F_OK) == 0)
//        {
//
//        }
//    }
//}

#endif
