/*
 * HttpConnector.cpp
 *
 *  Created on: 2014年5月8日
 *      Author: jacky
 */

#include "HttpConnector.h"
#include <errno.h>
#include "HttpRequest.h"
#include "IOStream.h"

#define RECV_BUF_LENGTH 1024
HttpConnector::HttpConnector()
{

}

HttpConnector::~HttpConnector()
{
	// TODO Auto-generated destructor stub
}

int
HttpConnector::read(){

	int32_t ret = 0;
	//FIXME
	//not constant
	char buf[4096];
	int32_t length = 0;
	while((ret = m_stream->Recv(buf, 4096) ) > 0){
		m_request->AppendUnparsedBuffer(buf, ret);
		length += ret;
	}
	return length;
}

int
HttpConnector::write()
{
	int32_t ret = 0;
	//FIXME
	//not constant
	char buf[4096];
	int32_t length = 0;
	while((ret = m_stream->Send(buf, 4096) ) > 0){
		m_request->AppendUnparsedBuffer(buf, ret);
		length += ret;
	}
	return length;
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
HttpConnector::handle_input (URE_Handle h)
{
	if(!m_request){
		m_request = new HttpRequest(this);
	}
	if(m_request == NULL){
		//TODO
	}
	//TODO
	//notify the request to continue process
	return 0;
}

int
HttpConnector::handle_output (URE_Handle h){
	if(!m_request){
		//TODO
		return -1;
	}
//	int32_t ret =  write();
	//TODO
	//notify the request to continue process
	 //notify(m_request->GetUTOID());

	 return 0;
}

int
HttpConnector::handle_timeout( const TimeValue & origts, long time_id, const void * act ){
	//TODO
	//	if(m_read_time_t == time_t ){
//
//	}
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
