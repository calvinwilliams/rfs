#include "rfs_api.h"

#define NODE_ID		"UFT"

#define TEST_ROPEN_TXT	"test_ropen.txt"
#define TEST_ROPEN3_TXT	"test_ropen3.txt"

/*
/home/calvin/rfs_root/ $ rmlog ; find . -name "test_r*.txt" -exec rm -f {} \; -print
*/

int main()
{
	int		fd ;
	
	int		nret = 0 ;
	
	nret = rset( NODE_ID ) ;
	if( nret == -1 )
	{
		printf( "*** ERROR : rset[%s] failed[%d]\n" , NODE_ID , nret );
		return 1;
	}
	else
	{
		printf( "rset[%s] ok\n" , NODE_ID );
	}
	
	fd = ropen( TEST_ROPEN_TXT , O_CREAT|O_EXCL|O_WRONLY ) ;
	if( fd == -1 )
	{
		printf( "*** ERROR : ropen[%s] failed[%d]\n" , TEST_ROPEN_TXT , fd );
		return 1;
	}
	else
	{
		printf( "ropen[%s] ok , fd[%d]\n" , TEST_ROPEN_TXT , fd );
		
		rclose( fd );
		printf( "rclose fd[%d]\n" , fd );
	}
	
	fd = ropen( TEST_ROPEN_TXT , O_CREAT|O_EXCL|O_WRONLY ) ;
	if( fd == -1 )
	{
		printf( "*** ERROR : ropen[%s] failed[%d]\n" , TEST_ROPEN_TXT , fd );
	}
	else
	{
		printf( "ropen[%s] ok , fd[%d]\n" , TEST_ROPEN_TXT , fd );
		
		rclose( fd );
		printf( "rclose fd[%d]\n" , fd );
		
		return 1;
	}
	
	fd = ropen3( TEST_ROPEN3_TXT , O_CREAT|O_EXCL|O_WRONLY , 0700 ) ;
	if( fd == -1 )
	{
		printf( "*** ERROR : ropen3[%s] failed[%d]\n" , TEST_ROPEN3_TXT , fd );
		return 1;
	}
	else
	{
		printf( "ropen3[%s] ok , fd[%d]\n" , TEST_ROPEN3_TXT , fd );
		
		rclose( fd );
		printf( "rclose fd[%d]\n" , fd );
	}
	
	fd = ropen3( TEST_ROPEN3_TXT , O_CREAT|O_EXCL|O_WRONLY , 0700 ) ;
	if( fd == -1 )
	{
		printf( "*** ERROR : ropen3[%s] failed[%d]\n" , TEST_ROPEN3_TXT , fd );
	}
	else
	{
		printf( "ropen3[%s] ok , fd[%d]\n" , TEST_ROPEN3_TXT , fd );
		
		rclose( fd );
		printf( "rclose fd[%d]\n" , fd );
		
		return 1;
	}
	
	return 0;
}

