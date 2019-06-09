#ifndef _H_IN_
#define _H_IN_

#include "rfs_util.h"

#include "IDL_rfs_conf.dsc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EPOLL_EVENT_COUNT	64

struct ListenSession
{
	int			listen_sock ;
} ;

struct AcceptedSession
{
	int			accepted_sock ;
	
	char			pathfilename[ (1<<16) + 1 ] ;
	int			pathfilename_len ;
	int			flags ;
	int			mode ;
	
	int			file_fd ;
	
	int			active_timestamp ;
	struct list_head	node_order_by_active_timestamp ;
} ;

int monitor( rfs_conf *p_rfs_conf );
int worker( rfs_conf *p_rfs_conf , int listen_sock , int accepting_mutex );

/*
 COMMAND : ropen
 VERSION : 1
 REQUEST : |'O':CHAR|'1':CHAR|pathfilename:L2VSTR|flags:INT4|
RESPONSE : |fd:INT4|errno:INT4|

 COMMAND : ropen3
 VERSION : 3
 REQUEST : |'O':CHAR|'3':CHAR|pathfilename:L2VSTR|flags:INT4|mode:INT4|
RESPONSE : |fd:INT4|errno:INT4|

 COMMAND : rclose
 VERSION : 1
 REQUEST : |'C':CHAR|'1':CHAR|fd:INT4|
RESPONSE : |ret:INT4|errno:INT4|

 COMMAND : rread
 VERSION : 1
 REQUEST : |'R':CHAR|'1':CHAR|fd:INT4|read_len:INT4|
RESPONSE : |buf:L4VSTR|errno:INT4|

 COMMAND : rwrite
 VERSION : 1
 REQUEST : |'W':CHAR|'1':CHAR|fd:INT4|buf:L4VSTR|
RESPONSE : |wrote_len:INT4|errno:INT4|
*/

int ropen( int accepted_sock , struct timeval *p_elapse , struct RemoteFileSession *p_session );
int ropen3( int accepted_sock , struct timeval *p_elapse , struct RemoteFileSession *p_session );
int rclose( int accepted_sock , struct timeval *p_elapse , struct RemoteFileSession *p_session );
int rread( int accepted_sock , struct timeval *p_elapse , struct RemoteFileSession *p_session );
int rwrite( int accepted_sock , struct timeval *p_elapse , struct RemoteFileSession *p_session );

#ifdef __cplusplus
}
#endif

#endif

