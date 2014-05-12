/*
 * HttpConnector.cpp
 *
 *  Created on: 2014年5月8日
 *      Author: jacky
 */

#include "HttpConnector.h"
#include <errno.h>
#define RECV_BUF_LENGTH 1024
HttpConnector::HttpConnector() {

	// TODO Auto-generated constructor stub

}

HttpConnector::~HttpConnector() {
	// TODO Auto-generated destructor stub
}


int
HttpConnector::handle_open  (const URE_Msg& msg){

	if( m_stream->GetHandle() != INVALID_SOCKET_HANDLE ) {
		register_handler( m_stream->GetHandle(), READ_MASK );
	}

	return 0;
}

int
HttpConnector::handle_close (UWorkEnv * orign_uwe, long retcode){

	return 0;
}

int
HttpConnector::handle_input (URE_Handle h){


	int32_t result = 0;
	if(m_status == CONNECT_ACCEPTED || m_status >= CONNECT_WROTE){
		 m_request = new HttpRequest(this);
	}
	while((result = m_stream->Recv(this->recv_buf, RECV_BUF_LENGTH)) > 0){
		//TODO
	}
	notify(this->m_request->GetUTOID());

//	if(errno == EAGAIN ||errno ){
//		TimeValue tv;
//		const void * comment = (const void *)"read request again";
//		tv.SetTime( 0, 1000 );
//		m_read_time_t = schedule_timer( tv, comment );
//	}


	return 0;
}

int
HttpConnector::handle_output (URE_Handle h){

	return 0;
}

int
HttpConnector::handle_timeout( const TimeValue & origts, long time_id, const void * act ){
	if(m_read_time_t == time_t ){

	}
	return 0;
}

int
HttpConnector::handle_message( const URE_Msg & msg ) {

	return -1;
}

int
HttpConnector::handle_failed_message( const URE_Msg & msg ) {

	return -1;
}

int
HttpConnector::handle_read( URE_AioBlock * aib ) {

	return 0;
}

int
HttpConnector::handle_write( URE_AioBlock * aib ) {

	return 0;
}
