/** @file

  A brief file description

  @section license License

  Licensed to the Apache Software Foundation (ASF) under one
  or more contributor license agreements.  See the NOTICE file
  distributed with this work for additional information
  regarding copyright ownership.  The ASF licenses this file
  to you under the Apache License, Version 2.0 (the
  "License"); you may not use this file except in compliance
  with the License.  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 */

/****************************************************************************

   HttpClientSession.h

   Description:

 ****************************************************************************/

#ifndef _HTTP_CLIENT_SESSION_H_
#define _HTTP_CLIENT_SESSION_H_

#include "libts.h"
#include "HTTP.h"
#include "HttpConfig.h"
#include "BaseARE/UTaskObj.h"
#include "IOStream.h"

extern ink_mutex debug_cs_list_mutex;

class HttpStateMachine;
class HttpServerSession;

class SecurityContext;

class HttpClientSession: public UTaskObj, IOStream
{

public :

	HttpClientSession();
	virtual ~HttpClientSession(){}
	void	 cleanup();
	virtual void destroy();

	static HttpClientSession *allocate();

	virtual int handle_open  (const URE_Msg& msg);
	virtual int handle_close (UWorkEnv * orign_uwe, long retcode);
	virtual int handle_shutdown(int32_t howto);
	virtual int handle_wait_for_close( const TimeValue & origts, long time_id, const void * act);

	//net io
	virtual int handle_input (URE_Handle h);
	virtual int handle_output(URE_Handle h);

	//timer
	virtual int handle_timeout( const TimeValue & origts, long time_id, const void * act );

	//message
	virtual int handle_message( const URE_Msg & msg ) ;
	virtual int handle_failed_message( const URE_Msg & msg ) ;

	//disk io read
	virtual int32_t  do_io_read(UTaskObj* obj, int64_t nbytes = INT64_MAX, MIOBuffer * buf = 0);
      virtual int32_t  do_io_write(UTaskObj* obj, int64_t nbytes = INT64_MAX, IOBufferReader * buf = 0, bool owner = false);

	virtual void	new_connection(Connector * new_vc, bool backdoor = false);
	virtual void  reenable();
	virtual void  release(IOBufferReader * r);
	virtual void  attach_server_session(HttpServerSession * ssession, bool transaction_done = true);

public :
	void 	 set_half_close_flag()
	{
		half_close = true;
	}

	Connector  *get_connector() const
	{
		return client_vc;
	}

	HttpServerSession *get_server_session() const
	{
		return bound_ss;
	}

	// Used for the cache authenticated HTTP content feature
	HttpServerSession *get_bound_ss();

	int32_t	get_transact_count() const
	{
		return  transact_count;
	}

	void* get_user_arg(int ix) const
	{
		return user_args[ix];
	}

	void set_user_arg(int ix, void* arg)
	{
		user_args[ix] = arg;
	}

private:

	HttpClientSession(HttpClientSession &);

	int32_t  state_keep_alive(int event, void *data);
	int32_t  state_slave_keep_alive(int event, void *data);
	int32_t  state_wait_for_close(int event, void *data);
	int32_t  state_api_callout(int event, void *data);

	void 	set_tcp_init_cwnd();
	void 	handle_api_return(int event);


	void do_api_callout(const URE_Msg& msg);

	virtual void new_transaction();

	enum C_Read_State
	{
		HCS_INIT,
		HCS_ACTIVE_READER,
		HCS_KEEP_ALIVE,
		HCS_HALF_CLOSED,
		HCS_CLOSED
	};

private:

	int32_t 	magic;
	bool 		tcp_init_cwnd_set;
	int64_t 	transact_count;
	bool		half_close;
	bool 		conn_decrease;
	void *		user_args[HTTP_SSN_TXN_MAX_USER_ARG];
	int32_t 	cur_msgs;
	bool		proxy_allocated;

private:

	Connector *				client_vc;
	HttpServerSession *		bound_ss;

	MIOBuffer *				read_buffer;
	IOBufferReader *			sm_reader;
	HttpStateMachine *		current_reader;
	C_Read_State 				read_state;

//	VIO *						ka_vio;
//	VIO *						slave_ka_vio;

	OC_HTTP_MSG_ID 		cur_msg_id;
	URE_Msg* 				cur_msg;

//	Link<HttpClientSession> debug_link;

public:

	bool		backdoor_connect;
//	int32_t		hooks_set;

	/// Local address for outbound connection.
	IpAddr		outbound_ip4;

	/// Local address for outbound connection.
	IpAddr		outbound_ip6;

	/// Local port for outbound connection.
	uint16_t	outbound_port;

	/// Set outbound connection to transparent.
	bool		 f_outbound_transparent;

	/// Transparently pass-through non-HTTP traffic.
	bool 		_transparent_passthrough;

	// for DI. An active connection is one that a request has
	// been successfully parsed (PARSE_DONE) and it remains to
	// be active until the transaction goes through or the client
	// aborts.
	bool		m_active;

	/// acl method mask - cache IpAllow::match() call
	uint32_t	acl_method_mask;

	// Session specific debug flag
	bool		debug_on;

	/// DNS resolution preferences.
	//HostResStyle host_res_style;

	std::string  session_id;

};

extern ClassAllocator<HttpClientSession> httpClientSessionAllocator;

#endif
