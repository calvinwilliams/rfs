#include "rfs_api.h"

#define TEST_RFWRITE_TXT	"test_rfwrite.txt"

/*
/home/calvin/rfs_root/ $ rmlog ; find . -name "test_r*.txt" -exec rm -f {} \; -print
*/

int main()
{
	FILE		*rfp = NULL ;
	time_t		now ;
	struct tm	now_time ;
	char		now_time_str[ 20 + 1 ] ;
	ssize_t		wrote_len ;
	
	rfp = rfopen( TEST_RFWRITE_TXT , "a+" ) ;
	if( rfp == NULL )
	{
		printf( "*** ERROR : rfopen[%s] failed , errno[%d]\n" , TEST_RFWRITE_TXT , errno );
		return 1;
	}
	else
	{
		printf( "rfopen[%s] ok , rfp[%p]\n" , TEST_RFWRITE_TXT , rfp );
	}
	
	now = time(NULL) ;
	localtime_r( & now , & now_time );
	strftime( now_time_str , sizeof(now_time_str) , "%Y-%m-%d %H:%M:%S" , & now_time );
	strcat( now_time_str , "\n" );
	wrote_len = rfwrite( now_time_str , 1 , strlen(now_time_str) , rfp ) ;
	if( wrote_len == -1 )
	{
		printf( "*** ERROR : rfwrite[%s][%s] failed[%d]\n" , TEST_RFWRITE_TXT , now_time_str , (int)wrote_len );
		return 1;
	}
	else
	{
		printf( "rfwrite[%s][%s] ok\n" , TEST_RFWRITE_TXT , now_time_str );
	}
	
	rfclose( rfp );
	printf( "rfclose fd[%p]\n" , rfp );
	
	return 0;
}

