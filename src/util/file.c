#include "rfs_util.h"

int RFSDupFileContent( int *p_fd , char **pp_file_content , char *format , ... )
{
	char		pathfilename[ PATH_MAX ] ;
	int		pathfilename_len ;
	va_list		valist ;
	int		fd ;
	int		file_len ;
	char		*file_content = NULL ;
	size_t		read_len ;
	
	memset( pathfilename , 0x00 , sizeof(pathfilename) );
	va_start( valist , format );
	pathfilename_len = vsnprintf( pathfilename , sizeof(pathfilename)-1 , format , valist ) ;
	va_end( valist );
	if( pathfilename_len == -1 || pathfilename_len >= sizeof(pathfilename)-1 )
		return -1;
	
	fd = open( pathfilename , O_RDONLY ) ;
	if( fd == -1 )
		return -2;
	
	file_len = (int)lseek( fd , 0 , SEEK_END );
	lseek( fd , 0 , SEEK_SET );
	file_content = (char*)malloc( file_len+1 ) ;
	if( file_content == NULL )
	{
		close( fd );
		return -3;
	}
	memset( file_content , 0x00 , file_len+1 );
	read_len = read( fd , file_content , file_len ) ;
	if( read_len == -1 )
	{
		close( fd );
		return -4;
	}
	
	if( p_fd )
		(*p_fd) = fd ;
	else
		close( fd );
	if( pp_file_content )
		(*pp_file_content) = file_content ;
	return 0;
}

