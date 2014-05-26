/*
 * HttpStateMachine.cpp
 *
 *  Created on: 2014年5月19日
 *      Author: jacky
 */

#include "HttpStateMachine.h"
#include "ink_atomic.h"


static int next_sm_id = 0;
#define HTTP_SM_MAGIC_ALIVE 1
#define HTTP_SM_MAGIC_DEAD  0

ClassAllocator<HttpStateMachine> httpSMAllocator("httpClientSessionAllocator");
HttpStateMachine::HttpStateMachine()
	:client_session(0),
	 debug_on(false)
{

}

HttpStateMachine::~HttpStateMachine()
{

}

inline HttpStateMachine *
HttpStateMachine::allocate()
{
	  extern ClassAllocator<HttpStateMachine> httpSMAllocator;
	  return httpSMAllocator.alloc();
}


int
HttpStateMachine::handle_open  (const URE_Msg& msg)
{
	//  milestones.sm_start = ink_get_hrtime();

	  magic = HTTP_SM_MAGIC_ALIVE;

	  statemachine_id = 0;
//	  enable_redirection = false;
//	  api_enable_redirection = true;
//	  redirect_url = NULL;
//	  redirect_url_len = 0;

	  // Unique state machine identifier.
	  //  changed next_sm_id from int64_t to int because
	  //  atomic(32) is faster than atomic64.  The id is just
	  //  for debugging, so it's OK if it wraps every few days,
	  //  as long as the http_info bucket hash still works.
	  //  (To test this, initialize next_sm_id to 0x7ffffff0)
	  //  Leaving sm_id as int64_t to minimize code changes.

	  statemachine_id = (int64_t) ink_atomic_increment((&next_sm_id), 1);
//	  t_state.state_machine_id = sm_id;
//	  t_state.state_machine = this;

//	  t_state.http_config_param = HttpConfig::acquire();

	  // Simply point to the global config for the time being, no need to copy this
	  // entire struct if nothing is going to change it.
//	  t_state.txn_conf = &t_state.http_config_param->oride;

	  // update the cache info config structure so that
	  // selection from alternates happens correctly.
//	  t_state.cache_info.config.cache_global_user_agent_header = t_state.http_config_param->global_user_agent_header ? true : false;
//	  t_state.cache_info.config.ignore_accept_mismatch = t_state.http_config_param->ignore_accept_mismatch;
//	  t_state.cache_info.config.ignore_accept_language_mismatch = t_state.http_config_param->ignore_accept_language_mismatch ;
//	  t_state.cache_info.config.ignore_accept_encoding_mismatch = t_state.http_config_param->ignore_accept_encoding_mismatch;
//	  t_state.cache_info.config.ignore_accept_charset_mismatch = t_state.http_config_param->ignore_accept_charset_mismatch;
//	  t_state.cache_info.config.cache_enable_default_vary_headers = t_state.http_config_param->cache_enable_default_vary_headers ? true : false;
//
//	  t_state.cache_info.config.cache_vary_default_text = t_state.http_config_param->cache_vary_default_text;
//	  t_state.cache_info.config.cache_vary_default_images = t_state.http_config_param->cache_vary_default_images;
//	  t_state.cache_info.config.cache_vary_default_other = t_state.http_config_param->cache_vary_default_other;
//
//	  t_state.init();
//	  t_state.srv_lookup = hostdb_srv_enabled;

	  // Added to skip dns if the document is in cache. DNS will be forced if there is a ip based ACL in
	  // cache control or parent.config or if the doc_in_cache_skip_dns is disabled or if http caching is disabled
	  // TODO: This probably doesn't honor this as a per-transaction overridable config.
//	  t_state.force_dns = (ip_rule_in_CacheControlTable() || t_state.parent_params->ParentTable->ipMatch ||
//	                       !(t_state.txn_conf->doc_in_cache_skip_dns) || !(t_state.txn_conf->cache_http));

	http_parser_init(&http_parser);

	set_handler(&HttpStateMachine::main_handler);

	//SET_HANDLER(&HttpStateMachine::main_handler);

	return 0;
}

int
HttpStateMachine::handle_close (UWorkEnv * orign_uwe, long retcode)
{
	return 0;
}

int
HttpStateMachine::handle_input (URE_Handle h)
{
	return 0;
}

int
HttpStateMachine::handle_output(URE_Handle h)
{
	return 0;
}

//timer
int
HttpStateMachine::handle_timeout( const TimeValue & origts, long time_id, const void * act )
{
	return 0;
}

//message
int
HttpStateMachine::handle_message( const URE_Msg & msg )
{
	current_hook_type = msg.GetType();
	current_hook_id = msg.GetInt64();
	do_api_callout();
	return 0;
}

int HttpStateMachine::handle_failed_message( const URE_Msg & msg )
{
	return 0;
}

int32_t
HttpStateMachine::do_api_callout()
{
	switch(current_hook_type){
	case HTTP_SM_START:
//		  setup_blind_tunnel_port();
		setup_client_read_request_header();
		break;

	case HTTP_READ_REQUEST_HDR:
	case HTTP_PRE_REMAP:
	case HTTP_REMAP_REQUEST:
	case HTTP_POST_REMAP:
	case HTTP_POST_REMAP_SKIP:
	case HTTP_OS_DNS:
	case HTTP_READ_CACHE_HDR:
	case HTTP_CACHE_LOOKUP_COMPLETE:
	    call_transact_and_set_next_state(NULL);
	   break;
	case HTTP_SEND_REQUEST_HDR:
	    setup_server_send_request();
	    return 0;
	case HTTP_READ_RESPONSE_HDR:
	case HTTP_SEND_RESPONSE_HDR:
		return 0;
	case HTTP_REDIRECT_READ:
	case HTTP_SM_SHUTDOWN:
	    state_remove_from_list(EVENT_NONE, NULL);
	    return 0;
	  default:
	    //ink_release_assert("! Not reached");
	    break;
	}

	switch (current_hook_id) {
		  case HttpTransact::TRANSFORM_READ:
		    {
		      HttpTunnelProducer *p = setup_transfer_from_transform();
		      perform_transform_cache_write_action();
		      tunnel.tunnel_run(p);
		      break;
		    }
		  case HttpTransact::SERVER_READ:
		    {
		      setup_server_transfer();
		      perform_cache_write_action();
		      tunnel.tunnel_run();
		      break;
		    }
		  case HttpTransact::SERVE_FROM_CACHE:
		    {
		      setup_cache_read_transfer();
		      tunnel.tunnel_run();
		      break;
		    }

		  case HttpTransact::PROXY_INTERNAL_CACHE_WRITE:
		    {
		      if (cache_sm.cache_write_vc) {
		        setup_internal_transfer(&HttpStateMachine::tunnel_handler_cache_fill);
		      } else {
		        setup_internal_transfer(&HttpStateMachine::tunnel_handler);
		      }
		      break;
		    }

		  case HttpTransact::PROXY_INTERNAL_CACHE_NOOP:
		  case HttpTransact::PROXY_INTERNAL_CACHE_DELETE:
		  case HttpTransact::PROXY_INTERNAL_CACHE_UPDATE_HEADERS:
		  case HttpTransact::PROXY_SEND_ERROR_CACHE_NOOP:
		    {
		      setup_internal_transfer(&HttpStateMachine::tunnel_handler);
		      break;
		    }

		  case HttpTransact::REDIRECT_READ:
		    {
		     // call_transact_and_set_next_state(HttpTransact::HandleRequest);
		      break;
		    }

		  default:
		    {
		     // ink_release_assert(!"Should not get here");
		    }
	}
	return 0;
}

int32_t
HttpStateMachine::start_sub_state_machine()
{
	statemachine_id = 0;

	//TODO
	http_parser_init(&http_parser);

	return 0;
}

int32_t
HttpStateMachine::attach_client_session(HttpClientSession* client_session, IOBufferReader *sm)
{
	this->client_session = client_session;

	if (client_session->debug_on)
	{
		debug_on = true;
	}

	start_sub_state_machine();

	http_parser_init(&http_parser);

	return 0;
}

int32_t
HttpStateMachine::setup_client_read_request_header(){

	client_session-> do_io_read( this,INT64_MAX, ua_buffer_reader->mbuf);
	if(ua_buffer_reader->read_avail() > 0){
		//TODO
		//do_handle(VC_EVENT_READ_READY, ua_entry-> )
		//handle(VC_EVENT_READ_READY, ua_entry->read_vio);
	}
	return 0;
}

int32_t
HttpStateMachine::call_transact_and_set_next_state(){
	return 0;
}

int32_t
HttpStateMachine::setup_server_send_request(){
	return 0;
}

HttpTunnelProducer *
HttpStateMachine::setup_transfer_from_transform(){

//	int64_t alloc_index = find_server_buffer_size();
//
//	// TODO change this call to new_empty_MIOBuffer()
//	MIOBuffer *buf = new_MIOBuffer(alloc_index);
//	buf->water_mark = (int) t_state.txn_conf->default_buffer_water_mark;
//	IOBufferReader *buf_start = buf->alloc_reader();
//
//	HttpTunnelConsumer *c = tunnel.get_consumer(transform_info.vc);
//	ink_assert(c != NULL);
//	ink_assert(c->vc == transform_info.vc);
//	ink_assert(c->vc_type == HT_TRANSFORM);

	// Now dump the header into the buffer
//	ink_assert(t_state.hdr_info.client_response.status_get() != HTTP_STATUS_NOT_MODIFIED);
//	client_response_hdr_bytes = write_response_header_into_buffer(&t_state.hdr_info.client_response, buf);
//
//	HTTP_SM_SET_DEFAULT_HANDLER(&HttpStateMachine::tunnel_handler);
//
//	HttpTunnelProducer *p = tunnel.add_producer(transform_info.vc,
//												INT64_MAX,
//												buf_start,
//												&HttpStateMachine::tunnel_handler_transform_read,
//												HT_TRANSFORM,
//												"transform read");
//	tunnel.chain(c, p);
//
//	tunnel.add_consumer(ua_entry->vc, transform_info.vc, &HttpStateMachine::tunnel_handler_ua, HT_HTTP_CLIENT, "user agent");
//
//	transform_info.entry->in_tunnel = true;
//	ua_entry->in_tunnel = true;
//
//	this->setup_plugin_agents(p);
//
//	if ( t_state.client_info.receive_chunked_response ) {
//	  tunnel.set_producer_chunking_action(p, client_response_hdr_bytes, TCA_CHUNK_CONTENT);
//	  tunnel.set_producer_chunking_size(p, t_state.txn_conf->http_chunking_size);
//	}

//	return p;
	return NULL;
}
