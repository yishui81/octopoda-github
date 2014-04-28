#! /bin/bash

path=`pwd`

function usage()
{
	cat <<-USAGE
	usage:
		 $0 -v      : 	show the version	
		 $0 start   : 	start the userver
		 $0 stop    :  	stop the userver
		 $0 restart :	start the userver
	USAGE
	return 0
}

function do_start()
{
	PID_CROND=`ps -eo "pid,comm"|awk '"'"crond"'"==$2 && "'"$$"'"!=$1 {print $1}'`
	if [ -n "$PID_CROND" ]; then
		/sbin/service crond restart
	else
		/sbin/service crond start
	fi

	ulimit -c unlimited -n 81920 -s 1024
	${path}/UServerHawk -m -t
}

function do_stop()
{
	killall UServerHawk
}

function do_version()
{

	export LD_LIBRARY_PATH=$BASEDIR/cache/lib
	"$path/UServerHawk" -v
}

function parseArgs()
{
	if [ $# == 0 ]; then
		usage
	fi
	while [ $# -gt 0 ]
	do
		case "$1" in
		-d)
			set -x;;
		-v) 
			do_version;;
		start)
			do_start;;
		stop)
			do_stop;;
		restart)
			do_stop
			do_start;;
		*)
			usage;;
		esac

		shift
	done
}

#### MAIN ####

echo `date +"[%Y-%m-%d %H:%M]"`: $0 $@

parseArgs $*
exit $ret
