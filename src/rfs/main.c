#include "in.h"

char	g_rfs_conf_filename[ PATH_MAX ] = "" ;
char	g_rfs_conf_main_filename[ PATH_MAX ] = "" ;

int main( int argc , char *argv[] )
{
	int		c ;
	char		*file_content = NULL ;
	rfs_conf	rfs_conf ;
	
	int		nret = 0 ;
	
	umask(0);
	
	for( c = 1 ; c < argc ; c++ )
	{
		if( STRCMP( argv[c] , == , "-v" ) )
		{
			printf( "rfs v%s\n" , __RFS_VERSION );
			exit(0);
		}
		else if( STRCMP( argv[c] , == , "-f" ) && c + 1 < argc )
		{
			c++;
			strncpy( g_rfs_conf_filename , argv[c] , sizeof(g_rfs_conf_filename)-1 );
		}
		else
		{
			printf( "*** ERROR : command parameter[%s] invalid\n" , argv[c] );
			exit(7);
		}
	}
	
	if( g_rfs_conf_filename[0] == '\0' )
	{
		printf( "*** ERROR : use '-f' set rfs.conf\n" );
		exit(7);
	}
	else
	{
		char	*p = NULL ;
		
		p = strrchr( g_rfs_conf_filename , '/' ) ;
		if( p )
		{
			strcpy( g_rfs_conf_main_filename , p );
		}
		else
		{
			strcpy( g_rfs_conf_main_filename , g_rfs_conf_filename );
		}
		
		p = strchr( g_rfs_conf_main_filename , '.' ) ;
		if( p )
			(*p) = '\0' ;
	}
	
	nret = RFSDupFileContent( NULL , & file_content , g_rfs_conf_filename ) ;
	if( nret )
	{
		printf( "*** ERROR : can't open config file[%s] [%d] , errno[%d]\n" , g_rfs_conf_filename , nret , errno );
		exit(1);
	}
	
	memset( & rfs_conf , 0x00 , sizeof(rfs_conf) );
	nret = DSCDESERIALIZE_JSON_rfs_conf( NULL , file_content , NULL , & rfs_conf ) ;
	if( nret )
	{
		printf( "*** ERROR : parse config[%s] failed[%d] , errno[%d]\n" , file_content , nret , errno );
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
	
	if( rfs_conf.listen.ip[0] == '\0' )
	{
		printf( "*** ERROR : rfs_conf.listen.ip[%s] invalid\n" , rfs_conf.listen.ip );
		return 5;
	}
	else
	{
		printf( "rfs_conf.listen.ip[%s]\n" , rfs_conf.listen.ip );
	}
	
	if( rfs_conf.listen.port <= 1 )
	{
		printf( "*** ERROR : rfs_conf.listen.port[%d] invalid\n" , rfs_conf.listen.port );
		return 5;
	}
	else
	{
		printf( "rfs_conf.listen.port[%d]\n" , rfs_conf.listen.port );
	}
	
	if( rfs_conf.fs.root[0] == '\0' )
	{
		printf( "*** ERROR : rfs_conf.fs.root[%s] invalid\n" , rfs_conf.fs.root );
		return 5;
	}
	else
	{
		printf( "rfs_conf.fs.root[%s]\n" , rfs_conf.fs.root );
	}
	
	if( RFSConvertLogLevelString(rfs_conf.log.log_level) == LOGCLEVEL_INVALID )
	{
		printf( "*** ERROR : rfs_conf.log.log_level[%s] invalid\n" , rfs_conf.log.log_level );
		return 5;
	}
	else
	{
		printf( "rfs_conf.log.log_level[%s]\n" , rfs_conf.log.log_level );
	}
	
	return monitor( & rfs_conf );
}

