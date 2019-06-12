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
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <sys/uio.h>

#include "LOGC.h"

#ifdef __cplusplus
extern "C" {
#endif

extern char *__RFS_VERSION ;

/* general error */

#define RFS_ERROR_ALLOC			-11
#define RFS_ERROR_POLL			-16

#define RFS_ERROR_INTERNAL		-91

/* semaphore */

union semun {
	int              val;    /* Value for SETVAL */
	struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
	unsigned short  *array;  /* Array for GETALL, SETALL */
	struct seminfo  *__buf;  /* Buffer for IPC_INFO
	                           (Linux-specific) */
} ;

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
	if( (_elapse_).tv_sec < 0 ) \
	{ \
		(_elapse_).tv_sec = 0 ; \
		(_elapse_).tv_usec = 0 ; \
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

int RFSSendData( int sock , char *data , uint64_t data_len , uint64_t *p_sent_len , struct timeval *p_elapse );
int RFSReceiveData( int sock , char *data , uint64_t data_len , uint64_t *p_received_len , struct timeval *p_elapse );

int RFSSendInt4( int sock , int h4 , struct timeval *p_elapse );
int RFSReceiveInt4( int sock , int *p_h4 , struct timeval *p_elapse );

int RFSSendChar( int sock , char ch , struct timeval *p_elapse );
int RFSReceiveChar( int sock , char *p_ch , struct timeval *p_elapse );

int RFSSendString( int sock , char *buf , uint64_t data_len , struct timeval *p_elapse );
int RFSReceiveString( int sock , char *buf , uint64_t data_len , struct timeval *p_elapse );

int RFSSendL2VString( int sock , char *buf , int data_len , struct timeval *p_elapse );
int RFSReceiveL2VString( int sock , char *buf , int *p_data_len , struct timeval *p_elapse );

int RFSSendL4VString( int sock , char *buf , int data_len , struct timeval *p_elapse );
int RFSReceiveL4VString( int sock , char *buf , int *p_data_len , struct timeval *p_elapse );
int RFSReceiveL4VString_DUP( int sock , char **s_buf_ptr , int *p_data_len , struct timeval *p_elapse );

typedef void funcAdjustVectors( struct iovec *iov , struct iovec **pp_iov_ptr , int *p_iovcnt );
int RFSSendDataVectors( int sock , struct iovec *send_iov , struct iovec **pp_send_iov_ptr , int *p_send_iovcnt , funcAdjustVectors *pfuncAdjustSendVectors , struct timeval *p_elapse );
int RFSReceiveDataVectors( int sock , struct iovec *recv_iov , struct iovec **pp_recv_iov_ptr , int *p_recv_iovcnt , funcAdjustVectors *pfuncAdjustReceiveVectors , struct timeval *p_elapse );

/* log */

int RFSConvertLogLevelString( char *log_level_str );

/* file */

char *RFSDupFileContent( char *format , ... );

/* tcp */

void RFSSetTcpReuseAddr( int sock );
void RFSSetTcpNonblock( int sock );
void RFSSetTcpBlock( int sock );
void RFSSetTcpNodelay( int sock , int onoff );
void RFSSetTcpLinger( int sock , int onoff , int linger );
void RFSSetTcpCloseExec( int sock );

#ifdef __cplusplus
}
#endif

#endif

