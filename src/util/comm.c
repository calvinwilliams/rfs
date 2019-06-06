#include "rfs_util.h"

static int RFSSendData( int sock , char *data , uint64_t data_len , uint64_t *p_sent_len , struct timeval *p_elapse )
{
	DEF_TIMEVAL_FOR_DIFF
	uint64_t		sent_len ;
	uint64_t		remain_len ;
	int			timeout ;
	struct pollfd		pollfd ;
	uint64_t		len ;
	
	int			nret = 0 ;
	
	sent_len = 0 ;
	remain_len = data_len ;
	while( remain_len > 0 )
	{
		GET_BEGIN_TIMEVAL
		
		TIMEVAL_TO_MILLISECONDS( (*p_elapse) , timeout )
		pollfd.fd = sock ;
		pollfd.events = POLLOUT|POLLERR ;
		nret = poll( & pollfd , 1 , timeout ) ;
		
		GET_END_TIMEVAL_AND_DIFF
		REDUCE_TIMEVAL( (*p_elapse) , DIFF_TIMEVAL )
		
		if( nret == -1 )
		{
			if( p_sent_len )
				(*p_sent_len) = sent_len ;
			return RFS_ERROR_POLL;
		}
		else if( nret == 0 )
		{
			if( p_sent_len )
				(*p_sent_len) = sent_len ;
			return RFS_ERROR_SENDING_TIMEOUT;
		}
		else
		{
			if( pollfd.revents & POLLERR )
			{
				if( p_sent_len )
					(*p_sent_len) = sent_len ;
				return RFS_ERROR_SENDING_2;
			}
		}
		
		len = write( sock , data+sent_len , remain_len ) ;
		if( len == -1 )
		{
			if( p_sent_len )
				(*p_sent_len) = sent_len ;
			return RFS_ERROR_SENDING;
		}
		
		sent_len += len ;
		remain_len -= len ;
	}
	
	if( p_sent_len )
		(*p_sent_len) = sent_len ;
	
	return 0;
}

static int RFSReceiveData( int sock , char *data , uint64_t data_len , uint64_t *p_received_len , struct timeval *p_elapse )
{
	DEF_TIMEVAL_FOR_DIFF
	uint64_t		received_len ;
	uint64_t		remain_len ;
	int			timeout ;
	struct pollfd		pollfd ;
	uint64_t		len ;
	
	int			nret = 0 ;
	
	received_len = 0 ;
	remain_len = data_len ;
	while( remain_len > 0 )
	{
		GET_BEGIN_TIMEVAL
		
		TIMEVAL_TO_MILLISECONDS( (*p_elapse) , timeout )
		pollfd.fd = sock ;
		pollfd.events = POLLIN|POLLERR ;
		nret = poll( & pollfd , 1 , timeout ) ;
		
		GET_END_TIMEVAL_AND_DIFF
		REDUCE_TIMEVAL( (*p_elapse) , DIFF_TIMEVAL )
		
		if( nret == -1 )
		{
			if( p_received_len )
				(*p_received_len) = received_len ;
			return RFS_ERROR_POLL;
		}
		else if( nret == 0 )
		{
			if( p_received_len )
				(*p_received_len) = received_len ;
			return RFS_ERROR_SRECEIVING_TIMEOUT;
		}
		else
		{
			if( pollfd.revents & POLLERR )
			{
				if( p_received_len )
					(*p_received_len) = received_len ;
				return RFS_ERROR_RECEIVING_2;
			}
		}
		
		len = read( sock , data+received_len , remain_len ) ;
		if( len == 0 )
		{
			if( p_received_len )
				(*p_received_len) = received_len ;
			return RFS_ERROR_CLOSED;
		}
		else if( len == -1 )
		{
			if( p_received_len )
				(*p_received_len) = received_len ;
			return RFS_ERROR_RECEIVING;
		}
		
		received_len += len ;
		remain_len -= len ;
	}
	
	if( p_received_len )
		(*p_received_len) = received_len ;
	
	return 0;
}

int RFSSendInt4( int sock , uint32_t h4 , struct timeval *p_elapse )
{
	uint32_t	n4 ;
	
	n4 = htonl(h4) ;
	return RFSSendData( sock , (char*) & n4 , sizeof(uint32_t) , NULL , p_elapse );
}

int RFSReceiveInt4( int sock , uint32_t *p_h4 , struct timeval *p_elapse )
{
	uint32_t	n4 ;
	
	int		nret = 0 ;
	
	nret = RFSReceiveData( sock , (char*) & n4 , sizeof(uint32_t) , NULL , p_elapse );
	if( nret )
		return nret;
	
	(*p_h4) = ntohl( n4 ) ;
	
	return 0;
}

int RFSSendChar( int sock , char ch , struct timeval *p_elapse )
{
	return RFSSendData( sock , & ch , 1 , NULL , p_elapse );
}

int RFSReceiveChar( int sock , char *p_ch , struct timeval *p_elapse )
{
	return RFSReceiveData( sock , p_ch , 1 , NULL , p_elapse );
}

int RFSSendString( int sock , char *buf , uint64_t data_len , struct timeval *p_elapse )
{
	return RFSSendData( sock , buf , data_len , NULL , p_elapse );
}

int RFSReceiveString( int sock , char *buf , uint64_t data_len , struct timeval *p_elapse )
{
	return RFSReceiveData( sock , buf , data_len , NULL , p_elapse );
}

int RFSSendL2VString( int sock , char *buf , uint16_t data_len , struct timeval *p_elapse )
{
	uint16_t	n2 ;
	
	int		nret = 0 ;
	
	n2 = htons(data_len) ;
	
	nret = RFSSendData( sock , (char*) & n2 , sizeof(uint16_t) , NULL , p_elapse ) ;
	if( nret )
		return nret;
	
	nret = RFSSendData( sock , buf , data_len , NULL , p_elapse ) ;
	if( nret )
		return nret;
	
	return 0;
}

int RFSReceiveL2VString( int sock , char *buf , uint16_t *p_data_len , struct timeval *p_elapse )
{
	uint16_t	n2 ;
	uint16_t	data_len ;
	
	int		nret = 0 ;
	
	nret = RFSReceiveData( sock , (char*) & n2 , sizeof(uint16_t) , NULL , p_elapse ) ;
	if( nret )
		return nret;
	
	data_len = ntohs( n2 ) ;
	
	nret = RFSReceiveData( sock , buf , data_len , NULL , p_elapse ) ;
	if( nret )
		return nret;
	
	if( p_data_len )
		(*p_data_len) = data_len ;
	
	return 0;
}

int RFSSendL4VString( int sock , char *buf , uint32_t data_len , struct timeval *p_elapse )
{
	uint32_t	n4 ;
	
	int		nret = 0 ;
	
	n4 = htonl(data_len) ;
	
	nret = RFSSendData( sock , (char*) & n4 , sizeof(uint32_t) , NULL , p_elapse ) ;
	if( nret )
		return nret;
	
	nret = RFSSendData( sock , buf , data_len , NULL , p_elapse ) ;
	if( nret )
		return nret;
	
	return 0;
}

int RFSReceiveL4VString_DUP( int sock , char **s_buf_ptr , uint32_t *p_data_len , struct timeval *p_elapse )
{
	uint32_t	n4 ;
	uint32_t	data_len ;
	static char	*s_buf = NULL ;
	static uint32_t	s_buf_size ;
	
	int		nret = 0 ;
	
	nret = RFSReceiveData( sock , (char*) & n4 , sizeof(uint32_t) , NULL , p_elapse ) ;
	if( nret )
		return nret;
	
	data_len = ntohl( n4 ) ;
	if( s_buf == NULL )
	{
		s_buf = (char*)malloc( data_len+1 ) ;
		if( s_buf == NULL )
			return RFS_ERROR_ALLOC;
		s_buf_size = data_len+1 ;
	}
	else if( s_buf_size < data_len+1 )
	{
		char	*tmp = NULL ;
		
		tmp = (char*)realloc( s_buf , data_len+1 ) ;
		if( tmp == NULL )
			return RFS_ERROR_ALLOC;
		s_buf = tmp ;
		s_buf_size = data_len+1 ;
	}
	
	nret = RFSReceiveData( sock , s_buf , data_len , NULL , p_elapse ) ;
	if( nret )
		return nret;
	
	if( s_buf_size > 10*1024*1024 )
	{
		free( s_buf ); s_buf = NULL ;
		s_buf_size = 0 ;
	}
	
	if( s_buf_ptr )
		(*s_buf_ptr) = s_buf ;
	if( p_data_len )
		(*p_data_len) = data_len ;
	
	return 0;
}

