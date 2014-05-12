/*
 * octopoda.cpp
 *
 *  Created on: 2014年5月1日
 *      Author: jacky
 */

#include "Octopoda.h"

#include <libio.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>

//#include <BaseARE/UWorkEnv.h>
#include <oc_auto_config.h>
#include <oc_auto_headers.h>

#include "../../../SealsARE/BaseARE/SfcLog.h"
#include "../../../SealsARE/BaseARE/UREData.h"
#include "../../../SealsARE/BaseARE/UTaskObj.h"
#include "../../../SealsARE/BaseARE/UWEManager.h"
#include "http/HttpAcceptor.h"
#include "oc_cpu.h"
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <getopt.h>




class UWorkEnv;


static int g_exit_flag = 0;
static int g_signal_num = 0;
#define APPBASE_OUTPUT( fmt, args... ) fprintf( stdout, fmt"\n", ##args )


static void g_do_signal( int signum, siginfo_t * o, void * p )
{
	switch( signum )
	{
	case SIGPIPE :
	case SIGQUIT :
		// just ignore
		break;

	case SIGHUP :
		// maybe for reconfig
		g_signal_num = signum;
		break;

	case SIGINT :
	case SIGTERM :
		g_signal_num = signum;
		g_exit_flag = 1;
		break;

	default :
		break;
	}

	return;
}


Octopoda::Octopoda():
	m_debug_mode(false),
	m_test_mode(false),
	m_daemon_mode(false),
	m_cfg(NULL),
	m_uwemngr(NULL),
	m_acceptor(NULL)
{

}

Octopoda::~Octopoda()
{
	delete m_cfg, m_cfg = NULL;
}


int Octopoda::do_signal( int signum, int code, void * sigvalue )
{
	if( signum == SIGUSR1 ) {
		PR_CRIT("cfg file reload!");
		if( m_test_mode ){
			//g_app_ctrl->ReadFromDisk();
		}
		return do_parse_cfg();
	}
	return 0;
}

int Octopoda::Start()
{
	PR_DEBUG( "Octopoda Start." );
	//TODO
	m_acceptor = new HttpAcceptor();
	std::string ip = "0.0.0.0";
	uint32_t    port = 8081;
	m_acceptor->SetHostName(ip);
	m_acceptor->SetPort(port);
	m_acceptor->SetBackLog(1024);
	m_acceptor->enableTcpDefferAccept(1);

	UWEManager* manager =   GetUweMngr();
	UWorkEnv* uwv = manager->GetSharedUWE();
	m_acceptor->EnterWorkEnv(uwv);


	return 0;
}

int Octopoda::Stop()
{
	PR_DEBUG( "Octopoda Stop." );
	//TODO

	return 0;
}




///////////////////////////////////////////////////
////	ApplicationBase implement

int Octopoda::SignalStop( int code )
{
	union sigval sv;
	memset( &sv, 0, sizeof(sv) );
	sv.sival_int = code;
	return sigqueue( getpid(), SIGTERM, sv );
}

int Octopoda::Init( int argc, char * argv[], bool restart )
{
	// <1> command line
	if( do_cmdline( argc, argv ) < 0 )
		return -1;

	// <2> signal
	struct sigaction sigact;
	memset( &sigact, 0, sizeof(sigact) );
	sigact.sa_flags = SA_RESTART | SA_SIGINFO; // restart system call.
	sigact.sa_sigaction = g_do_signal;
	sigemptyset( &sigact.sa_mask );
	if( sigaction( SIGPIPE, &sigact, NULL ) < 0
			|| sigaction( SIGQUIT, &sigact, NULL ) < 0
			|| sigaction( SIGHUP, &sigact, NULL ) < 0
			|| sigaction( SIGINT, &sigact, NULL ) < 0
			|| sigaction( SIGTERM, &sigact, NULL ) < 0 )
	{
		APPBASE_OUTPUT( "register sigaction failed, errno %d", errno );
		return -1;
	}

	sigset_t ss, oldss;
	sigemptyset( &ss );
	sigaddset( &ss, SIGPIPE );
	sigaddset( &ss, SIGQUIT );
	sigaddset( &ss, SIGHUP );
	sigaddset( &ss, SIGINT );
	sigaddset( &ss, SIGTERM );
	sigaddset( &ss, SIGUSR1 );
	sigprocmask( SIG_BLOCK, &ss, &oldss );

	// <3> config file
	// home_dir / log_xxxx
	if( do_parse_cfg() < 0 ) {
		return -1;
	}

	// <4> create daemon
	if( m_daemon_mode ) {
		if( do_create_daemon(restart) < 0 )
			return -1;
	}

	// <5> create uwe
	if( do_create_uwe() < 0 ) {
		return -1;
	}

	return 0;
}

int Octopoda::Run( void )
{
	// m_isrunning = true;

	sigset_t ss;
	sigemptyset( &ss );
	// sigaddset( &ss, SIGPIPE );
	// sigaddset( &ss, SIGQUIT );
	sigaddset( &ss, SIGHUP );
	sigaddset( &ss, SIGINT );
	sigaddset( &ss, SIGTERM );
	sigaddset( &ss, SIGUSR1 );

	struct timespec ts;
	ts.tv_sec = 10;
	ts.tv_nsec = 0;

	int signum;
	siginfo_t sinfo;

	while( g_exit_flag == 0 ) {
		signum = sigtimedwait( &ss, &sinfo, &ts );
		if( signum < 0 )
			continue;

		PR_TRACE( "get signal %d, info.code %d, info.value %d", signum, sinfo.si_code, sinfo.si_value.sival_int );
		if( signum == SIGTERM || signum == SIGINT ) {
			g_exit_flag = 1;
			break;
		}

		if( do_signal( signum, sinfo.si_code, sinfo.si_value.sival_ptr ) < 0 ) {
			g_exit_flag = 1;
			break;
		}
	}

	// m_isrunning = false;

	return 0;
}

int Octopoda::Fini( void )
{
	if( m_uwemngr != NULL ) {
		int i;
		for( i = 0; i < 20 && m_uwemngr->GetUtoNum() > 0; i++ ) {
			PR_TRACE( "total uwe %ld, total uto %ld", m_uwemngr->GetUweNum(), m_uwemngr->GetUtoNum() );
			usleep( 30 * 1000 );
		}
		m_uwemngr->DestroyUWE();
	}

	return 0;
}


int Octopoda::do_cmdline( int argc, char * argv[] )
{
	const char * cfgfile = get_cfgfile();

	while( 1 )
	{
		int longopt_id;
		int optchar;
		static const struct option long_opts[] = {
			{ "config", 1, 0, 'c' },
			{ "help", 0, 0, 'h' },
			{ "version", 0, 0, 'v' },
			{ "debug", 0, 0, 'd' },
			{ "depend", 0, 0, 'l' },
			{ "test", 0, 0, 't' },
			{ "deamon", 0, 0, 'm' },
			{ NULL, 0, 0, 0 }
		};

		optchar = getopt_long( argc, argv, "c:hvdtml", long_opts, &longopt_id );

		if( optchar == -1 )
			break;
		switch( optchar )
		{
			case 'c' :
				cfgfile = optarg;
				break;

			case 'v' :
				do_output_version();
				// never come here
				return -1;
				break;
			case 'h' :
				do_output_usage();
				// never come here
				return -1;
				break;
			case 'l' :
				do_output_depend();
				return -1;
			case 'd' :
				m_debug_mode = true;
				break;

			case 't':
				m_test_mode = true;
				break;

			case 'm':
				m_daemon_mode = true;
				break;

			default :
				break;
		}
	}

	// open config file
	if( cfgfile != NULL ) {
		int ret;
		m_cfg = new ConfigSet();
		m_cfgfile = cfgfile;
		if( m_cfg->LoadFromFile( m_cfgfile.c_str(), &ret ) < 0 ) {
			APPBASE_OUTPUT( "failed to open or parse file [%s], errno %d, ret %d", m_cfgfile.c_str(), errno, ret );
			delete m_cfg, m_cfg = NULL;
			return -1;
		}
	}

	return 0;
}

int Octopoda::do_output_version()
{
	APPBASE_OUTPUT( "%s : %s", get_progname(), get_progdesc() );
	APPBASE_OUTPUT( "CopyRight (c) 2005, 2006 Onewave, Inc. All Right Reserved." );
	APPBASE_OUTPUT( "Version : %s", get_progversion() );
	APPBASE_OUTPUT( "Build : %s", get_buildtime() );
	exit(0);
}

int Octopoda::do_output_depend()
{
	APPBASE_OUTPUT( "%s : %s", get_progname(), get_progdesc() );
	APPBASE_OUTPUT( "CopyRight (c) 2005, 2006 Onewave, Inc. All Right Reserved." );
	APPBASE_OUTPUT( "Version : %s", get_progversion() );
	APPBASE_OUTPUT( "Build : %s", get_buildtime() );
	exit(0);
}

int Octopoda::do_output_usage()
{
	APPBASE_OUTPUT( "%s : %s", get_progname(), get_progdesc() );
	APPBASE_OUTPUT( "CopyRight (c) 2005, 2006 Onewave, Inc. All Right Reserved." );
	APPBASE_OUTPUT( "Version : %s", get_progversion() );
	APPBASE_OUTPUT( "Build : %s", get_buildtime() );
	APPBASE_OUTPUT( "Usage : %s [ -c <config file> ] [ -v ] [ -h ]", get_progname() );
	APPBASE_OUTPUT( "    -c --config <file>    set config file name." );
	APPBASE_OUTPUT( "                          (default : %s )", get_cfgfile() );
	APPBASE_OUTPUT( "    -h --help             show this help message." );
	APPBASE_OUTPUT( "    -l --depend           show depend library version." );
	APPBASE_OUTPUT( "    -v --version          show version." );
	exit(0);
	return 0;
}

int Octopoda::do_parse_cfg()
{
	const char * sect = get_cfgsection();
	const char * cfval = NULL;

	if( m_cfg == NULL || sect == NULL )
		return 0;

	int ret = 0;
	if( m_cfg->LoadFromFile( m_cfgfile.c_str(), &ret ) < 0 ) {
		APPBASE_OUTPUT( "failed to open or parse file [%s], errno %d, ret %d", m_cfgfile.c_str(), errno, ret );
		delete m_cfg, m_cfg = NULL;
		return -1;
	}

	// <1> home dir
	cfval = m_cfg->GetValue( sect, "home_dir" );
	if( cfval != NULL ) {
		if( chdir( cfval ) < 0 ) {
			APPBASE_OUTPUT( "failed to change to dir [%s], errno %d, QUIT!", cfval, errno );
			return -1;
		}
	}

	// <2> log
	int logoutput = 0;
	int syslognum = 0;
	int loglevel = LOG_LEVEL_WARN;
	const char * logfilename = NULL;

	const char * logser_ipstr = NULL;
	int logser_port = 0;
	int logser_facility = 0;
	int logser_severity = 0;

	cfval = m_cfg->GetValue( sect, "log_stderr" );
	if( cfval != NULL && strcmp( cfval, "yes" ) == 0 ) {
		logoutput |= LOG_OUTPUT_STDERR;
	}
	cfval = m_cfg->GetValue( sect, "log_localfile" );
	if( cfval != NULL ) {
		logoutput |= LOG_OUTPUT_FILE;
		logfilename = cfval;
	}
	cfval = m_cfg->GetValue( sect, "log_level" );
	if( cfval != NULL && *cfval >='0' && *cfval <= '7' ) {
		loglevel = (*cfval - '0' );
	}

	int syslog_enable = m_cfg->GetIntVal( sect, "syslog_enable", 0 );
	if( syslog_enable != 0 ) {

		cfval = m_cfg->GetValue( sect, "log_syslog_num" );
		if( cfval != NULL && *cfval >= '0' && *cfval <= '7' ) {
			logoutput |= LOG_OUTPUT_SYSLOG;
			syslognum = (*cfval - '0');
		}
		cfval = m_cfg->GetValue( sect,"syslog_udp_ip" );
		if( cfval != NULL ) {
			logoutput |= LOG_OUTPUT_UDP;
			logser_ipstr = cfval;
			logser_port = m_cfg->GetIntVal( sect, "syslog_udp_port", 514 );
			logser_facility = m_cfg->GetIntVal( sect, "syslog_udp_facility", syslognum );
			logser_severity = m_cfg->GetIntVal( sect, "syslog_udp_severity", loglevel );
		}
	}

	if( m_debug_mode ) {
		logoutput |= LOG_OUTPUT_STDERR;
		loglevel = LOG_LEVEL_TRACE;
	}

	SFC_LOG_INIT( loglevel, logoutput );
	if( (logoutput & LOG_OUTPUT_FILE) )
		SFC_LOG_SETFILE( logfilename );
	if( (logoutput & LOG_OUTPUT_SYSLOG) )
		SFC_LOG_SETSYSLOG( syslognum );
	if( (logoutput & LOG_OUTPUT_UDP ) ) {
		SFC_LOG_SETUDP( logser_ipstr, logser_port );
		SFC_LOG_SETPROG( get_progname() );
		SFC_LOG_SETPRI( logser_facility, logser_severity );
	}

	return 0;
}

int Octopoda::do_create_uwe()
{
	const char * sect = get_cfgsection();

	int aiostyle = 1;
	int maxfd = 8192;
	int uwenum = 8;

	if( m_cfg != NULL && sect != NULL ) {
		aiostyle = m_cfg->GetIntVal( sect, "uwe_fastio_mode", 1 );
		if( aiostyle < 1 || aiostyle > 3 )
			aiostyle = 1;
		uwenum = m_cfg->GetIntVal( sect, "uwe_num", 0 );
		if( uwenum < 0 || uwenum > 8192 ){
			uwenum = get_cpu_num();
		}

		maxfd = m_cfg->GetIntVal( sect, "uwe_maxfd", 8192 );

		int eventnum = m_cfg->GetIntVal( sect, "uwe_eventnum", 0 );
		int aionum = m_cfg->GetIntVal( sect, "uwe_aionum", 0 );
		int tasknum = m_cfg->GetIntVal( sect, "uwe_tasknum", 0 );
		int aiothreads = m_cfg->GetIntVal( sect, "uwe_aio_threads", 8 );
		int taskthreads = m_cfg->GetIntVal( sect, "uwe_task_threads", 1 );


		UWEManager::SetSharedParam( eventnum, aionum, aiothreads, tasknum, taskthreads );

		eventnum = m_cfg->GetIntVal( sect, "uwe_sole_eventnum", 0 );
		aionum = m_cfg->GetIntVal( sect, "uwe_sole_aionum", 0 );
		tasknum = m_cfg->GetIntVal( sect, "uwe_sole_tasknum", 0 );
		aiothreads = m_cfg->GetIntVal( sect, "uwe_sole_aio_threads", 1 );
		taskthreads = m_cfg->GetIntVal( sect, "uwe_sole_task_threads", 2 );

		UWEManager::SetSoledParam( eventnum, aionum, aiothreads, tasknum, taskthreads );
	}

	static URE_AioImpStyle_e s_aiostyle_en[4] = { URE_AIOIMP_SIMULATE, URE_AIOIMP_SIMULATE,
											URE_AIOIMP_NATIVE, URE_AIOIMP_POSIX };

	m_uwemngr = UWEManager::Create( s_aiostyle_en[aiostyle], uwenum, maxfd );

	if( m_uwemngr == NULL ) {
		APPBASE_OUTPUT( "create UWEManager failed!" );
		return -1;
	}

	return 0;
}

int Octopoda::do_create_daemon(bool restart)
{
	//do the deamon
	if( daemon( 1, 0 ) < 0 ) {
		printf( "call daemon() function error!!\n" );
		exit( -1 );
	}
	// only in daemon mode, we make an monitor process
	int last_dead_time = time(NULL);
	while( 1 ) {
		int pid;
		if( (pid = fork()) < 0 ) {
			fprintf( stderr, "call fork() function error!!\n" );
			exit(-4);
		}
		else if( pid == 0 ) {
			break;
		}

		// parent monitor child
		int ret = -1, status = 0;
		while( 1 ) {
			ret = waitpid( pid, &status, 0 );
			if( ret >= 0 || errno != EINTR )
				break;
			if( g_exit_flag != 0 )
				exit( 0 );
		}
		// dont monitor, killall Server.exe
		PR_CRIT( "our process exit" );
		if(restart &&  !WIFEXITED(status))
			PR_ERR("maybe our process core down, restart.");
		else
			return -1;
			//exit( 0 );
		int now = time(NULL);
		if( now - last_dead_time < 10 )
			sleep( 10 );
		last_dead_time = now;
		PR_ERR( "monitor find child dead, ret = %d, status = %d", ret, status );
	} // while forever for parent to monitor

	return 0;
}




static Octopoda* octopoda;
int main(int argc, char* argv[])
{

	SFC_LOG_INIT( LOG_LEVEL_DEBUG, LOG_OUTPUT_STDERR );
	octopoda = new Octopoda();

	if( octopoda->Init( argc, argv ) < 0 ) {
		PR_ERR( "init failed!" );
		goto end_init;
	}

	if( octopoda->Start() < 0 ) {
		PR_ERR( "start failed!" );
		goto end_start;
	}

	if( octopoda->Run() < 0 ) {
		PR_ERR( "run failed!" );
	}

	if( octopoda->Stop() < 0 ) {
		PR_ERR( "stop failed!" );
	}

end_start:
	if( octopoda->Fini() < 0 ) {
		PR_ERR( "fini failed!" );
	}

end_init:
	delete octopoda, octopoda = NULL;

	return 0;
}



