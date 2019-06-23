#ifndef _H_IN_
#define _H_IN_

#include "rfs_util.h"

#include "IDL_rfs_conf.dsc.h"

#ifdef __cplusplus
extern "C" {
#endif

struct LocalFd
{
	int		local_fd ;
	struct rb_node	local_fd_rbnode_by_order_by_local_fd ;
} ;

struct LocalFds
{
	struct rb_root	local_fds_rbtree_order_by_local_fd ;
} ;

int LinkLocalFdsTreeNodeByLocalFd( struct LocalFds *p_local_fds , struct LocalFd *p_local_fd );
struct LocalFd *QueryLocalFdsTreeNodeByLocalFd( struct LocalFds *p_local_fds , struct LocalFd *p_local_fd );
void UnlinkLocalFdsTreeNodeByLocalFd( struct LocalFds *p_local_fds , struct LocalFd *p_local_fd );
struct LocalFd *TravelLocalFdsTreeByLocalFd( struct LocalFds *p_local_fds , struct LocalFd *p_local_fd );
void DestroyLocalFdsTree( struct LocalFds *p_local_fds );

extern char	g_rfs_conf_filename[ PATH_MAX ] ;
extern char	g_rfs_conf_main_filename[ PATH_MAX ] ;

int monitor( rfs_conf *p_rfs_conf );
int worker( rfs_conf *p_rfs_conf , int process_index , int listen_sock , int accepting_mutex );
int comm( rfs_conf *p_rfs_conf , int process_index , int accepted_sock );

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

 COMMAND : reof
 VERSION : 1
 REQUEST : |'C':CHAR|'1':CHAR|fd:INT4|
RESPONSE : |ret:INT4|errno:INT4|
*/

int ropen( int accepted_sock , struct LocalFds *p_local_fds , struct timeval *p_elapse );
int ropen3( int accepted_sock , struct LocalFds *p_local_fds , struct timeval *p_elapse );
int rclose( int accepted_sock , struct LocalFds *p_local_fds , struct timeval *p_elapse );

int rread( int accepted_sock , struct LocalFds *p_local_fds , struct timeval *p_elapse );
int rwrite( int accepted_sock , struct LocalFds *p_local_fds , struct timeval *p_elapse );

int reof( int accepted_sock , struct LocalFds *p_local_fds , struct timeval *p_elapse );

#ifdef __cplusplus
}
#endif

#endif

