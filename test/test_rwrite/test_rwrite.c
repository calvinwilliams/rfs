#include "rfs_api.h"

#define NODE_ID		"UFT"

#define TEST_RWRITE_TXT	"test_rwrite.txt"

/*
/home/calvin/rfs_root/ $ rmlog ; find . -name "*.txt" -exec rm -f {} \; -print
*/

int main()
{
	int		fd ;
	time_t		now ;
	struct tm	now_time ;
	char		now_time_str[ 19 + 1 ] ;
	ssize_t		wrote_len ;
	
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
	
	fd = ropen3( TEST_RWRITE_TXT , O_CREAT|O_APPEND|O_WRONLY , 0666 ) ;
	if( fd == -1 )
	{
		printf( "*** ERROR : ropen3[%s] failed[%d]\n" , TEST_RWRITE_TXT , fd );
		return 1;
	}
	else
	{
		printf( "ropen3[%s] ok , fd[%d]\n" , TEST_RWRITE_TXT , fd );
	}
	
	now = time(NULL) ;
	localtime_r( & now , & now_time );
	strftime( now_time_str , sizeof(now_time_str) , "%Y-%m-%d %H:%M:%S" , & now_time );
	wrote_len = rwrite( fd , now_time_str , strlen(now_time_str) ) ;
	if( wrote_len == -1 )
	{
		printf( "*** ERROR : rwrite[%s][%s] failed[%d]\n" , TEST_RWRITE_TXT , now_time_str , nret );
		return 1;
	}
	else
	{
		printf( "rwrite[%s][%s] ok\n" , NODE_ID , now_time_str );
	}
	
	wrote_len = rwrite( fd , "\n" , 1 ) ;
	if( wrote_len == -1 )
	{
		printf( "*** ERROR : rwrite[%s][\\n] failed[%d]\n" , TEST_RWRITE_TXT , nret );
		return 1;
	}
	else
	{
		printf( "rwrite[%s][\\n] ok\n" , NODE_ID );
	}
	
	rclose( fd );
	printf( "rclose fd[%d]\n" , fd );
	
	return 0;
}

