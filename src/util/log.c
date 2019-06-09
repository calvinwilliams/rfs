#include "rfs_util.h"

int RFSConvertLogLevelString( char *log_level_str )
{
	if( STRICMP( log_level_str , == , "NOLOG" ) )
		return LOGCLEVEL_NOLOG;
	else if( STRICMP( log_level_str , == , "FATAL" ) )
		return LOGCLEVEL_FATAL;
	else if( STRICMP( log_level_str , == , "ERROR" ) )
		return LOGCLEVEL_ERROR;
	else if( STRICMP( log_level_str , == , "WARN" ) )
		return LOGCLEVEL_WARN;
	else if( STRICMP( log_level_str , == , "NOTICE" ) )
		return LOGCLEVEL_NOTICE;
	else if( STRICMP( log_level_str , == , "INFO" ) )
		return LOGCLEVEL_INFO;
	else if( STRICMP( log_level_str , == , "DEBUG" ) )
		return LOGCLEVEL_DEBUG;
	else
		return LOGCLEVEL_NOLOG;
}
