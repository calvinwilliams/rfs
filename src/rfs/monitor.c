#include "in.h"

int monitor( rfs_conf *p_rfs_conf )
{
	int			listen_sock ;
	struct sockaddr_in	listen_addr ;
	
	pid_t			*a_pids = NULL ;
	int			process_index ;
	pid_t			pid ;
	int			status ;
	
	union semun		semopts ;
	int			accepting_mutex ;
	
	int			nret = 0 ;
	
	SetLogcFile( "%s/log/%s_monitor.log" , getenv("HOME") , g_rfs_conf_main_filename );
	SetLogcLevel( RFSConvertLogLevelString(p_rfs_conf->log.log_level) );
	
	listen_sock = socket( AF_INET , SOCK_STREAM , IPPROTO_TCP ) ;
	if( listen_sock == -1 )
	{
		FATALLOGC( "socket failed , errno[%d]" , errno )
		return -1;
	}
	
	memset( & listen_addr , 0x00 , sizeof(struct sockaddr_in) );
	listen_addr.sin_family = AF_INET ;
	if( p_rfs_conf->listen.ip[0] == '\0' )
		listen_addr.sin_addr.s_addr = INADDR_ANY ;
	else
		listen_addr.sin_addr.s_addr = inet_addr(p_rfs_conf->listen.ip) ;
	listen_addr.sin_port = htons( (unsigned short)(p_rfs_conf->listen.port) );
	nret = bind( listen_sock , (struct sockaddr *) & (listen_addr) , sizeof(struct sockaddr) ) ;
	if( nret == -1 )
	{
		FATALLOGC( "bind[%s:%d][%d] failed , errno[%d]" , p_rfs_conf->listen.ip , p_rfs_conf->listen.port , listen_sock , errno )
		return -1;
	}
	
	nret = listen( listen_sock , 10240 ) ;
	if( nret == -1 )
	{
		FATALLOGC( "listen[%s:%d][%d] failed , errno[%d]" , p_rfs_conf->listen.ip , p_rfs_conf->listen.port , listen_sock , errno )
		return -1;
	}
	else
	{
		INFOLOGC( "listen[%s:%d][%d] ok" , p_rfs_conf->listen.ip , p_rfs_conf->listen.port , listen_sock )
	}
	
	RFSSetTcpReuseAddr( listen_sock );
	
	a_pids = (pid_t*)malloc( sizeof(pid_t) * p_rfs_conf->process_model.process_count ) ;
	if( a_pids == NULL )
	{
		FATALLOGC( "malloc failed , errno[%d]" , errno )
		return -1;
	}
	memset( a_pids , 0x00 , sizeof(pid_t) * p_rfs_conf->process_model.process_count );
	
	accepting_mutex = semget( IPC_PRIVATE , 1 , IPC_CREAT | 0600 ) ;
	if( accepting_mutex == -1 )
	{
		FATALLOGC( "semget failed , errno[%d]" , errno )
		return -1;
	}
	
	memset( & semopts , 0x00 , sizeof(union semun) );
	semopts.val = 1 ;
	nret = semctl( accepting_mutex , 0 , SETVAL , semopts ) ;
	if( nret == -1 )
	{
		FATALLOGC( "semctl failed , errno[%d]" , errno )
		return -1;
	}
	
	signal( SIGCLD , NULL );
	signal( SIGCHLD , NULL );
	
	for( process_index = 0 ; process_index < p_rfs_conf->process_model.process_count ; process_index++ )
	{
		a_pids[process_index] = fork() ;
		if( a_pids[process_index] == -1 )
		{
			FATALLOGC( "fork[%d] failed , errno[%d]" , process_index , errno )
			break;
		}
		else if( a_pids[process_index] == 0 )
		{
			INFOLOGC( "fork[%d] ok" , process_index )
			exit( -worker( p_rfs_conf , process_index , listen_sock , accepting_mutex ) );
		}
	}
	
	while(1)
	{
		while(1)
		{
			pid = waitpid( -1 , & status , WNOHANG ) ;
			if( pid == -1 )
			{
				FATALLOGC( "waitpid failed , errno[%d]" , errno )
				break;
			}
			else if( pid == 0 )
			{
				break;
			}
			
			if( WEXITSTATUS(status) || WIFSIGNALED(status) || WTERMSIG(status) || WCOREDUMP(status) )
			{
				ERRORLOGC( "pid[%d] WIFEXITED[%d] WEXITSTATUS[%d] WIFSIGNALED[%d] WTERMSIG[%d] WCOREDUMP[%d]" , pid , WIFEXITED(status) , WEXITSTATUS(status) , WIFSIGNALED(status) , WTERMSIG(status) , WCOREDUMP(status) )
			}
			else
			{
				INFOLOGC( "pid[%d] WIFEXITED[%d] WEXITSTATUS[%d] WIFSIGNALED[%d] WTERMSIG[%d] WCOREDUMP[%d]" , pid , WIFEXITED(status) , WEXITSTATUS(status) , WIFSIGNALED(status) , WTERMSIG(status) , WCOREDUMP(status) )
			}
			
			for( process_index = 0 ; process_index < p_rfs_conf->process_model.process_count ; process_index++ )
			{
				if( pid == a_pids[process_index] )
					break;
			}
			if( process_index >= p_rfs_conf->process_model.process_count )
			{
				ERRORLOGC( "unknow worker , pid[%d]" , pid )
				continue;
			}
			
			INFOLOGC( "reboot worker , pid[%d]" , pid )
			
			a_pids[process_index] = fork() ;
			if( a_pids[process_index] == -1 )
			{
				FATALLOGC( "fork[%d] failed , errno[%d]" , process_index , errno )
				break;
			}
			else if( a_pids[process_index] == 0 )
			{
				INFOLOGC( "fork[%d] ok" , process_index )
				exit( -worker( p_rfs_conf , process_index , listen_sock , accepting_mutex ) );
			}
		}
		
		INFOLOGC( "no more workers exit" )
		
		sleep(1);
		
		/*
		pid = fork() ;
		if( pid == -1 )
		{
			FATALLOGC( "fork failed , errno[%d]" , errno )
			break;
		}
		else if( pid == 0 )
		{
			exit( -worker( p_rfs_conf , accepted_sock , & accepted_addr ) );
		}
		
		close( accepted_sock );
		*/
	}
	
	close( listen_sock );
	INFOLOGC( "close listen[%s:%d][%d]" , p_rfs_conf->listen.ip , p_rfs_conf->listen.port , listen_sock )
	
	return 0;
}

