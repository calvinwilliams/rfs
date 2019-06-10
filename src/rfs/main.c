#include "in.h"

int main()
{
	char		rfs_conf_pathfilename[ PATH_MAX ] ;
	FILE		*fp = NULL ;
	long		file_len ;
	char		*file_content = NULL ;
	rfs_conf	rfs_conf ;
	
	int		nret = 0 ;
	
	umask(0);
	
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
		printf( "*** ERROR : Parse config[%s] failed[%d] , errno[%d]\n" , file_content , nret , errno );
		free( file_content );
		return 4;
	}
	
	free( file_content );
	
	if( rfs_conf.process_model.process_count <= 0 )
	{
		printf( "*** ERROR : rfs_conf.process_model.process_count[%d] invalid\n" , rfs_conf.process_model.process_count );
		return 5;
	}
	else
	{
		printf( "rfs_conf.process_model.process_count[%d]\n" , rfs_conf.process_model.process_count );
	}
	
	if( rfs_conf.node.id[0] == '\0' )
	{
		printf( "*** ERROR : rfs_conf.node.id[%s] invalid\n" , rfs_conf.node.id );
		return 5;
	}
	else
	{
		printf( "node.id[%s]\n" , rfs_conf.node.id );
	}
	
	if( rfs_conf.node.user[0] == '\0' )
	{
		printf( "*** ERROR : rfs_conf.node.user[%s] invalid\n" , rfs_conf.node.user );
		return 5;
	}
	else
	{
		printf( "rfs_conf.node.user[%s]\n" , rfs_conf.node.user );
	}
	
	if( rfs_conf.node.pass[0] == '\0' )
	{
		printf( "*** ERROR : rfs_conf.node.pass[%s] invalid\n" , rfs_conf.node.pass );
		return 5;
	}
	else
	{
		printf( "rfs_conf.node.pass[%s]\n" , rfs_conf.node.pass );
	}
	
	if( rfs_conf.node.server.ip[0] == '\0' )
	{
		printf( "*** ERROR : rfs_conf.node.server.ip[%s] invalid\n" , rfs_conf.node.server.ip );
		return 5;
	}
	else
	{
		printf( "rfs_conf.node.server.ip[%s]\n" , rfs_conf.node.server.ip );
	}
	
	if( rfs_conf.node.server.port <= 1 )
	{
		printf( "*** ERROR : rfs_conf.node.server.port[%d] invalid\n" , rfs_conf.node.server.port );
		return 5;
	}
	else
	{
		printf( "rfs_conf.node.server.port[%d]\n" , rfs_conf.node.server.port );
	}
	
	if( rfs_conf.file_system.root[0] == '\0' )
	{
		printf( "*** ERROR : rfs_conf.file_system.root[%s] invalid\n" , rfs_conf.file_system.root );
		return 5;
	}
	else
	{
		printf( "rfs_conf.file_system.root[%s]\n" , rfs_conf.file_system.root );
	}
	
	if( RFSConvertLogLevelString(rfs_conf.log_level) == LOGCLEVEL_INVALID )
	{
		printf( "*** ERROR : rfs_conf.log_level[%s] invalid\n" , rfs_conf.log_level );
		return 5;
	}
	else
	{
		printf( "rfs_conf.log_level[%s]\n" , rfs_conf.log_level );
	}
	
	return monitor( & rfs_conf );
}

