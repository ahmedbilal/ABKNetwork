
#ifndef ABKERROR_H
#define ABKERROR_H

#define SUCCESS "SUCCESS"
#define ERROR "ERROR"
#define LOOP "LOOP" // you got your own message
#define __ABK_TRANSACTION_COMPLETE__ "__ABK_TRANSACTION_COMPLETE__"
#include <stdio.h>
#include <errno.h> // errno, strerror()
#include <string.h>
#include <stdlib.h>


void dwerror()
{
    printf("%s\n", strerror(errno));
    exit(-1);
}

void ddwerror(char * s) // don't die with error
{
    printf("Error occurred in %s: %s\n", s, strerror(errno));
}


void not_implemented()
{
    printf("A function that is not implemented is being called\n");
    exit(1);
}


#endif