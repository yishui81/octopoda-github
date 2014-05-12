/*
 * HttpConnector.h
 *
 *  Created on: 2014年5月8日
 *      Author: jacky
 */

#ifndef HTTPCONNECTOR_H_
#define HTTPCONNECTOR_H_
#include <BaseARE/UTaskObj.h>
#include <BaseARE/TimeValue.h>
#include <BaseARE/SockStream.h>
/*
 *
 */
enum{
	UNKNOWN = -1,
	CONNECT_ACCEPTED,
	CONNECT_READING,
	CONNECT_READED,
	CONNECT_WRITING,
	CONNECT_WROTE,
	CONNECT_FINISH
};

class HttpConnector : public Connector {
public:
	HttpConnector();
	virtual ~HttpConnector();

public:
	virtual int32_t read();
	virtual int32_t write();

    virtual int handle_open  (const URE_Msg& msg);
    virtual int handle_close (UWorkEnv * orign_uwe, long retcode);

   //net event
    virtual int handle_input (URE_Handle h);
    virtual int handle_output (URE_Handle h);

    //timer
    virtual	int handle_timeout( const TimeValue & origts, long time_id, const void * act );

   //message
    virtual int handle_message( const URE_Msg & msg ) { return -1; }
    virtual int handle_failed_message( const URE_Msg & msg ) ;

   //aio
    virtual int handle_read( URE_AioBlock * aib ) { return 0; }
    virtual int handle_write( URE_AioBlock * aib ) { return 0; }


public:
    inline void increase_requests_num(){
    		m_requests++;
    }

private:
    uint64_t   		m_read_time_t;
    bool                  m_status;
    bool 			m_keepalive;
    std::string 		m_sessionid;
    uint64_t			m_requests;
    SockStream* 	m_stream;
    HttpParser* 		m_paser;
    HttpChunkContext*  m_chunk;
    HttpMethod* 	m_method;
    u_char 			recv_buf[1024];
    Request*    		m_request;
};

#endif /* HTTPCONNECTOR_H_ */
