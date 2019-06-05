#include "in.h"

int main()
{
	char		rfs_conf_pathfilename[ PATH_MAX ] ;
	FILE		*fp = NULL ;
	long		file_len ;
	char		*file_content = NULL ;
	rfs_conf	rfs_conf ;
	
	int		nret = 0 ;
	
	memset( rfs_conf_pathfilename , 0x00 , sizeof(rfs_conf_pathfilename) );
	snprintf( rfs_conf_pathfilename , sizeof(rfs_conf_pathfilename)-1 , "%s/etc/rfs.conf" , getenv("HOME") );
	fp = fopen( rfs_conf_pathfilename , "r" ) ;
	if( fp == NULL )
	{
		printf( "*** ERROR : Can't open config file[%s] , errno[%d]\n" , rfs_conf_pathfilename , errno );
		return 1;
	}
	
	fseek( fp , 0 , SEEK_END );
	file_len = ftell( fp ) ;
	fseek( fp , 0 , SEEK_SET );
	file_content = (char*)malloc( file_len+1 ) ;
	if( file_content == NULL )
	{
		printf( "*** ERROR : Alloc failed , errno[%d]\n" , errno );
		return 2;
	}
	memset( file_content , 0x00 , file_len+1 );
	nret = fread( file_content , file_len , 1 , fp ) ;
	if( nret != 1 )
	{
		printf( "*** ERROR : Read config file[%s] failed , errno[%d]\n" , rfs_conf_pathfilename , errno );
		free( file_content );
		return 3;
	}
	
	memset( & rfs_conf , 0x00 , sizeof(rfs_conf) );
	nret = DSCDESERIALIZE_JSON_rfs_conf( NULL , file_content , NULL , & rfs_conf ) ;
	if( nret )
	{
		printf( "*** ERROR : Parse config[%s] failed , errno[%d]\n" , file_content , errno );
		free( file_content );
		return 4;
	}
	
	free( file_content );
	
	printf( "node.id[%s]\n" , rfs_conf.node.id );
	printf( "node.user[%s]\n" , rfs_conf.node.user );
	printf( "node.pass[%s]\n" , rfs_conf.node.pass );
	printf( "node.server.ip[%s]\n" , rfs_conf.node.server.ip );
	printf( "node.server.port[%d]\n" , rfs_conf.node.server.port );
	
	return monitor( & rfs_conf );
}

