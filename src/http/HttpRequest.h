/*
 * HttpRequest.h
 *
 *  Created on: 2014年5月28日
 *      Author: jacky
 */

#ifndef HTTPREQUEST_H_
#define HTTPREQUEST_H_
#include "Http.h"

class HostDBInfo;

enum StateMachineAction_t
{
	STATE_MACHINE_ACTION_UNDEFINED = 0,

	DNS_LOOKUP,
	REVERSE_DNS_LOOKUP,
	//AUTH_LOOKUP,

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

	HTTP_API_SM_START,

	HTTP_API_READ_REQUEST_HDR,
	HTTP_API_PRE_REMAP,
	HTTP_REMAP_REQUEST,
	HTTP_API_POST_REMAP,
	HTTP_POST_REMAP_SKIP,

	HTTP_API_OS_DNS,
	HTTP_API_SEND_REQUEST_HDR,
	HTTP_API_READ_CACHE_HDR,
	HTTP_API_CACHE_LOOKUP_COMPLETE,
	HTTP_API_READ_RESPONSE_HDR,
	HTTP_API_SEND_RESPONSE_HDR,
	REDIRECT_READ,
	HTTP_API_SM_SHUTDOWN
};


class HttpRequest {
public:
	HttpRequest(HttpStateMachine* state_machine);
	virtual ~HttpRequest();

public:
     HttpStateMachine* state_machine;
     TransportType	  port_attribute;
     int64_t			  next_step;

     Arena arena;

     HttpConfigParams *http_config_param;
//     CacheLookupInfo cache_info;
     bool force_dns;
//     DNSLookupInfo dns_info;
//     RedirectInfo redirect_info;
     unsigned int updated_server_version;
     bool is_revalidation_necessary;     //Added to check if revalidation is necessary - YTS Team, yamsat
     bool request_will_not_selfloop;     // To determine if process done - YTS Team, yamsat
     HttpConnectionAttributes client_info;
     HttpConnectionAttributes icp_info;
     HttpConnectionAttributes parent_info;
     HttpConnectionAttributes server_info;
     HttpConnectionAttributes router_info;

     Source_t source;
     Source_t pre_transform_source;
     HttpRequestFlavor_t req_flavor;

     CurrentInfo  current;
	  HeaderInfo 	hdr_info;
     SquidLogInfo squid_codes;
     HttpApiInfo  api_info;
     // To handle parent proxy case, we need to be
     //  able to defer some work in building the request


     // Sandbox of Variables
     StateMachineAction_t cdn_saved_next_action;
//     void (*cdn_saved_transact_return_point) (State* s);
     bool cdn_remap_complete;
     bool first_dns_lookup;

     ParentConfigParams *parent_params;
     ParentResult parent_result;
     HttpRequestData request_data;
     CacheControlResult cache_control;
     CacheLookupResult_t cache_lookup_result;
      FilterResult             content_control;
     bool backdoor_request;      // internal
     bool cop_test_page;         // internal

//     StateMachineAction_t next_action;   // out
//     StateMachineAction_t api_next_action;       // out
     void (*transact_return_point) (HttpTransact::State* s);    // out
     char *internal_msg_buffer;  // out
     char *internal_msg_buffer_type;     // out
     int64_t internal_msg_buffer_size;       // out
     int64_t internal_msg_buffer_fast_allocator_size;
     int64_t internal_msg_buffer_index;      // out

     bool icp_lookup_success;    // in
     struct sockaddr_in icp_ip_result;   // in

     int scheme;                 // out
     int next_hop_scheme;        // out
     int orig_scheme;            // pre-mapped scheme
     int method;
     HostDBInfo host_db_info;    // in
     int cause_of_death_errno;   // in

     ink_time_t client_request_time;     // internal
     ink_time_t request_sent_time;       // internal
     ink_time_t response_received_time;  // internal
     ink_time_t plugin_set_expire_time;

     char via_string[MAX_VIA_INDICES + 1];

     int64_t state_machine_id;

     HttpAuthParams auth_params;

     StatBlock first_stats;
     StatBlock *current_stats;

     // for negative caching
     bool negative_caching;
     // for srv_lookup
     bool srv_lookup;
     // for authenticated content caching
     CacheAuth_t www_auth_content;

     // new ACL filtering result (calculated immediately after remap)
     bool client_connection_enabled;
     bool acl_filtering_performed;

     // INK API/Remap API plugin interface
//     remap_plugin_info::_tsremap_os_response *fp_tsremap_os_response;
     void* remap_plugin_instance;
     HTTPStatus http_return_code;
     int return_xbuf_size;
     bool return_xbuf_plain;
     char return_xbuf[HTTP_TRANSACT_STATE_MAX_XBUF_SIZE];
     void *user_args[HTTP_SSN_TXN_MAX_USER_ARG];

     int api_txn_active_timeout_value;
     int api_txn_connect_timeout_value;
     int api_txn_dns_timeout_value;
     int api_txn_no_activity_timeout_value;

     // Used by INKHttpTxnCachedReqGet and INKHttpTxnCachedRespGet SDK functions
     // to copy part of HdrHeap (only the writable portion) for cached response headers
     // and request headers
     // These ptrs are deallocate when transaction is over.
     HdrHeapSDKHandle *cache_req_hdr_heap_handle;
     HdrHeapSDKHandle *cache_resp_hdr_heap_handle;
     bool api_release_server_session;
     bool api_cleanup_cache_read;
     bool api_server_response_no_store;
     bool api_server_response_ignore;
     bool api_http_sm_shutdown;
     bool api_modifiable_cached_resp;

     bool api_server_request_body_set;
       //请求是否缓存
     bool req_cacheable;
      //回源的返回的内容是否缓存
     bool response_cacheable;
       //是否指定了回源的IP
     bool server_addr_set;
//     UpdateCachedObject_t api_update_cached_object;
//     LockUrl_t api_lock_url;
//     StateMachineAction_t saved_update_next_action;
//     CacheAction_t saved_update_cache_action;

     // Remap plugin processor support
     UrlMappingContainer url_map;
     host_hdr_info hh_info;

     // congestion control
     CongestionEntry *pCongestionEntry;
     StateMachineAction_t congest_saved_next_action;
     int congestion_control_crat;        // 'client retry after'
     int congestion_congested_or_failed;
     int congestion_connection_opened;

     bool reverse_proxy;
     bool url_remap_success;
     char *remap_redirect;
     unsigned int filter_mask;

     bool already_downgraded;
     URL pristine_url;  // pristine url is the url before remap

     bool api_skip_all_remapping;

     // Http Range: related variables
//     RangeSetup_t range_setup;
     int64_t num_range_fields;
     int64_t range_output_cl;
//     RangeRecord *ranges;

     OverridableHttpConfigParams *txn_conf;
     OverridableHttpConfigParams my_txn_conf; // Storage for plugins, to avoid malloc

     bool transparent_passthrough;

     // Methods
     void
     init()
     {
       parent_params = ParentConfig::acquire();
       current_stats = &first_stats;
     }
};

#endif /* HTTPREQUEST_H_ */
