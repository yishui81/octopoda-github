/*
 * HttpRequest.cpp
 *
 *  Created on: 2014年5月5日
 *      Author: jacky
 */

#include "HttpRequest.h"
#include "HttpConnector.h"



HttpRequest::HttpRequest(Connector* connector) {
	m_connector = connector;
}

HttpRequest::~HttpRequest() {
	// TODO Auto-generated destructor stub
}

int32_t HttpRequest::Read(){
	m_connector->read();
	return 0;
}

int32_t HttpRequest::Write(){

	return 0;
}


int32_t HttpRequest::handle_request(){
	switch(m_phase){
		case OC_HTTP_UNKOWN_PHASE:
		case OC_HTTP_POST_READ_PHASE:

		case OC_HTTP_SERVER_REWRITE_PHASE:
		case OC_HTTP_FIND_CONFIG_PHASE:
		case OC_HTTP_REWRITE_PHASE:
		case OC_HTTP_POST_REWRITE_PHASE:
		case OC_HTTP_PREACCESS_PHASE:
		case OC_HTTP_ACCESS_PHASE:
		case OC_HTTP_POST_ACCESS_PHASE:
		case OC_HTTP_TRY_FILES_PHASE:
		case OC_HTTP_CONTENT_PHASE:
		case OC_HTTP_LOG_PHASE:
			break;
		default:
	}
	return 0;
}

int
HttpRequest::handle_open  (const URE_Msg& msg){
	return 0;
}

int
HttpRequest::handle_close (UWorkEnv * orign_uwe, long retcode){
	return 0;
}

//timer
int
HttpRequest::handle_timeout( const TimeValue & origts, long time_id, const void * act ){
	return 0;
}

//message
int
HttpRequest::handle_message( const URE_Msg & msg ) {
	uint64_t phase = msg.GetType();
	int64_t   sub_phase = msg.GetInt64();
	int ret = 0;
	if(phase > OC_HTTP_UNKOWN_PHASE ){
		m_phase = phase;
		//m_sub_phase = sub_phase;
	}else{
		if(m_phase == OC_HTTP_UNKOWN_PHASE){
			m_phase = OC_HTTP_POST_READ_PHASE;
		}
	}
	return handle_request();
}


int
handle_failed_message( const URE_Msg & msg ) {
	return 0;
}

//aio
int
handle_read( URE_AioBlock * aib ) {

	return 0;
}

int
handle_write( URE_AioBlock * aib ) {

	return 0;
}



