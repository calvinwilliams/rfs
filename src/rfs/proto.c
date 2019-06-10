#include "in.h"

int ropen( int accepted_sock , struct timeval *p_elapse , struct RemoteFileSession *p_session )
{
	int		nret = 0 ;
	
	if( p_session->file_fd != -1 )
	{
		WARNLOGC( "close fd[%d]" , p_session->file_fd )
		close( p_session->file_fd );
	}
	
	SECONDS_TO_TIMEVAL( 60 , (*p_elapse) )
	
	memset( p_session->pathfilename , 0x00 , sizeof(p_session->pathfilename) );
	nret = RFSReceiveL2VString( accepted_sock , p_session->pathfilename , & (p_session->pathfilename_len) , p_elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSReceiveL2VString pathfilename failed[%d]" , nret )
		return nret;
	}
	else
	{
		DEBUGLOGC( "RFSReceiveL2VString pathfilename[%s] ok" , p_session->pathfilename )
	}
	
	nret = RFSReceiveInt4( accepted_sock , & (p_session->flags) , p_elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSReceiveInt4 flags failed[%d]" , nret )
		return nret;
	}
	else
	{
		DEBUGLOGC( "RFSReceiveInt4 flags[%d] ok" , p_session->flags )
	}
	
	INFOLOGC( "call open[%s][%d] ..." , p_session->pathfilename , p_session->flags )
	p_session->file_fd = open( p_session->pathfilename , p_session->flags ) ;
	if( p_session->file_fd == -1 )
	{
		ERRORLOGC( "call open[%s][%d] failed[%d] , errno[%d]" , p_session->pathfilename , p_session->flags , p_session->file_fd , errno )
	}
	else
	{
		INFOLOGC( "call open[%s][%d] ok , fd[%d]" , p_session->pathfilename , p_session->flags , p_session->file_fd )
	}
	
	nret = RFSSendInt4( accepted_sock , p_session->file_fd , p_elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendInt4 fd[%d] failed[%d]" , p_session->file_fd , nret )
		return nret;
	}
	else
	{
		DEBUGLOGC( "RFSSendInt4 fd[%d] ok" , p_session->file_fd )
	}
	
	nret = RFSSendInt4( accepted_sock , errno , p_elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendInt4 errno[%d] failed[%d]" , errno , nret )
		return nret;
	}
	else
	{
		DEBUGLOGC( "RFSSendInt4 errno[%d] ok" , errno )
	}
	
	return 0;
}

int ropen3( int accepted_sock , struct timeval *p_elapse , struct RemoteFileSession *p_session )
{
	int		nret = 0 ;
	
	if( p_session->file_fd != -1 )
	{
		WARNLOGC( "close fd[%d]" , p_session->file_fd )
		close( p_session->file_fd );
	}
	
	SECONDS_TO_TIMEVAL( 60 , (*p_elapse) )
	
	memset( p_session->pathfilename , 0x00 , sizeof(p_session->pathfilename) );
	nret = RFSReceiveL2VString( accepted_sock , p_session->pathfilename , & (p_session->pathfilename_len) , p_elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSReceiveL2VString pathfilename failed[%d]" , nret )
		return nret;
	}
	else
	{
		DEBUGLOGC( "RFSReceiveL2VString pathfilename[%s] ok" , p_session->pathfilename )
	}
	
	nret = RFSReceiveInt4( accepted_sock , & (p_session->flags) , p_elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSReceiveInt4 flags failed[%d]" , nret )
		return nret;
	}
	else
	{
		DEBUGLOGC( "RFSReceiveInt4 flags[%d] ok" , p_session->flags )
	}
	
	nret = RFSReceiveInt4( accepted_sock , & (p_session->mode) , p_elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSReceiveInt4 mode failed[%d]" , nret )
		return nret;
	}
	else
	{
		DEBUGLOGC( "RFSReceiveInt4 mode[%d] ok" , p_session->mode )
	}
	
	INFOLOGC( "call open[%s][%d][%d] ..." , p_session->pathfilename , p_session->flags , p_session->mode )
	p_session->file_fd = open( p_session->pathfilename , p_session->flags , p_session->mode ) ;
	if( p_session->file_fd == -1 )
	{
		ERRORLOGC( "call open[%s][%d][%d] failed[%d] , errno[%d]" , p_session->pathfilename , p_session->flags , p_session->mode , p_session->file_fd , errno )
	}
	else
	{
		DEBUGLOGC( "call open[%s][%d][%d] ok , fd[%d]" , p_session->pathfilename , p_session->flags , p_session->mode , p_session->file_fd )
	}
	
	nret = RFSSendInt4( accepted_sock , p_session->file_fd , p_elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendInt4 fd[%d] failed[%d]" , p_session->file_fd , nret )
		return nret;
	}
	else
	{
		DEBUGLOGC( "RFSSendInt4 fd[%d] ok" , p_session->file_fd )
	}
	
	nret = RFSSendInt4( accepted_sock , errno , p_elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendInt4 errno[%d] failed[%d]" , errno , nret )
		return nret;
	}
	else
	{
		DEBUGLOGC( "RFSSendInt4 errno[%d] ok" , errno )
	}
	
	return 0;
}

int rclose( int accepted_sock , struct timeval *p_elapse , struct RemoteFileSession *p_session )
{
	int		ret ;
	
	int		nret = 0 ;
	
	if( p_session->file_fd == -1 )
		return RFS_ERROR_INTERNAL;
	
	SECONDS_TO_TIMEVAL( 60 , (*p_elapse) )
	
	nret = RFSReceiveInt4( accepted_sock , & (p_session->file_fd) , p_elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSReceiveInt4 flags failed[%d]" , nret )
		return nret;
	}
	else
	{
		DEBUGLOGC( "RFSReceiveInt4 flags[%d] ok" , p_session->file_fd )
	}
	
	INFOLOGC( "call close[%d] ..." , p_session->file_fd )
	ret = close( p_session->file_fd ) ;
	if( ret == -1 )
	{
		ERRORLOGC( "call close[%s][%d] failed[%d] , errno[%d]" , p_session->pathfilename , p_session->file_fd , ret , errno )
	}
	else
	{
		INFOLOGC( "call close[%s][%d] ok" , p_session->pathfilename , p_session->file_fd )
	}
	
	nret = RFSSendInt4( accepted_sock , ret , p_elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendInt4 ret[%d] failed[%d]" , ret , nret )
		return nret;
	}
	else
	{
		DEBUGLOGC( "RFSSendInt4 ret[%d] ok" , ret )
	}
	
	nret = RFSSendInt4( accepted_sock , errno , p_elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendInt4 errno failed[%d]" , errno , nret )
		return nret;
	}
	else
	{
		DEBUGLOGC( "RFSSendInt4 errno ok" , errno )
	}
	
	return 0;
}

static __thread char		*sg_buf = NULL ;
static __thread uint32_t	sg_buf_size = 0 ;

int rread( int accepted_sock , struct timeval *p_elapse , struct RemoteFileSession *p_session )
{
	int		read_len ;
	int		data_len ;
	
	int		nret = 0 ;
	
	if( p_session->file_fd == -1 )
		return RFS_ERROR_INTERNAL;
	
	SECONDS_TO_TIMEVAL( 60 , (*p_elapse) )
	
	nret = RFSReceiveInt4( accepted_sock , & (p_session->file_fd) , p_elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSReceiveInt4 fd failed[%d]" , nret )
		return nret;
	}
	else
	{
		DEBUGLOGC( "RFSReceiveInt4 fd[%d] ok" , p_session->file_fd )
	}
	
	nret = RFSReceiveInt4( accepted_sock , & read_len , p_elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSReceiveInt4 read_len failed[%d]" , nret )
		return nret;
	}
	else
	{
		DEBUGLOGC( "RFSReceiveInt4 read_len[%d] ok" , read_len )
	}
	
	if( sg_buf == NULL )
	{
		sg_buf = (char*)malloc( read_len+1 ) ;
		if( sg_buf == NULL )
			return RFS_ERROR_ALLOC;
		sg_buf_size = read_len+1 ;
	}
	else if( sg_buf_size < read_len+1 )
	{
		char	*tmp = NULL ;
		
		tmp = (char*)realloc( sg_buf , read_len+1 ) ;
		if( tmp == NULL )
			return RFS_ERROR_ALLOC;
		sg_buf = tmp ;
		sg_buf_size = read_len+1 ;
	}
	
	INFOLOGC( "call read[%d][0x%X][%d] ..." , p_session->file_fd , sg_buf , read_len )
	data_len = read( p_session->file_fd , sg_buf , read_len ) ;
	if( data_len == -1 )
	{
		ERRORLOGC( "call read[%s][%d] failed[%d] , errno[%d] , read_len[%d]" , p_session->pathfilename , p_session->file_fd , nret , errno , data_len )
	}
	else
	{
		INFOLOGC( "call read[%s][%d] ok , read_len[%d]" , p_session->pathfilename , p_session->file_fd , data_len )
	}
	
	nret = RFSSendL4VString( accepted_sock , sg_buf , data_len , p_elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendL4VString data_len[%d] failed[%d]" , data_len , nret )
		return nret;
	}
	else
	{
		DEBUGLOGC( "RFSSendL4VString data_len[%d] ok" , data_len )
		DEBUGHEXLOGC( sg_buf , data_len , "data_len[%d]bytes" , data_len )
	}
	
	nret = RFSSendInt4( accepted_sock , errno , p_elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendInt4 errno failed[%d]" , errno , nret )
		return nret;
	}
	else
	{
		DEBUGLOGC( "RFSSendInt4 errno ok" , errno )
	}
	
	if( sg_buf_size > 10*1024*1024 )
	{
		free( sg_buf ); sg_buf = NULL ;
		sg_buf_size = 0 ;
	}
	
	return 0;
}

int rwrite( int accepted_sock , struct timeval *p_elapse , struct RemoteFileSession *p_session )
{
	int		write_len ;
	int		wrote_len ;
	char		*p_buf = NULL ;
	
	int		nret = 0 ;
	
	if( p_session->file_fd == -1 )
		return RFS_ERROR_INTERNAL;
	
	SECONDS_TO_TIMEVAL( 60 , (*p_elapse) )
	
	nret = RFSReceiveInt4( accepted_sock , & (p_session->file_fd) , p_elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSReceiveInt4 fd failed[%d]" , nret )
		return nret;
	}
	else
	{
		DEBUGLOGC( "RFSReceiveInt4 fd[%d] ok" , p_session->file_fd )
	}
	
	nret = RFSReceiveL4VString_DUP( accepted_sock , & p_buf , & write_len , p_elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSReceiveL4VString_DUP write_len failed[%d]" , nret )
		return nret;
	}
	else
	{
		DEBUGLOGC( "RFSReceiveL4VString_DUP write_len[%d] ok" , write_len )
		DEBUGHEXLOGC( p_buf , write_len , "data_len[%d]bytes" , write_len )
	}
	
	INFOLOGC( "call write[%d][0x%X][%d] ..." , p_session->file_fd , p_buf , write_len )
	wrote_len = write( p_session->file_fd , p_buf , write_len ) ;
	if( wrote_len == -1 )
	{
		ERRORLOGC( "call write[%s][%d] failed[%d] , errno[%d] , write_len[%d]" , p_session->pathfilename , p_session->file_fd , nret , errno , write_len )
	}
	else
	{
		INFOLOGC( "call write[%s][%d] ok , write_len[%d]" , p_session->pathfilename , p_session->file_fd , write_len )
	}
	
	nret = RFSSendInt4( accepted_sock , wrote_len , p_elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendInt4 wrote[%d] failed[%d]" , wrote_len , nret )
		return nret;
	}
	else
	{
		DEBUGLOGC( "RFSSendInt4 wrote[%d] ok" , wrote_len )
	}
	
	nret = RFSSendInt4( accepted_sock , errno , p_elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendInt4 errno failed[%d]" , errno , nret )
		return nret;
	}
	else
	{
		DEBUGLOGC( "RFSSendInt4 errno ok" , errno )
	}
	
	return 0;
}

