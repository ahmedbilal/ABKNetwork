
#ifndef ABKUTILITY_H
#define ABKUTILITY_H

#include "ABKError.h"
#include <stdio.h>
#include <time.h>

long get_file_size_f(FILE * f)
{
    if (f == NULL)
        return -1;
        
    int size = 0;
    int cur_loc = ftell(f); // record cur loc

    fseek(f, 0, SEEK_END); // seek to end
    size = ftell(f); // get file size
    fseek(f, cur_loc, SEEK_SET); // seek to cur loc

    return size;
}

long get_file_size_s(char * filename)
{
    FILE * f = fopen(filename, "r");
    if (f == NULL)
        return -1;
    int size = 0;
    int cur_loc = ftell(f); // record cur loc

    fseek(f, 0, SEEK_END); // seek to end
    size = ftell(f); // get file size
    fseek(f, cur_loc, SEEK_SET); // seek to cur loc

    return size;
}

char * filext(char * filename, int size)
{
    not_implemented();
    int i;
    for (int i = size - 1; i >= 0; i--)
        if (filename[i] == '.')
            break;
    
}

int buffer_to_file(char * buffer, int size, char * filename)
{
    FILE * f = fopen(filename, "w");
    fwrite(buffer, 1, size, f);
    fclose(f);
}

// create a string (256 byte long) containing stats of file
char * create_stat(char * file)
{
    char * file_stats = (char *)malloc(256);

    struct stat f_stat;
    stat(file, &f_stat);

    strcpy(file_stats, file); // abc.txt
    strcat(file_stats, " - "); // abc.txt -

    struct tm * c_time = localtime(&f_stat.st_ctim.tv_sec); // abc.txt - 12 Jan 2018
    char * c_time_s = asctime(c_time);
    c_time_s[strlen(c_time_s) - 1] = '\0';
    strcat(file_stats, c_time_s);
    strcat(file_stats, ", "); // abc.txt - 12 Jan 2018,

    char size[32] = { 0 };
    sprintf(size, "%ld", f_stat.st_size);
    strcat(file_stats, size); // abc.txt - 12 Jan 2018, 2048
    strcat(file_stats, ", ");// abc.txt - 12 Jan 2018, 2048,
    int mode = f_stat.st_mode;
    if (S_ISBLK(mode))
    {
        strcat(file_stats, "Block Special File");
    }
    else if (S_ISCHR(mode))
    {
        strcat(file_stats, "Character Special File");
    }
    else if (S_ISDIR(mode))
    {
        strcat(file_stats, "Directory");
    }
    else if (S_ISFIFO(mode))
    {
        strcat(file_stats, "Pipe or FIFO File");
    }
    else if (S_ISREG(mode))
    {
        strcat(file_stats, "Regular File");
    }
    else if (S_ISLNK(mode))
    {
        strcat(file_stats, "Symbolic Link");
    }
    else
    {
        strcat(file_stats, "Unknown Link");
    }
    return file_stats;
}


#endif