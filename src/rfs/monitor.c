#include "in.h"

int monitor( rfs_conf *p_rfs_conf )
{
	int			listen_sock ;
	struct sockaddr_in	listen_addr ;
	
	socklen_t		accepted_addr_len ;
	struct sockaddr_in	accepted_addr ;
	int			accepted_sock ;
	
	pid_t			pid ;
	
	int			nret = 0 ;
	
	SetLogcFile( "%s/log/rfs_monitor.log" , getenv("HOME") );
	SetLogcLevel( LOGCLEVEL_DEBUG );
	
	listen_sock = socket( AF_INET , SOCK_STREAM , IPPROTO_TCP ) ;
	if( listen_sock == -1 )
	{
		FATALLOGC( "socket failed , errno[%d]" , errno )
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
		FATALLOGC( "bind[%s:%d][%d] failed , errno[%d]" , p_rfs_conf->node.server.ip , p_rfs_conf->node.server.port , listen_sock , errno )
		return -1;
	}
	
	nret = listen( listen_sock , 10240 ) ;
	if( nret == -1 )
	{
		FATALLOGC( "listen[%s:%d][%d] failed , errno[%d]" , p_rfs_conf->node.server.ip , p_rfs_conf->node.server.port , listen_sock , errno )
		return -1;
	}
	else
	{
		INFOLOGC( "listen[%s:%d][%d] ok" , p_rfs_conf->node.server.ip , p_rfs_conf->node.server.port , listen_sock )
	}
	
	signal( SIGCLD , NULL );
	signal( SIGCHLD , NULL );
	
	while(1)
	{
		accepted_addr_len = sizeof(struct sockaddr) ;
		accepted_sock = accept( listen_sock , (struct sockaddr *) & accepted_addr , & accepted_addr_len ) ;
		if( accepted_sock == -1 )
		{
			FATALLOGC( "accept failed , errno[%d]" , errno )
			return -1;
		}
		else
		{
			INFOLOGC( "accept ok" )
		}
		
		pid = fork() ;
		if( pid == -1 )
		{
			FATALLOGC( "fork failed , errno[%d]" , errno )
			return -1;
		}
		else if( pid == 0 )
		{
			exit( -worker( p_rfs_conf , accepted_sock , & accepted_addr ) );
		}
	}
	
	close( listen_sock );
	INFOLOGC( "close listen[%s:%d][%d]" , p_rfs_conf->node.server.ip , p_rfs_conf->node.server.port , listen_sock )
	
	return 0;
}

