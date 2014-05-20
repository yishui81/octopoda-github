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
#include "HttpSM.h"
#include "HttpDebugNames.h"
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
			f_outbound_transparent(false),
			_transparent_passthrough(false),
			sm_reader(NULL),
			acl_method_mask(0),
			m_active(false),
			debug_on(false),
			cur_msg(NULL),
			cur_msg_id(0)
{

	memset(user_args, 0, sizeof(user_args));
}

void
HttpClientSession::cleanup()
{

	ink_release_assert(client_vc == NULL);
	ink_release_assert(bound_ss == NULL);
	ink_assert(read_buffer);
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
	return 0;
}

int
HttpClientSession::handle_close (UWorkEnv * orign_uwe, long retcode)
{

	if (read_state == HCS_ACTIVE_READER) {

		if (m_active) {
			m_active = false;
		}
	}

	// Prevent double closing
	ink_release_assert(read_state != HCS_CLOSED);

	// If we have an attached server session, release
	//   it back to our shared pool
	if (bound_ss) {
		bound_ss->release();
		bound_ss = NULL;
	}

	if (half_close) {

		read_state = HCS_HALF_CLOSED;
//		SET_HANDLER(&HttpClientSession::state_wait_for_close);

		client_vc->shutdown(IO_SHUTDOWN_WRITE);

		client_vc->read(INT64_MAX, read_buffer);

		sm_reader->consume(sm_reader->read_avail());

//		client_vc->set_active_timeout()

//		client_vc->set_active_timeout(HRTIME_SECONDS(current_reader->t_state.txn_conf->keep_alive_no_activity_timeout_out));


	} else {

		read_state = HCS_CLOSED;
		client_vc->close(0);
		client_vc->handle_close(GetWorkEnv(),0);
		//client_vc->do_io_close(alerrno);

		client_vc = NULL;
		conn_decrease = false;

		do_api_callout(OC_HTTP_SSN_CLOSE_HOOK);
	}
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
	if (backdoor_connect == 0)
	{
		state_api_callout(0, NULL);
//		SET_HANDLER(&HttpClientSession::state_api_callout);
//		cur_hook = NULL;
//		cur_hooks = 0;
	} else {
	//	handle_api_return(HTTP_API_CONTINUE);
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
	//DebugSsn("http_cs", "[%s ] Starting transaction %d using sm [%" PRId64 "]", session_id, transact_count, current_reader->sm_id);

	current_reader->attach_client_session(this, sm_reader);
}

inline void
HttpClientSession::do_api_callout(const URE_Msg& msg)
{
	uint64_t type = msg.GetType();
	switch(type){
	case OC_HTTP_SSN_START_HOOK:
	case OC_HTTP_SSN_CLOSE_HOOK:
	default:
	}
}

void
HttpClientSession::new_connection(Connector * new_vc, bool backdoor)
{

	client_vc = new_vc;
	magic = HTTP_CS_MAGIC_ALIVE;

	this->backdoor_connect = backdoor;

	conn_decrease = true;

	read_buffer = new_MIOBuffer(HTTP_HEADER_BUFFER_SIZE_INDEX);
	sm_reader = read_buffer->alloc_reader();

	do_api_callout(OC_HTTP_SSN_START_HOOK);

}


int32_t
HttpClientSession::do_io_read(UTaskObj * c, int64_t nbytes, MIOBuffer * buf)
{
	  return client_vc->read( nbytes, buf);
}

int32_t
HttpClientSession::do_io_write(UTaskObj * c, int64_t nbytes, IOBufferReader * buf, bool owner)
{
	  /* conditionally set the tcp initial congestion window
		 before our first write. */
	  if(!tcp_init_cwnd_set) {
			tcp_init_cwnd_set = true;
			set_tcp_init_cwnd();
	  }
	  return client_vc->write(nbytes, buf, owner);
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

	}
}

int32_t
HttpClientSession::handle_shutdown(int32_t howto)
{
	return client_vc->shutdown(howto);
}


int
HttpClientSession::state_wait_for_close(int event, void *data)
{

	//STATE_ENTER(&HttpClientSession::state_wait_for_close, event, data);

//	ink_assert(data == ka_vio);
	ink_assert(read_state == HCS_HALF_CLOSED);

	switch (event) {
//	case VC_EVENT_EOS:
//	case VC_EVENT_ERROR:
//	case VC_EVENT_ACTIVE_TIMEOUT:
//	case VC_EVENT_INACTIVITY_TIMEOUT:
		half_close = false;
		this->handle_close(GetWorkEnv(), 0);
		break;
//	case VC_EVENT_READ_READY:
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
HttpClientSession::state_slave_keep_alive(int event, void *data)
{

  //STATE_ENTER(&HttpClientSession::state_slave_keep_alive, event, data);

 // ink_assert(data == slave_ka_vio);
  //ink_assert(bound_ss != NULL);

  switch (event) {
		default:
	//	case VC_EVENT_READ_COMPLETE:
	//		// These events are bogus
	//		ink_assert(0);
	//	/* Fall Through */
	//	case VC_EVENT_ERROR:
	//	case VC_EVENT_READ_READY:
	//	case VC_EVENT_EOS:
	//		// The server session closed or something is amiss
	//		bound_ss->do_io_close();
	//		bound_ss = NULL;
	//		slave_ka_vio = NULL;
	//		break;
	//
	//	case VC_EVENT_ACTIVE_TIMEOUT:
	//	case VC_EVENT_INACTIVITY_TIMEOUT:
	//		// Timeout - place the session on the shared pool
	//		bound_ss->release();
	//		bound_ss = NULL;
	//		slave_ka_vio = NULL;
	//		break;
	}

	return 0;
}

int
HttpClientSession::state_keep_alive(int event, void *data)
{
	// Route the event.  It is either for client vc or
	//  the origin server slave vc
//	if (data && data == slave_ka_vio) {

		  return state_slave_keep_alive(event, data);

//	} else {

//		ink_assert(data && data == ka_vio);
		ink_assert(read_state == HCS_KEEP_ALIVE);

//	}

	//STATE_ENTER(&HttpClientSession::state_keep_alive, event, data);

	switch (event) {
	//	  case VC_EVENT_READ_READY:
	//			// New transaction, need to spawn of new sm to process
	//			// request
	//			new_transaction();
	//			break;
	//
	//	  case VC_EVENT_EOS:
	//			// If there is data in the buffer, start a new
	//			//  transaction, otherwise the client gave up
	//			if (sm_reader->read_avail() > 0) {
	//			  new_transaction();
	//			} else {
	//			  this->do_io_close();
	//			}
	//			break;
	//
	//	  case VC_EVENT_READ_COMPLETE:
	//	  default:
	//			// These events are bogus
	//			ink_assert(0);
	//			// Fall through
	//	  case VC_EVENT_ERROR:
	//	  case VC_EVENT_ACTIVE_TIMEOUT:
	//	  case VC_EVENT_INACTIVITY_TIMEOUT:
	//			// Keep-alive timed out
	//			handle_close(this->uwe, 0);
	//			break;
	  }

	return 0;
}

int
HttpClientSession::state_api_callout(int event, void * /* data ATS_UNUSED */)
{
	switch (event) {
//	case EVENT_NONE:
//	case EVENT_INTERVAL:
//	case HTTP_API_CONTINUE:
//		if ((cur_hook_id >= 0) && (cur_hook_id < TS_HTTP_LAST_HOOK)) {
//			if (!cur_hook) {
//				if (cur_hooks == 0) {
//					cur_hook = http_global_hooks->get(cur_hook_id);
//					cur_hooks++;
//				}
//			}
//			if (!cur_hook) {
//				if (cur_hooks == 1) {
//					cur_hook = api_hooks.get(cur_hook_id);
//					cur_hooks++;
//				}
//			}
//
//			if (cur_hook) {
//
//				bool plugin_lock;
////				Ptr<ProxyMutex> plugin_mutex;
//
//				if (cur_hook->m_cont->mutex) {
//					plugin_mutex = cur_hook->m_cont->mutex;
//					plugin_lock = MUTEX_TAKE_TRY_LOCK(cur_hook->m_cont->mutex, mutex->thread_holding);
//
//					if (!plugin_lock) {
//						SET_HANDLER(&HttpClientSession::state_api_callout);
//						mutex->thread_holding->schedule_in(this, HRTIME_MSECONDS(10), ET_NET);
//						return 0;
//					}
//
//				} else {
//					plugin_lock = false;
//				}
//
//				APIHook *hook = cur_hook;
//				cur_hook = cur_hook->next();
//
//				hook->invoke(TS_EVENT_HTTP_READ_REQUEST_HDR + cur_hook_id, this);
//
//				if (plugin_lock) {
//					// BZ 51246
//					Mutex_unlock(plugin_mutex, this_ethread());
//				}
//
//				return 0;
//			}
//		}
//
//		handle_api_return(event);
//		break;
//
//	default:
//		ink_assert(false);
//	case HTTP_API_ERROR:
		handle_api_return(event);
		break;
	}

	return 0;
}

void
HttpClientSession::handle_api_return(int event)
{
	//SET_HANDLER(&HttpClientSession::state_api_callout);
	cur_msgs = 0;

	switch (cur_msg_id) {

	case OC_HTTP_SSN_START_HOOK:

		//if (event != HTTP_API_ERROR) {
			new_transaction();
		//} else {
			handle_close(GetWorkEnv(), 0);
			return;
		//}
		break;

	case OC_HTTP_SSN_CLOSE_HOOK:
		destroy();
		break;

	default:
		ink_release_assert(0);
		break;
	}
}

void
HttpClientSession::reenable()
{
//	client_vc->reenable(vio);
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
		//this->do_io_close();
		this->handle_close(GetWorkEnv(), 0);
		return;
	}

	if (sm_reader->read_avail() > 0) {

		new_transaction();

	} else {

		read_state = HCS_KEEP_ALIVE;
//		SET_HANDLER(&HttpClientSession::state_keep_alive);
//		ka_vio = this->do_io_read(this, INT64_MAX, read_buffer);
//		ink_assert(slave_ka_vio != ka_vio);
//
//		client_vc->set_inactivity_timeout(HRTIME_SECONDS(ka_in));
//		client_vc->cancel_active_timeout();
	}
}

HttpServerSession *
HttpClientSession::get_bound_ss()
{
	  return bound_ss;
}
