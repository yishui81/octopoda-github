/*
 * HttpStateMachine.cpp
 *
 *  Created on: 2014年5月19日
 *      Author: jacky
 */

#include "HttpStateMachine.h"
#include "ink_atomic.h"
#include "HttpTunnelProducer.h"
#include "HttpTunnelConsumer.h"
#include "P_IOBuffer.h"

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
	magic = HTTP_SM_MAGIC_ALIVE;

	statemachine_id = 0;
	statemachine_id = (int64_t) ink_atomic_increment((&next_sm_id), 1);

	http_parser_init(&http_parser);

	sm_msg = msg;
	msg.SetInt64(0);
	msg.SetType(HTTP_SM_START);

	current_hook_type = HTTP_SM_START;
	request = new HttpRequest(this);
	do_api_callout();

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
		if (request->port_attribute == TRANSPORT_BLIND_TUNNEL)
		{
			setup_blind_tunnel_port();
		}
		else
		{
			setup_client_read_request_header();
		}
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
	    //state_remove_from_list(EVENT_NONE, NULL);
	    return 0;
	  default:
	    //ink_release_assert("! Not reached");
	    break;
	}

	switch (current_hook_id) {
		  case TRANSFORM_READ:
		    {
		      HttpTunnelProducer *p = setup_transfer_from_transform();
		      perform_transform_cache_write_action();
		      tunnel.tunnel_run(p);
		      break;
		    }
		  case SERVER_READ:
		    {
//		      setup_server_transfer();
//		      perform_cache_write_action();
//		      tunnel.tunnel_run();
		      break;
		    }
		  case SERVE_FROM_CACHE:
		    {
//		      setup_cache_read_transfer();
//		      tunnel.tunnel_run();
		      break;
		    }

		  case PROXY_INTERNAL_CACHE_WRITE:
		    {
//		      if (cache_sm.cache_write_vc) {
//		        setup_internal_transfer(&HttpStateMachine::tunnel_handler_cache_fill);
//		      } else {
//		        setup_internal_transfer(&HttpStateMachine::tunnel_handler);
//		      	}
		      break;
		    }

		  case PROXY_INTERNAL_CACHE_NOOP:
		  case PROXY_INTERNAL_CACHE_DELETE:
		  case PROXY_INTERNAL_CACHE_UPDATE_HEADERS:
		  case PROXY_SEND_ERROR_CACHE_NOOP:
		    {
//		      setup_internal_transfer(&HttpStateMachine::tunnel_handler);
		      break;
		    }

		  case REDIRECT_READ:
		    {
		     // call_transact_and_set_next_state(HandleRequest);
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
		do_handle(CON_EVENT_READ_READY, ua_entry->read_vio);
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
	int64_t alloc_index = 0;
//
//	// TODO change this call to new_empty_MIOBuffer()
	MIOBuffer *buf = new_MIOBuffer(alloc_index);
	buf->water_mark = 0;
	//buf->water_mark = (int) request->txn_conf->default_buffer_water_mark;
	IOBufferReader *buf_start = buf->alloc_reader();
//
	HttpTunnelConsumer *c = tunnel.get_consumer(transform_info.vc);
//	ink_assert(c != NULL);
//	ink_assert(c->vc == transform_info.vc);
//	ink_assert(c->vc_type == HT_TRANSFORM);

	// Now dump the header into the buffer
//	ink_assert(request->hdr_info.client_response.status_get() != HTTP_STATUS_NOT_MODIFIED);
	client_response_hdr_bytes = write_response_header_into_buffer(&request->hdr_info.client_response, buf);
//
	HTTP_SM_SET_DEFAULT_HANDLER(&HttpStateMachine::tunnel_handler);
//
	HttpTunnelProducer *p = tunnel.add_producer(transform_info.vc,
												INT64_MAX,
												buf_start,
												&HttpStateMachine::tunnel_handler_transform_read,
												HT_TRANSFORM,
												"transform read");
	tunnel.chain(c, p);

	tunnel.add_consumer(ua_entry->vc, transform_info.vc, &HttpStateMachine::tunnel_handler_ua, HT_HTTP_CLIENT, "user agent");

	transform_info.entry->in_tunnel = true;
	ua_entry->in_tunnel = true;

	this->setup_plugin_agents(p);
//
//	if ( request->client_info.receive_chunked_response ) {
//	  tunnel.set_producer_chunking_action(p, client_response_hdr_bytes, TCA_CHUNK_CONTENT);
//	  tunnel.set_producer_chunking_size(p, request->txn_conf->http_chunking_size);
//	}

	return p;
	//return NULL;
}


void
HttpStateMachine::setup_blind_tunnel_port()
{
  // We gotten a connect on a port for blind tunneling so
  //  call transact figure out where it is going
  call_transact_and_set_next_state(HandleBlindTunnel);
}

void
HttpStateMachine::call_transact_and_set_next_state(TransactEntryFunc_t f)
{
//  last_action = request->next_action;    // remember where we were
//
//  // The callee can either specify a method to call in to Transact,
//  //   or call with NULL which indicates that Transact should use
//  //   its stored entry point.
//  if (f == NULL) {
//    ink_release_assert(request->transact_return_point != NULL);
//    request->transact_return_point(&t_state);
//  } else {
//    f(&t_state);
//  }
//
//  DebugSM("http", "[%" PRId64 "] State Transition: %s -> %s",
//        sm_id, HttpDebugNames::get_action_name(last_action), HttpDebugNames::get_action_name(request->next_action));

  set_next_state();

  return;
}



void
HttpStateMachine::set_next_state()
{
  ///////////////////////////////////////////////////////////////////////
  // Use the returned "next action" code to set the next state handler //
  ///////////////////////////////////////////////////////////////////////
  switch (request->next_step) {
  case HTTP_API_READ_REQUEST_HDR:
  case HTTP_API_PRE_REMAP:
  case HTTP_API_POST_REMAP:
  case HTTP_API_OS_DNS:
  case HTTP_API_SEND_REQUEST_HDR:
  case HTTP_API_READ_CACHE_HDR:
  case HTTP_API_READ_RESPONSE_HDR:
  case HTTP_API_SEND_RESPONSE_HDR:
  case HTTP_API_CACHE_LOOKUP_COMPLETE:
	{
		//request.api_next_action = request.next_action;
		do_api_callout();
		break;
	}

  case HTTP_POST_REMAP_SKIP:
    {
      call_transact_and_set_next_state(NULL);
      break;
    }

  case HTTP_REMAP_REQUEST:
    {
    	do_rewrite_request(false);
      break;
    }

  case DNS_LOOKUP:
    {
      sockaddr const* addr;

      if (request->server_addr_set) {
        /* If the API has set the server address before the OS DNS lookup
         * then we can skip the lookup
          */
        ip_text_buffer ipb;
        //DebugSM("dns", "[HandleRequest] Skipping DNS lookup for API supplied target %s.\n", ats_ip_ntop(&request->server_info.addr, ipb, sizeof(ipb)));
        // this seems wasteful as we will just copy it right back
        //ats_ip_copy(request->host_db_info.ip(), &request->server_info.addr);
        //request->dns_info.lookup_success = true;
        call_transact_and_set_next_state(NULL);
        break;
      } else if (url_remap_mode == 2 && request->first_dns_lookup) {
       // DebugSM("cdn", "Skipping DNS Lookup");
        // skip the DNS lookup
    	  request->first_dns_lookup = false;
        call_transact_and_set_next_state(HandleFiltering);
        break;
      } else  if (request->http_config_param->use_client_target_addr
        && !request->url_remap_success
        && request->parent_result.r != PARENT_SPECIFIED
        && request->client_info.is_transparent
        && request->dns_info.os_addr_style == DNSLookupInfo::OS_ADDR_TRY_DEFAULT
        && ats_is_ip(addr = request->state_machine->ua_session->get_netvc()->get_local_addr())
      ) {
        /* If the connection is client side transparent and the URL
         * was not remapped/directed to parent proxy, we can use the
         * client destination IP address instead of doing a DNS
         * lookup. This is controlled by the 'use_client_target_addr'
         * configuration parameter.
         */
        if (is_debug_tag_set("dns")) {
          ip_text_buffer ipb;
          //DebugSM("dns", "[HandleRequest] Skipping DNS lookup for client supplied target %s.\n", ats_ip_ntop(addr, ipb, sizeof(ipb)));
        	}
        //ats_ip_copy(request->host_db_info.ip(), addr);
        /* Since we won't know the server HTTP version (no hostdb lookup), we assume it matches the
         * client request version. Seems to be the most correct thing to do in the transparent use-case.
         */
        if (request->hdr_info.client_request.version_get() == HTTPVersion(0, 9)){
        	//request->host_db_info.app.http_data.http_version =  HostDBApplicationInfo::HTTP_VERSION_09;
        }else if (request->hdr_info.client_request.version_get() == HTTPVersion(1, 0)){
        	//request->host_db_info.app.http_data.http_version =  HostDBApplicationInfo::HTTP_VERSION_10;
        }else{
        	//request->host_db_info.app.http_data.http_version =  HostDBApplicationInfo::HTTP_VERSION_11;
           }

        request->dns_info.lookup_success = true;
        // cache this result so we don't have to unreliably duplicate the
        // logic later if the connect fails.
        request->dns_info.os_addr_style = DNSLookupInfo::OS_ADDR_TRY_CLIENT;
        call_transact_and_set_next_state(NULL);
        break;
      } else if (request->parent_result.r == PARENT_UNDEFINED && request->dns_info.lookup_success) {
        // Already set, and we don't have a parent proxy to lookup
//        ink_assert(ats_is_ip(request->host_db_info.ip()));
//        DebugSM("dns", "[HandleRequest] Skipping DNS lookup, provided by plugin");
        call_transact_and_set_next_state(NULL);
        break;
      } else if (request->dns_info.looking_up == ORIGIN_SERVER &&
    		  request->http_config_param->no_dns_forward_to_parent){

        if (request->cop_test_page)
          ats_ip_copy(request->host_db_info.ip(), request->state_machine->ua_session->get_netvc()->get_local_addr());

        request->dns_info.lookup_success = true;
        call_transact_and_set_next_state(NULL);
        break;
      }

      HTTP_SM_SET_DEFAULT_HANDLER(&HttpStateMachine::state_hostdb_lookup);

      ink_assert(request->dns_info.looking_up != UNDEFINED_LOOKUP);
      do_hostdb_lookup();
      break;
    }

  case REVERSE_DNS_LOOKUP:
    {
      HTTP_SM_SET_DEFAULT_HANDLER(&HttpStateMachine::state_hostdb_reverse_lookup);
      do_hostdb_reverse_lookup();
      break;
    }

  case CACHE_LOOKUP:
    {
      HTTP_SM_SET_DEFAULT_HANDLER(&HttpStateMachine::state_cache_open_read);
      do_cache_lookup_and_read();
      break;
    }

  case ORIGIN_SERVER_OPEN:
    {
      if (congestionControlEnabled && (request->congest_saved_next_action == STATE_MACHINE_ACTION_UNDEFINED)) {
        request->congest_saved_next_action = ORIGIN_SERVER_OPEN;
        HTTP_SM_SET_DEFAULT_HANDLER(&HttpStateMachine::state_congestion_control_lookup);
        if (!do_congestion_control_lookup())
          break;
      }
      HTTP_SM_SET_DEFAULT_HANDLER(&HttpStateMachine::state_http_server_open);

      // We need to close the previous attempt
      if (server_entry) {
        ink_assert(server_entry->vc_type == HTTP_SERVER_VC);
        vc_table.cleanup_entry(server_entry);
        server_entry = NULL;
        server_session = NULL;
      } else {
        // Now that we have gotten the user agent request, we can cancel
        // the inactivity timeout associated with it.  Note, however, that
        // we must not cancel the inactivity timeout if the message
        // contains a body (as indicated by the non-zero request_content_length
        // field).  This indicates that a POST operation is taking place and
        // that the client is still sending data to the origin server.  The
        // origin server cannot reply until the entire request is received.  In
        // light of this dependency, TS must ensure that the client finishes
        // sending its request and for this reason, the inactivity timeout
        // cannot be cancelled.
        if (ua_session && !request->hdr_info.request_content_length) {
          ua_session->get_netvc()->cancel_inactivity_timeout();
        }
      }

      do_http_server_open();
      break;
    }

  case SERVER_PARSE_NEXT_HDR:
    {
      setup_server_read_response_header();
      break;
    }

  case PROXY_INTERNAL_100_RESPONSE:
    {
      setup_100_continue_transfer();
      break;
    }

  case SERVER_READ:
    {
      request->source = SOURCE_HTTP_ORIGIN_SERVER;

      if (transform_info.vc) {
        ink_assert(request->hdr_info.client_response.valid() == 0);
        ink_assert((request->hdr_info.transform_response.valid()? true : false) == true);
        HttpTunnelProducer *p = setup_server_transfer_to_transform();
        perform_cache_write_action();
        tunnel.tunnel_run(p);
      } else {
        ink_assert((request->hdr_info.client_response.valid()? true : false) == true);
        request->api_next_action = HTTP_API_SEND_RESPONSE_HDR;

        if (hooks_set) {
          do_api_callout_internal();
        } else {
          do_redirect();
          handle_api_return();
        }

      }
      break;
    }

  case SERVE_FROM_CACHE:
    {
      ink_assert(request->cache_info.action == CACHE_DO_SERVE ||
                 request->cache_info.action == CACHE_DO_SERVE_AND_DELETE ||
                 request->cache_info.action == CACHE_DO_SERVE_AND_UPDATE);
      release_server_session(true);
      request->source = SOURCE_CACHE;

      if (transform_info.vc) {
        ink_assert(request->hdr_info.client_response.valid() == 0);
        ink_assert((request->hdr_info.transform_response.valid()? true : false) == true);
        request->hdr_info.cache_response.create(HTTP_TYPE_RESPONSE);
        request->hdr_info.cache_response.copy(&request->hdr_info.transform_response);

        HttpTunnelProducer *p = setup_cache_transfer_to_transform();
        perform_cache_write_action();
        tunnel.tunnel_run(p);
      } else {
        ink_assert((request->hdr_info.client_response.valid()? true : false) == true);
        request->hdr_info.cache_response.create(HTTP_TYPE_RESPONSE);
        request->hdr_info.cache_response.copy(&request->hdr_info.client_response);

        perform_cache_write_action();
        request->api_next_action = HTTP_API_SEND_RESPONSE_HDR;
        if (hooks_set) {
          do_api_callout_internal();
        } else {
          do_redirect();
          handle_api_return();
        }
      }
      break;
    }

  case CACHE_ISSUE_WRITE:
    {
      ink_assert(cache_sm.cache_write_vc == NULL);
      HTTP_SM_SET_DEFAULT_HANDLER(&HttpStateMachine::state_cache_open_write);

      do_cache_prepare_write();
      break;

    }

  case PROXY_INTERNAL_CACHE_WRITE:
    {
      request->api_next_action = HTTP_API_SEND_RESPONSE_HDR;
      do_api_callout();
      break;
    }

  case PROXY_INTERNAL_CACHE_NOOP:
    {
      if (server_entry == NULL || server_entry->in_tunnel == false) {
        release_server_session();
      }
      // If we're in state SEND_API_RESPONSE_HDR, it means functions
      // registered to hook SEND_RESPONSE_HDR have already been called. So we do not
      // need to call do_api_callout. Otherwise TS loops infinitely in this state !
      if (request->api_next_action == HTTP_API_SEND_RESPONSE_HDR) {
        handle_api_return();
      } else {
        request->api_next_action = HTTP_API_SEND_RESPONSE_HDR;
        do_api_callout();
      }
      break;
    }

  case PROXY_INTERNAL_CACHE_DELETE:
    {
      // Nuke all the alternates since this is mostly likely
      //   the result of a delete method
      cache_sm.end_both();
      do_cache_delete_all_alts(NULL);

      release_server_session();
      request->api_next_action = HTTP_API_SEND_RESPONSE_HDR;
      do_api_callout();
      break;
    }

  case PROXY_INTERNAL_CACHE_UPDATE_HEADERS:
    {
      issue_cache_update();
      cache_sm.close_read();

      release_server_session();
      request->api_next_action = HTTP_API_SEND_RESPONSE_HDR;
      do_api_callout();
      break;

    }

  case PROXY_SEND_ERROR_CACHE_NOOP:
    {
      setup_error_transfer();
      break;
    }


  case PROXY_INTERNAL_REQUEST:
    {
      HTTP_SM_SET_DEFAULT_HANDLER(&HttpStateMachine::state_handle_stat_page);
      Action *action_handle = statPagesManager.handle_http(this, &request->hdr_info.client_request);

      if (action_handle != ACTION_RESULT_DONE) {
        pending_action = action_handle;
        historical_action = pending_action;
      }

      break;
    }

  case OS_RR_MARK_DOWN:
    {
      HTTP_SM_SET_DEFAULT_HANDLER(&HttpStateMachine::state_mark_os_down);

      ink_assert(request->dns_info.looking_up == ORIGIN_SERVER);

      // TODO: This might not be optimal (or perhaps even correct), but it will
      // effectively mark the host as down. What's odd is that state_mark_os_down
      // above isn't triggering.
      HttpStateMachine::do_hostdb_update_if_necessary();

      do_hostdb_lookup();
      break;
    }

  case SSL_TUNNEL:
    {
      setup_blind_tunnel(true);
      break;
    }

  case ORIGIN_SERVER_RAW_OPEN:{
      if (congestionControlEnabled && (request->congest_saved_next_action == STATE_MACHINE_ACTION_UNDEFINED)) {
        request->congest_saved_next_action = ORIGIN_SERVER_RAW_OPEN;
        HTTP_SM_SET_DEFAULT_HANDLER(&HttpStateMachine::state_congestion_control_lookup);
        if (!do_congestion_control_lookup())
          break;
      }

      HTTP_SM_SET_DEFAULT_HANDLER(&HttpStateMachine::state_raw_http_server_open);

      ink_assert(server_entry == NULL);
      do_http_server_open(true);
      break;
    }

  case ICP_QUERY:
    {
      HTTP_SM_SET_DEFAULT_HANDLER(&HttpStateMachine::state_icp_lookup);
      do_icp_lookup();
      break;
    }

  case CACHE_ISSUE_WRITE_TRANSFORM:
    {
      ink_assert(request->cache_info.transform_action == CACHE_PREPARE_TO_WRITE);

      if (transform_cache_sm.cache_write_vc) {
        // We've already got the write_vc that
        //  didn't use for the untransformed copy
        ink_assert(cache_sm.cache_write_vc == NULL);
        ink_assert(request->api_info.cache_untransformed == false);
        request->cache_info.write_lock_state = CACHE_WL_SUCCESS;
        call_transact_and_set_next_state(NULL);
      } else {
        HTTP_SM_SET_DEFAULT_HANDLER(&HttpStateMachine::state_cache_open_write);

        do_cache_prepare_write_transform();
      }
      break;
    }

  case TRANSFORM_READ:
    {
      request->api_next_action = HTTP_API_SEND_RESPONSE_HDR;
      do_api_callout();
      break;
    }

  case READ_PUSH_HDR:
    {
      setup_push_read_response_header();
      break;
    }

  case STORE_PUSH_BODY:
    {
      setup_push_transfer_to_cache();
      tunnel.tunnel_run();
      break;
    }

  case PREPARE_CACHE_UPDATE:
    {
      ink_assert(request->api_update_cached_object == UPDATE_CACHED_OBJECT_CONTINUE);
      do_cache_prepare_update();
      break;
    }
  case ISSUE_CACHE_UPDATE:
    {
      if (request->api_update_cached_object == UPDATE_CACHED_OBJECT_ERROR) {
        request->cache_info.object_read = NULL;
        cache_sm.close_read();
      }
      issue_cache_update();
      call_transact_and_set_next_state(NULL);
      break;
    }

#ifdef PROXY_DRAIN
  case PROXY_DRAIN_REQUEST_BODY:
    {
      do_drain_request_body();
      break;
    }
#endif /* PROXY_DRAIN */

  case SEND_QUERY_TO_INCOMING_ROUTER:
  case CONTINUE:
    {
      //ink_release_assert(!"Not implemented");
    }

  default:
    {
      //ink_release_assert("!Unknown next action");
    }
  }
}


int
HttpStateMachine::main_handler(int event, void *data)
{

  if(magic == HTTP_SM_MAGIC_ALIVE){
	  return 0;
  }

  HttpSMHandler jump_point = NULL;
 // ink_assert(reentrancy_count >= 0);
 // reentrancy_count++;

  // Don't use the state enter macro since it uses history
  //  space that we don't care about
 // DebugSM("http", "[%" PRId64 "] [HttpSM::main_handler, %s]", sm_id, HttpDebugNames::get_event_name(event));

//  HttpVCTableEntry *vc_entry = NULL;

  if (data != NULL) {
    // Only search the VC table if the event could have to
    //  do with a VIO to save a few cycles

    if (event < CON_EVENT_EVENTS_START + 100) {
      vc_entry = vc_table.find_entry((VIO *) data);
    }
  }

  if (vc_entry) {
    jump_point = vc_entry->vc_handler;
    ink_assert(jump_point != (HttpSMHandler)NULL);
    ink_assert(vc_entry->vc != (VConnection *)NULL);
    (this->*jump_point) (event, data);
  } else {
    ink_assert(default_handler != (HttpSMHandler)NULL);
    (this->*default_handler) (event, data);
  }

  // The sub-handler signals when it is time for the state
  //  machine to exit.  We can only exit if we are not reentrantly
  //  called otherwise when the our call unwinds, we will be
  //  running on a dead state machine
  //
  // Because of the need for an api shutdown hook, kill_this()
  // is also reentrant.  As such, we don't want to decrement
  // the reentrancy count until after we run kill_this()
  //
  if (terminate_sm == true && reentrancy_count == 1) {
    kill_this();
  } else {
    reentrancy_count--;
    ink_assert(reentrancy_count >= 0);
  }

  return (CON_EVENT_CONT);

}

void
HttpStateMachine::perform_transform_cache_write_action()
{
//  DebugSM("http", "[%" PRId64 "] perform_transform_cache_write_action %s", sm_id,
//        HttpDebugNames::get_cache_action_name(request->cache_info.action));

//  if (request->range_setup)
//    return;
//
//  switch (request->cache_info.transform_action) {
//  case CACHE_DO_NO_ACTION:
//    {
//      // Nothing to do
//      transform_cache_sm.end_both();
//      break;
//    }
//
//  case CACHE_DO_WRITE:
//    {
//      transform_cache_sm.close_read();
//      request->cache_info.transform_write_status = CACHE_WRITE_IN_PROGRESS;
//      setup_cache_write_transfer(&transform_cache_sm,
//                                 transform_info.entry->vc,
//                                 &request->cache_info.transform_store, client_response_hdr_bytes, "cache write t");
//      break;
//    }
//
//  default:
//    ink_release_assert(0);
//    break;
//  }

}


int32_t
HttpStateMachine::do_rewrite_request(bool )
{
	return 0;
}
