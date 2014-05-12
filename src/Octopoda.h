/*
 * octopoda.h
 *
 *  Created on: 2014年5月4日
 *      Author: jacky
 */

#ifndef OCTOPODA_H_
#define OCTOPODA_H_

#include <oc_core.h>
#include <stdarg.h>
#include <string>
#include <core/Acceptor.h>


#define OC_VER "Octopoda 1.0.0"

#define oc_linefeed(p)          *p++ = LF;
#define OC_LINEFEED_SIZE        1
#define OC_LINEFEED             "\x0a"

#define OC_MAX_ERROR_STR   2048

#define PROG_VERSION "Octopoda 0.0.0.1 20140304"

class Octopoda {

public:
	Octopoda();
	~Octopoda();

public:
	// Init Paramenter
	 int32_t Init( int argc, char * argv[], bool restart = false );

	// start the server, UWE has already running
	int32_t Start( void );

	// running the server, user need not do anything
	int32_t Run( void );

	// stop the server, UWE still running, user should close all work,
	int32_t Stop( void );

	// release all release, stop UWE
	int32_t  Fini( void );


public:
	const char * get_cfgsection() { return "global"; }
	const char * get_cfgfile()
	{
		if(m_cfg == NULL){
			return "etc/UServerHawk.conf";
		}else{
			return m_cfg;
		}
	};
	const char * get_progname() { return "UServerHawk"; }
	const char * get_progdesc() { return "Advaneced Powerful Universal Streaming Server."; }
	const char * get_progversion() { return PROG_VERSION; }
	const char * get_buildtime() { return __DATE__; }

private: // child class can reimplement
	UWEManager * GetUweMngr() { return m_uwemngr; }

	static int32_t SignalStop( int code );
	int32_t do_signal( int signum, int code, void * sigvalue );

	int32_t do_parse_cfg();
	int32_t do_create_uwe();

	int32_t do_cmdline( int argc, char * argv[] );
	int32_t do_output_version();
	int32_t do_output_usage();
	int32_t do_output_depend();
	int32_t do_create_daemon(bool restart);



private:
	bool m_debug_mode;
	bool m_test_mode;
	bool m_daemon_mode;
	ConfigSet * m_cfg;
	std::string	m_cfgfile;
	UWEManager * m_uwemngr;
	Acceptor* m_acceptor ;
};

#endif /* OCTOPODA_H_ */
