#########################################
# Makefile for UServerEagle
# 2006-01
# zhoujg@onewaveinc.com

# NOTE : be care of dependance

#MY_LIBS = \
	../Proto_Mpeg2Ts/libProto_Mpeg2Ts.a \
	../Proto_HFC/libProto_HFC.a \
	../Proto_ISMA/libProto_ISMA.a \
	../Proto_PTP/libProto_PTP.a \
	../Proto_AMS/libProto_AMS.a \
	../libTimeShift/libTimeShift.a \
	../Proto_WMV_CSM/libProto_WMV_CSM.a \
	../Proto_HTTP/libProto_HTTP.a \
	../App_MSCtrl/libApp_MSCtrl.a \
	../UCoreSvr/libUCoreSvr.a \
	../UMK/libUMK.a \
	../USvrCommon/libUSvrCommon.a \
	../Proto_Common/libProto_Common.a \
	../Proto_FTP/libProto_FTP.a \

#../libUSMI/libUSMI.a

CFLAGS += $(foreach mlib, $(MY_LIBS), -I $(dir $(mlib)) )
LDFLAGS += $(foreach mlib, $(MY_LIBS), -L $(dir $(mlib)) -l$(patsubst lib%.a,%, $(notdir $(mlib))) )



# TODO : new license setting
# ifdef USE_LICENSE
# USER_LIBS += ../license/liblicense.a
# endif
# ifdef USE_LICENSE
# USER_CFLAGS += -DUSE_LICENSE
# endif

LDFLAGS += -Wl,-rpath=./lib

LDFLAGS += -L$(MPEG4IP_DIR)/$(LIB_STR)/$(LIB_ARCH) -lxml2
LDFLAGS += -L$(SEALSARE_DIR)/UtilARE -lUtilARE -L$(SEALSARE_DIR)/BaseARE -lBaseARE
LDFLAGS += -lcrypt -laio -lrt -lpthread 
#LDFLAGS += -L../Proto_Rtmp/rtmp-include/ -lrtmp -lutils -lssl -llua 

# CXX_PCH = ../UCoreSvr/UCoreSvr_all.h

SRCS = main.cpp

# NOTE : only an flag to tell MK.template, we set real value below
APP_TARGET = UServerHawk

TOP_DIR:=..
include $(TOP_DIR)/MK.config

ifeq ($(strip $(SCONF_PLAT_BUILD_TYPE)),IPTV)
CFLAGS += -DSCONF_PLAT_BUILD_TYPE_IPTV
endif
ifeq ($(strip $(SCONF_PLAT_BUILD_TYPE)),PC)
CFLAGS += -DSCONF_PLAT_BUILD_TYPE_PC
endif
ifeq ($(strip $(SCONF_PLAT_BUILD_TYPE)),ATCA)
CFLAGS += -DSCONF_PLAT_BUILD_TYPE_ATCA
endif

ifneq ($(findstring ppc,$(PLATFORM)),)
LIB_ARCH = ppc
else
LIB_ARCH = x86
endif

all : post_target

.PHONY: post_target

#post_target : $(APP_TARGET)
#	mv $(APP_TARGET) $(APP_TARGET)_$(SCONF_PLAT_BUILD_TYPE)

#####################################################

$(APP_TARGET) : $(MY_LIBS)

.PHONY: $(MY_LIBS)

$(MY_LIBS) :
	$(MAKE) -C $(dir $@)

install: $(APP_TARGET)
	@echo "begin to install..."

package: install
	@ln -s $(MPEG4IP_DIR)/$(LIB_STR)/$(LIB_ARCH) $(LIB_STR)
	@cp -b install_userver.sh install.sh
	@echo "begin to tar package..."
	-@tar czf $(APP_TARGET).tar.gz $(APP_TARGET) \
		install.sh \
		etc/$(APP_TARGET).conf \
		etc/userverhawk.logrt \
		$(LIB_STR)/lib*
	@rm -f $(LIB_STR) install.sh
