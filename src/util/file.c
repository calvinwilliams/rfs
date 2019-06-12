#include "rfs_util.h"

char *RFSDupFileContent( char *format , ... )
{
	char		pathfilename[ PATH_MAX ] ;
	va_list		valist ;
	int		pathfilename_len ;
	FILE		*fp = NULL ;
	ssize_t		file_len ;
	char		*file_content = NULL ;
	size_t		read_len ;
	
	memset( pathfilename , 0x00 , sizeof(pathfilename) );
	va_start( valist , format );
	pathfilename_len = vsnprintf( pathfilename , sizeof(pathfilename)-1 , format , valist ) ;
	va_end( valist );
	if( pathfilename_len == -1 || pathfilename_len >= sizeof(pathfilename)-1 )
		return NULL;
	
	fp = fopen( pathfilename , "rb" ) ;
	if( fp == NULL )
		return NULL;
	
	fseek( fp , 0 , SEEK_END );
	file_len = ftell( fp ) ;
	fseek( fp , 0 , SEEK_SET );
	file_content = (char*)malloc( file_len+1 ) ;
	if( file_content == NULL )
		return NULL;
	memset( file_content , 0x00 , file_len+1 );
	read_len = fread( file_content , file_len , 1 , fp ) ;
	if( read_len != 1 )
		return NULL;
	
	return file_content;
}

