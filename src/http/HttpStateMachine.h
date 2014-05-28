/*
 * HttpStateMachine.h
 *
 *  Created on: 2014年5月19日
 *      Author: jacky
 */

#ifndef HTTPSTATEMACHINE_H_
#define HTTPSTATEMACHINE_H_
#include "Handle.h"
/*
 *
 */

#define HTTP_CONTINUE   6000
#define HTTP_ERROR         6001

enum StateMachineAction
{
	STATE_MACHINE_ACTION_UNDEFINED = 0,
	DNS_LOOKUP,
	REVERSE_DNS_LOOKUP,
	CACHE_LOOKUP,
	CACHE_ISSUE_WRITE,
	CACHE_ISSUE_WRITE_TRANSFORM,
	ISSUE_CACHE_DELETE,
	PREPARE_CACHE_UPDATE,
	ISSUE_CACHE_UPDATE,

	ICP_QUERY,

	ORIGIN_SERVER_OPEN,
	ORIGIN_SERVER_RAW_OPEN,
	OS_RR_MARK_DOWN,

	READ_PUSH_HDR,
	STORE_PUSH_BODY,

	PROXY_INTERNAL_CACHE_DELETE,
	PROXY_INTERNAL_CACHE_NOOP,
	PROXY_INTERNAL_CACHE_UPDATE_HEADERS,
	PROXY_INTERNAL_CACHE_WRITE,
	PROXY_INTERNAL_100_RESPONSE,
	PROXY_INTERNAL_REQUEST,
	PROXY_SEND_ERROR_CACHE_NOOP,
	#ifdef PROXY_DRAIN
	PROXY_DRAIN_REQUEST_BODY,
	#endif /* PROXY_DRAIN */

	SEND_QUERY_TO_INCOMING_ROUTER,
	SERVE_FROM_CACHE,
	SERVER_READ,
	SERVER_PARSE_NEXT_HDR,
	TRANSFORM_READ,

	SSL_TUNNEL,
	EXTENSION_METHOD_TUNNEL,

	CONTINUE,

	HTTP_SM_START,
	HTTP_READ_REQUEST_HDR,
	HTTP_PRE_REMAP,
	HTTP_REMAP_REQUEST,
	HTTP_POST_REMAP,
	HTTP_POST_REMAP_SKIP,

	HTTP_OS_DNS,
	HTTP_SEND_REQUEST_HDR,
	HTTP_READ_CACHE_HDR,
	HTTP_CACHE_LOOKUP_COMPLETE,
	HTTP_READ_RESPONSE_HDR,
	HTTP_SEND_RESPONSE_HDR,
	HTTP_REDIRECT_READ,
	HTTP_SM_SHUTDOWN
};




class HttpStateMachine : public UTaskObj, Handle{
public:
	HttpStateMachine();
	virtual ~HttpStateMachine();
	static HttpStateMachine* allocate();

	virtual int handle_open  (const URE_Msg& msg);
	virtual int handle_close (UWorkEnv * orign_uwe, long retcode);

	//net io
	virtual int handle_input (URE_Handle h);
	virtual int handle_output(URE_Handle h);

	//timer
	virtual int handle_timeout( const TimeValue & origts, long time_id, const void * act );

	//message
	virtual int handle_message( const URE_Msg & msg ) ;
	virtual int handle_failed_message( const URE_Msg & msg ) ;

	virtual int32_t attach_client_session(HttpClientSession* client_session, IOBufferReader *sm);

protected:
	virtual int32_t do_api_callout();
	virtual int32_t start_sub_state_machine();

	virtual int32_t setup_client_read_request_header();
	virtual int32_t setup_server_send_request();

	virtual int32_t call_transact_and_set_next_state();


	virtual  int32_t init();
	virtual int32_t  main_handler(int32_t event, void* data);

	HttpTunnelProducer * setup_transfer_from_transform();

	//HttpTransformInfo transform_info;
	void 	setup_blind_tunnel_port();
	void 	call_transact_and_set_next_state(TransactEntryFunc_t f);
	void 	set_next_state();
	void	perform_transform_cache_write_action();
	int32_t do_rewrite_request(bool );

protected:
	HttpClientSession*   client_session;
	HTTPParser 		    	http_parser;
	IOBufferReader * 	   ua_buffer_reader;
	IOBufferReader *     ua_raw_buffer_reader;
	HttpTunnel*				tunnel;
	HttpRequest* 			request;

	URE_Msg					sm_msg;

private:
	bool 			debug_on;
	int32_t 		magic;
	int64_t		statemachine_id;
	int64_t     current_hook_type;
	int64_t     current_hook_id;
	int64_t		current_state;
	int32_t		redirection_tries;
	bool			enable_redirection;

};

#endif /* HTTPSTATEMACHINE_H_ */
