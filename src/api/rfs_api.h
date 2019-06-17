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
int rclose( int fd );

ssize_t rread( int fd , void *buf , size_t count );
ssize_t rwrite( int fd , char *buf , size_t count );

FILE *rfopen( char *pathfilename , char *mode );
int rfclose( FILE *fp );

size_t rfread( void *ptr , size_t size , size_t nmemb , FILE *fp );
size_t rfwrite( void *ptr , size_t size , size_t nmemb , FILE *fp );
int rfeof( FILE *fp );

#ifdef __cplusplus
}
#endif

#endif

