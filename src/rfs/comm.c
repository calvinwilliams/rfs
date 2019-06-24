#include "in.h"

int comm( rfs_conf *p_rfs_conf , int process_index , int accepted_sock )
{
	struct LocalFds		local_fds ;
	struct LocalFd		*p_local_fd = NULL ;
	
	struct LocalFps		local_fps ;
	struct LocalFp		*p_local_fp = NULL ;
	
	struct timeval		elapse ;
	char			command[8+1] ;
	char			version ;
	
	int			nret = 0 ;
	
	memset( & local_fds , 0x00 , sizeof(struct LocalFds) );
	memset( & local_fps , 0x00 , sizeof(struct LocalFps) );
	
	while(1)
	{
		SECONDS_TO_TIMEVAL( 24*3600 , elapse )
		
		memset( command , 0x00 , sizeof(command) );
		nret = RFSReceiveString( accepted_sock , command , 8 , & elapse ) ;
		if( nret == RFS_ERROR_CLOSED )
		{
			INFOLOGC( "remote socket closed" )
			break;
		}
		if( nret )
		{
			ERRORLOGC( "RFSReceiveString COMMAND failed[%d]" , nret )
			break;
		}
		else
		{
			INFOLOGC( "RFSReceiveString COMMAND[%s] ok" , command )
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
		
		if( STRCMP( command , == , "open" ) && version == '1' )
		{
			nret = rfs_open( accepted_sock , & local_fds , & local_fps , & elapse ) ;
			if( nret )
				break;
		}
		else if( STRCMP( command , == , "open3" ) && version == '1' )
		{
			nret = rfs_open3( accepted_sock , & local_fds , & local_fps , & elapse ) ;
			if( nret )
				break;
		}
		else if( STRCMP( command , == , "close" ) && version == '1' )
		{
			nret = rfs_close( accepted_sock , & local_fds , & local_fps , & elapse ) ;
			if( nret )
				break;
		}
		else if( STRCMP( command , == , "read" ) && version == '1' )
		{
			nret = rfs_read( accepted_sock , & local_fds , & local_fps , & elapse ) ;
			if( nret )
				break;
		}
		else if( STRCMP( command , == , "write" ) && version == '1' )
		{
			nret = rfs_write( accepted_sock , & local_fds , & local_fps , & elapse ) ;
			if( nret )
				break;
		}
		else if( STRCMP( command , == , "fopen" ) && version == '1' )
		{
			nret = rfs_fopen( accepted_sock , & local_fds , & local_fps , & elapse ) ;
			if( nret )
				break;
		}
		else if( STRCMP( command , == , "fclose" ) && version == '1' )
		{
			nret = rfs_fclose( accepted_sock , & local_fds , & local_fps , & elapse ) ;
			if( nret )
				break;
		}
#if 0
		else if( STRCMP( command , == , "fread" ) && version == '1' )
		{
			nret = rfs_fread( accepted_sock , & local_fds , & local_fps , & elapse ) ;
			if( nret )
				break;
		}
		else if( STRCMP( command , == , "fwrite" ) && version == '1' )
		{
			nret = rfs_fwrite( accepted_sock , & local_fds , & local_fps , & elapse ) ;
			if( nret )
				break;
		}
		else if( STRCMP( command , == , "fgets" ) && version == '1' )
		{
			nret = rfs_fgets( accepted_sock , & local_fds , & local_fps , & elapse ) ;
			if( nret )
				break;
		}
		else if( STRCMP( command , == , "fputs" ) && version == '1' )
		{
			nret = rfs_fputs( accepted_sock , & local_fds , & local_fps , & elapse ) ;
			if( nret )
				break;
		}
		else if( STRCMP( command , == , "fgetc" ) && version == '1' )
		{
			nret = rfs_fgetc( accepted_sock , & local_fds , & local_fps , & elapse ) ;
			if( nret )
				break;
		}
		else if( STRCMP( command , == , "fputc" ) && version == '1' )
		{
			nret = rfs_fputc( accepted_sock , & local_fds , & local_fps , & elapse ) ;
			if( nret )
				break;
		}
#endif
		else
		{
			ERRORLOGC( "unknow COMMAND[%s] and VERSION[%c]" , command , version )
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
	
	p_local_fp = NULL ;
	while(1)
	{
		p_local_fp = TravelLocalFpsTreeByLocalFp( & local_fps , p_local_fp ) ;
		if( p_local_fp == NULL )
			break;
		
		WARNLOGC( "fclose[%p]" , p_local_fp->local_fp )
		fclose( p_local_fp->local_fp );
	}
	DestroyLocalFpsTree( & local_fps );
	
	return 0;
}

