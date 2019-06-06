#include "in.h"

int worker( rfs_conf *p_rfs_conf , int accepted_sock , struct sockaddr_in *p_accepted_addr )
{
	struct timeval			elapse ;
	char				command ;
	char				version ;
	char				reversed[ 2 + 1 ] ;
	
	struct RemoteFileSession	session ;
	
	int				nret = 0 ;
	
	SetLogcFile( "%s/log/rfs_worker_%d.log" , getenv("HOME") , getpid()%10 );
	SetLogcLevel( LOGCLEVEL_DEBUG );
	
	chdir( p_rfs_conf->root );
	
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
			return nret;
		}
		else
		{
			INFOLOGC( "RFSReceiveChar COMMAND ok" )
			DEBUGHEXLOGC( & version , 1 , "len[%d]" , 1 )
		}
		
		SECONDS_TO_TIMEVAL( 60 , elapse )
		
		nret = RFSReceiveChar( accepted_sock , & version , & elapse ) ;
		if( nret )
		{
			ERRORLOGC( "RFSReceiveChar VERSION failed[%d]" , nret )
			return nret;
		}
		else
		{
			INFOLOGC( "RFSReceiveChar REVERSED ok" )
			DEBUGHEXLOGC( & version , 1 , "len[%d]" , 1 )
		}
		
		nret = RFSReceiveString( accepted_sock , reversed , 2 , & elapse ) ;
		if( nret )
		{
			ERRORLOGC( "RFSReceiveString REVERSED failed[%d]" , nret )
			return nret;
		}
		else
		{
			INFOLOGC( "RFSReceiveString REVERSED ok" )
			DEBUGHEXLOGC( reversed , 2 , "len[%d]" , 2 )
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
			return -1;
		}
	}
	
	if( session.fd >= 0 )
		close( session.fd );
	
	close( accepted_sock );
	
	return 0;
}

