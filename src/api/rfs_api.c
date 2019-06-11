#include "rfs_util.h"
#include "rfs_api.h"

#include "IDL_rfs_api_conf.dsc.h"

#define RFSAPI_CONNECT_SOCK_MAX		8

__thread rfs_api_conf	*g_rfs_api_conf = NULL ;
__thread struct pollfd	g_rfs_connect_pollfds[ RFSAPI_CONNECT_SOCK_MAX ] = { 0 } ;
__thread int		g_rfs_connecting_sock_count = 0 ;
__thread int		g_rfs_connected_sock_count = 0 ;

static int rloadconfig()
{
	char		*file_content = NULL ;
	
	int		i ;
	
	int		nret = 0 ;
	
	SetLogcFile( "%s/log/rfs_api.log" , getenv("HOME") );
	SetLogcLevel( LOGCLEVEL_ERROR );
	
	file_content = RFSDupFileContent( "%s/etc/rfs_api.conf" , getenv("HOME") ) ;
	if( file_content == NULL )
	{
		printf( "*** ERROR : Can't open config file[%s/etc/rfs_api.conf] , errno[%d]\n" , getenv("HOME") , errno );
		exit(1);
	}
	
	memset( g_rfs_api_conf , 0x00 , sizeof(rfs_api_conf) );
	nret = DSCDESERIALIZE_JSON_rfs_api_conf( NULL , file_content , NULL , g_rfs_api_conf ) ;
	if( nret )
	{
		ERRORLOGC( "*** ERROR : Parse config[%s] failed[%d]\n" , file_content , nret );
		free( file_content );
		return -1;
	}
	
	free( file_content );
	
	/*
	{
	int		i , j ;
	for( i = 0 ; i < g_rfs_api_conf->_nodes_count ; i++ )
	{
		printf( "nodes[%d].id[%s]\n" , i , g_rfs_api_conf->nodes[i].id );
		printf( "nodes[%d].user[%s]\n" , i , g_rfs_api_conf->nodes[i].user );
		printf( "nodes[%d].pass[%s]\n" , i , g_rfs_api_conf->nodes[i].pass );
		for( j = 0 ; j < g_rfs_api_conf->nodes[i]._servers_count ; j++ )
		{
			printf( "nodes[%d].servers[%d].ip[%s]\n" , i , j , g_rfs_api_conf->nodes[i].servers[j].ip );
			printf( "nodes[%d].servers[%d].port[%d]\n" , i , j , g_rfs_api_conf->nodes[i].servers[j].port );
		}
	}
	}
	*/
	
	for( i = 0 ; i < RFSAPI_CONNECT_SOCK_MAX ; i++ )
	{
		g_rfs_connect_pollfds[i].fd = -1 ;
	}
	
	g_rfs_connecting_sock_count = 0 ;
	g_rfs_connected_sock_count = 0 ;
	
	SetLogcLevel( LOGCLEVEL_DEBUG );
	
	return 0;
}

static __thread int			g_remote_fd = -1 ;

static int rconnect()
{
	int			i ;
	struct timeval		elapse ;
	DEF_TIMEVAL_FOR_DIFF
	
	for( i = 0 ; i < g_rfs_api_conf->_node_count ; i++ )
	{
		if( g_rfs_connect_pollfds[i].fd == -1 )
		{
			struct sockaddr_in	connect_addr ;
			
			g_rfs_connect_pollfds[i].fd = socket( AF_INET , SOCK_STREAM , IPPROTO_TCP ) ;
			if( g_rfs_connect_pollfds[i].fd == -1 )
			{
				ERRORLOGC( "socket failed[%d] , errno[%d]" , g_rfs_connect_pollfds[i].fd , errno )
				return -1;
			}
			
			RFSSetTcpNonblock( g_rfs_connect_pollfds[i].fd );
			
			memset( & connect_addr , 0x00 , sizeof(struct sockaddr_in) );
			connect_addr.sin_family = AF_INET ;
			if( g_rfs_api_conf->nodes[g_rfs_node_index].servers[0].ip[0] == '\0' )
				connect_addr.sin_addr.s_addr = INADDR_ANY ;
			else
				connect_addr.sin_addr.s_addr = inet_addr(g_rfs_api_conf->nodes[g_rfs_node_index].servers[0].ip) ;
			connect_addr.sin_port = htons( (unsigned short)(g_rfs_api_conf->nodes[g_rfs_node_index].servers[0].port) );
			nret = connect( g_rfs_connect_pollfds[i].fd , (struct sockaddr *) & (connect_addr) , sizeof(struct sockaddr) ) ;
			if( nret == -1 )
			{
				if( errno == EINPROGRESS )
				{
					g_rfs_connect_pollfds[i].events = POLLOUT ;
					g_rfs_connect_pollfds[i].revents = 0 ;
					g_rfs_connecting_sock_count++;
				}
				else
				{
					ERRORLOGC( "connect[%s:%d] failed[%d] , errno[%d]" , g_rfs_api_conf->nodes[g_rfs_node_index].servers[0].ip , g_rfs_api_conf->nodes[g_rfs_node_index].servers[0].port , nret , errno )
					return -1;
				}
			}
			else
			{
				INFOLOGC( "connect[%s:%d] ok , sock[%d]" , g_rfs_api_conf->nodes[g_rfs_node_index].servers[0].ip , g_rfs_api_conf->nodes[g_rfs_node_index].servers[0].port , g_rfs_connect_pollfds[i].fd )
				g_rfs_connect_pollfds[i].events = 0 ;
				g_rfs_connect_pollfds[i].revents = 0 ;
				g_rfs_connected_sock_count++;
			}
			
		}
		else
		{
			g_rfs_connect_pollfds[i].events = 0 ;
			g_rfs_connect_pollfds[i].revents = 0 ;
		}
	}
	
	SECONDS_TO_TIMEVAL( 5 , elapse )
	
	while(1)
	{
		GET_BEGIN_TIMEVAL
		
		TIMEVAL_TO_MILLISECONDS( elapse , timeout )
		nret = poll( & g_rfs_connect_pollfds , g_rfs_api_conf->_node_count , timeout ) ;
		
		GET_END_TIMEVAL_AND_DIFF
		REDUCE_TIMEVAL( elapse , DIFF_TIMEVAL )
		
		if( nret == -1 )
		{
			FATALLOGC( "poll failed[%d] , errno[%d]" , nret , errno )
			return -1;
		}
		else if( nret == 0 )
		{
			for( i = 0 ; i < g_rfs_api_conf->_node_count ; i++ )
			{
				if( g_rfs_connect_pollfds[i].events == POLLOUT )
				{
					ERRORLOGC( "Close sock[%d] for connecting timeout" , g_rfs_connect_pollfds[i].fd )
					close( g_rfs_connect_pollfds[i].fd ); g_rfs_connect_pollfds[i].fd = -1 ;
					g_rfs_connect_pollfds[i].events = 0 ;
					g_rfs_connecting_sock_count--;
				}
			}
			
			if( g_rfs_connecting_sock_count == 0 && g_rfs_connected_sock_count == 0 )
				return -1;
			else
				return 0;
		}
		
		for( i = 0 ; i < g_rfs_api_conf->_node_count ; i++ )
		{
			if( g_rfs_connect_pollfds[i].events == POLLOUT )
			{
				if( g_rfs_connect_pollfds[i].revents & POLLERR )
				{
					int		code , error ;
					int		addr_len ;
					
					addr_len = sizeof(int) ;
					code = getsockopt( ibma_config_space->alive_report_netaddr.sock , SOL_SOCKET , SO_ERROR , & error , & addr_len ) ;
					if( code < 0 || ( code == 0 && error ) )
					{
						ERRORLOGC( "connect2 failed" )
						g_rfs_connecting_sock_count--;
					}
					else
					{
						INFOLOGC( "connect2 ok" )
						g_rfs_connecting_sock_count--;
						g_rfs_connected_sock_count++;
					}
				}
			}
		}
	}
	
	return 0;
}

static rio( char *send_head , int send_head_len , char *send_body , int send_body_len , char *recv_head[RFSAPI_CONNECT_SOCK_MAX] , int recv_head_len , char *recv_body , int *p_recv_len )
{
	int		rfs_io_sock_count ;
	int		send_head_offset[ RFSAPI_CONNECT_SOCK_MAX ] = { 0 } ;
	int		recv_head_offset[ RFSAPI_CONNECT_SOCK_MAX ] = { 0 } ;
	int		send_body_offset[ RFSAPI_CONNECT_SOCK_MAX ] = { 0 } ;
	int		recv_body_offset[ RFSAPI_CONNECT_SOCK_MAX ] = { 0 } ;
	
	int		nret = 0 ;
	
	rfs_io_sock_count = g_rfs_connected_sock_count ;
	while(1)
	{
		if( g_rfs_connected_sock_count == 0 )
		{
			ERRORLOGC( "No connection" )
			return -1;
		}
		
		if( rfs_io_sock_count == 0 )
			break;
		
		GET_BEGIN_TIMEVAL
		
		TIMEVAL_TO_MILLISECONDS( elapse , timeout )
		nret = poll( & g_rfs_connect_pollfds , g_rfs_api_conf->_node_count , timeout ) ;
		
		GET_END_TIMEVAL_AND_DIFF
		REDUCE_TIMEVAL( elapse , DIFF_TIMEVAL )
		
		if( nret == -1 )
		{
			FATALLOGC( "poll failed[%d] , errno[%d]" , nret , errno )
			return -1;
		}
		else if( nret == 0 )
		{
			for( i = 0 ; i < g_rfs_api_conf->_node_count ; i++ )
			{
				if( g_rfs_connect_pollfds[i].fd != -1 )
				{
					ERRORLOGC( "Close sock[%d] for net-io timeout" , g_rfs_connect_pollfds[i].fd )
					close( g_rfs_connect_pollfds[i].fd ); g_rfs_connect_pollfds[i].fd = -1 ;
					g_rfs_connect_pollfds[i].events = 0 ;
					g_rfs_connected_sock_count--;
				}
			}
			
			return -1;
		}
		
		for( i = 0 ; i < g_rfs_api_conf->_node_count ; i++ )
		{
			if( g_rfs_connect_pollfds[i].fd != -1 )
			{
				if( g_rfs_connect_pollfds[i].revents & POLLERR )
				{
					ERRORLOGC( "Close sock[%d] for net-io error" , g_rfs_connect_pollfds[i].fd )
					close( g_rfs_connect_pollfds[i].fd ); g_rfs_connect_pollfds[i].fd = -1 ;
					g_rfs_connect_pollfds[i].events = 0 ;
					g_rfs_connected_sock_count--;
				}
				else if( g_rfs_connect_pollfds[i].revents == POLLOUT )
				{
					int		wrote_len ;
					
					if( send_head_offset[i] < send_head_len )
					{
						wrote_len = write( g_rfs_connect_pollfds[i].fd , send_head+send_head_offset[i] , send_head_len-send_head_offset[i] ) ;
						if( wrote_len == -1 )
						{
							if( errno == EAGAIN )
							{
								;
							}
							else
							{
								ERRORLOGC( "write head sock[%d] failed , errno[%d]" , g_rfs_connect_pollfds[i].fd , errno )
								close( g_rfs_connect_pollfds[i].fd ); g_rfs_connect_pollfds[i].fd = -1 ;
								g_rfs_connect_pollfds[i].events = 0 ;
								g_rfs_connected_sock_count--;
							}
						}
						else
						{
							INFOLOGC( "send head [%d]bytes increase [%d]bytes" , send_head_offset[i] , send_head_offset[i]+wrote_len )
							send_head_offset[i] += wrote_len ;
							if( send_head_offset[i] == send_head_len )
							{
								if( send_body_len == 0 )
								{
									INFOLOGC( "send finish" )
									g_rfs_connect_pollfds[i].events = POLLIN ;
								}
							}
						}
					}
					else
					{
						wrote_len = write( g_rfs_connect_pollfds[i].fd , send_body+send_body_offset[i] , send_body_len-send_body_offset[i] ) ;
						if( wrote_len == -1 )
						{
							if( errno == EAGAIN )
							{
								;
							}
							else
							{
								ERRORLOGC( "write body sock[%d] failed , errno[%d]" , g_rfs_connect_pollfds[i].fd , errno )
								close( g_rfs_connect_pollfds[i].fd ); g_rfs_connect_pollfds[i].fd = -1 ;
								g_rfs_connect_pollfds[i].events = 0 ;
								g_rfs_connected_sock_count--;
							}
						}
						else
						{
							INFOLOGC( "send body [%d]bytes increase [%d]bytes" , send_body_offset[i] , send_body_offset[i]+wrote_len )
							send_body_offset[i] += wrote_len ;
							if( send_body_offset[i] == send_body_len )
							{
								INFOLOGC( "send finish" )
								g_rfs_connect_pollfds[i].events = POLLIN ;
							}
						}
					}
				}
				else if( g_rfs_connect_pollfds[i].revents == POLLIN )
				{
					int		read_len ;
					
					if( recv_head_offset[i] < recv_head_len )
					{
						read_len = read( g_rfs_connect_pollfds[i].fd , read_buffer[i]+recv_head_offset[i] , recv_head_len-recv_head_offset[i] ) ;
						if( read_len == -1 )
						{
							if( errno == EAGAIN )
							{
								;
							}
							else
							{
								ERRORLOGC( "read head sock[%d] failed , errno[%d]" , g_rfs_connect_pollfds[i].fd , errno )
								close( g_rfs_connect_pollfds[i].fd ); g_rfs_connect_pollfds[i].fd = -1 ;
								g_rfs_connect_pollfds[i].events = 0 ;
								g_rfs_connected_sock_count--;
							}
						}
						else
						{
							INFOLOGC( "recv head [%d]bytes increase [%d]bytes" , recv_head_offset[i] , recv_head_offset[i]+read_len )
							recv_head_offset[i] += read_len ;
							if( recv_head_offset[i] == recv_head_len )
							{
								if( p_recv_body_len == NULL || (*p_recv_body_len) == 0 )
								{
									INFOLOGC( "recv finish" )
									rfs_io_sock_count--;
								}
							}
						}
					}
					else
					{
						read_len = read( g_rfs_connect_pollfds[i].fd , read_body[i]+recv_body_offset[i] , (*p_recv_body_len)-recv_body_offset[i] ) ;
						if( read_len == -1 )
						{
							if( errno == EAGAIN )
							{
								;
							}
							else
							{
								ERRORLOGC( "read body sock[%d] failed , errno[%d]" , g_rfs_connect_pollfds[i].fd , errno )
								close( g_rfs_connect_pollfds[i].fd ); g_rfs_connect_pollfds[i].fd = -1 ;
								g_rfs_connect_pollfds[i].events = 0 ;
								g_rfs_connected_sock_count--;
							}
						}
						else
						{
							INFOLOGC( "recv body [%d]bytes increase [%d]bytes" , recv_body_offset[i] , recv_body_offset[i]+read_len )
							recv_body_offset[i] += read_len ;
							if( recv_body_offset[i] == (*p_recv_head_len) )
							{
								INFOLOGC( "recv finish" )
								rfs_io_sock_count--;
							}
						}
					}
				}
			}
		}
	}
	
	return 0;
}

int ropen( char *pathfilename , int flags )
{
	struct timeval	elapse ;
	uint16_t	pathfilename_len ;
	uint16_t	pathfilename_len_htons ;
	uint32_t	flags_htonl ;
	char		send_head[ 1+1+2+PATH_MAX+4 + 1 ] ;
	char		*send_ptr = NULL ;
	char		recv_head[ RFSAPI_CONNECT_SOCK_MAX ][ 4+4 + 1 ] ;
	
	int		nret = 0 ;
	
	INFOLOGC( "rfs_api v%s" , __RFS_VERSION )
	
	if( g_rfs_api_conf == NULL )
	{
		nret = rloadconfig() ;
		if( nret )
			return nret;
	}
	
	nret = rconnect() ;
	if( nret )
		return nret;
	
	pathfilename_len=(uint16_t)strlen(pathfilename);
	if( pathfilename_len > PATH_MAX )
		return -1;
	pathfilename_len_htons=htons(pathfilename_len);
	flags_htonl=htonl((uint32_t)flags);
	
	send_ptr = send_head ;
	(*send_ptr)='O'; send_ptr++;
	(*send_ptr)='1'; send_ptr++;
	memcpy(send_ptr,&pathfilename_len_htons,2); send_ptr+=2;
	memcpy(send_ptr,pathfilename,pathfilename_len); send_ptr+=pathfilename_len;
	memcpy(send_ptr,flags_htonl,4); send_ptr+=4;
	
	for( i = 0 ; i < g_rfs_api_conf->_node_count ; i++ )
	{
		if( g_rfs_connect_pollfds[i].fd != -1 )
		{
			g_rfs_connect_pollfds[i].events = POLLOUT ;
			g_rfs_connect_pollfds[i].revents == POLLOUT
		}
		
		memset( recv_head[i] , 0x00 , sizeof(recv_head[i]) );
	}
	
	SECONDS_TO_TIMEVAL( 10 , elapse )
	
	rfs_io_sock_count = g_rfs_connected_sock_count ;
	
	nret = rio( send_head , sizeof(send_head)-1 , NULL , -1 , recv_head , sizeof(recv_head)-1 , NULL , NULL ) ;
	if( nret )
	{
		ERRORLOGC( "rio failed[%d]" , nret )
		return nret;
	}
	else
	{
		INFOLOGC( "rio ok" )
	}
	
	for( i = 0 ; i < g_rfs_api_conf->_node_count ; i++ )
	{
		if( g_rfs_connect_pollfds[i].fd != -1 )
		{
			file_fd = ntohl((int*)(recv_head[i])) ;
			errno = ntohl((int*)(recv_head[i]+4)) ;
			if( file_fd == -1 )
			{
				ERRORLOGC( "[%d] file_fd[%d] errno[%d]" , i , file_id , errno )
				return -1;
			}
			else
			{
				INFOLOGC( "[%d] file_fd[%d] errno[%d]" , i , file_id , errno )
			}
		}
	}
	
	return 0;
}

int ropen3( char *pathfilename , int flags , mode_t mode )
{
	struct timeval	elapse ;
	int		connect_sock ;
	
	int		nret = 0 ;
	
	connect_sock = rconnect() ;
	if( connect_sock == -1 )
		return connect_sock;
	
	SECONDS_TO_TIMEVAL( 600 , elapse )
	
	INFOLOGC( "request open3[%s][%d][%d]" , pathfilename , flags , mode )
	
	nret = RFSSendChar( connect_sock , 'O' , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendChar command[O] failed[%d]" , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSSendChar command[O] ok" )
	}
	
	nret = RFSSendChar( connect_sock , '3' , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendChar version[3] failed[%d]" , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSSendChar version[3] ok" )
	}
	
	nret = RFSSendL2VString( connect_sock , pathfilename , strlen(pathfilename) , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendL2VString pathfilename[%s] failed[%d]" , pathfilename , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSSendL2VString pathfilename[%s] ok" , pathfilename )
	}
	
	nret = RFSSendInt4( connect_sock , flags , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendInt4 flags[%d] failed[%d]" , flags , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSSendInt4 flags[%d] ok" , flags )
	}
	
	nret = RFSSendInt4( connect_sock , mode , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendInt4 mode[%d] failed[%d]" , mode , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSSendInt4 mode[%d] ok" , mode )
	}
	
	nret = RFSReceiveInt4( connect_sock , & g_remote_fd , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSReceiveInt4 remote_fd failed[%d]" , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSReceiveInt4 remote_fd[%d] ok" , g_remote_fd )
	}
	
	nret = RFSReceiveInt4( connect_sock , & errno , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSReceiveInt4 errno[%d] failed[%d]" , errno , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSReceiveInt4 errno[%d] ok" , errno )
	}
	
	if( g_remote_fd == -1 )
	{
		ERRORLOGC( "request open[%s][%d] response[%d][%d]" , pathfilename , flags , g_remote_fd , errno )
		INFOLOGC( "close connect_sock[%d]" , connect_sock )
		close( connect_sock ); connect_sock = -1 ;
		return -1;
	}
	else
	{
		INFOLOGC( "request open[%s][%d] response[%d][%d]" , pathfilename , flags , g_remote_fd , errno )
		return connect_sock;
	}
}

int rclose( int connect_sock )
{
	struct timeval	elapse ;
	int		ret ;
	
	int		nret = 0 ;
	
	if( connect_sock == -1 )
		return -1;
	
	SECONDS_TO_TIMEVAL( 600 , elapse )
	
	INFOLOGC( "request close[%d]" , connect_sock )
	
	nret = RFSSendChar( connect_sock , 'C' , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendChar command[C] failed[%d]" , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSSendChar command[C] ok" )
	}
	
	nret = RFSSendChar( connect_sock , '1' , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendChar version[1] failed[%d]" , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSSendChar version[1] ok" )
	}
	
	nret = RFSSendInt4( connect_sock , g_remote_fd , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendInt4 fd[%d] failed[%d]" , g_remote_fd , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSSendInt4 fd[%d] ok" , g_remote_fd )
	}
	
	nret = RFSReceiveInt4( connect_sock , & ret , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSReceiveInt4 ret failed[%d]" , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSReceiveInt4 ret[%d] ok" , ret )
	}
	
	nret = RFSReceiveInt4( connect_sock , & errno , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSReceiveInt4 errno failed[%d]" , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSReceiveInt4 errno[%d] ok" , errno )
	}
	
	if( ret == -1 )
	{
		ERRORLOGC( "request close[%d] response[%d][%d]" , connect_sock , ret , errno )
	}
	else
	{
		INFOLOGC( "request close[%d] response[%d][%d]" , connect_sock , ret , errno )
	}
	
	return ret;
}

ssize_t rread( int connect_sock , void *buf , size_t count )
{
	struct timeval	elapse ;
	int		read_len ;
	
	int		nret = 0 ;
	
	if( connect_sock < 0 )
		return -1;
	
	SECONDS_TO_TIMEVAL( 600 , elapse )
	
	INFOLOGC( "request read[%d][0x%X][%d]" , connect_sock , buf , count )
	
	nret = RFSSendChar( connect_sock , 'R' , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendChar command[R] failed[%d]" , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSSendChar command[R] ok" )
	}
	
	nret = RFSSendChar( connect_sock , '1' , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendChar version[1] failed[%d]" , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSSendChar version[1] ok" )
	}
	
	nret = RFSSendInt4( connect_sock , g_remote_fd , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendInt4 fd[%d] failed[%d]" , g_remote_fd , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSSendInt4 fd[%d] ok" , g_remote_fd )
	}
	
	nret = RFSSendInt4( connect_sock , count , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendInt4 count[%d] failed[%d]" , count , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSSendInt4 count[%d] ok" , g_remote_fd )
	}
	
	nret = RFSReceiveL4VString( connect_sock , buf , & read_len , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSReceiveL4VString read_len failed[%d]" , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSReceiveInt4 read_len[%d] ok" , read_len )
		DEBUGHEXLOGC( buf , read_len , "read_len[%d]bytes" , read_len )
	}
	
	nret = RFSReceiveInt4( connect_sock , & errno , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSReceiveInt4 errno failed[%d]" , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSReceiveInt4 errno[%d] ok" , errno )
	}
	
	if( read_len == -1 )
	{
		ERRORLOGC( "request read[%d][0x%X][%d] response[%d][%d]" , connect_sock , buf , count , read_len , errno )
	}
	else
	{
		INFOLOGC( "request read[%d][0x%X][%d] response[%d][%d]" , connect_sock , buf , count , read_len , errno )
	}
	
	return (ssize_t)read_len;
}

ssize_t rwrite( int connect_sock , char *buf , size_t count )
{
	struct timeval	elapse ;
	int		wrote_len ;
	
	int		nret = 0 ;
	
	if( connect_sock < 0 )
		return -1;
	
	SECONDS_TO_TIMEVAL( 600 , elapse )
	
	INFOLOGC( "request write[%d][0x%X][%d]" , connect_sock , buf , count )
	
	nret = RFSSendChar( connect_sock , 'W' , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendChar command[W] failed[%d]" , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSSendChar command[W] ok" )
	}
	
	nret = RFSSendChar( connect_sock , '1' , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendChar version[1] failed[%d]" , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSSendChar version[1] ok" )
	}
	
	nret = RFSSendInt4( connect_sock , g_remote_fd , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendInt4 fd[%d] failed[%d]" , g_remote_fd , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSSendInt4 fd[%d] ok" , g_remote_fd )
	}
	
	nret = RFSSendL4VString( connect_sock , buf , count , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendL4VString data_len failed[%d]" , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSSendL4VString data_len ok" )
		DEBUGHEXLOGC( buf , count , "data_len[%d]bytes" , count )
	}
	
	nret = RFSReceiveInt4( connect_sock , & wrote_len , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSReceiveInt4 wrote_len failed[%d]" , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSReceiveInt4 wrote_len[%d] ok" , wrote_len )
	}
	
	nret = RFSReceiveInt4( connect_sock , & errno , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSReceiveInt4 errno failed[%d]" , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSReceiveInt4 errno[%d] ok" , errno )
	}
	
	if( wrote_len == -1 )
	{
		ERRORLOGC( "request write[%d][0x%X][%d] response[%d][%d]" , connect_sock , buf , count , wrote_len , errno )
	}
	else
	{
		INFOLOGC( "request write[%d][0x%X][%d] response[%d][%d]" , connect_sock , buf , count , wrote_len , errno )
	}
	
	return wrote_len;
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

