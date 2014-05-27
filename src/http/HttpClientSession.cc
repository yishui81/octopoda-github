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


#include "ink_config.h"
#include "Allocator.h"
#include "HttpClientSession.h"
#include "HttpStateMachine.h"
#include "HttpServerSession.h"
#include "BaseARE/SockStream.h"
#include "ts/I_IOBuffer.h"
#include "Connector.h"

enum
{
  HTTP_CS_MAGIC_ALIVE = 0x0123F00D,
  HTTP_CS_MAGIC_DEAD = 0xDEADF00D
};

ClassAllocator<HttpClientSession> httpClientSessionAllocator("httpClientSessionAllocator");

HttpClientSession::HttpClientSession()
			:  magic(0),
			tcp_init_cwnd_set(false),
			transact_count(0),
			half_close(false),
			conn_decrease(false),
			proxy_allocated(false),
			cur_msgs(0),
			session_id(""),
			client_vc(NULL),
			magic(HTTP_CS_MAGIC_DEAD),
			tcp_init_cwnd_set(false),
			transact_count(0),
			half_close(false),
			conn_decrease(false),
			bound_ss(NULL),
			read_buffer(NULL),
			current_reader(NULL),
			read_state(HCS_INIT),
			proxy_allocated(false),
			backdoor_connect(false),
			outbound_port(0),
			outbound_transparent(false),
			transparent_passthrough(false),
			sm_reader(NULL),
			acl_method_mask(0),
			m_active(false),
			debug_on(false),
			current_hook(0),
			current_step(0)
			//cur_msg(NULL)
{

	memset(user_args, 0, sizeof(user_args));
}

void
HttpClientSession::cleanup()
{
	magic = HTTP_CS_MAGIC_DEAD;
	if (read_buffer) {
		free_MIOBuffer(read_buffer);
		read_buffer = NULL;
	}

	debug_on = false;

	if (conn_decrease) {
		conn_decrease = false;
	}
}

HttpClientSession *
HttpClientSession::allocate()
{
	return  httpClientSessionAllocator.alloc();
}

int
HttpClientSession::handle_open  (const URE_Msg& msg)
{
	session_msg = msg;
	session_msg.SetType(OC_HTTP_SSN_START_HOOK); //set hook
	session_msg.SetInt64(0);  							//set step;
	session_msg.SetInt(0);								//event
	magic = HTTP_CS_MAGIC_ALIVE;

	conn_decrease = true;

	read_buffer = new_MIOBuffer(BUFFER_SIZE_INDEX_4K);
	sm_reader = read_buffer->alloc_reader();

	do_api_callout(OC_HTTP_SSN_START_HOOK);

	return 0;
}

int
HttpClientSession::handle_close (UWorkEnv * orign_uwe, long retcode)
{
	cleanup();
	client_vc->handle_close(client_vc->GetWorkEnv(), 0);
	httpClientSessionAllocator.free(this);
	return 0;
}

int
HttpClientSession::handle_input (URE_Handle h)
{
	return 0;
}

int
HttpClientSession::handle_output(URE_Handle h)
{
	return 0;
}

//timer
int
HttpClientSession::handle_timeout( const TimeValue & origts, long time_id, const void * act )
{
	return 0;
}

//message
int
HttpClientSession::handle_message( const URE_Msg & msg )
{

	int64_t hook_id = msg.GetType();
	int64_t event = 	msg.GetInt64();

	if(hook_id == current_hook && event != HTTP_ERROR)
	{
		state_api_callout(0, NULL);
	}

	if (backdoor_connect == 0)
	{
		state_api_callout(0, NULL);
		set_handler(&HttpClientSession::state_api_callout);
	}
	else
	{
		handle_api_return(HTTP_CONTINUE);
	}

	return 0;
}

int
HttpClientSession::handle_failed_message( const URE_Msg & msg )
{
	return 0;
}

void
HttpClientSession::new_transaction()
{
	ink_assert(current_reader == NULL);

	read_state = HCS_ACTIVE_READER;

	current_reader = HttpStateMachine::allocate();
	current_reader->init();
	transact_count++;
	current_reader->attach_client_session(this, sm_reader);
	sm_msg = URE_Msg(current_reader->GetUTOID(), GetUTOID());
	sm_msg.SetType(OC_HTTP_TXN_START_HOOK);
	sm_msg.SetInt64(0);
	current_reader->EnterWorkEnv(GetWorkEnv(), sm_msg);
}

inline void
HttpClientSession::do_api_callout(const URE_Msg& msg)
{
	uint64_t type = msg.GetType();
	int64_t  event = msg.GetInt64();

	switch(type){

	case OC_HTTP_SSN_START_HOOK:

		if (event != HTTP_ERROR) {
			new_transaction();
		} else {
			handle_close(GetWorkEnv(), 0);
			//do_io_close(0);
		}
		break;

	case OC_HTTP_SSN_CLOSE_HOOK:

		handle_close(GetWorkEnv(), 0);
		break;

	default:

		ink_release_assert(0);
		break;
	}

}

void
HttpClientSession::attach_connector(Connector * new_vc, bool backdoor)
{
	client_vc = new_vc;
	backdoor_connect = backdoor;
}


int32_t
HttpClientSession::do_io_read(UTaskObj * c, int64_t nbytes, MIOBuffer * buf)
{
	  return client_vc->do_io_read( c, nbytes, buf);
}

int32_t
HttpClientSession::do_io_write(UTaskObj * c, int64_t nbytes, IOBufferReader * buf, bool owner)
{
	  /* conditionally set the tcp initial congestion window before our first write. */
	  if(!tcp_init_cwnd_set) {
			tcp_init_cwnd_set = true;
			set_tcp_init_cwnd();
	  }
	  return client_vc->do_io_write(c, nbytes, buf, owner);
}

int32_t
HttpClientSession::do_io_close(int32_t errr_no){

	if (read_state == HCS_ACTIVE_READER) {

		if (m_active) {
			m_active = false;
		}

	}

	// Prevent double closing
	ink_release_assert(read_state != HCS_CLOSED);

	if (half_close) {

		read_state = HCS_HALF_CLOSED;

		set_handler(&HttpClientSession::state_wait_for_close);

		client_vc->do_io_shutdown(IO_SHUTDOWN_WRITE);
		client_vc->do_io_read(this, INT64_MAX, read_buffer);
		sm_reader->consume(sm_reader->read_avail());

//		client_vc->set_active_timeout(HRTIME_SECONDS(current_reader->t_state.txn_conf->keep_alive_no_activity_timeout_out));

	} else {

		read_state = HCS_CLOSED;
		client_vc->do_io_close(errr_no);

		client_vc = NULL;
		conn_decrease = false;
		do_api_callout(OC_HTTP_SSN_CLOSE_HOOK);

	}

	return 0;
}

int32_t
HttpClientSession::do_io_shutdown(int32_t howto)
{
	return client_vc->do_io_shutdown(howto);
}

void
HttpClientSession::set_tcp_init_cwnd()
{
//	int desired_tcp_init_cwnd = current_reader->t_state.txn_conf->server_tcp_init_cwnd;
	int desired_tcp_init_cwnd = 0;

	if(desired_tcp_init_cwnd == 0) {
		return;
	}

	if(client_vc->set_tcp_init_cwnd(desired_tcp_init_cwnd) != 0){
		//TODO
	}
}


int32_t
HttpClientSession::state_wait_for_close(int event, void *data)
{
	switch (event) {

	case CON_EVENT_EOS:
	case CON_EVENT_ERROR:

		half_close = false;
		handle_close(GetWorkEnv(), 0);
		break;

	case CON_EVENT_READ_READY:
		// Drain any data read
		sm_reader->consume(sm_reader->read_avail());
		break;

	default:
		ink_release_assert(0);
		break;

	}
	return 0;
}


int
HttpClientSession::state_api_callout(int event, void * /* data ATS_UNUSED */)
{
	switch (event) {
	case EVENT_NONE:
	case HTTP_CONTINUE:

		if(session_msg.GetType() <= OC_HTTP_SSN_START_HOOK)
		{
			session_msg.SetType(OC_HTTP_SSN_START_HOOK);
			session_msg.SetInt64(0);
		}

		handle_api_return(event);
		break;

	default:
		ink_assert(false);

	case HTTP_ERROR:
		handle_api_return(event);
	break;
	  }

	  return 0;
}

void
HttpClientSession::handle_api_return(int event)
{
	//ink_assert(cur_hook_id == TS_HTTP_SSN_START_HOOK || cur_hook_id == TS_HTTP_SSN_CLOSE_HOOK);

	switch (session_msg.GetType()) {

	case OC_HTTP_SSN_START_HOOK:

		if (event != HTTP_ERROR) {
			new_transaction();
		} else {
			handle_close(GetWorkEnv(), 0);
			return;
		}
		break;

	case OC_HTTP_SSN_CLOSE_HOOK:

		handle_close(GetWorkEnv(), 0);
		break;

	default:

		ink_release_assert(0);
		break;
	}
}

void
HttpClientSession::reenable()
{
	//client_vc->reenable(vio);
}

void
HttpClientSession::attach_server_session(HttpServerSession * ssession, bool transaction_done)
{
	//TODO
}

void
HttpClientSession::release(IOBufferReader * r)
{
	ink_assert(read_state == HCS_ACTIVE_READER);
	ink_assert(current_reader != NULL);
	//MgmtInt ka_in = current_reader->t_state.txn_conf->keep_alive_no_activity_timeout_in;

	current_reader = NULL;

	// handling potential keep-alive here
	if (m_active) {
		m_active = false;
	}

	// Make sure that the state machine is returning
	//  correct buffer reader
	ink_assert(r == sm_reader);
	if (r != sm_reader) {
		this->do_io_close(0);
		return;
	}

	if (sm_reader->read_avail() > 0) {

		new_transaction();

	} else {

		read_state = HCS_KEEP_ALIVE;
		set_handler(&HttpClientSession::state_keep_alive);

		do_io_read(this, INT64_MAX, read_buffer);
//		ink_assert(slave_ka_vio != ka_vio);
//		client_vc->set_inactivity_timeout(HRTIME_SECONDS(ka_in));
//		client_vc->cancel_active_timeout();
	}
}

