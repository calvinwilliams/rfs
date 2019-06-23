#include "in.h"

int comm( rfs_conf *p_rfs_conf , int process_index , int accepted_sock )
{
	struct LocalFds		local_fds ;
	struct LocalFd		*p_local_fd = NULL ;
	
	struct timeval		elapse ;
	char			command ;
	char			version ;
	
	int			nret = 0 ;
	
	memset( & local_fds , 0x00 , sizeof(struct LocalFds) );
	
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
			nret = ropen( accepted_sock , & local_fds , & elapse ) ;
			if( nret )
				break;
		}
		else if( command == 'O' && version == '3' )
		{
			nret = ropen3( accepted_sock , & local_fds , & elapse ) ;
			if( nret )
				break;
		}
		else if( command == 'C' && version == '1' )
		{
			nret = rclose( accepted_sock , & local_fds , & elapse ) ;
			if( nret )
				break;
		}
		else if( command == 'R' && version == '1' )
		{
			nret = rread( accepted_sock , & local_fds , & elapse ) ;
			if( nret )
				break;
		}
		else if( command == 'W' && version == '1' )
		{
			nret = rwrite( accepted_sock , & local_fds , & elapse ) ;
			if( nret )
				break;
		}
		else if( command == 'E' && version == '1' )
		{
			nret = reof( accepted_sock , & local_fds , & elapse ) ;
			if( nret )
				break;
		}
		else
		{
			ERRORLOGC( "unknow COMMAND[%c] and VERSION[%c]" , command , version )
			break;
		}
	}
	
	p_local_fd = NULL ;
	while(1)
	{
		p_local_fd = TravelLocalFdsTreeByLocalFd( & local_fds , p_local_fd ) ;
		if( p_local_fd == NULL )
			break;
		
		WARNLOGC( "close[%d]" , p_local_fd->local_fd )
		close( p_local_fd->local_fd );
	}
	DestroyLocalFdsTree( & local_fds );
	
	return 0;
}

