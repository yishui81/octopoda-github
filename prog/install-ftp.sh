#install the pure-ftp rpm
#create local user and passwd
#start the pure-ftp server

PUREDIR=/usr/local/bin
PURERDIR=/usr/local/sbin
PURECDIR=/etc/
PURECONF=pure-ftpd.conf

function usage()
{
	echo "$0 start"
        echo "$0 install src" 
	return 0
}

function install_rpm()
{
	if [[ -f /etc/pure-ftpd.conf ]]; then
		rm -f /etc/pure-ftpd.conf
		rpm -e pure-ftpd 2>&1 >/dev/null
	fi
	if [[ -f /etc/pureftpd.pdb ]]; then
		echo "========clear the /etc/pureftpd.pdb========"
		rm -f /etc/pureftpd.pdb
	fi
	if [[ -f /etc/pureftpd.passwd ]]; then
		echo "========clear the /etc/pureftpd.passwd========"
		rm -f /etc/pureftpd.passwd
	fi
	
	rpm -ivh $1 --force 2>&1 >/dev/null
	if [ $? -eq 0 ]; then
		echo "========intall rpm $1 succ========"
	else
		echo "========intall rmp $1 failed========"
		exit $?
	fi

	return 0
}

function create_conf()
{
	if [ -f $PURECDIR$PURECONF ]; then
		cat > $PURECDIR$PURECONF <<-END
		############################################################
		#                                                          #
		#         Configuration file for pure-ftpd wrappers        #
		#                                                          #
		############################################################

		# If you want to run Pure-FTPd with this configuration   
		# instead of command-line options, please run the
		# following command :
		#
		# /usr/local/sbin/pure-config.pl /etc/pure-ftpd.conf
		#
		# Please don't forget to have a look at documentation at
		# http://www.pureftpd.org/documentation.shtml for a complete list of
		# options.
		ChrootEveryone              yes
		BrokenClientsCompatibility  no
		# to set this
		#MaxClientsNumber            50
		Daemonize                   yes
		# to set this
		#MaxClientsPerIP             1000 
		VerboseLog                  yes 
		DisplayDotFiles             no 
		AnonymousOnly               no
		NoAnonymous                 yes 
		SyslogFacility              ftp
		DontResolve                 yes
		MaxIdleTime                 15
		PureDB                        /etc/pureftpd.pdb 
		LimitRecursion              200000 18
		AnonymousCanCreateDirs      no
		MaxLoad                     4
		AntiWarez                   yes
		# to set this
		#Bind                      192.168.100.123,2121
		# the file:dir umask
		Umask                       133:022
		MinUID                      100
		AllowUserFXP                no
		AllowAnonymousFXP           no
		ProhibitDotFilesWrite       no
		ProhibitDotFilesRead        no
		AutoRename                  no
		AnonymousCantUpload         yes 
		CreateHomeDir               yes
		MaxDiskUsage               99
		#NoRename                  yes
		CustomerProof              yes
		END
	else
		cat > $PURECDIR$PURECONF <<-END
		############################################################
		#                                                          #
		#         Configuration file for pure-ftpd wrappers        #
		#                                                          #
		############################################################

		# If you want to run Pure-FTPd with this configuration   
		# instead of command-line options, please run the
		# following command :
		#
		# /usr/local/sbin/pure-config.pl /etc/pure-ftpd.conf
		#
		# Please don't forget to have a look at documentation at
		# http://www.pureftpd.org/documentation.shtml for a complete list of
		# options.
		ChrootEveryone              yes
		BrokenClientsCompatibility  no
		# to set this
		#MaxClientsNumber            50
		Daemonize                   yes
		# to set this
		#MaxClientsPerIP             1000 
		VerboseLog                  yes 
		DisplayDotFiles             no 
		AnonymousOnly               no
		NoAnonymous                 yes 
		SyslogFacility              ftp
		DontResolve                 yes
		MaxIdleTime                 15
		#PureDB                        /usr/local/pureftpd/etc/pureftpd.pdb 
		LimitRecursion              200000 18
		AnonymousCanCreateDirs      no
		MaxLoad                     4
		AntiWarez                   yes
		# to set this
		#Bind                      192.168.100.123,2121
		# the file:dir umask
		Umask                       133:022
		MinUID                      100
		AllowUserFXP                no
		AllowAnonymousFXP           no
		ProhibitDotFilesWrite       no
		ProhibitDotFilesRead        no
		AutoRename                  no
		AnonymousCantUpload         yes 
		CreateHomeDir               yes
		MaxDiskUsage               99
		#NoRename                  yes
		CustomerProof              yes
		END
	fi
	# read the listen ip and port 
	local ip port
	read -ep "Please input the listen port [21]:" port || return
	read -ep "Please input the listen ip [*.*.*.*]:" ip || return
	echo "Bind					$ip,$port" >> $PURECDIR$PURECONF 
	
	# read the maxclient num 
	local num
	read -ep "Please input max ftpclient perip [3000]:" num || return
	echo "MaxClientsPerIP			$num" >> $PURECDIR$PURECONF 
	
	read -ep "Please input max ftpclient number [5000]:" num || return
	echo "MaxClientsNumber           $num" >> $PURECDIR$PURECONF 
}

function create_user()
{
	local tmp=cat /etc/group 2>&1|grep ftpgroup
	[ -n $tmp ]
	if [ $? -ne 0 ]; then
		/usr/sbin/groupadd ftpgroup
	else
		echo "========there have ftpgroup========"
	fi

	if [ $? -ne 0 ]; then
		echo "========add ftpgroup failed========"
	fi
	
	tmp=cat /etc/passwd 2>&1|grep ftpuser
	[ -n $tmp ]
	if [ $? -ne 0 ]; then
		/usr/sbin/useradd -g ftpgroup -d /dev/null -s /etc ftpuser
	else
		echo "========there have ftpuser========"
	fi

	if [ $? -ne 0 ]; then
		echo "========add ftpuser failed========"
	fi
}

function init_pure_ftp()
{
	$PUREDIR/pure-pw useradd owtest -u ftpuser -d /home/owtest/ -m 2>&1 >/dev/null <<-END
	owtest
	owtest
	END
	if [ $? -ne 0 ]; then
		echo "========add user "owtest" failed========"
		return 1;
	fi

	$PUREDIR/pure-pw mkdb
	if [ $? -ne 0 ]; then
		echo "========make puredb failed========"
		return 1;
	fi
	echo "========install $1 succ========"
}

function start_pure_ftp()
{
	local PID=`ps -eo "pid,comm"|awk '"'"pure-ftpd"'"==$2 && "'"$$"'"!=$1 {print $1}'`
	[ -n $PID ]
	if [ $? -eq 0 ]; then
		killall -9 pure-ftpd
	fi
	$PURERDIR/pure-config.pl $PURECDIR$PURECONF 2>&1 >./install.log
	echo "========start pure ftp succ========"
}

function do_install()
{
	install_rpm $@
	if [ $? -ne 0 ]; then
		return 1;
	fi
	create_conf
	if [ $? -ne 0 ]; then
		return 1;
	fi
	create_user
	if [ $? -ne 0 ]; then
		return 1;
	fi
	init_pure_ftp $@
	if [ $? -ne 0 ]; then
		return 1;
	fi
	start_pure_ftp
	if [ $? -ne 0 ]; then
		return 1;
	fi
}

function do_start()
{
	start_pure_ftp
	if [ $? -ne 0 ]; then
		return 1;
	fi
}

## MAIN ##

echo $@
if [ $# -gt 0 ]; then
	CMD=$1
	shift
	case "$CMD" in
	install)
		if [ $# -ne 1 ]; then
			usage&&exit $?
		fi
		do_install $@
		exit $?;;
	start)
		do_start $@
		exit $?;;
	*)
		usage;;
	esac
else
	usage
fi
exit 255
