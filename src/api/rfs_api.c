#include "rfs_util.h"
#include "rfs_api.h"

#include "IDL_rfs_api_conf.dsc.h"

#define RFSAPI_COMM_CONNECTING_TIMEOUT_DEFAULT		5

#define RFSAPI_CONNECT_SOCK_MAX		8

__thread char		g_rfs_api_conf_filename[ PATH_MAX ] = "rfs_api.conf" ;
__thread rfs_api_conf	*g_p_rfs_api_conf = NULL ;
__thread int		g_rfd_api_conf_fd = -1 ;

__thread struct pollfd	g_connect_pollfds[ RFSAPI_CONNECT_SOCK_MAX ] ;
__thread char		g_is_connected[ RFSAPI_CONNECT_SOCK_MAX ] ; ;
__thread int		g_connecting_sock_count ;
__thread int		g_connected_sock_count ;

struct RemoteFd
{
	int		rfd ;
	int		remote_fd[ RFSAPI_CONNECT_SOCK_MAX ] ;
	struct rb_node	remote_fd_rbnode_by_order_by_rfd ;
} ;

struct RemoteFds
{
	struct rb_root	remote_fds_rbtree_order_by_rfd ;
} ;

__thread struct RemoteFds	g_remote_fds ;

LINK_RBTREENODE_INT( LinkRemoteFdsTreeNodeByRfd , struct RemoteFds , remote_fds_rbtree_order_by_rfd , struct RemoteFd , remote_fd_rbnode_by_order_by_rfd , rfd )
QUERY_RBTREENODE_INT( QueryRemoteFdsTreeNodeByRfd , struct RemoteFds , remote_fds_rbtree_order_by_rfd , struct RemoteFd , remote_fd_rbnode_by_order_by_rfd , rfd )
UNLINK_RBTREENODE( UnlinkRemoteFdsTreeNodeByRfd , struct RemoteFds , remote_fds_rbtree_order_by_rfd , struct RemoteFd , remote_fd_rbnode_by_order_by_rfd )
TRAVEL_RBTREENODE( TravelRemoteFdsTreeRfd , struct RemoteFds , remote_fds_rbtree_order_by_rfd , struct RemoteFd , remote_fd_rbnode_by_order_by_rfd )
DESTROY_RBTREE( DestroyRemoteFdsTreeByRfd , struct RemoteFds , remote_fds_rbtree_order_by_rfd , struct RemoteFd , remote_fd_rbnode_by_order_by_rfd , FREE_RBTREENODEENTRY_DIRECTLY )

int LinkRemoteFdsTreeNodeByRfd( struct RemoteFds *p_remote_fds , struct RemoteFd *p_remote_fd );
struct RemoteFd *QueryRemoteFdsTreeNodeByRfd( struct RemoteFds *p_remote_fds , struct RemoteFd *p_remote_fd );
void UnlinkRemoteFdsTreeNodeByRfd( struct RemoteFds *p_remote_fds , struct RemoteFd *p_remote_fd );
struct RemoteFd *TravelRemoteFdsTreeRfd( struct RemoteFds *p_remote_fds , struct RemoteFd *p_remote_fd );
void DestroyRemoteFdsTreeByRfd( struct RemoteFds *p_remote_fds );

int rsetconf( char *format , ... )
{
	va_list		valist ;
	int		filename_len ;
	
	memset( g_rfs_api_conf_filename , 0x00 , sizeof(g_rfs_api_conf_filename) );
	va_start( valist , format );
	filename_len = vsnprintf( g_rfs_api_conf_filename , sizeof(g_rfs_api_conf_filename)-1 , format , valist ) ;
	va_end( valist );
	if( filename_len == -1 || filename_len >= sizeof(g_rfs_api_conf_filename)-1 )
		return -1;
	
	return 0;
}

static int rloadconfig()
{
	char		rfs_api_conf_main_filename[ PATH_MAX ] ;
	char		*p = NULL ;
	char		*file_content = NULL ;
	int		i ;
	
	int		nret = 0 ;
	
	strcpy( rfs_api_conf_main_filename , g_rfs_api_conf_filename );
	p = strchr( rfs_api_conf_main_filename , '.' ) ;
	if( p )
		(*p) = '\0' ;
	SetLogcFile( "%s/log/%s.log" , getenv("HOME") , rfs_api_conf_main_filename );
	SetLogcLevel( LOGCLEVEL_ERROR );
	
	file_content = RFSDupFileContent( & g_rfd_api_conf_fd , "%s/etc/%s" , getenv("HOME") , g_rfs_api_conf_filename ) ;
	if( file_content == NULL )
	{
		printf( "*** ERROR : can't open config file[%s/etc/%s] , errno[%d]\n" , getenv("HOME") , g_rfs_api_conf_filename , errno );
		exit(1);
	}
	
	g_p_rfs_api_conf = (rfs_api_conf*)malloc( sizeof(rfs_api_conf) ) ;
	if( g_p_rfs_api_conf == NULL )
		return -1;
	memset( g_p_rfs_api_conf , 0x00 , sizeof(rfs_api_conf) );
	nret = DSCDESERIALIZE_JSON_rfs_api_conf( NULL , file_content , NULL , g_p_rfs_api_conf ) ;
	if( nret )
	{
		ERRORLOGC( "*** ERROR : parse config[%s] failed[%d]\n" , file_content , nret );
		free( file_content );
		free( g_p_rfs_api_conf ); g_p_rfs_api_conf = NULL ;
		return -1;
	}
	
	free( file_content );
	
	/*
	{
	int		i , j ;
	for( i = 0 ; i < g_p_rfs_api_conf->_nodes_count ; i++ )
	{
		printf( "nodes[%d].id[%s]\n" , i , g_p_rfs_api_conf->servers[i].id );
		printf( "nodes[%d].user[%s]\n" , i , g_p_rfs_api_conf->servers[i].user );
		printf( "nodes[%d].pass[%s]\n" , i , g_p_rfs_api_conf->servers[i].pass );
		for( j = 0 ; j < g_p_rfs_api_conf->servers[i]._servers_count ; j++ )
		{
			printf( "nodes[%d].servers[%d].ip[%s]\n" , i , j , g_p_rfs_api_conf->servers[i].servers[j].ip );
			printf( "nodes[%d].servers[%d].port[%d]\n" , i , j , g_p_rfs_api_conf->servers[i].servers[j].port );
		}
	}
	}
	*/
	
	if( g_p_rfs_api_conf->comm.connecting_timeout <= 0 )
		g_p_rfs_api_conf->comm.connecting_timeout = RFSAPI_COMM_CONNECTING_TIMEOUT_DEFAULT ;
	
	memset( g_connect_pollfds , 0x00 , sizeof(g_connect_pollfds) );
	for( i = 0 ; i < g_p_rfs_api_conf->_servers_count ; i++ )
		g_connect_pollfds[i].fd = -1 ;
	g_connecting_sock_count = 0 ;
	g_connected_sock_count = 0 ;
	memset( & g_remote_fds , 0x00 , sizeof(struct RemoteFds) );
	
	SetLogcLevel( RFSConvertLogLevelString(g_p_rfs_api_conf->log.log_level) );
	
	return 0;
}

void rdisconnect()
{
	int		i ;
	
	for( i = 0 ; i < g_p_rfs_api_conf->_servers_count ; i++ )
	{
		if( g_connect_pollfds[i].fd != -1 )
		{
			INFOLOGC( "close sock[%d]" , g_connect_pollfds[i].fd )
			close( g_connect_pollfds[i].fd ); g_connect_pollfds[i].fd = -1 ;
			
			if( g_is_connected[i] == 0 )
			{
				g_connecting_sock_count--;
			}
			else
			{
				g_connected_sock_count--;
			}
		}
	}
	
	return;
}

int rconnect()
{
	int			i ;
	struct timeval		elapse ;
	int			timeout ;
	DEF_TIMEVAL_FOR_DIFF
	
	int			nret = 0 ;
	
	for( i = 0 ; i < g_p_rfs_api_conf->_servers_count ; i++ )
	{
		if( g_connect_pollfds[i].fd == -1 )
		{
			struct sockaddr_in	connect_addr ;
			
			g_is_connected[i] = 0 ;
			g_connect_pollfds[i].events = 0 ;
			g_connect_pollfds[i].revents = 0 ;
			
			g_connect_pollfds[i].fd = socket( AF_INET , SOCK_STREAM , IPPROTO_TCP ) ;
			if( g_connect_pollfds[i].fd == -1 )
			{
				ERRORLOGC( "socket failed[%d] , errno[%d]" , g_connect_pollfds[i].fd , errno )
				return -1;
			}
			else
			{
				DEBUGLOGC( "socket ok , sock[%d]" , g_connect_pollfds[i].fd )
			}
			
			RFSSetTcpNonblock( g_connect_pollfds[i].fd );
			
			memset( & connect_addr , 0x00 , sizeof(struct sockaddr_in) );
			connect_addr.sin_family = AF_INET ;
			if( g_p_rfs_api_conf->servers[i].ip[0] == '\0' )
				connect_addr.sin_addr.s_addr = INADDR_ANY ;
			else
				connect_addr.sin_addr.s_addr = inet_addr(g_p_rfs_api_conf->servers[i].ip) ;
			connect_addr.sin_port = htons( (unsigned short)(g_p_rfs_api_conf->servers[i].port) );
			nret = connect( g_connect_pollfds[i].fd , (struct sockaddr *) & (connect_addr) , sizeof(struct sockaddr) ) ;
			if( nret == -1 )
			{
				if( errno == EINPROGRESS )
				{
					g_connect_pollfds[i].events = POLLOUT ;
					g_connecting_sock_count++;
					INFOLOGC( "connect[%s:%d] sock[%d] progressing" , g_p_rfs_api_conf->servers[i].ip , g_p_rfs_api_conf->servers[i].port , g_connect_pollfds[i].fd )
				}
				else
				{
					ERRORLOGC( "connect[%s:%d] sock[%d] failed[%d] , errno[%d]" , g_p_rfs_api_conf->servers[i].ip , g_p_rfs_api_conf->servers[i].port , g_connect_pollfds[i].fd , nret , errno )
				}
			}
			else
			{
				INFOLOGC( "connect[%s:%d] sock[%d] ok" , g_p_rfs_api_conf->servers[i].ip , g_p_rfs_api_conf->servers[i].port , g_connect_pollfds[i].fd )
				g_is_connected[i] = 1 ;
				g_connected_sock_count++;
			}
			
		}
	}
	if( g_connecting_sock_count == 0 && g_connected_sock_count == 0 )
		return -1;
	
	SECONDS_TO_TIMEVAL( g_p_rfs_api_conf->comm.connecting_timeout , elapse )
	
	while(1)
	{
		if( g_connecting_sock_count == 0 )
			break;
		
		GET_BEGIN_TIMEVAL
		TIMEVAL_TO_MILLISECONDS( elapse , timeout )
		nret = poll( g_connect_pollfds , g_p_rfs_api_conf->_servers_count , timeout ) ;
		GET_END_TIMEVAL_AND_DIFF
		REDUCE_TIMEVAL( elapse , DIFF_TIMEVAL )
		if( nret == -1 )
		{
			FATALLOGC( "poll failed[%d] , errno[%d]" , nret , errno )
			return -1;
		}
		else if( nret == 0 )
		{
			ERRORLOGC( "poll timeout , errno[%d]" , errno )
			
			for( i = 0 ; i < g_p_rfs_api_conf->_servers_count ; i++ )
			{
				if( g_connect_pollfds[i].fd != -1 )
				{
					if( g_connect_pollfds[i].events == POLLOUT )
					{
						ERRORLOGC( "close sock[%d] for connecting timeout" , g_connect_pollfds[i].fd )
						close( g_connect_pollfds[i].fd ); g_connect_pollfds[i].fd = -1 ;
						g_connecting_sock_count--;
					}
				}
			}
		}
		else
		{
			INFOLOGC( "poll catch [%d]events" , nret )
			
			for( i = 0 ; i < g_p_rfs_api_conf->_servers_count ; i++ )
			{
				if( g_connect_pollfds[i].events == POLLOUT )
				{
					if( g_connect_pollfds[i].revents & POLLERR )
					{
						ERRORLOGC( "close sock[%d] for connecting error" , g_connect_pollfds[i].fd )
						close( g_connect_pollfds[i].fd ); g_connect_pollfds[i].fd = -1 ;
						g_connecting_sock_count--;
					}
					else if( g_connect_pollfds[i].revents & POLLOUT )
					{
						int		code , error ;
						socklen_t	addr_len ;
						
						addr_len = sizeof(int) ;
						code = getsockopt( g_connect_pollfds[i].fd , SOL_SOCKET , SO_ERROR , & error , & addr_len ) ;
						if( code < 0 || ( code == 0 && error ) )
						{
							ERRORLOGC( "connect2 sock[%d] failed" , g_connect_pollfds[i].fd )
							g_connecting_sock_count--;
						}
						else
						{
							INFOLOGC( "connect2 sock[%d] ok" , g_connect_pollfds[i].fd )
							g_is_connected[i] = 1 ;
							g_connecting_sock_count--;
							g_connected_sock_count++;
						}
					}
				}
			}
		}
	}
	
	if( g_connected_sock_count == 0 )
	{
		ERRORLOGC( "no one connected" )
		return -1;
	}
	
	return 0;
}

struct riovec
{
	struct iovec		send_iov[6][RFSAPI_CONNECT_SOCK_MAX] ;
	struct iovec		recv_iov[3][RFSAPI_CONNECT_SOCK_MAX] ;
	struct iovec		*p_send_iov_ptr[RFSAPI_CONNECT_SOCK_MAX] ;
	struct iovec		*p_recv_iov_ptr[RFSAPI_CONNECT_SOCK_MAX] ;
	int			send_iovcnt[RFSAPI_CONNECT_SOCK_MAX] ;
	int			recv_iovcnt[RFSAPI_CONNECT_SOCK_MAX] ;
	funcAdjustVectors	*pfuncAdjustSendVectors ;
	funcAdjustVectors	*pfuncAdjustReceiveVectors ;
} ;

static int rio( struct riovec *p_riovec , struct timeval *p_elapse )
{
	int		rfs_io_doing_count ;
	DEF_TIMEVAL_FOR_DIFF
	int		timeout ;
	int		i ;
	
	int		nret = 0 ;
	
	for( i = 0 ; i < g_p_rfs_api_conf->_servers_count ; i++ )
	{
		if( g_connect_pollfds[i].fd != -1 )
		{
			DEBUGLOGC( "set sock[%d] event POLLOUT" , g_connect_pollfds[i].fd )
			g_connect_pollfds[i].events = POLLOUT ;
			g_connect_pollfds[i].revents = 0 ;
		}
	}
	
	rfs_io_doing_count = g_connected_sock_count ;
	while(1)
	{
		if( g_connected_sock_count == 0 )
		{
			ERRORLOGC( "all connection borken" )
			return -1;
		}
		
		if( rfs_io_doing_count == 0 )
			break;
		
		GET_BEGIN_TIMEVAL
		TIMEVAL_TO_MILLISECONDS( (*p_elapse) , timeout )
		nret = poll( g_connect_pollfds , g_p_rfs_api_conf->_servers_count , timeout ) ;
		GET_END_TIMEVAL_AND_DIFF
		REDUCE_TIMEVAL( (*p_elapse) , DIFF_TIMEVAL )
		if( nret == -1 )
		{
			FATALLOGC( "poll failed[%d] , errno[%d]" , nret , errno )
			return -1;
		}
		else if( nret == 0 )
		{
			ERRORLOGC( "poll timeout , errno[%d]" , errno )
			rdisconnect();
			return -1;
		}
		
		for( i = 0 ; i < g_p_rfs_api_conf->_servers_count ; i++ )
		{
			if( g_connect_pollfds[i].fd != -1 )
			{
				if( g_connect_pollfds[i].revents & POLLERR )
				{
					ERRORLOGC( "close sock[%d] for net-io error" , g_connect_pollfds[i].fd )
					close( g_connect_pollfds[i].fd ); g_connect_pollfds[i].fd = -1 ;
					g_connected_sock_count--;
				}
				else if( g_connect_pollfds[i].revents == POLLOUT )
				{
					nret = RFSSendDataVectors( g_connect_pollfds[i].fd , p_riovec->send_iov[i] , & (p_riovec->p_send_iov_ptr[i]) , & (p_riovec->send_iovcnt[i]) , p_riovec->pfuncAdjustSendVectors , p_elapse ) ;
					if( nret == 1 )
					{
						DEBUGLOGC( "send sock[%d] again" , g_connect_pollfds[i].fd )
					}
					else if( nret == -1 )
					{
						ERRORLOGC( "send sock[%d] failed , errno[%d]" , g_connect_pollfds[i].fd , errno )
						close( g_connect_pollfds[i].fd ); g_connect_pollfds[i].fd = -1 ;
						g_connected_sock_count--;
					}
					else if( nret == 0 )
					{
						INFOLOGC( "send sock[%d] finish" , g_connect_pollfds[i].fd )
						g_connect_pollfds[i].events = POLLIN ;
					}
				}
				else if( g_connect_pollfds[i].revents == POLLIN )
				{
					nret = RFSReceiveDataVectors( g_connect_pollfds[i].fd , p_riovec->recv_iov[i] , & (p_riovec->p_recv_iov_ptr[i]) , & (p_riovec->recv_iovcnt[i]) , p_riovec->pfuncAdjustReceiveVectors , p_elapse ) ;
					if( nret == 1 )
					{
						DEBUGLOGC( "recv sock[%d] again" , g_connect_pollfds[i].fd )
					}
					else if( nret == -1 )
					{
						ERRORLOGC( "recv sock[%d] failed , errno[%d]" , g_connect_pollfds[i].fd , errno )
						close( g_connect_pollfds[i].fd ); g_connect_pollfds[i].fd = -1 ;
						g_connected_sock_count--;
					}
					else if( nret == 0 )
					{
						INFOLOGC( "recv sock[%d] finish" , g_connect_pollfds[i].fd )
						g_connect_pollfds[i].events = 0 ;
						rfs_io_doing_count--;
					}
				}
			}
		}
	}
	
	return 0;
}

int ropen( char *pathfilename , int flags_h )
{
	uint16_t	pathfilename_len_h ;
	uint16_t	pathfilename_len_n ;
	uint32_t	flags_n ;
	uint32_t	remote_fd_n[RFSAPI_CONNECT_SOCK_MAX] ;
	uint32_t	errno_n[RFSAPI_CONNECT_SOCK_MAX] ;
	struct riovec	riovec ;
	int		i ;
	struct timeval	elapse ;
	struct RemoteFd	*p_remote_fd = NULL ;
	signed char	ret ;
	
	int		nret = 0 ;
	
	INFOLOGC( "--- rfs_api v%s --- ropen[%s][%d]" , __RFS_VERSION , pathfilename , flags_h )
	
	if( g_p_rfs_api_conf == NULL )
	{
		nret = rloadconfig() ;
		if( nret )
			return nret;
	}
	
	if( g_connected_sock_count < g_p_rfs_api_conf->_servers_count )
	{
		nret = rconnect() ;
		if( nret < 0 )
			return nret;
	}
	
	pathfilename_len_h=(uint16_t)strlen(pathfilename);
	if( pathfilename_len_h > PATH_MAX-1 )
		return -1;
	pathfilename_len_n=htons(pathfilename_len_h);
	flags_n=htonl((uint32_t)flags_h);
	
	memset( & riovec , 0x00 , sizeof(struct riovec) );
	for( i = 0 ; i < g_p_rfs_api_conf->_servers_count ; i++ )
	{
		riovec.send_iov[i][0].iov_base = "O" ; riovec.send_iov[i][0].iov_len = 1 ;
		riovec.send_iov[i][1].iov_base = "1" ; riovec.send_iov[i][1].iov_len = 1 ;
		riovec.send_iov[i][2].iov_base = & pathfilename_len_n ; riovec.send_iov[i][2].iov_len = 2 ;
		riovec.send_iov[i][3].iov_base = pathfilename ; riovec.send_iov[i][3].iov_len = pathfilename_len_h ;
		riovec.send_iov[i][4].iov_base = & flags_n ; riovec.send_iov[i][4].iov_len = 4 ;
		riovec.p_send_iov_ptr[i] = riovec.send_iov[i] ;
		riovec.send_iovcnt[i] = 5 ;
		
		riovec.recv_iov[i][0].iov_base = & (remote_fd_n[i]) ; riovec.recv_iov[i][0].iov_len = 4 ;
		riovec.recv_iov[i][1].iov_base = & (errno_n[i]) ; riovec.recv_iov[i][1].iov_len = 4 ;
		riovec.p_recv_iov_ptr[i] = riovec.recv_iov[i] ;
		riovec.recv_iovcnt[i] = 2 ;
	}
	riovec.pfuncAdjustSendVectors = NULL ;
	riovec.pfuncAdjustReceiveVectors = NULL ;
	
	SECONDS_TO_TIMEVAL( 5 , elapse )
	nret = rio( & riovec , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "rio failed[%d]" , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "rio ok" )
	}
	
	p_remote_fd = (struct RemoteFd *)malloc( sizeof(struct RemoteFd) ) ;
	if( p_remote_fd == NULL )
	{
		FATALLOGC( "malloc failed , errno[%d]" , errno )
		return -1;
	}
	memset( p_remote_fd , 0x00 , sizeof(struct RemoteFd) );
	
	p_remote_fd->rfd = dup( g_rfd_api_conf_fd ) ;
	if( p_remote_fd->rfd == -1 )
	{
		FATALLOGC( "dup failed , errno[%d]" , errno )
		free( p_remote_fd );
		return -1;
	}
	
	for( i = 0 ; i < g_p_rfs_api_conf->_servers_count ; i++ )
	{
		p_remote_fd->rfd = -1 ;
	}
	
	nret = LinkRemoteFdsTreeNodeByRfd( & g_remote_fds , p_remote_fd ) ;
	if( nret )
	{
		ERRORLOGC( "LinkRemoteFdsTreeNodeByRfd failed , errno[%d] , rfd[%d]" , errno , p_remote_fd->rfd )
		free( p_remote_fd );
		return -1;
	}
	
	ret = 0 ;
	for( i = 0 ; i < g_p_rfs_api_conf->_servers_count ; i++ )
	{
		if( g_connect_pollfds[i].fd != -1 && g_is_connected[i] == 1 )
		{
			p_remote_fd->remote_fd[i] = ntohl((uint32_t)(remote_fd_n[i])) ;
			errno = ntohl((uint32_t)(errno_n[i])) ;
			if( p_remote_fd->remote_fd[i] == -1 )
			{
				ERRORLOGC( "[%d] file_fd[%d] errno[%d]" , i , p_remote_fd->remote_fd[i] , errno )
				ret = -1 ;
			}
			else
			{
				INFOLOGC( "[%d] file_fd[%d] errno[%d]" , i , p_remote_fd->remote_fd[i] , errno )
			}
		}
	}
	if( ret == -1 )
	{
		nret = rclose( p_remote_fd->rfd );
		if( nret )
		{
			ERRORLOGC( "rclose[%d] failed , errno[%d]" , p_remote_fd->rfd , errno )
		}
		else
		{
			INFOLOGC( "rclose[%d] ok" , p_remote_fd->rfd )
		}
		
		UnlinkRemoteFdsTreeNodeByRfd( & g_remote_fds , p_remote_fd );
		free( p_remote_fd );
		
		ERRORLOGC( "--- rfs_api v%s --- ropen[%s][%d] return[-1]" , __RFS_VERSION , pathfilename , flags_h )
		return -1;
	}
	else
	{
		INFOLOGC( "--- rfs_api v%s --- ropen[%s][%d] return[%d]" , __RFS_VERSION , pathfilename , flags_h , p_remote_fd->rfd )
		return p_remote_fd->rfd;
	}
}

#if 0

int ropen3( char *pathfilename , int flags_h , mode_t mode_h )
{
	int		rfd ;
	uint16_t	pathfilename_len_h ;
	uint16_t	pathfilename_len_n ;
	uint32_t	flags_n ;
	uint32_t	mode_n ;
	uint32_t	remote_fd_n[RFSAPI_CONNECT_SOCK_MAX] ;
	uint32_t	errno_n[RFSAPI_CONNECT_SOCK_MAX] ;
	struct riovec	riovec ;
	int		i ;
	struct timeval	elapse ;
	
	int		nret = 0 ;
	
	INFOLOGC( "--- rfs_api v%s --- ropen3[%s][%d][%d]" , __RFS_VERSION , pathfilename , flags_h , mode_h )
	
	if( g_p_rfs_api_conf == NULL )
	{
		nret = rloadconfig() ;
		if( nret )
			return nret;
	}
	
	if( g_connected_sock_count < g_p_rfs_api_conf->_servers_count )
	{
		nret = rconnect() ;
		if( nret < 0 )
			return nret;
	}
	
	pathfilename_len_h=(uint16_t)strlen(pathfilename);
	if( pathfilename_len_h > PATH_MAX-1 )
		return -1;
	pathfilename_len_n=htons(pathfilename_len_h);
	flags_n=htonl((uint32_t)flags_h);
	mode_n=htonl((uint32_t)mode_h);
	
	memset( & riovec , 0x00 , sizeof(struct riovec) );
	for( i = 0 ; i < g_p_rfs_api_conf->_servers_count ; i++ )
	{
		riovec.send_iov[i][0].iov_base = "O" ; riovec.send_iov[i][0].iov_len = 1 ;
		riovec.send_iov[i][1].iov_base = "3" ; riovec.send_iov[i][1].iov_len = 1 ;
		riovec.send_iov[i][2].iov_base = & pathfilename_len_n ; riovec.send_iov[i][2].iov_len = 2 ;
		riovec.send_iov[i][3].iov_base = pathfilename ; riovec.send_iov[i][3].iov_len = pathfilename_len_h ;
		riovec.send_iov[i][4].iov_base = & flags_n ; riovec.send_iov[i][4].iov_len = 4 ;
		riovec.send_iov[i][5].iov_base = & mode_n ; riovec.send_iov[i][5].iov_len = 4 ;
		riovec.p_send_iov_ptr[i] = riovec.send_iov[i] ;
		riovec.send_iovcnt[i] = 6 ;
		
		riovec.recv_iov[i][0].iov_base = & (remote_fd_n[i]) ; riovec.recv_iov[i][0].iov_len = 4 ;
		riovec.recv_iov[i][1].iov_base = & (errno_n[i]) ; riovec.recv_iov[i][1].iov_len = 4 ;
		riovec.p_recv_iov_ptr[i] = riovec.recv_iov[i] ;
		riovec.recv_iovcnt[i] = 2 ;
	}
	riovec.pfuncAdjustSendVectors = NULL ;
	riovec.pfuncAdjustReceiveVectors = NULL ;
	
	SECONDS_TO_TIMEVAL( 5 , elapse )
	nret = rio( & riovec , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "rio failed[%d]" , nret )
		for( i = 0 ; i < g_p_rfs_api_conf->_servers_count ; i++ )
		{
			if( g_connect_pollfds[i].fd != -1 )
			{
				INFOLOGC( "close sock[%d]" , g_connect_pollfds[i].fd )
				close( g_connect_pollfds[i].fd ); g_connect_pollfds[i].fd = -1 ;
			}
		}
		return nret;
	}
	else
	{
		INFOLOGC( "rio ok" )
	}
	
	for( i = 0 ; i < g_p_rfs_api_conf->_servers_count ; i++ )
	{
		if( g_connect_pollfds[i].fd != -1 )
		{
			remote_fd[i] = ntohl((uint32_t)(remote_fd_n[i])) ;
			errno = ntohl((uint32_t)(errno_n[i])) ;
			if( remote_fd[i] == -1 )
			{
				ERRORLOGC( "[%d] file_fd[%d] errno[%d]" , i , remote_fd[i] , errno )
				for( i = 0 ; i < g_p_rfs_api_conf->_servers_count ; i++ )
				{
					if( g_connect_pollfds[i].fd != -1 )
					{
						INFOLOGC( "close sock[%d]" , g_connect_pollfds[i].fd )
						close( g_connect_pollfds[i].fd ); g_connect_pollfds[i].fd = -1 ;
					}
				}
				return -1;
			}
			else
			{
				INFOLOGC( "[%d] file_fd[%d] errno[%d]" , i , remote_fd[i] , errno )
			}
		}
	}
	
	INFOLOGC( "--- rfs_api v%s --- ropen[%s][%d] return[%d]" , __RFS_VERSION , pathfilename , flags_h , rfd )
	
	return rfd;
}

#endif

int rclose( int rfd )
{
	struct RemoteFd	remote_fd , *p_remote_fd = NULL ;
	int		fd_n[RFSAPI_CONNECT_SOCK_MAX] ;
	uint32_t	ret_n[RFSAPI_CONNECT_SOCK_MAX] ;
	int		ret_h ;
	uint32_t	errno_n[RFSAPI_CONNECT_SOCK_MAX] ;
	int		errno_h ;
	struct riovec	riovec ;
	int		i ;
	struct timeval	elapse ;
	signed char	ret ;
	
	int		nret = 0 ;
	
	INFOLOGC( "--- rfs_api v%s --- rclose[%d]" , __RFS_VERSION , rfd )
	
	memset( & remote_fd , 0x00 , sizeof(struct RemoteFd) );
	remote_fd.rfd = rfd ;
	p_remote_fd = QueryRemoteFdsTreeNodeByRfd( & g_remote_fds , & remote_fd ) ;
	if( p_remote_fd == NULL )
	{
		ERRORLOGC( "QueryRemoteFdsTreeNodeByRfd failed , rfd[%d]" , remote_fd.rfd )
		return -1;
	}
	
	memset( & riovec , 0x00 , sizeof(struct riovec) );
	for( i = 0 ; i < g_p_rfs_api_conf->_servers_count ; i++ )
	{
		riovec.send_iov[i][0].iov_base = "C" ; riovec.send_iov[i][0].iov_len = 1 ;
		riovec.send_iov[i][1].iov_base = "1" ; riovec.send_iov[i][1].iov_len = 1 ;
		fd_n[i]=htonl((uint32_t)(p_remote_fd->remote_fd[i]));
		riovec.send_iov[i][2].iov_base = & (fd_n[i]) ; riovec.send_iov[i][2].iov_len = 4 ;
		riovec.p_send_iov_ptr[i] = riovec.send_iov[i] ;
		riovec.send_iovcnt[i] = 3 ;
		
		riovec.recv_iov[i][0].iov_base = & (ret_n[i]) ; riovec.recv_iov[i][0].iov_len = 4 ;
		riovec.recv_iov[i][1].iov_base = & (errno_n[i]) ; riovec.recv_iov[i][1].iov_len = 4 ;
		riovec.p_recv_iov_ptr[i] = riovec.recv_iov[i] ;
		riovec.recv_iovcnt[i] = 2 ;
	}
	riovec.pfuncAdjustSendVectors = NULL ;
	riovec.pfuncAdjustReceiveVectors = NULL ;
	
	SECONDS_TO_TIMEVAL( 5 , elapse )
	nret = rio( & riovec , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "rio failed[%d]" , nret )
		return nret;
	}
	else
	{
		INFOLOGC( "rio ok" )
	}
	
	ret = 0 ;
	for( i = 0 ; i < g_p_rfs_api_conf->_servers_count ; i++ )
	{
		if( g_connect_pollfds[i].fd != -1 && g_is_connected[i] == 1 )
		{
			ret_h = ntohl((uint32_t)(ret_n[i])) ;
			errno_h = ntohl((uint32_t)(errno_n[i])) ;
			if( errno == 0 && errno_h != 0 )
				errno = errno_h ;
			if( ret_h == -1 )
			{
				ERRORLOGC( "[%d] file_fd[%d] errno[%d]" , i , ret_h , errno_h )
				ret = -1 ;
			}
			else
			{
				INFOLOGC( "[%d] file_fd[%d] errno[%d]" , i , ret_h , errno_h )
			}
		}
	}
	
	UnlinkRemoteFdsTreeNodeByRfd( & g_remote_fds , p_remote_fd );
	free( p_remote_fd );
	
	INFOLOGC( "--- rfs_api v%s --- rclose[%d] return[%d]" , __RFS_VERSION , rfd , ret )
	
	return 0;
}

#if 0

static funcAdjustVectors AdjustVectors_rread ;
void AdjustVectors_rread( struct iovec *recv_iov , struct iovec **pp_recv_iov_ptr , int *p_recv_iovcnt )
{
	if( (*p_recv_iovcnt) == 0 && (*pp_recv_iov_ptr) == & (recv_iov[1]) )
	{
		uint32_t	*p_read_len_n = (uint32_t*)(recv_iov[0].iov_base) ;
		uint32_t	read_len_h = ntohl(*p_read_len_n) ;
		DEBUGLOGC( "set recv_iov[1].iov_len[%d] -> [%d]" , recv_iov[1].iov_len , read_len_h )
		(*pp_recv_iov_ptr)->iov_len = read_len_h ;
		DEBUGLOGC( "(*pp_recv_iov_ptr)->iov_len[%d]" , (*pp_recv_iov_ptr)->iov_len )
		(*p_recv_iovcnt) = 2 ;
	}
	
	return;
}

ssize_t rread( int rfd , void *buf , size_t count_h )
{
	int		fd_n[RFSAPI_CONNECT_SOCK_MAX] ;
	int		count_n[RFSAPI_CONNECT_SOCK_MAX] ;
	int		read_len_n[RFSAPI_CONNECT_SOCK_MAX] ;
	int		read_len_h[RFSAPI_CONNECT_SOCK_MAX] ;
	ssize_t		max_read_len ;
	uint32_t	errno_n[RFSAPI_CONNECT_SOCK_MAX] ;
	struct riovec	riovec ;
	int		i ;
	struct timeval	elapse ;
	
	int		nret = 0 ;
	
	INFOLOGC( "--- rfs_api v%s --- rread[%d][0x%X][%d]" , __RFS_VERSION , rfd , buf , count_h )
	
	if( rfd < 0 )
		return -1;
	if( unit_enable != 1 )
		return -1;
	
	memset( & riovec , 0x00 , sizeof(struct riovec) );
	for( i = 0 ; i < g_p_rfs_api_conf->_servers_count ; i++ )
	{
		riovec.send_iov[i][0].iov_base = "R" ; riovec.send_iov[i][0].iov_len = 1 ;
		riovec.send_iov[i][1].iov_base = "1" ; riovec.send_iov[i][1].iov_len = 1 ;
		fd_n[i]=htonl((uint32_t)remote_fd[i]);
		riovec.send_iov[i][2].iov_base = & (fd_n[i]) ; riovec.send_iov[i][2].iov_len = 4 ;
		count_n[i]=htonl((uint32_t)count_h);
		riovec.send_iov[i][3].iov_base = & (count_n[i]) ; riovec.send_iov[i][3].iov_len = 4 ;
		riovec.p_send_iov_ptr[i] = riovec.send_iov[i] ;
		riovec.send_iovcnt[i] = 4 ;
		
		riovec.recv_iov[i][0].iov_base = & (read_len_n[i]) ; riovec.recv_iov[i][0].iov_len = 4 ;
		riovec.recv_iov[i][1].iov_base = buf ; riovec.recv_iov[i][1].iov_len = 0 ;
		riovec.recv_iov[i][2].iov_base = & (errno_n[i]) ; riovec.recv_iov[i][2].iov_len = 4 ;
		riovec.p_recv_iov_ptr[i] = riovec.recv_iov[i] ;
		riovec.recv_iovcnt[i] = 1 ;
	}
	riovec.pfuncAdjustSendVectors = NULL ;
	riovec.pfuncAdjustReceiveVectors = & AdjustVectors_rread ;
	
	SECONDS_TO_TIMEVAL( 300 , elapse )
	nret = rio( & riovec , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "rio failed[%d]" , nret )
		return nret;
	}
	else
	{
		INFOLOGC( "rio ok" )
	}
	
	max_read_len = -1 ;
	for( i = 0 ; i < g_p_rfs_api_conf->_servers_count ; i++ )
	{
		if( g_connect_pollfds[i].fd != -1 )
		{
			read_len_h[i] = ntohl(read_len_n[i]) ;
			if( read_len_h[i] > max_read_len )
				max_read_len = read_len_h[i] ;
		}
	}
	
	INFOLOGC( "--- rfs_api v%s --- rread[%d][0x%X][%d] return[%d]" , __RFS_VERSION , rfd , buf , count_h , max_read_len )
	
	return max_read_len;
}

ssize_t rwrite( int rfd , char *buf , size_t count_h )
{
	int		fd_n[RFSAPI_CONNECT_SOCK_MAX] ;
	int		count_n[RFSAPI_CONNECT_SOCK_MAX] ;
	int		wrote_len_n[RFSAPI_CONNECT_SOCK_MAX] ;
	int		wrote_len_h[RFSAPI_CONNECT_SOCK_MAX] ;
	ssize_t		max_wrote_len ;
	uint32_t	errno_n[RFSAPI_CONNECT_SOCK_MAX] ;
	struct riovec	riovec ;
	int		i ;
	struct timeval	elapse ;
	
	int		nret = 0 ;
	
	INFOLOGC( "--- rfs_api v%s --- rwrite[%d][0x%X][%d]" , __RFS_VERSION , rfd , buf , count_h )
	
	if( rfd < 0 )
		return -1;
	if( unit_enable != 1 )
		return -1;
	
	memset( & riovec , 0x00 , sizeof(struct riovec) );
	for( i = 0 ; i < g_p_rfs_api_conf->_servers_count ; i++ )
	{
		riovec.send_iov[i][0].iov_base = "W" ; riovec.send_iov[i][0].iov_len = 1 ;
		riovec.send_iov[i][1].iov_base = "1" ; riovec.send_iov[i][1].iov_len = 1 ;
		fd_n[i]=htonl((uint32_t)remote_fd[i]);
		riovec.send_iov[i][2].iov_base = & (fd_n[i]) ; riovec.send_iov[i][2].iov_len = 4 ;
		count_n[i]=htonl((uint32_t)count_h);
		riovec.send_iov[i][3].iov_base = & (count_n[i]) ; riovec.send_iov[i][3].iov_len = 4 ;
		riovec.send_iov[i][4].iov_base = buf ; riovec.send_iov[i][4].iov_len = count_h ;
		riovec.p_send_iov_ptr[i] = riovec.send_iov[i] ;
		riovec.send_iovcnt[i] = 5 ;
		
		riovec.recv_iov[i][0].iov_base = & (wrote_len_n[i]) ; riovec.recv_iov[i][0].iov_len = 4 ;
		riovec.recv_iov[i][1].iov_base = & (errno_n[i]) ; riovec.recv_iov[i][1].iov_len = 4 ;
		riovec.p_recv_iov_ptr[i] = riovec.recv_iov[i] ;
		riovec.recv_iovcnt[i] = 2 ;
	}
	riovec.pfuncAdjustSendVectors = NULL ;
	riovec.pfuncAdjustReceiveVectors = NULL ;
	
	SECONDS_TO_TIMEVAL( 300 , elapse )
	nret = rio( & riovec , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "rio failed[%d]" , nret )
		return nret;
	}
	else
	{
		INFOLOGC( "rio ok" )
	}
	
	max_wrote_len = -1 ;
	for( i = 0 ; i < g_p_rfs_api_conf->_servers_count ; i++ )
	{
		if( g_connect_pollfds[i].fd != -1 )
		{
			wrote_len_h[i] = ntohl(wrote_len_n[i]) ;
			if( wrote_len_h[i] > max_wrote_len )
				max_wrote_len = wrote_len_h[i] ;
		}
	}
	
	INFOLOGC( "--- rfs_api v%s --- rwrite[%d][0x%X][%d] return[%d]" , __RFS_VERSION , rfd , buf , count_h , max_wrote_len )
	
	return max_wrote_len;
}

FILE *rfopen( char *pathfilename , char *mode )
{
	return NULL;
}

int rfclose( FILE *fp )
{
	return 0;
}

size_t rfread( void *ptr , size_t size , size_t nmemb , FILE *fp )
{
	return 0;
}

size_t rfwrite( void *ptr , size_t size , size_t nmemb , FILE *fp )
{
	return 0;
}

int rfeof( FILE *fp )
{
	return 0;
}

#endif

