################################################
# Makefile for UServerEagle
# zhoujg@onewaveinc.com
# 2005-05

PROG_LIB_DIRS :=  
PROG_DIR = prog


SUB_DIRS_TARGET = $(PROG_DIR) $(PROG_LIB_DIRS)

TOP_DIR ?= .
include $(TOP_DIR)/MK.config

$(PROG_DIR) : $(PROG_LIB_DIRS)

# TODO : new license setting
# ifdef LICENSE_FILE
# LICENSE_FILE_REALPATH = \
# 	$(shell cd `dirname $(LICENSE_FILE)`; \
# 			echo `pwd`/`basename $(LICENSE_FILE)`)
# export LICENSE_FILE_REALPATH
# export USE_LICENSE = 1
# SUB_DIRS += license
# endif

# TODO : new dist setting
# Make dist. There must be a "../BUILD_NUMBER" directory.
# See "../BUILD_NUMBER/README.txt" for details.
# dist :
# 	( \
# 	oldpwd=`pwd`; \
# 	cd ../BUILD_NUMBER; \
# 	export BUILD_NUMBER_DIR=`pwd`; \
# 	cd $$oldpwd; \
# 	\
# 	make release BUILD_NUMBER_DIR=$$BUILD_NUMBER_DIR; \
# 	[ $$? -eq 0 ] || exit 1; \
# 	\
# 	PROG_VERSION=`awk -F= '{print $$2}' PROG_VERSION.def`; \
# 	bash scripts/makedist-usrv.sh $$PROG_VERSION; \
# 	[ $$? -eq 0 ] || exit 1; \
# 	\
# 	bash $$BUILD_NUMBER_DIR/inc-bldnum.sh; \
# 	)

