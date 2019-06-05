#include "rfs_api.h"

#define NODE_ID		"UFT"

int main()
{
	int		nret = 0 ;
	
	nret = rset( NODE_ID ) ;
	if( nret )
	{
		printf( "*** ERROR : rset[%s] failed[%d]\n" , NODE_ID , nret );
		return 1;
	}
	else
	{
		printf( "rset[%s] ok\n" , NODE_ID );
	}
	
	
	
	
	
	return 0;
}

