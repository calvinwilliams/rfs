#include "in.h"

int monitor( rfs_conf *p_rfs_conf )
{
	int			listen_sock ;
	struct sockaddr_in	listen_addr ;
	
	int			nret = 0 ;
	
	SetLogcFile( "%s/log/rfs_monitor.log" , getenv("HOME") );
	SetLogcLevel( LOGCLEVEL_DEBUG );
	
	listen_sock = socket( AF_INET , SOCK_STREAM , IPPROTO_TCP ) ;
	if( listen_sock == -1 )
	{
		ERRORLOGC( "socket failed , errno[%d]" , errno )
		return -1;
	}
	
	memset( & listen_addr , 0x00 , sizeof(struct sockaddr_in) );
	listen_addr.sin_family = AF_INET ;
	if( p_rfs_conf->node.server.ip[0] == '\0' )
		listen_addr.sin_addr.s_addr = INADDR_ANY ;
	else
		listen_addr.sin_addr.s_addr = inet_addr(p_rfs_conf->node.server.ip) ;
	listen_addr.sin_port = htons( (unsigned short)(p_rfs_conf->node.server.port) );
	nret = bind( listen_sock , (struct sockaddr *) & (listen_addr) , sizeof(struct sockaddr) ) ;
	if( nret == -1 )
	{
		ERRORLOGC( "bind[%s:%d][%d] failed , errno[%d]" , p_rfs_conf->node.server.ip , p_rfs_conf->node.server.port , listen_sock , errno )
		return -1;
	}
	
	nret = listen( listen_sock , 10240 ) ;
	if( nret == -1 )
	{
		ERRORLOGC( "listen[%s:%d][%d] failed , errno[%d]" , p_rfs_conf->node.server.ip , p_rfs_conf->node.server.port , listen_sock , errno )
		return -1;
	}
	else
	{
		INFOLOGC( "listen[%s:%d][%d] ok" , p_rfs_conf->node.server.ip , p_rfs_conf->node.server.port , listen_sock )
	}
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	return 0;
}

