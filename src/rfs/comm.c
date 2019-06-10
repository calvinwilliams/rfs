#include "in.h"

int comm( rfs_conf *p_rfs_conf , int process_index , int accepted_sock )
{
	struct RemoteFileSession	session ;
	
	struct timeval			elapse ;
	char				command ;
	char				version ;
	
	int				nret = 0 ;
	
	memset( & session , 0x00 , sizeof(struct RemoteFileSession) );
	session.file_fd = -1 ;
	
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
			
			if( session.file_fd == -1 )
				break;
		}
		else if( command == 'O' && version == '3' )
		{
			nret = ropen3( accepted_sock , & elapse , & session ) ;
			if( nret )
				break;
			
			if( session.file_fd == -1 )
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
	
	if( session.file_fd >= 0 )
	{
		INFOLOGC( "close file fd[%d]" , session.file_fd )
		close( session.file_fd );
	}
	
	return 0;
}
