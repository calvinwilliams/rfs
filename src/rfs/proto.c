#include "in.h"

int ropen( int accepted_sock , struct timeval *p_elapse , struct RemoteFileSession *p_session )
{
	int		nret = 0 ;
	
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
		INFOLOGC( "RFSReceiveL2VString pathfilename ok" )
		DEBUGHEXLOGC( p_session->pathfilename , p_session->pathfilename_len , "len[%u]" , p_session->pathfilename_len )
	}
	
	nret = RFSReceiveInt4( accepted_sock , & (p_session->flags) , p_elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSReceiveInt4 flags failed[%d]" , nret )
		return nret;
	}
	else
	{
		INFOLOGC( "RFSReceiveInt4 flags ok" )
		DEBUGHEXLOGC( (char*) & (p_session->flags) , sizeof(int) , "len[%d]" , sizeof(int) )
	}
	
	p_session->fd = open( p_session->pathfilename , p_session->flags ) ;
	
	nret = RFSSendInt4( accepted_sock , p_session->fd , p_elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendInt4 fd failed[%d]" , nret )
		return nret;
	}
	else
	{
		INFOLOGC( "RFSSendInt4 fd ok" )
		DEBUGHEXLOGC( (char*) & (p_session->fd) , sizeof(int) , "len[%d]" , sizeof(int) )
	}
	
	nret = RFSSendInt4( accepted_sock , errno , p_elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendInt4 errno failed[%d]" , nret )
		return nret;
	}
	else
	{
		INFOLOGC( "RFSSendInt4 errno ok" )
		DEBUGHEXLOGC( (char*) & errno , sizeof(int) , "len[%d]" , sizeof(int) )
	}
	
	return 0;
}

int ropen3( int accepted_sock , struct timeval *p_elapse , struct RemoteFileSession *p_session )
{
	int		nret = 0 ;
	
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
		INFOLOGC( "RFSReceiveL2VString pathfilename ok" )
		DEBUGHEXLOGC( p_session->pathfilename , p_session->pathfilename_len , "len[%u]" , p_session->pathfilename_len )
	}
	
	nret = RFSReceiveInt4( accepted_sock , & (p_session->flags) , p_elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSReceiveInt4 flags failed[%d]" , nret )
		return nret;
	}
	else
	{
		INFOLOGC( "RFSReceiveInt4 flags ok" )
		DEBUGHEXLOGC( (char*) & (p_session->flags) , sizeof(int) , "len[%d]" , sizeof(int) )
	}
	
	nret = RFSReceiveInt4( accepted_sock , & (p_session->mode) , p_elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSReceiveInt4 mode failed[%d]" , nret )
		return nret;
	}
	else
	{
		INFOLOGC( "RFSReceiveInt4 flags ok" )
		DEBUGHEXLOGC( (char*) & (p_session->mode) , sizeof(int) , "len[%d]" , sizeof(int) )
	}
	
	p_session->fd = open( p_session->pathfilename , p_session->flags , p_session->mode ) ;
	
	nret = RFSSendInt4( accepted_sock , p_session->fd , p_elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendInt4 fd failed[%d]" , nret )
		return nret;
	}
	else
	{
		INFOLOGC( "RFSSendInt4 fd ok" )
		DEBUGHEXLOGC( (char*) & (p_session->fd) , sizeof(int) , "len[%d]" , sizeof(int) )
	}
	
	nret = RFSSendInt4( accepted_sock , errno , p_elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendInt4 errno failed[%d]" , nret )
		return nret;
	}
	else
	{
		INFOLOGC( "RFSSendInt4 errno ok" )
		DEBUGHEXLOGC( (char*) & errno , sizeof(int) , "len[%d]" , sizeof(int) )
	}
	
	return 0;
}

int rclose( int accepted_sock , struct timeval *p_elapse , struct RemoteFileSession *p_session )
{
	int		nret = 0 ;
	
	SECONDS_TO_TIMEVAL( 60 , (*p_elapse) )
	
	nret = RFSReceiveInt4( accepted_sock , & (p_session->fd) , p_elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSReceiveInt4 flags failed[%d]" , nret )
		return nret;
	}
	else
	{
		INFOLOGC( "RFSReceiveInt4 flags ok" )
		DEBUGHEXLOGC( (char*) & (p_session->fd) , sizeof(int) , "len[%d]" , sizeof(int) )
	}
	
	nret = close( p_session->fd ) ;
	
	nret = RFSSendInt4( accepted_sock , nret , p_elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendInt4 fd failed[%d]" , nret )
		return nret;
	}
	else
	{
		INFOLOGC( "RFSSendInt4 fd ok" )
		DEBUGHEXLOGC( (char*) & nret , sizeof(int) , "len[%d]" , sizeof(int) )
	}
	
	nret = RFSSendInt4( accepted_sock , errno , p_elapse ) ;
	if( nret )
	{
		ERRORLOGC( "RFSSendInt4 errno failed[%d]" , nret )
		return nret;
	}
	else
	{
		INFOLOGC( "RFSSendInt4 errno ok" )
		DEBUGHEXLOGC( (char*) & errno , sizeof(int) , "len[%d]" , sizeof(int) )
	}
	
	return 0;
}

int rread( int accepted_sock , struct timeval *p_elapse , struct RemoteFileSession *p_session )
{
	return 0;
}

int rwrite( int accepted_sock , struct timeval *p_elapse , struct RemoteFileSession *p_session )
{
	return 0;
}

