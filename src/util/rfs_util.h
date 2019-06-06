#ifndef _H_RFS_UTIL_
#define _H_RFS_UTIL_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <poll.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdint.h>
#include <arpa/inet.h>

#include "LOGC.h"

#ifdef __cplusplus
extern "C" {
#endif

/* general error */

#define RFS_ERROR_ALLOC			-11
#define RFS_ERROR_POLL			-16

#define RFS_ERROR_INTERNAL		-91

/* time */

#define DEF_TIMEVAL_FOR_DIFF \
	struct timeval	begin_timeval , end_timeval , diff_timeval ; \

#define GET_BEGIN_TIMEVAL \
	gettimeofday( & begin_timeval , NULL ); \

#define GET_END_TIMEVAL_AND_DIFF \
	gettimeofday( & end_timeval , NULL ); \
	diff_timeval.tv_sec = end_timeval.tv_sec - begin_timeval.tv_sec ; \
	diff_timeval.tv_usec = end_timeval.tv_usec - begin_timeval.tv_usec ; \
	while( diff_timeval.tv_usec < 0 ) \
	{ \
		diff_timeval.tv_sec--; \
		diff_timeval.tv_usec += 1000000 ; \
	} \

#define REDUCE_TIMEVAL(_elapse_,_reduce_) \
	(_elapse_).tv_sec -= (_reduce_).tv_sec ; \
	(_elapse_).tv_usec -= (_reduce_).tv_usec ; \
	while( (_elapse_).tv_usec < 0 ) \
	{ \
		(_elapse_).tv_sec--; \
		(_elapse_).tv_usec += 1000000 ; \
	} \

#define DIFF_TIMEVAL	diff_timeval

#define TIMEVAL_TO_MILLISECONDS(_timeval_,_milliseconds_)	(_milliseconds_)=(_timeval_).tv_sec*1000+(_timeval_).tv_usec/1000;

#define SECONDS_TO_TIMEVAL(_seconds_,_timeval_)			(_timeval_).tv_sec=(_seconds_),(_timeval_).tv_usec=0;

/* comm */

#define RFS_ERROR_SENDING		-31
#define RFS_ERROR_SENDING_2		-32
#define RFS_ERROR_SENDING_TIMEOUT	-34
#define RFS_ERROR_RECEIVING		-41
#define RFS_ERROR_RECEIVING_2		-42
#define RFS_ERROR_SRECEIVING_TIMEOUT	-44
#define RFS_ERROR_CLOSED		-46
#define RFS_INFO_CLOSED			46

int RFSSendInt4( int sock , int h4 , struct timeval *p_elapse );
int RFSReceiveInt4( int sock , int *p_h4 , struct timeval *p_elapse );

int RFSSendChar( int sock , char ch , struct timeval *p_elapse );
int RFSReceiveChar( int sock , char *p_ch , struct timeval *p_elapse );

int RFSSendString( int sock , char *buf , uint64_t data_len , struct timeval *p_elapse );
int RFSReceiveString( int sock , char *buf , uint64_t data_len , struct timeval *p_elapse );

int RFSSendL2VString( int sock , char *buf , int data_len , struct timeval *p_elapse );
int RFSReceiveL2VString( int sock , char *buf , int *p_data_len , struct timeval *p_elapse );

int RFSSendL4VString( int sock , char *buf , int data_len , struct timeval *p_elapse );
int RFSReceiveL4VString_DUP( int sock , char **s_buf_ptr , int *p_data_len , struct timeval *p_elapse );




#ifdef __cplusplus
}
#endif

#endif

