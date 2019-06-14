#include "rfs_util.h"

char *RFSDupFileContent( int *p_fd , char *format , ... )
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
		return NULL;
	
	fd = open( pathfilename , O_RDONLY ) ;
	if( fd == -1 )
		return NULL;
	
	file_len = (int)lseek( fd , 0 , SEEK_END );
	lseek( fd , 0 , SEEK_SET );
	file_content = (char*)malloc( file_len+1 ) ;
	if( file_content == NULL )
	{
		close( fd );
		return NULL;
	}
	memset( file_content , 0x00 , file_len+1 );
	read_len = read( fd , file_content , file_len ) ;
	if( read_len != 1 )
	{
		close( fd );
		return NULL;
	}
	
	if( p_fd )
		(*p_fd) = fd ;
	else
		close( fd );
	return file_content;
}

