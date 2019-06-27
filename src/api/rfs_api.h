#ifndef _H_RFS_API_
#define _H_RFS_API_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

int rsetconf( char *format , ... );
void rdisconnect();
int rconnect();

int ropen( char *pathfilename , int flags );
int ropen3( char *pathfilename , int flags , mode_t mode );
int rclose( int rfd );

ssize_t rread( int rfd , void *buf , size_t count );
ssize_t rwrite( int rfd , char *buf , size_t count );

FILE *rfopen( char *pathfilename , char *mode );
int rfclose( FILE *rfp );

size_t rfread( void *ptr , size_t size , size_t nmemb , FILE *rfp );
size_t rfwrite( void *ptr , size_t size , size_t nmemb , FILE *rfp );

char *rfgets( char *s , int size , FILE *fp );
int rfputs( char *s , FILE *fp );

int rfgetc( FILE *fp );
int rfputc( int c , FILE *fp );

int rfeof( FILE *rfp );

#ifdef __cplusplus
}
#endif

#endif

