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
		return -2;
	}
	memset( file_content , 0x00 , file_len+1 );
	nret = fread( file_content , file_len , 1 , fp ) ;
	if( nret != 1 )
	{
		ERRORLOGC( "*** ERROR : Read config file[%s] failed , errno[%d]\n" , rfs_api_conf_pathfilename , errno );
		free( file_content );
		return -3;
	}
	
	g_rfs_api_conf = (rfs_api_conf *)malloc( sizeof(rfs_api_conf) ) ;
	if( g_rfs_api_conf == NULL )
	{
		ERRORLOGC( "*** ERROR : Alloc failed , errno[%d]\n" , errno );
		free( file_content );
		return -5;
	}
	memset( g_rfs_api_conf , 0x00 , sizeof(rfs_api_conf) );
	nret = DSCDESERIALIZE_JSON_rfs_api_conf( NULL , file_content , NULL , g_rfs_api_conf ) ;
	if( nret )
	{
		ERRORLOGC( "*** ERROR : Parse config[%s] failed[%d]\n" , file_content , nret );
		free( file_content );
		return -5;
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

static __thread int			g_connect_sock = -1 ;
static __thread struct sockaddr_in	g_connect_addr = { 0 } ;

static int rconnect()
{
	int		nret = 0 ;	
	
	g_connect_sock = socket( AF_INET , SOCK_STREAM , IPPROTO_TCP ) ;
	if( g_connect_sock == -1 )
	{
		ERRORLOGC( "socket failed[%d] , errno[%d]" , g_connect_sock , errno )
		return -1;
	}
	
	memset( & g_connect_addr , 0x00 , sizeof(struct sockaddr_in) );
	g_connect_addr.sin_family = AF_INET ;
	if( g_rfs_api_conf->nodes[g_rfs_node_index].servers[0].ip[0] == '\0' )
		g_connect_addr.sin_addr.s_addr = INADDR_ANY ;
	else
		g_connect_addr.sin_addr.s_addr = inet_addr(g_rfs_api_conf->nodes[g_rfs_node_index].servers[0].ip) ;
	g_connect_addr.sin_port = htons( (unsigned short)(g_rfs_api_conf->nodes[g_rfs_node_index].servers[0].port) );
	nret = connect( g_connect_sock , (struct sockaddr *) & (g_connect_addr) , sizeof(struct sockaddr) ) ;
	if( nret == -1 )
	{
		ERRORLOGC( "connect[%s:%d] failed[%d] , errno[%d]" , g_rfs_api_conf->nodes[g_rfs_node_index].servers[0].ip , g_rfs_api_conf->nodes[g_rfs_node_index].servers[0].port , errno )
		return -2;
	}
	else
	{
		INFOLOGC( "connect[%s:%d] ok , sock[%d]" , g_rfs_api_conf->nodes[g_rfs_node_index].servers[0].ip , g_rfs_api_conf->nodes[g_rfs_node_index].servers[0].port , g_connect_sock )
	}
	
	return 0;
}

int ropen( char *pathfilename , int flags )
{
	struct timeval	elapse ;
	char		reversed[ 2 + 1 ] = "" ;
	int		remote_fd ;
	
	int		nret = 0 ;
	
	nret = rconnect() ;
	if( nret )
		return nret;
	
	SECONDS_TO_TIMEVAL( 600 , elapse )
	
	nret = RFSSendChar( g_connect_sock , 'O' , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendChar command failed[%d]" , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSSendChar command ok" )
	}
	
	nret = RFSSendChar( g_connect_sock , '1' , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendChar version failed[%d]" , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSSendChar version ok" )
	}
	
	nret = RFSSendString( g_connect_sock , reversed , 2 , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendString reversed failed[%d]" , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSSendString reversed ok" )
	}
	
	nret = RFSSendL2VString( g_connect_sock , pathfilename , strlen(pathfilename) , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendL2VString pathfilename[%s] failed[%d]" , pathfilename , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSSendL2VString pathfilename[%s] ok" , pathfilename )
	}
	
	nret = RFSSendInt4( g_connect_sock , flags , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendInt4 flags[%x] failed[%d]" , flags , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSSendInt4 flags[%x] ok" , flags )
	}
	
	nret = RFSReceiveInt4( g_connect_sock , & remote_fd , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSReceiveInt4 remote_fd[%d] failed[%d]" , remote_fd , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSReceiveInt4 remote_fd[%d] ok" , remote_fd )
	}
	
	nret = RFSReceiveInt4( g_connect_sock , & errno , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSReceiveInt4 errno[%d] failed[%d]" , errno , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSReceiveInt4 errno[%d] ok" , errno )
	}
	
	return g_connect_sock;
}

int ropen3( char *pathfilename , int flags , mode_t mode )
{
	struct timeval	elapse ;
	char		reversed[ 2 + 1 ] = "" ;
	int		remote_fd ;
	
	int		nret = 0 ;
	
	nret = rconnect() ;
	if( nret )
		return nret;
	
	SECONDS_TO_TIMEVAL( 600 , elapse )
	
	nret = RFSSendChar( g_connect_sock , 'O' , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendChar command failed[%d]" , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSSendChar command ok" )
	}
	
	nret = RFSSendChar( g_connect_sock , '3' , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendChar version failed[%d]" , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSSendChar version ok" )
	}
	
	nret = RFSSendString( g_connect_sock , reversed , 2 , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendString reversed failed[%d]" , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSSendString reversed ok" )
	}
	
	nret = RFSSendL2VString( g_connect_sock , pathfilename , strlen(pathfilename) , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendL2VString pathfilename[%s] failed[%d]" , pathfilename , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSSendL2VString pathfilename[%s] ok" , pathfilename )
	}
	
	nret = RFSSendInt4( g_connect_sock , flags , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendInt4 flags[%x] failed[%d]" , flags , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSSendInt4 flags[%x] ok" , flags )
	}
	
	nret = RFSSendInt4( g_connect_sock , mode , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendInt4 mode[%x] failed[%d]" , mode , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSSendInt4 mode[%x] ok" , mode )
	}
	
	nret = RFSReceiveInt4( g_connect_sock , & remote_fd , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSReceiveInt4 remote_fd[%d] failed[%d]" , remote_fd , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSReceiveInt4 remote_fd[%d] ok" , remote_fd )
	}
	
	nret = RFSReceiveInt4( g_connect_sock , & errno , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSReceiveInt4 errno[%d] failed[%d]" , errno , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSReceiveInt4 errno[%d] ok" , errno )
	}
	
	return g_connect_sock;
}

int rclose( int fd )
{
	struct timeval	elapse ;
	char		reversed[ 2 + 1 ] = "" ;
	int		ret ;
	
	int		nret = 0 ;
	
	nret = rconnect() ;
	if( nret )
		return nret;
	
	SECONDS_TO_TIMEVAL( 600 , elapse )
	
	nret = RFSSendChar( g_connect_sock , 'O' , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendChar command failed[%d]" , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSSendChar command ok" )
	}
	
	nret = RFSSendChar( g_connect_sock , '3' , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendChar version failed[%d]" , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSSendChar version ok" )
	}
	
	nret = RFSSendString( g_connect_sock , reversed , 2 , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendString reversed failed[%d]" , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSSendString reversed ok" )
	}
	
	nret = RFSSendInt4( g_connect_sock , fd , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendInt4 fd[%d] failed[%d]" , fd , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSSendInt4 fd[%d] ok" , fd )
	}
	
	nret = RFSReceiveInt4( g_connect_sock , & ret , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSReceiveInt4 ret[%d] failed[%d]" , ret , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSReceiveInt4 ret[%d] ok" , ret )
	}
	
	nret = RFSReceiveInt4( g_connect_sock , & errno , & elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSReceiveInt4 errno[%d] failed[%d]" , errno , nret )
		return -1;
	}
	else
	{
		INFOLOGC( "RFSReceiveInt4 errno[%d] ok" , errno )
	}
	
	return ret;
}

ssize_t rread( int fd , void *buf , size_t count )
{
	return 0;
}

ssize_t rwrite( int fd , char *buf , size_t count )
{
	return 0;
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

