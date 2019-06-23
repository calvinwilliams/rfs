#include "rfs_api.h"

#define TEST_RWRITE_TXT	"test_rwrite.txt"

/*
/home/calvin/rfs_root/ $ rmlog ; find . -name "test_r*.txt" -exec rm -f {} \; -print
*/

int main()
{
	int		fd ;
	char		read_buf[ 4096 ] ;
	int		read_len ;
	
	fd = ropen3( TEST_RWRITE_TXT , O_RDONLY , 0666 ) ;
	if( fd == -1 )
	{
		printf( "*** ERROR : ropen[%s] failed[%d]\n" , TEST_RWRITE_TXT , fd );
		return 1;
	}
	else
	{
		printf( "ropen[%s] ok , fd[%d]\n" , TEST_RWRITE_TXT , fd );
	}
	
	while(1)
	{
		memset( read_buf , 0x00 , sizeof(read_buf) );
		read_len = rread( fd , read_buf , sizeof(read_buf)-1 ) ;
		if( read_len == -1 )
		{
			printf( "*** ERROR : rread[%s] failed[%d]\n" , TEST_RWRITE_TXT , fd );
			break;
		}
		else if( read_len == 0 )
		{
			printf( "rread[%s] end\n" , TEST_RWRITE_TXT );
			break;
		}
		else
		{
			printf( "rread[%s] ok , [%d]bytes\n" , TEST_RWRITE_TXT , read_len );
			printf( "[%s]\n" , read_buf );
		}
	}
	
	rclose( fd );
	printf( "rclose fd[%d]\n" , fd );
	
	return 0;
}

