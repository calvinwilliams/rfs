# 此文件由makeobj.sh自动生成
############################################################
# 项目名 : 
# 模块名 : 
# 备  注 : 
############################################################

###### 源文件配置区
#@ c_FILE
c_FILE		=	\
			rbtree.c \
			LOGC.c \
			fasterjson.c \
			comm.c \
			log.c \
			file.c \
			ver.c \
			tcpopt.c \

###### 目标文件、安装目录配置区
include makeinstall
LIB		=	librfs_util.so
LIBINST		=	$(_LIBINST)
HDER		= \
			rbtree.h \
			rbtree_tpl.h \
			LOGC.h \
			fasterjson.h \
			rfs_util.h \

HDERINST	=	$(_HDERINST)

###### 编译选项
CFLAGS		=	$(_CFLAGS) \
			-std=gnu99 \

###### 链接选项
LFLAGS		=	$(_LFLAGS) \

###### 额外宏定义区
CLEAN_ADDITION	=

PREMAKE		=

###### 加载mktpl模板库
#@ make_all
#@ make_clean_test
#@ make_clean
#@ make_install
#@ make_install_LIBINST
#@ make_install_HDERINST
#@ make_install_DFTHDERINST
#@ make_uninstall
#@ make_uninstall_LIBINST
#@ make_uninstall_HDERINST
#@ make_uninstall_DFTHDERINST
include $(MKTPL2_HOME)/makeobj_$(MKTPL2_OS).inc

###### 目标文件依赖关系
librfs_util.so		:	$(c_FILE_o)
	$(CC) -o $@ $(c_FILE_o) $(SOFLAGS) $(LFLAGS)

