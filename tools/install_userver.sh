#! /bin/bash

COMPATIBILITY_DEGRADE=3.0.15.b11
COMPONENT="userver"

BASEDIR=/opt/contex
LOGDIR=/var/log
DBDIR=/data/db
ULOGDIR=/data/log

function usage()
{
	cat <<-USAGE
	usage:
	     $0 install --src=... --dst=... --component=...
	     $0 upgrade --src=... --dst=... --component=...
	     $0 degrade --src=... --dst=... --component=...
	     $0 backup  --src=... --dst=...
	     $0 restore --src=... --dst=...
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

function safe_cp()
{
	cp -rf "$1" "$2" || return 1
	cmp -s "$1" "$2" || return 1
	[ -x "$1" ] && {
		chmod +x $2 || return 1
	}
	return 0
}

function do_install()
{
	parseArgs $@
	[ -z "${ARG_src:-}" ] && usage && return 1;
	[ -z "${ARG_dst:-}" ] && usage && return 1;
	[ -z "${ARG_component:-}" ] && usage && return 1;

	echo "Installing $ARG_component ..."
	mkdir -p $ARG_dst/etc
	mkdir -p $ARG_dst/lib
	mkdir -p $ARG_dst/log	
	safe_cp $ARG_src/UServerHawk $ARG_dst/UServerHawk || return 1
	safe_cp $ARG_src/owuserver.sh $ARG_dst/owuserver.sh || return 1
	safe_cp $ARG_src/etc/UServerHawk.conf $ARG_dst/etc/UServerHawk.conf || return 1
	safe_cp $ARG_src/etc/Test.conf $ARG_dst/etc/Test.conf || return 1
	safe_cp $ARG_src/etc/userverhawk.logrt /etc/logrotate.d/userverhawk.logrt || return 1
	echo "alias owconfig='killall -10 UServerHawk'" >> /etc/bashrc
	echo "alias owconfig='killall -10 UServerHawk'" >> /etc/profile
	source /etc/profile
	service crond restart

	# @sheng 2009-04-09 compatibility x86-32 ppc64
	#safe_cp $ARG_src/lib/libmp4av.so $ARG_dst/lib/libmp4av.so || return 1
	#safe_cp $ARG_src/lib/libmp4util.so $ARG_dst/lib/libmp4util.so || return 1
	#safe_cp $ARG_src/lib/libmp4v2.so $ARG_dst/lib/libmp4v2.so || return 1
	#safe_cp $ARG_src/lib/libsdp.so $ARG_dst/lib/libsdp.so || return 1
	rm -f $ARG_dst/lib/*
	cp $ARG_src/lib*/* $ARG_dst/lib/ || return 1

	ln -s $ARG_dst/lib/libmp4av.so $ARG_dst/lib/libmp4av.so.0 || return 1
	ln -s $ARG_dst/lib/libmp4util.so $ARG_dst/lib/libmp4util.so.0 || return 1
	ln -s $ARG_dst/lib/libmp4v2.so $ARG_dst/lib/libmp4v2.so.0 || return 1
	ln -s $ARG_dst/lib/libsdp.so $ARG_dst/lib/libsdp.so.0 || return 1
	#safe_cp $ARG_src/install.sh $ARG_dst/install.sh || return 1
	return 0
}

function do_backup()
{
	parseArgs $@
	[ -z "${ARG_src:-}" ] && usage && return 1;
	[ -z "${ARG_dst:-}" ] && usage && return 1;
	echo "Backuping from $ARG_src to $ARG_dst ..."
	mkdir -p $ARG_dst/etc
	mkdir -p $ARG_dst/lib
	safe_cp $ARG_src/UServerHawk $ARG_dst/UServerHawk || return 1
	safe_cp $ARG_src/etc/UServerHawk.conf $ARG_dst/etc/UServerHawk.conf || return 1
	safe_cp $ARG_src/etc/userverhawk.logrt $ARG_dst/etc/userverhawk.logrt || return 1
	service crond restart
	safe_cp $ARG_src/lib/libmp4av.so $ARG_dst/lib/libmp4av.so || return 1
	safe_cp $ARG_src/lib/libmp4util.so $ARG_dst/lib/libmp4util.so || return 1
	safe_cp $ARG_src/lib/libmp4v2.so $ARG_dst/lib/libmp4v2.so || return 1
	safe_cp $ARG_src/lib/libsdp.so $ARG_dst/lib/libsdp.so || return 1
	#safe_cp $ARG_src/install.sh $ARG_dst/install.sh || return 1
	return 0
}

function do_restore()
{
	parseArgs $@
	[ -z "${ARG_src:-}" ] && usage && return 1;
	[ -z "${ARG_dst:-}" ] && usage && return 1;
	echo "Restoreing from $ARG_src to $ARG_dst ..."
	mkdir -p $ARG_dst/etc
	mkdir -p $ARG_dst/lib
	safe_cp $ARG_src/UServerHawk $ARG_dst/UServerHawk || return 1
	safe_cp $ARG_src/etc/UServerHawk.conf $ARG_dst/etc/UServerHawk.conf || return 1
	safe_cp $ARG_src/etc/userverhawk.logrt $ARG_dst/userverhawk.logrt || return 1
	safe_cp $ARG_src/lib/libmp4av.so $ARG_dst/lib/libmp4av.so || return 1
	safe_cp $ARG_src/lib/libmp4util.so $ARG_dst/lib/libmp4util.so || return 1
	safe_cp $ARG_src/lib/libmp4v2.so $ARG_dst/lib/libmp4v2.so || return 1
	safe_cp $ARG_src/lib/libsdp.so $ARG_dst/lib/libsdp.so || return 1
	safe_cp $ARG_src/install.sh $ARG_dst/install.sh || return 1
	return 0
}

### @sheng 2009-01-04 part upgrade
function do_upgrade()
{
	parseArgs $@
	[ -z "${ARG_src:-}" ] && usage && return 1;
	[ -z "${ARG_dst:-}" ] && usage && return 1;
	[ -z "${ARG_component:-}" ] && usage && return 1;

	echo "upgrade userver ..."
	mkdir -p $ARG_dst/etc
	mkdir -p $ARG_dst/lib
	[ ! -f $ARG_src/UServerHawk ] || safe_cp $ARG_src/UServerHawk $ARG_dst/UServerHawk || return 1
	[ ! -f $ARG_src/etc/UServerHawk.conf ] || safe_cp $ARG_src/etc/UServerHawk.conf $ARG_dst/etc/UServerHawk.conf || return 1
	[ ! -f $ARG_src/etc/userverhawk.logrt ] || safe_cp $ARG_src/etc/userverhawk.logrt /etc/logrotate.d/userverhawk.logrt || return 1
	service crond restart
	[ ! -f $ARG_src/lib/libmp4av.so ] || safe_cp $ARG_src/lib/libmp4av.so $ARG_dst/lib/libmp4av.so || return 1
	[ ! -f $ARG_src/lib/libmp4util.so ] || safe_cp $ARG_src/lib/libmp4util.so $ARG_dst/lib/libmp4util.so || return 1
	[ ! -f $ARG_src/lib/libmp4v2.so ] || safe_cp $ARG_src/lib/libmp4v2.so $ARG_dst/lib/libmp4v2.so || return 1
	[ ! -f $ARG_src/lib/libsdp.so ] || safe_cp $ARG_src/lib/libsdp.so $ARG_dst/lib/libsdp.so || return 1
	#safe_cp $ARG_src/install.sh $ARG_dst/install.sh || return 1
	return 0
}

### @sheng 2008-09-26 do degrade
function do_degrade()
{
        parseArgs $@
        [ -z "${ARG_src:-}" ] && usage && return 1;
        [ -z "${ARG_dst:-}" ] && usage && return 1;
        [ -z "${ARG_component:-}" ] && usage && return 1;

        echo "degrade userver ..."
        mkdir -p $ARG_dst/etc
        mkdir -p $ARG_dst/lib
        safe_cp $ARG_src/UServerHawk $ARG_dst/UServerHawk || return 1
        safe_cp $ARG_src/etc/UServerHawk.conf $ARG_dst/etc/UServerHawk.conf || return 1
		safe_cp $ARG_src/etc/userverhawk.logrt /etc/logrotate.d/userverhawk.logrt || return 1
		service crond restart
        safe_cp $ARG_src/lib/libmp4av.so $ARG_dst/lib/libmp4av.so || return 1
        safe_cp $ARG_src/lib/libmp4util.so $ARG_dst/lib/libmp4util.so || return 1
        safe_cp $ARG_src/lib/libmp4v2.so $ARG_dst/lib/libmp4v2.so || return 1
        safe_cp $ARG_src/lib/libsdp.so $ARG_dst/lib/libsdp.so || return 1
        #safe_cp $ARG_src/install.sh $ARG_dst/install.sh || return 1
        return 0
}

## MAIN ##

echo $@
if [ $# -gt 0 ]; then
	CMD=$1
	shift
	case "$CMD" in
	install)
		do_install $@
		exit $?;;
	backup)
		do_backup $@
		exit $?;;
	restore)
		do_restore $@
		exit $?;;
	upgrade)
		do_upgrade $@
		exit $?;;
	degrade)
		do_degrade $@
		exit $?;;
	*)
		usage;;
	esac
else
	usage
fi
exit 255

