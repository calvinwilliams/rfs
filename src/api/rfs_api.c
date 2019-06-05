#include "rfs_api.h"

#include "LOGC.h"
#include "fasterjson.h"

#include "IDL_rfs_api_conf.dsc.h"

__thread rfs_api_conf	*g_rfs_api_conf = NULL ;
__thread int		g_rfs_node_index = -1 ;

static int rloadconfig()
{
	char		rfs_api_conf_pathfilename[ PATH_MAX ] ;
	FILE		*fp = NULL ;
	long		file_len ;
	char		*file_content = NULL ;
	
	int		nret = 0 ;
	
	SetLogcFile( "%s/log/rfs_api.log" , getenv("HOME") );
	SetLogcLevel( LOGCLEVEL_ERROR );
	
	memset( rfs_api_conf_pathfilename , 0x00 , sizeof(rfs_api_conf_pathfilename) );
	snprintf( rfs_api_conf_pathfilename , sizeof(rfs_api_conf_pathfilename)-1 , "%s/etc/rfs_api.conf" , getenv("HOME") );
	fp = fopen( rfs_api_conf_pathfilename , "r" ) ;
	if( fp == NULL )
	{
		ERRORLOGC( "*** ERROR : Can't open config file[%s] , errno[%d]\n" , rfs_api_conf_pathfilename , errno );
		return -1;
	}
	
	fseek( fp , 0 , SEEK_END );
	file_len = ftell( fp ) ;
	fseek( fp , 0 , SEEK_SET );
	file_content = (char*)malloc( file_len+1 ) ;
	if( file_content == NULL )
	{
		ERRORLOGC( "*** ERROR : Alloc failed , errno[%d]\n" , errno );
		return -2;
	}
	memset( file_content , 0x00 , file_len+1 );
	nret = fread( file_content , file_len , 1 , fp ) ;
	if( nret != 1 )
	{
		ERRORLOGC( "*** ERROR : Read config file[%s] failed , errno[%d]\n" , rfs_api_conf_pathfilename , errno );
		free( file_content );
		return -3;
	}
	
	g_rfs_api_conf = (rfs_api_conf *)malloc( sizeof(rfs_api_conf) ) ;
	if( g_rfs_api_conf == NULL )
	{
		ERRORLOGC( "*** ERROR : Alloc failed , errno[%d]\n" , errno );
		free( file_content );
		return -5;
	}
	memset( g_rfs_api_conf , 0x00 , sizeof(rfs_api_conf) );
	nret = DSCDESERIALIZE_JSON_rfs_api_conf( NULL , file_content , NULL , g_rfs_api_conf ) ;
	if( nret )
	{
		ERRORLOGC( "*** ERROR : Parse config[%s] failed[%d]\n" , file_content , nret );
		free( file_content );
		return -5;
	}
	
	free( file_content );
	
	{
	int		i , j ;
	for( i = 0 ; i < g_rfs_api_conf->_nodes_count ; i++ )
	{
		printf( "nodes[%d].id[%s]\n" , i , g_rfs_api_conf->nodes[i].id );
		printf( "nodes[%d].user[%s]\n" , i , g_rfs_api_conf->nodes[i].user );
		printf( "nodes[%d].pass[%s]\n" , i , g_rfs_api_conf->nodes[i].pass );
		for( j = 0 ; j < g_rfs_api_conf->nodes[i]._servers_count ; j++ )
		{
			printf( "nodes[%d].servers[%d].ip[%s]\n" , i , j , g_rfs_api_conf->nodes[i].servers[j].ip );
			printf( "nodes[%d].servers[%d].port[%d]\n" , i , j , g_rfs_api_conf->nodes[i].servers[j].port );
		}
	}
	}
	
	return 0;
}

static int rselectnode( char *node_id )
{
	int		i ;
	
	for( i = 0 ; i < g_rfs_api_conf->_nodes_count ; i++ )
	{
		if( STRCMP( g_rfs_api_conf->nodes[i].id , == , node_id ) )
		{
			g_rfs_node_index = i ;
			return 0;
		}
	}
	
	return -1;
}

int rset( char *node_id )
{
	int		nret = 0 ;
	
	if( g_rfs_api_conf == NULL )
	{
		nret = rloadconfig() ;
		if( nret )
			return nret;
	}
	
	nret = rselectnode( node_id ) ;
	if( nret )
		return nret;
	
	
	
	return 0;
}

int ropen( char *pathfilename , int flags )
{
	return 0;
}

int ropen3( char *pathfilename , int flags , mode_t mode )
{
	return 0;
}

int rclose( int fd )
{
	return 0;
}

ssize_t rread( int fd , void *buf , size_t count )
{
	return 0;
}

ssize_t rwrite( int fd , char *buf , size_t count )
{
	return 0;
}

FILE *rfopen( char *pathfilename , char *mode )
{
	return NULL;
}

int rfclose( FILE *fp )
{
	return 0;
}

size_t rfread( void *ptr , size_t size , size_t nmemb , FILE *fp )
{
	return 0;
}

size_t rfwrite( void *ptr , size_t size , size_t nmemb , FILE *fp )
{
	return 0;
}

int rfeof( FILE *fp )
{
	return 0;
}

