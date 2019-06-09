#include "in.h"

int worker( rfs_conf *p_rfs_conf , int process_index , int listen_sock , int accepting_mutex )
{
	int				epoll_fd ;
	struct epoll_event		event ;
	struct epoll_event		events[ EPOLL_EVENT_COUNT ] ;
	struct epoll_event		*p_event = NULL ;
	
	struct ListenSession		listen_session ;
	
	struct sembuf			sembuf ;
	
	struct timeval			elapse ;
	char				command ;
	char				version ;
	
	/*
	struct RemoteFileSession	session ;
	*/
	struct list_head		list_order_by_active_timestamp ;
	
	int				nret = 0 ;
	
	SetLogcFile( "%s/log/rfs_worker.log" , getenv("HOME") );
	SetLogcLevel( RFSConvertLogLevelString(p_rfs_conf->log_level) );
	
	epoll_fd = epoll_create( 1024 ) ;
	if( epoll_fd == -1 )
	{
		FATALLOGC( "epoll_create failed , errno[%d]" , errno )
		return -1;
	}
	
	listen_session.listen_sock = listen_sock ;
	
	memset( & event , 0x00 , sizeof(struct epoll_event) );
	event.events = EPOLLIN ;
	event.data.ptr = & listen_session ;
	nret = epoll_ctl( epoll_fd , EPOLL_CTL_ADD , listen_sock , & event ) ;
	if( nret == -1 )
	{
		FATALLOGC( "epoll_ctl listen socket failed , errno[%d]" , errno )
		return -1;
	}
	
	chdir( p_rfs_conf->file_system.root );
	
	while(1)
	{
		INFOLOGC( "enter accepting mutex ..." )
		
		memset( & sembuf , 0x00 , sizeof(struct sembuf) );
		sembuf.sem_num = 0 ;
		sembuf.sem_op = -1 ;
		sembuf.sem_flg = SEM_UNDO ;
		nret = semop( accepting_mutex , & sembuf , 1 ) ;
		if( nret == -1 )
		{
			FATALLOGC( "epoll_ctl listen socket failed , errno[%d]" , errno )
			return -1;
		}
		
		INFOLOGC( "enter accepting mutex ok" )
		
		
		
		
	}
	
	
	
	
	
	
	
	
	
	
	
	
	/*
	memset( & session , 0x00 , sizeof(struct RemoteFileSession) );
	session.fd = -1 ;
	
	while(1)
	{
		SECONDS_TO_TIMEVAL( 24*3600 , elapse )
		
		nret = RFSReceiveChar( accepted_sock , & command , & elapse ) ;
		if( nret == RFS_ERROR_CLOSED )
		{
			INFOLOGC( "remote socket closed" )
			break;
		}
		if( nret )
		{
			ERRORLOGC( "RFSReceiveChar COMMAND failed[%d]" , nret )
			break;
		}
		else
		{
			INFOLOGC( "RFSReceiveChar COMMAND[%c] ok" , command )
		}
		
		SECONDS_TO_TIMEVAL( 60 , elapse )
		
		nret = RFSReceiveChar( accepted_sock , & version , & elapse ) ;
		if( nret )
		{
			ERRORLOGC( "RFSReceiveChar VERSION failed[%d]" , nret )
			break;
		}
		else
		{
			INFOLOGC( "RFSReceiveChar VERSION[%c] ok" , version )
		}
		
		if( command == 'O' && version == '1' )
		{
			nret = ropen( accepted_sock , & elapse , & session ) ;
			if( nret )
				break;
			
			if( session.fd == -1 )
				break;
		}
		else if( command == 'O' && version == '3' )
		{
			nret = ropen3( accepted_sock , & elapse , & session ) ;
			if( nret )
				break;
			
			if( session.fd == -1 )
				break;
		}
		else if( command == 'C' && version == '1' )
		{
			nret = rclose( accepted_sock , & elapse , & session ) ;
			if( nret )
				break;
			
			break;
		}
		else if( command == 'R' && version == '1' )
		{
			nret = rread( accepted_sock , & elapse , & session ) ;
			if( nret )
				break;
		}
		else if( command == 'W' && version == '1' )
		{
			nret = rwrite( accepted_sock , & elapse , & session ) ;
			if( nret )
				break;
		}
		else
		{
			ERRORLOGC( "unknow COMMAND[%c] and VERSION[%c]" , command , version )
			break;
		}
	}
	
	if( session.fd >= 0 )
	{
		INFOLOGC( "close file fd[%d]" , session.fd )
		close( session.fd );
	}
	*/
	
	/*
	INFOLOGC( "close accepted sock[%d]" , accepted_sock )
	close( accepted_sock );
	*/
	
	return 0;
}

