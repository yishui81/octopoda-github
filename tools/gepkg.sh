#!/bin/bash

MPEG4IP_32ppc=libMpeg4Ip/lib/ppc/
MPEG4IP_64ppc=libMpeg4Ip/lib64/ppc/
MPEG4IP_32x86=libMpeg4Ip/lib/x86/
MPEG4IP_64x86=libMpeg4Ip/lib64/x86/

function usage()
{
	cat <<-USAGE
	usage:
		 $0 pkg --type=... [type 32ppc: ppc 32bit]
                           [type 64ppc: ppc 64bit]
                           [type 32x86: x86 32bit]
                           [type 64x86: x86 64bit]
	USAGE
	return 0
}

function parseArgs()
{
	while [ $# -gt 0 ];
	do
		optname=`expr "x$1" : 'x--\([^=]*\)=.*'`;
		optarg=`expr "x$1" : 'x--[^=]*=\(.*\)'`;
		eval ARG_$optname=$optarg
		shift
	done
}

function do_pkg()
{
	parseArgs $@
	file="./prog/etc/UServerHawk.conf \
		  ./prog/etc/Test.conf \
		  ./prog/install_userver.sh \
		  ./prog/UServerHawk"

	lib=""
	
	case "$ARG_type" in
	32ppc)
		lib=$MPEG4IP_32ppc ;;
	64ppc)
		lib=$MPEG4IP_64ppc ;;
	32x86)
		lib=$MPEG4IP_32x86 ;;
	64x86)
		lib=$MPEG4IP_64x86 ;;
	*)
		usage
		exit;;
	esac

	if [ -d ./userver ]; then
		rm -rf userver
	fi
	
	mkdir userver || return 0
	mkdir userver/lib/ || return 0
	mkdir userver/etc/ || return 0

	cp ./prog/etc/UServerHawk.conf userver/etc/
	cp ./prog/etc/userverhawk.logrt userver/etc/
	cp ./prog/etc/Test.conf userver/etc/
	cp ./tools/install_userver.sh userver/
	cp ./tools/owuserver.sh userver/
	cp ./prog/UServerHawk userver/
	cp $lib/lib* userver/lib/
	ver=`cat ./PROG_VERSION.def | sed 's/PROG_VERSION=//g' `
	ver=$ver.$ARG_type

	#tar zvcf UServerHawk-$ver.tar.gz -C ./prog/ $file
	tar zvcf $ver.tar.gz userver 

}

## MAIN ##

echo $@
if [ $# -gt 0 ]; then
	CMD=$1
	#shift
	case "$CMD" in
	pkg)
		do_pkg $@
		exit $?;;
	*)
		usage;;
	esac
else
	usage
fi
exit 255
