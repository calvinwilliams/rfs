#include "in.h"

LINK_RBTREENODE_INT( LinkLocalFdsTreeNodeByLocalFd , struct LocalFds , local_fds_rbtree_order_by_local_fd , struct LocalFd , local_fd_rbnode_by_order_by_local_fd , local_fd )
QUERY_RBTREENODE_INT( QueryLocalFdsTreeNodeByLocalFd , struct LocalFds , local_fds_rbtree_order_by_local_fd , struct LocalFd , local_fd_rbnode_by_order_by_local_fd , local_fd )
UNLINK_RBTREENODE( UnlinkLocalFdsTreeNodeByLocalFd , struct LocalFds , local_fds_rbtree_order_by_local_fd , struct LocalFd , local_fd_rbnode_by_order_by_local_fd )
TRAVEL_RBTREENODE( TravelLocalFdsTreeByLocalFd , struct LocalFds , local_fds_rbtree_order_by_local_fd , struct LocalFd , local_fd_rbnode_by_order_by_local_fd )
DESTROY_RBTREE( DestroyLocalFdsTree , struct LocalFds , local_fds_rbtree_order_by_local_fd , struct LocalFd , local_fd_rbnode_by_order_by_local_fd , FREE_RBTREENODEENTRY_DIRECTLY )

LINK_RBTREENODE_POINTER( LinkLocalFpsTreeNodeByLocalFp , struct LocalFps , local_fps_rbtree_order_by_local_fp , struct LocalFp , local_fp_rbnode_by_order_by_local_fp , local_fp )
QUERY_RBTREENODE_POINTER( QueryLocalFpsTreeNodeByLocalFp , struct LocalFps , local_fps_rbtree_order_by_local_fp , struct LocalFp , local_fp_rbnode_by_order_by_local_fp , local_fp )
UNLINK_RBTREENODE( UnlinkLocalFpsTreeNodeByLocalFp , struct LocalFps , local_fps_rbtree_order_by_local_fp , struct LocalFp , local_fp_rbnode_by_order_by_local_fp )
TRAVEL_RBTREENODE( TravelLocalFpsTreeByLocalFp , struct LocalFps , local_fps_rbtree_order_by_local_fp , struct LocalFp , local_fp_rbnode_by_order_by_local_fp )
DESTROY_RBTREE( DestroyLocalFpsTree , struct LocalFps , local_fps_rbtree_order_by_local_fp , struct LocalFp , local_fp_rbnode_by_order_by_local_fp , FREE_RBTREENODEENTRY_DIRECTLY )

int rfs_open( int accepted_sock , struct LocalFds *p_local_fds , struct LocalFps *p_local_fps , struct timeval *p_elapse )
{
	char		pathfilename[ (1<<16) + 1 ] ;
	uint16_t	pathfilename_len ;
	uint32_t	flags ;
	
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

int rfs_open3( int accepted_sock , struct LocalFds *p_local_fds , struct LocalFps *p_local_fps , struct timeval *p_elapse )
{
	char		pathfilename[ (1<<16) + 1 ] ;
	uint16_t	pathfilename_len ;
	uint32_t	flags ;
	uint32_t	mode ;
	
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

int rfs_close( int accepted_sock , struct LocalFds *p_local_fds , struct LocalFps *p_local_fps , struct timeval *p_elapse )
{
	uint32_t	file_fd ;
	
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

int rfs_read( int accepted_sock , struct LocalFds *p_local_fds , struct LocalFps *p_local_fps , struct timeval *p_elapse )
{
	uint32_t	file_fd ;
	
	struct LocalFd	local_fd ;
	struct LocalFd	*p_local_fd = NULL ;
	
	uint32_t	read_len ;
	uint32_t	data_len ;
	
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

int rfs_write( int accepted_sock , struct LocalFds *p_local_fds , struct LocalFps *p_local_fps , struct timeval *p_elapse )
{
	uint32_t	file_fd ;
	
	struct LocalFd	local_fd ;
	struct LocalFd	*p_local_fd = NULL ;
	
	uint32_t	write_len ;
	uint32_t	wrote_len ;
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

#if 0
int reof( int accepted_sock , struct LocalFds *p_local_fds , struct timeval *p_elapse )
{
	int		file_fd ;
	
	struct LocalFd	local_fd ;
	struct LocalFd	*p_local_fd = NULL ;
	struct stat	stat ;
	off_t		offset ;
	
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
		INFOLOGC( "call eof[%d] ..." , file_fd )
		
		memset( & stat , 0x00 , sizeof(struct stat) );
		ret = fstat( file_fd , & stat ) ;
		if( ret == -1 )
		{
			ERRORLOGC( "call fstat[%d] failed[%d] , errno[%d]" , file_fd , ret , errno )
		}
		else
		{
			INFOLOGC( "call fstat[%d] ok" , file_fd )
			
			offset = (int)lseek( file_fd , 0 , SEEK_CUR ) ;
			if( offset == -1 )
			{
				ERRORLOGC( "call lseek[%d] failed[%d] , errno[%d]" , file_fd , offset , errno )
				ret = offset ;
			}
			else
			{
				INFOLOGC( "call lseek[%d] ok" , file_fd )
				
				if( ret == stat.st_size )
					ret = 1 ;
				else
					ret = 0 ;
			}
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
#endif

int rfs_fopen( int accepted_sock , struct LocalFds *p_local_fds , struct LocalFps *p_local_fps , struct timeval *p_elapse )
{
	char		pathfilename[ (1<<16) + 1 ] ;
	uint16_t	pathfilename_len ;
	char		mode[ (1<<8) + 1 ] ;
	
	FILE		*file_fp = NULL ;
	
	struct LocalFp	*p_local_fp = NULL ;
	
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
	
	memset( mode , 0x00 , sizeof(mode) );
	nret = RFSReceiveL1VString( accepted_sock , mode , p_elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSReceiveL1VString mode failed[%d]" , nret )
		return nret;
	}
	else
	{
		DEBUGLOGC( "RFSReceiveL1VString mode[%s] ok" , mode )
	}
	
	INFOLOGC( "call open[%s][%s] ..." , pathfilename , mode )
	file_fp = fopen( pathfilename , mode ) ;
	if( file_fp == NULL )
	{
		ERRORLOGC( "call fopen[%s][%s] failed , errno[%d]" , pathfilename , mode , errno )
	}
	else
	{
		INFOLOGC( "call fopen[%s][%s] ok , fp[%p]" , pathfilename , mode , file_fp )
		
		p_local_fp = (struct LocalFp *)malloc( sizeof(struct LocalFp) ) ;
		if( p_local_fp == NULL )
		{
			ERRORLOGC( "malloc failed , errno[%d]" , errno )
			fclose( file_fp ); file_fp = NULL ;
		}
		else
		{
			memset( p_local_fp , 0x00 , sizeof(struct LocalFp) );
			p_local_fp->local_fp = file_fp ;
			nret = LinkLocalFpsTreeNodeByLocalFp( p_local_fps , p_local_fp ) ;
			if( nret )
			{
				ERRORLOGC( "LinkLocalFdsTreeNodeByLocalFd failed[%d]" , nret )
				fclose( file_fp ); file_fp = NULL ;
			}
		}
	}
	
	nret = RFSSendInt8( accepted_sock , (uint64_t) file_fp , p_elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendInt8 fp[%p] failed[%d]" , file_fp , nret )
		return nret;
	}
	else
	{
		DEBUGLOGC( "RFSSendInt8 fp[%p] ok" , file_fp )
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

int rfs_fclose( int accepted_sock , struct LocalFds *p_local_fds , struct LocalFps *p_local_fps , struct timeval *p_elapse )
{
	FILE		*file_fp = NULL ;
	
	struct LocalFp	local_fp ;
	struct LocalFp	*p_local_fp = NULL ;
	
	int		ret ;
	
	int		nret = 0 ;
	
	SECONDS_TO_TIMEVAL( 60 , (*p_elapse) )
	
	nret = RFSReceiveInt8( accepted_sock , (uint64_t*) & file_fp , p_elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSReceiveInt8 file_fp failed[%d]" , nret )
		return nret;
	}
	else
	{
		DEBUGLOGC( "RFSReceiveInt8 file_fp[%p] ok" , file_fp )
	}
	
	memset( & local_fp , 0x00 , sizeof(struct LocalFp) );
	local_fp.local_fp = file_fp ;
	p_local_fp = QueryLocalFpsTreeNodeByLocalFp( p_local_fps , & local_fp ) ;
	if( p_local_fp == NULL )
	{
		ERRORLOGC( "file_fp[%p] is not opened" , file_fp )
		ret = -1 ;
		errno = EINVAL ;
	}
	else
	{
		INFOLOGC( "call fclose[%p] ..." , file_fp )
		ret = fclose( file_fp ) ;
		if( ret == -1 )
		{
			ERRORLOGC( "call fclose[%p] failed[%d] , errno[%d]" , file_fp , ret , errno )
		}
		else
		{
			INFOLOGC( "call fclose[%d] ok" , file_fp )
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
	
	if( p_local_fp )
	{
		UnlinkLocalFpsTreeNodeByLocalFp( p_local_fps , p_local_fp );
		free( p_local_fp );
	}
	
	return 0;
}

