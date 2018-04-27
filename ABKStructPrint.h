#ifndef ABKSTRUCTPRINT_H
#define ABKSTRUCTPRINT_H

#include "ABKTorrent.h"


void print_torrentinfo(TorrentInfo t_i)
{
    printf("Filename: %s\n", t_i.filename);
    printf("Size: %d\n", t_i.size);
    printf("Type: %s\n", t_i.type);
    printf("Number of seeders: %d\n", t_i.n_seeders);
    for (int i = 0; i < t_i.n_seeders; i++)
    {
        printf("Address: %s:%s", t_i.seeders[i].ip, t_i.seeders[i].port);
        
        printf(" Progress: ");
        for (int j = 0; j < sizeof(t_i.seeders[i].progress) / sizeof(int); j++)
            printf("%d ", t_i.seeders[i].progress[j]);
        printf("\n");
    }
}

void print_dft(DownloadedFileTable * dft)
{
    if (dft == NULL) return;
    printf("Number of files downloaded: %d\n", dft->files_downloaded);
    for (int i = 0; i < dft->files_downloaded; i++)
    {
        printf("Filename: %s, Seeding: %d\n", dft->progresses[i].filename, dft->progresses[i].seeding);
        printf("Size: %d\n", dft->progresses[i].size);
        printf("Progress: ");
        for (int j = 0; j < 10; j++)
            printf("%d ", dft->progresses[i].progress[j]);
        printf("\n");
    }
}


#endif