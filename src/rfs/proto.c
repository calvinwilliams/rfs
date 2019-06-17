#include "in.h"

LINK_RBTREENODE_INT( LinkLocalFdsTreeNodeByLocalFd , struct LocalFds , local_fds_rbtree_order_by_local_fd , struct LocalFd , local_fd_rbnode_by_order_by_local_fd , local_fd )
QUERY_RBTREENODE_INT( QueryLocalFdsTreeNodeByLocalFd , struct LocalFds , local_fds_rbtree_order_by_local_fd , struct LocalFd , local_fd_rbnode_by_order_by_local_fd , local_fd )
UNLINK_RBTREENODE( UnlinkLocalFdsTreeNodeByLocalFd , struct LocalFds , local_fds_rbtree_order_by_local_fd , struct LocalFd , local_fd_rbnode_by_order_by_local_fd )
TRAVEL_RBTREENODE( TravelLocalFdsTreeByLocalFd , struct LocalFds , local_fds_rbtree_order_by_local_fd , struct LocalFd , local_fd_rbnode_by_order_by_local_fd )
DESTROY_RBTREE( DestroyLocalFdsTree , struct LocalFds , local_fds_rbtree_order_by_local_fd , struct LocalFd , local_fd_rbnode_by_order_by_local_fd , FREE_RBTREENODEENTRY_DIRECTLY )

int ropen( int accepted_sock , struct LocalFds *p_local_fds , struct timeval *p_elapse )
{
	char		pathfilename[ (1<<16) + 1 ] ;
	int		pathfilename_len ;
	int		flags ;
	
	int		file_fd ;
	
	struct LocalFd	*p_local_fd = NULL ;
	
	int		nret = 0 ;
	
	SECONDS_TO_TIMEVAL( 60 , (*p_elapse) )
	
	memset( pathfilename , 0x00 , sizeof(pathfilename) );
	nret = RFSReceiveL2VString( accepted_sock , pathfilename , & pathfilename_len , p_elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSReceiveL2VString pathfilename failed[%d]" , nret )
		return nret;
	}
	else
	{
		DEBUGLOGC( "RFSReceiveL2VString pathfilename[%s] ok" , pathfilename )
	}
	
	nret = RFSReceiveInt4( accepted_sock , & flags , p_elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSReceiveInt4 flags failed[%d]" , nret )
		return nret;
	}
	else
	{
		DEBUGLOGC( "RFSReceiveInt4 flags[%d] ok" , flags )
	}
	
	INFOLOGC( "call open[%s][%d] ..." , pathfilename , flags )
	file_fd = open( pathfilename , flags ) ;
	if( file_fd == -1 )
	{
		ERRORLOGC( "call open[%s][%d] failed[%d] , errno[%d]" , pathfilename , flags , file_fd , errno )
	}
	else
	{
		INFOLOGC( "call open[%s][%d] ok , fd[%d]" , pathfilename , flags , file_fd )
		
		p_local_fd = (struct LocalFd *)malloc( sizeof(struct LocalFd) ) ;
		if( p_local_fd == NULL )
		{
			ERRORLOGC( "malloc failed , errno[%d]" , errno )
			close( file_fd ); file_fd = -1 ;
		}
		else
		{
			memset( p_local_fd , 0x00 , sizeof(struct LocalFd) );
			p_local_fd->local_fd = file_fd ;
			nret = LinkLocalFdsTreeNodeByLocalFd( p_local_fds , p_local_fd ) ;
			if( nret )
			{
				ERRORLOGC( "LinkLocalFdsTreeNodeByLocalFd failed[%d]" , nret )
				close( file_fd ); file_fd = -1 ;
			}
		}
	}
	
	nret = RFSSendInt4( accepted_sock , file_fd , p_elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendInt4 fd[%d] failed[%d]" , file_fd , nret )
		return nret;
	}
	else
	{
		DEBUGLOGC( "RFSSendInt4 fd[%d] ok" , file_fd )
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

int ropen3( int accepted_sock , struct LocalFds *p_local_fds , struct timeval *p_elapse )
{
	char		pathfilename[ (1<<16) + 1 ] ;
	int		pathfilename_len ;
	int		flags ;
	int		mode ;
	
	int		file_fd ;
	
	struct LocalFd	*p_local_fd = NULL ;
	
	int		nret = 0 ;
	
	SECONDS_TO_TIMEVAL( 60 , (*p_elapse) )
	
	memset( pathfilename , 0x00 , sizeof(pathfilename) );
	nret = RFSReceiveL2VString( accepted_sock , pathfilename , & pathfilename_len , p_elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSReceiveL2VString pathfilename failed[%d]" , nret )
		return nret;
	}
	else
	{
		DEBUGLOGC( "RFSReceiveL2VString pathfilename[%s] ok" , pathfilename )
	}
	
	nret = RFSReceiveInt4( accepted_sock , & flags , p_elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSReceiveInt4 flags failed[%d]" , nret )
		return nret;
	}
	else
	{
		DEBUGLOGC( "RFSReceiveInt4 flags[%d] ok" , flags )
	}
	
	nret = RFSReceiveInt4( accepted_sock , & mode , p_elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSReceiveInt4 mode failed[%d]" , nret )
		return nret;
	}
	else
	{
		DEBUGLOGC( "RFSReceiveInt4 mode[%d] ok" , mode )
	}
	
	INFOLOGC( "call open[%s][%d][%d] ..." , pathfilename , flags , mode )
	file_fd = open( pathfilename , flags , mode ) ;
	if( file_fd == -1 )
	{
		ERRORLOGC( "call open[%s][%d][%d] failed[%d] , errno[%d]" , pathfilename , flags , mode , file_fd , errno )
	}
	else
	{
		DEBUGLOGC( "call open[%s][%d][%d] ok , fd[%d]" , pathfilename , flags , mode , file_fd )
		
		p_local_fd = (struct LocalFd *)malloc( sizeof(struct LocalFd) ) ;
		if( p_local_fd == NULL )
		{
			ERRORLOGC( "malloc failed , errno[%d]" , errno )
			close( file_fd ); file_fd = -1 ;
		}
		else
		{
			memset( p_local_fd , 0x00 , sizeof(struct LocalFd) );
			p_local_fd->local_fd = file_fd ;
			nret = LinkLocalFdsTreeNodeByLocalFd( p_local_fds , p_local_fd ) ;
			if( nret )
			{
				ERRORLOGC( "LinkLocalFdsTreeNodeByLocalFd failed[%d]" , nret )
				close( file_fd ); file_fd = -1 ;
			}
		}
	}
	
	nret = RFSSendInt4( accepted_sock , file_fd , p_elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendInt4 fd[%d] failed[%d]" , file_fd , nret )
		return nret;
	}
	else
	{
		DEBUGLOGC( "RFSSendInt4 fd[%d] ok" , file_fd )
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

int rclose( int accepted_sock , struct LocalFds *p_local_fds , struct timeval *p_elapse )
{
	int		file_fd ;
	
	struct LocalFd	local_fd ;
	struct LocalFd	*p_local_fd = NULL ;
	
	int		ret ;
	
	int		nret = 0 ;
	
	SECONDS_TO_TIMEVAL( 60 , (*p_elapse) )
	
	nret = RFSReceiveInt4( accepted_sock , & file_fd , p_elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSReceiveInt4 flags failed[%d]" , nret )
		return nret;
	}
	else
	{
		DEBUGLOGC( "RFSReceiveInt4 flags[%d] ok" , file_fd )
	}
	
	memset( & local_fd , 0x00 , sizeof(struct LocalFd) );
	local_fd.local_fd = file_fd ;
	p_local_fd = QueryLocalFdsTreeNodeByLocalFd( p_local_fds , & local_fd ) ;
	if( p_local_fd == NULL )
	{
		ERRORLOGC( "file_fd[%d] is not opened" , file_fd )
		ret = -1 ;
		errno = EINVAL ;
	}
	else
	{
		INFOLOGC( "call close[%d] ..." , file_fd )
		ret = close( file_fd ) ;
		if( ret == -1 )
		{
			ERRORLOGC( "call close[%d] failed[%d] , errno[%d]" , file_fd , ret , errno )
		}
		else
		{
			INFOLOGC( "call close[%d] ok" , file_fd )
		}
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
	
	if( p_local_fd )
	{
		UnlinkLocalFdsTreeNodeByLocalFd( p_local_fds , p_local_fd );
		free( p_local_fd );
	}
	
	return 0;
}

static __thread char		*sg_buf = NULL ;
static __thread uint32_t	sg_buf_size = 0 ;

int rread( int accepted_sock , struct LocalFds *p_local_fds , struct timeval *p_elapse )
{
	int		file_fd ;
	
	struct LocalFd	local_fd ;
	struct LocalFd	*p_local_fd = NULL ;
	
	int		read_len ;
	int		data_len ;
	
	int		nret = 0 ;
	
	SECONDS_TO_TIMEVAL( 60 , (*p_elapse) )
	
	nret = RFSReceiveInt4( accepted_sock , & file_fd , p_elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSReceiveInt4 fd failed[%d]" , nret )
		return nret;
	}
	else
	{
		DEBUGLOGC( "RFSReceiveInt4 fd[%d] ok" , file_fd )
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
	
	memset( & local_fd , 0x00 , sizeof(struct LocalFd) );
	local_fd.local_fd = file_fd ;
	p_local_fd = QueryLocalFdsTreeNodeByLocalFd( p_local_fds , & local_fd ) ;
	if( p_local_fd == NULL )
	{
		ERRORLOGC( "file_fd[%d] is not opened" , file_fd )
		data_len = -1 ;
		errno = EINVAL ;
	}
	else
	{
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
		
		INFOLOGC( "call read[%d][0x%X][%d] ..." , file_fd , sg_buf , read_len )
		data_len = read( file_fd , sg_buf , read_len ) ;
		if( data_len == -1 )
		{
			ERRORLOGC( "call read[%d] failed[%d] , errno[%d] , read_len[%d]" , file_fd , nret , errno , data_len )
		}
		else
		{
			INFOLOGC( "call read[%d] ok , read_len[%d]" , file_fd , data_len )
		}
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

int rwrite( int accepted_sock , struct LocalFds *p_local_fds , struct timeval *p_elapse )
{
	int		file_fd ;
	
	struct LocalFd	local_fd ;
	struct LocalFd	*p_local_fd = NULL ;
	
	int		write_len ;
	int		wrote_len ;
	char		*p_buf = NULL ;
	
	int		nret = 0 ;
	
	SECONDS_TO_TIMEVAL( 60 , (*p_elapse) )
	
	nret = RFSReceiveInt4( accepted_sock , & file_fd , p_elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSReceiveInt4 fd failed[%d]" , nret )
		return nret;
	}
	else
	{
		DEBUGLOGC( "RFSReceiveInt4 fd[%d] ok" , file_fd )
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
	
	memset( & local_fd , 0x00 , sizeof(struct LocalFd) );
	local_fd.local_fd = file_fd ;
	p_local_fd = QueryLocalFdsTreeNodeByLocalFd( p_local_fds , & local_fd ) ;
	if( p_local_fd == NULL )
	{
		ERRORLOGC( "file_fd[%d] is not opened" , file_fd )
		wrote_len = -1 ;
		errno = EINVAL ;
	}
	else
	{
		INFOLOGC( "call write[%d][0x%X][%d] ..." , file_fd , p_buf , write_len )
		wrote_len = write( file_fd , p_buf , write_len ) ;
		if( wrote_len == -1 )
		{
			ERRORLOGC( "call write[%d] failed[%d] , errno[%d] , write_len[%d]" , file_fd , nret , errno , write_len )
		}
		else
		{
			INFOLOGC( "call write[%d] ok , write_len[%d]" , file_fd , write_len )
		}
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

