#include "rfs_util.h"
#include "rfs_api.h"

#include "IDL_rfs_api_conf.dsc.h"

__thread rfs_api_conf	*g_rfs_api_conf = NULL ;
__thread int		g_rfs_node_index = -1 ;

static int rloadconfig()
{
	char		rfs_api_conf_pathfilename[ PATH_MAX ] ;
	FILE		*fp = NULL ;
	long		file_len ;
	char		*file_content = NULL ;
	
	int		nret = 0 ;
	
	SetLogcFile( "%s/log/rfs_api.log" , getenv("HOME") );
	SetLogcLevel( LOGCLEVEL_ERROR );
	
	memset( rfs_api_conf_pathfilename , 0x00 , sizeof(rfs_api_conf_pathfilename) );
	snprintf( rfs_api_conf_pathfilename , sizeof(rfs_api_conf_pathfilename)-1 , "%s/etc/rfs_api.conf" , getenv("HOME") );
	fp = fopen( rfs_api_conf_pathfilename , "r" ) ;
	if( fp == NULL )
	{
		ERRORLOGC( "*** ERROR : Can't open config file[%s] , errno[%d]\n" , rfs_api_conf_pathfilename , errno );
		return -1;
	}
	
	fseek( fp , 0 , SEEK_END );
	file_len = ftell( fp ) ;
	fseek( fp , 0 , SEEK_SET );
	file_content = (char*)malloc( file_len+1 ) ;
	if( file_content == NULL )
	{
		ERRORLOGC( "*** ERROR : Alloc failed , errno[%d]\n" , errno );
		return -1;
	}
	memset( file_content , 0x00 , file_len+1 );
	nret = fread( file_content , file_len , 1 , fp ) ;
	if( nret != 1 )
	{
		ERRORLOGC( "*** ERROR : Read config file[%s] failed , errno[%d]\n" , rfs_api_conf_pathfilename , errno );
		free( file_content );
		return -1;
	}
	
	g_rfs_api_conf = (rfs_api_conf *)malloc( sizeof(rfs_api_conf) ) ;
	if( g_rfs_api_conf == NULL )
	{
		ERRORLOGC( "*** ERROR : Alloc failed , errno[%d]\n" , errno );
		free( file_content );
		return -1;
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
	
	SetLogcLevel( LOGCLEVEL_DEBUG );
	
	return 0;
}

static int rselectnode( char *node_id )
{
	int		i ;
	
	for( i = 0 ; i < g_rfs_api_conf->_nodes_count ; i++ )
	{
		if( STRCMP( g_rfs_api_conf->nodes[i].id , == , node_id ) )
		{
			g_rfs_node_index = i ;
			return 0;
		}
	}
	
	return -1;
}

int rset( char *node_id )
{
	int		nret = 0 ;
	
	if( g_rfs_api_conf == NULL )
	{
		nret = rloadconfig() ;
		if( nret )
			return nret;
	}
	
	nret = rselectnode( node_id ) ;
	if( nret )
		return nret;
	
	return 0;
}

static __thread int			g_remote_fd = -1 ;

static int rconnect()
{
	int			connect_sock ;
	struct sockaddr_in	connect_addr ;
	
	int			nret = 0 ;	
	
	connect_sock = socket( AF_INET , SOCK_STREAM , IPPROTO_TCP ) ;
	if( connect_sock == -1 )
	{
		ERRORLOGC( "socket failed[%d] , errno[%d]" , connect_sock , errno )
		return -1;
	}
	
	memset( & connect_addr , 0x00 , sizeof(struct sockaddr_in) );
	connect_addr.sin_family = AF_INET ;
	if( g_rfs_api_conf->nodes[g_rfs_node_index].servers[0].ip[0] == '\0' )
		connect_addr.sin_addr.s_addr = INADDR_ANY ;
	else
		connect_addr.sin_addr.s_addr = inet_addr(g_rfs_api_conf->nodes[g_rfs_node_index].servers[0].ip) ;
	connect_addr.sin_port = htons( (unsigned short)(g_rfs_api_conf->nodes[g_rfs_node_index].servers[0].port) );
	nret = connect( connect_sock , (struct sockaddr *) & (connect_addr) , sizeof(struct sockaddr) ) ;
	if( nret == -1 )
	{
		ERRORLOGC( "connect[%s:%d] failed[%d] , errno[%d]" , g_rfs_api_conf->nodes[g_rfs_node_index].servers[0].ip , g_rfs_api_conf->nodes[g_rfs_node_index].servers[0].port , nret , errno )
		return -1;
	}
	else
	{
		INFOLOGC( "connect[%s:%d] ok , sock[%d]" , g_rfs_api_conf->nodes[g_rfs_node_index].servers[0].ip , g_rfs_api_conf->nodes[g_rfs_node_index].servers[0].port , connect_sock )
	}
	
	return connect_sock;
}

int ropen( char *pathfilename , int flags )
{
	struct timeval	elapse ;
	int		connect_sock ;
	
	int		nret = 0 ;
	
	connect_sock = rconnect() ;
	if( connect_sock == -1 )
		return connect_sock;
	
	SECONDS_TO_TIMEVAL( 600 , elapse )
	
	INFOLOGC( "request open[%s][%d] ..." , pathfilename , flags )
	
	nret = RFSSendChar( connect_sock , 'O' , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendChar command[O] failed[%d]" , nret )
		return -1;
	}
	else
	{
		DEBUGLOGC( "RFSSendChar command[O] ok" )
	}
	
	nret = RFSSendChar( connect_sock , '1' , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendChar version[1] failed[%d]" , nret )
		return -1;
	}
	else
	{
		DEBUGLOGC( "RFSSendChar version[1] ok" )
	}
	
	nret = RFSSendL2VString( connect_sock , pathfilename , strlen(pathfilename) , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendL2VString pathfilename[%s] failed[%d]" , pathfilename , nret )
		return -1;
	}
	else
	{
		DEBUGLOGC( "RFSSendL2VString pathfilename[%s] ok" , pathfilename )
	}
	
	nret = RFSSendInt4( connect_sock , flags , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendInt4 flags[%d] failed[%d]" , flags , nret )
		return -1;
	}
	else
	{
		DEBUGLOGC( "RFSSendInt4 flags[%d] ok" , flags )
	}
	
	nret = RFSReceiveInt4( connect_sock , & g_remote_fd , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSReceiveInt4 remote_fd failed[%d]" , nret )
		return -1;
	}
	else
	{
		DEBUGLOGC( "RFSReceiveInt4 remote_fd[%d] ok" , g_remote_fd )
	}
	
	nret = RFSReceiveInt4( connect_sock , & errno , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSReceiveInt4 errno failed[%d]" , nret )
		return -1;
	}
	else
	{
		DEBUGLOGC( "RFSReceiveInt4 errno[%d] ok" , errno )
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

