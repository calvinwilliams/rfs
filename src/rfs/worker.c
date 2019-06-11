#include "in.h"

int worker( rfs_conf *p_rfs_conf , int process_index , int listen_sock , int accepting_mutex )
{
	struct sembuf			sembuf ;
	
	socklen_t			accepted_addr_len ;
	struct sockaddr_in		accepted_addr ;
	int				accepted_sock ;
	
	int				nret = 0 ;
	
	SetLogcFile( "%s/log/%s_worker.log" , getenv("HOME") , g_rfs_conf_main_filename );
	SetLogcLevel( RFSConvertLogLevelString(p_rfs_conf->log.log_level) );
	
	chdir( p_rfs_conf->fs.root );
	
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
			break;
		}
		
		INFOLOGC( "enter accepting mutex ok" )
		
		memset( & sembuf , 0x00 , sizeof(struct sembuf) );
		sembuf.sem_num = 0 ;
		sembuf.sem_op = 1 ;
		sembuf.sem_flg = SEM_UNDO ;
		
		accepted_addr_len = sizeof(struct sockaddr) ;
		accepted_sock = accept( listen_sock , (struct sockaddr *) & accepted_addr , & accepted_addr_len ) ;
		if( accepted_sock == -1 )
		{
			FATALLOGC( "accept failed , errno[%d]" , errno )
			
			nret = semop( accepting_mutex , & sembuf , 1 ) ;
			if( nret == -1 )
			{
				FATALLOGC( "leave accepting mutex failed , errno[%d]" , errno )
				break;
			}
			else
			{
				INFOLOGC( "leave accepting mutex ok" )
			}
			
			break;
		}
		else
		{
			INFOLOGC( "accept ok" )
		}
		
		nret = semop( accepting_mutex , & sembuf , 1 ) ;
		if( nret == -1 )
		{
			FATALLOGC( "leave accepting mutex failed , errno[%d]" , errno )
			INFOLOGC( "close accepted sock[%d]" , accepted_sock )
			close( accepted_sock );
			break;
		}
		else
		{
			INFOLOGC( "leave accepting mutex ok" )
		}
		
		nret = comm( p_rfs_conf , process_index , accepted_sock ) ;
		if( nret )
		{
			ERRORLOGC( "comm return failed[%d]" , nret )
			INFOLOGC( "close accepted sock[%d]" , accepted_sock )
			close( accepted_sock );
			break;
		}
		else
		{
			INFOLOGC( "comm return ok" )
		}
		
		INFOLOGC( "close accepted sock[%d]" , accepted_sock )
		close( accepted_sock );
	}
	
	return 0;
}

