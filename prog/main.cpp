/* HttpContentTaskLet.cpp : The HttpContentTaskLet implementation.
 * caojt@onewaveinc.com
 * 2007-06
 */


#include "ApplicationBase.h"
#include "UWEManager.h"
#include "ConfigSet.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <getopt.h>


int    g_encode_type;
////////////////////////////////////////////////////////
////	CCM & CRM Ctrl App

class HawkApplication: public ApplicationBase
{
public:
	HawkApplication() {}
	virtual ~HawkApplication() {}

public:
	virtual int Start();
	virtual int Stop();

public:
	int DoDaemon( int argc, char * argv[] );

public:
	virtual const char * get_cfgsection() { return "global"; }
	virtual const char * get_cfgfile() { return "etc/UServerHawk.conf"; }
	virtual const char * get_progname() { return "UServerHawk"; }
	virtual const char * get_progdesc() { return "Advaneced Powerful Universal Streaming Server."; }
	virtual const char * get_progversion() { return PROG_VERSION; }
	virtual const char * get_buildtime() { return __DATE__; }

protected:
	virtual int do_signal( int signum, int code, void * sigvalue );
};

int HawkApplication::do_signal( int signum, int code, void * sigvalue )
{
	if( signum == SIGUSR1 ) {
		PR_CRIT("cfg file reload!");
		if( m_test_mode )
		//g_app_ctrl->ReadFromDisk();
		return do_parse_cfg();
	}
	return 0;
}

int HawkApplication::Start()
{
	PR_DEBUG( "HawkApplication Start." );

	// <0> Init App Logger
//	const char * cfval = NULL;
//	cfval = m_cfg->GetValue( "global", "log_appfile" );
//	if( cfval != NULL ) {
//		if( USLogger::Init( cfval ) < 0 )
//			PR_WARN( "init app log file failed. errno %d.", errno );
//	} else {
//		PR_DEBUG( "can't get appfile" );
//	}
//	// <1> Start UCoreSvr
//	g_ucoresvr = new UCoreSvrImp;
//
//	// <2> Start ContentMngr
//	g_content_mngr = new ContentMngr;
//
//	// <3> Start AppCtrl
//	g_app_ctrl = new AppCtrlUSvr;
//	UTOID cb = g_app_ctrl->Start( g_ucoresvr, g_content_mngr, m_cfg, get_progversion(), m_test_mode );
//	if( !cb.IsValid() ) {
//		PR_ERR( "AppCtrl Start failed." );
//		return -1;
//	}

//	g_ucoresvr->SetCallback( cb );
//	g_content_mngr->Init( cb, g_ucoresvr );
	
	sleep(2);

//	// <4> Add Protocols
//	// <4.1> http protocol
//	g_ucoresvr->AddProto( &g_proto_http );
//
//	g_ucoresvr->AddProto( &g_proto_iphone );
//
//	g_ucoresvr->AddProto( &g_proto_ngod_http );
//
//	g_ucoresvr->AddProto( &g_proto_ngod_ts );
//
//	// <4.2> flvr6http protocol
//	g_ucoresvr->AddProto( &g_proto_r6 );
//
//	g_ucoresvr->AddProto( &g_proto_rtmp );
//
//	// <4.3> wmv rtsp protocol
//	g_ucoresvr->AddProto( &g_proto_wmv_rtsp );
//
//	// <4.4> wmv http protocol
//	g_ucoresvr->AddProto( &g_proto_wmv_http );
//
//	// <4.5> real rtsp protocol
//	g_ucoresvr->AddProto( &g_proto_real_rtsp );
//
//	// <4.6> ftp protocol
//	g_ucoresvr->AddProto( &g_proto_ftp );
//
//	// <4.7> mpeg2 ts over ip protocol
//	g_ucoresvr->AddProto( &g_proto_mpeg2_tsip );
//
//	// <4.8> mpeg2 ts over ip qam protocol
//	g_ucoresvr->AddProto( &g_proto_mpeg2_tsqam );
//
//	// <4.9> isma over ip protocol
//	g_ucoresvr->AddProto( &g_proto_isma_ip );
//
//	// <4.10> isma over ip qam protocol
//	g_ucoresvr->AddProto( &g_proto_isma_qam );
//
//	// <4.11> h264 over isma ip protocol
//	g_ucoresvr->AddProto( &g_proto_h264_ip );
//
//	// <4.12> h264 over isma ipqam protocol
//	g_ucoresvr->AddProto( &g_proto_h264_qam );
//
//	// <4.13> h264 over ts ip protocol
//	g_ucoresvr->AddProto( &g_proto_h264_tsip );
//
//	// <4.14> h264 over ts ipqam protcol
//	g_ucoresvr->AddProto( &g_proto_h264_tsqam );
	

//	// <4.15> Ku6 protocol over http
//	g_ucoresvr->AddProto( &g_proto_ku6 );
//	g_ucoresvr->AddProto( &g_proto_at );
//
//	g_ucoresvr->AddProto( &g_proto_new_isma );
//	g_ucoresvr->AddProto( &g_proto_iptv2 );
//	g_ucoresvr->AddProto( &g_proto_unicom_iptv);
//
//	g_ucoresvr->AddProto( &g_proto_3gp_rtsp );
//	g_ucoresvr->AddProto( &g_proto_webm );
//	g_ucoresvr->AddProto( &g_proto_ogg );
//
//	g_ucoresvr->AddProto( &g_proto_joy );
//	// <4.16> ngod output protocol
//	g_ucoresvr->AddProto( &g_proto_ngod_output );

	// <5> Start read test config
//	if( m_test_mode ) {
//		PR_DEBUG( "Read from disk." );
//		g_app_ctrl->ReadFromDisk();
//	}

//	int keep = m_cfg->GetIntVal( "session_common", "session_keepalive_timeout" );
//	int exit = m_cfg->GetIntVal( "session_common", "session_exit_timeout", 3 );
//	int pause = m_cfg->GetIntVal( "session_common", "session_pause_timeout", 30*60 );
//	int start = m_cfg->GetIntVal( "session_common", "session_start_timeout", 30 );
//	g_encode_type = m_cfg->GetIntVal("source_common","filename_encode",0);

//	int32_t proto[32];
	
//	int32_t proto_num = g_ucoresvr->GetAllProtoType( proto, 32 );
//	if( proto_num < 0 ) {
//		PR_ERR( "Get Protocol Type failed." );
//	}
//
//	for( int32_t i = 0; i < proto_num; i++ ) {
//		PR_DEBUG( "keep %d, pause %d, exit %d.", keep, pause, exit );
//		g_ucoresvr->SetProtocolTimeout( proto[i], &keep, &pause, &start, &exit );
//	}
	return 0;
}

int HawkApplication::Stop()
{
//	PR_DEBUG( "HawkApplication Stop." );
//
//	for(int i = 0;i< MAX_PROTO_NUM;i++){
//		g_ucoresvr->RemoveProto(i);
//	}
//
//	if(g_ucoresvr != NULL)
//		delete g_ucoresvr,g_ucoresvr = NULL;
//
//	if(g_content_mngr != NULL){
//		g_content_mngr->Stop();
//		delete g_content_mngr,g_content_mngr = NULL;
//	}
//
//	if(g_app_ctrl != NULL){
//		g_app_ctrl->Stop();
//		g_app_ctrl = NULL;
//	}

#if 0
	// <1> Stop UCoreSvr
	if( m_ucoresvr != NULL )
		delete m_ucoresvr, m_ucoresvr = NULL;

	// <2> Stop ContentMngr
	if( m_content_mngr != NULL ) {
		m_content_mngr->Stop();
		delete m_content_mngr, m_content_mngr = NULL;
	}

	// <3> Stop AppCtrl
	if( m_app_ctrl != NULL ) {
		m_app_ctrl->Stop();
		m_app_ctrl  = NULL;
	}

	// <4> Stop Protocols
	if( m_proto_http != NULL )
		delete m_proto_http, m_proto_http = NULL;

	if( m_proto_wmv_rtsp != NULL )
		delete m_proto_wmv_rtsp, m_proto_wmv_rtsp = NULL;

	if( m_proto_wmv_http != NULL )
		delete m_proto_wmv_http, m_proto_wmv_http = NULL;
	
	if( m_proto_real_rtsp!= NULL )
		delete m_proto_real_rtsp, m_proto_real_rtsp = NULL;
#endif
	return 0;
}

//////////////////////////////////////////////////////////
////	main program

static HawkApplication * g_app;

int main( int argc, char * argv[] )
{
	SFC_LOG_INIT( LOG_LEVEL_DEBUG, LOG_OUTPUT_STDERR );
	g_app = new HawkApplication();

	if( g_app->Init( argc, argv ) < 0 ) {
		PR_ERR( "init failed!" );
		goto end_init;
	}

	if( g_app->Start() < 0 ) {
		PR_ERR( "start failed!" );
		goto end_start;
	}

	if( g_app->Run() < 0 ) {
		PR_ERR( "run failed!" );
	}

	if( g_app->Stop() < 0 ) {
		PR_ERR( "stop failed!" );
	}

end_start:
	if( g_app->Fini() < 0 ) {
		PR_ERR( "fini failed!" );
	}

end_init:
	delete g_app, g_app = NULL;

	return 0;
}

