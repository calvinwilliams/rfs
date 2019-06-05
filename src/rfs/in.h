#ifndef _H_IN_
#define _H_IN_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include "LOGC.h"

#include "IDL_rfs_conf.dsc.h"

#ifdef __cplusplus
extern "C" {
#endif

struct RemoteFileServiceEnv
{
} ;

int main();
int monitor( rfs_conf *p_rfs_conf );
int worker();

#ifdef __cplusplus
}
#endif

#endif

