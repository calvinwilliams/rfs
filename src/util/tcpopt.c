#include "rfs_util.h"

void RFSSetTcpReuseAddr( int sock )
{
	int	onoff = 1 ;
	
	setsockopt( sock , SOL_SOCKET , SO_REUSEADDR , (void *) & onoff , sizeof(int) );
	
	return;
}

void RFSSetTcpNonblock( int sock )
{
	int	opts ;
	
	opts = fcntl( sock , F_GETFL ) ;
	opts |= O_NONBLOCK ;
	fcntl( sock , F_SETFL , opts ) ;
	
	return;
}

void RFSSetTcpBlock( int sock )
{
	int	opts ;
	
	opts = fcntl( sock , F_GETFL ) ;
	opts &= ~O_NONBLOCK ;
	fcntl( sock , F_SETFL , opts ) ;
	
	return;
}

void RFSSetTcpNodelay( int sock , int onoff )
{
	setsockopt( sock , IPPROTO_TCP , TCP_NODELAY , (void*) & onoff , sizeof(int) );
	
	return;
}

void RFSSetTcpLinger( int sock , int onoff , int linger )
{
	struct linger   lg ;
	
	lg.l_onoff = onoff ;
	lg.l_linger = linger ;
	setsockopt( sock , SOL_SOCKET , SO_LINGER , (void*) & lg , sizeof(struct linger) );
	
	return;
}

void RFSSetTcpCloseExec( int sock )
{
	int	val ;
	
	val = fcntl( sock , F_GETFD ) ;
	val |= FD_CLOEXEC ;
	fcntl( sock , F_SETFD , val );
	
	return;
}

