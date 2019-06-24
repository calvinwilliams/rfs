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

struct LocalFp
{
	FILE		*local_fp ;
	struct rb_node	local_fp_rbnode_by_order_by_local_fp ;
} ;

struct LocalFps
{
	struct rb_root	local_fps_rbtree_order_by_local_fp ;
} ;

int LinkLocalFpsTreeNodeByLocalFp( struct LocalFps *p_local_fps , struct LocalFp *p_local_fp );
struct LocalFp *QueryLocalFpsTreeNodeByLocalFp( struct LocalFps *p_local_fps , struct LocalFp *p_local_fp );
void UnlinkLocalFpsTreeNodeByLocalFp( struct LocalFps *p_local_fps , struct LocalFp *p_local_fp );
struct LocalFp *TravelLocalFpsTreeByLocalFp( struct LocalFps *p_local_fps , struct LocalFp *p_local_fp );
void DestroyLocalFpsTree( struct LocalFps *p_local_fps );

extern char	g_rfs_conf_filename[ PATH_MAX ] ;
extern char	g_rfs_conf_main_filename[ PATH_MAX ] ;

int monitor( rfs_conf *p_rfs_conf );
int worker( rfs_conf *p_rfs_conf , int process_index , int listen_sock , int accepting_mutex );
int comm( rfs_conf *p_rfs_conf , int process_index , int accepted_sock );

/*
 COMMAND : ropen
 VERSION : 1
 REQUEST : |"open":STRING8|'1':CHAR|pathfilename:L2VSTR|flags:INT4|
RESPONSE : |fd:INT4|errno:INT4|

 COMMAND : ropen3
 VERSION : 3
 REQUEST : |"open3":STRING8|'1':CHAR|pathfilename:L2VSTR|flags:INT4|mode:INT4|
RESPONSE : |fd:INT4|errno:INT4|

 COMMAND : rclose
 VERSION : 1
 REQUEST : |"close":STRING8|'1':CHAR|fd:INT4|
RESPONSE : |ret:INT4|errno:INT4|

 COMMAND : rread
 VERSION : 1
 REQUEST : |"read":STRING8|'1':CHAR|fd:INT4|read_len:INT4|
RESPONSE : |buf:L4VSTR|errno:INT4|

 COMMAND : rwrite
 VERSION : 1
 REQUEST : |"write":STRING8|'1':CHAR|fd:INT4|buf:L4VSTR|
RESPONSE : |wrote_len:INT4|errno:INT4|
*/

int rfs_open( int accepted_sock , struct LocalFds *p_local_fds , struct LocalFps *p_local_fps , struct timeval *p_elapse );
int rfs_open3( int accepted_sock , struct LocalFds *p_local_fds , struct LocalFps *p_local_fps , struct timeval *p_elapse );
int rfs_close( int accepted_sock , struct LocalFds *p_local_fds , struct LocalFps *p_local_fps , struct timeval *p_elapse );

int rfs_read( int accepted_sock , struct LocalFds *p_local_fds , struct LocalFps *p_local_fps , struct timeval *p_elapse );
int rfs_write( int accepted_sock , struct LocalFds *p_local_fds , struct LocalFps *p_local_fps , struct timeval *p_elapse );

/*
 COMMAND : rfopen
 VERSION : 1
 REQUEST : |"fopen":STRING8|'1':CHAR|pathfilename:L2VSTR|mode:L1VSTR|
RESPONSE : |fp:INT8|errno:INT4|

 COMMAND : rfclose
 VERSION : 1
 REQUEST : |"fclose":STRING8|'1':CHAR|fp:INT8|
RESPONSE : |ret:INT4|errno:INT4|

 COMMAND : rfread
 VERSION : 1
 REQUEST : |"fread":STRING8|'1':CHAR|size:INT4|nmemb:INT4|fp:INT8|
RESPONSE : |buf:L4VSTR|errno:INT4|

 COMMAND : rfwrite
 VERSION : 1
 REQUEST : |"fwrite":STRING8|'1':CHAR|buf:L4VSTR|size:INT4|nmemb:INT4|fp:INT8|
RESPONSE : |ret:INT4|errno:INT4|

 COMMAND : rfgets
 VERSION : 1
 REQUEST : |"fread":STRING8|'1':CHAR|size:INT4|fp:INT8|
RESPONSE : |buf:L4VSTR|errno:INT4|

 COMMAND : rfputs
 VERSION : 1
 REQUEST : |"fwrite":STRING8|'1':CHAR|buf:L4VSTR|size:INT4|fp:INT8|
RESPONSE : |ret:INT4|errno:INT4|

 COMMAND : rfgetc
 VERSION : 1
 REQUEST : |"fread":STRING8|'1':CHAR|fp:INT8|
RESPONSE : |s:INT4|errno:INT4|

 COMMAND : rfputc
 VERSION : 1
 REQUEST : |"fwrite":STRING8|'1':CHAR|s:INT4|fp:INT8|
RESPONSE : |ret:INT4|errno:INT4|
*/

int rfs_fopen( int accepted_sock , struct LocalFds *p_local_fds , struct LocalFps *p_local_fps , struct timeval *p_elapse );
int rfs_fclose( int accepted_sock , struct LocalFds *p_local_fds , struct LocalFps *p_local_fps , struct timeval *p_elapse );

int rfs_fread( int accepted_sock , struct LocalFds *p_local_fds , struct LocalFps *p_local_fps , struct timeval *p_elapse );
int rfs_fwrite( int accepted_sock , struct LocalFds *p_local_fds , struct LocalFps *p_local_fps , struct timeval *p_elapse );

int rfs_fgets( int accepted_sock , struct LocalFds *p_local_fds , struct LocalFps *p_local_fps , struct timeval *p_elapse );
int rfs_fputs( int accepted_sock , struct LocalFds *p_local_fds , struct LocalFps *p_local_fps , struct timeval *p_elapse );

int rfs_fgetc( int accepted_sock , struct LocalFds *p_local_fds , struct LocalFps *p_local_fps , struct timeval *p_elapse );
int rfs_fputc( int accepted_sock , struct LocalFds *p_local_fds , struct LocalFps *p_local_fps , struct timeval *p_elapse );

#ifdef __cplusplus
}
#endif

#endif

