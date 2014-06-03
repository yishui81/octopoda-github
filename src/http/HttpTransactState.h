/*
 * HttpTransactState.h
 *
 *  Created on: 2014年5月26日
 *      Author: jacky
 */

#ifndef HTTPTRANSACTSTATE_H_
#define HTTPTRANSACTSTATE_H_
#include "HttpTransact.h"

class HttpTransactState {
public:
	HttpTransactState();
	virtual ~HttpTransactState();
   void record_transaction_stats();
   void destroy();
   void setup_per_txn_configs();
   void init();
    // Constructor

public:
     HttpTransactMagic_t m_magic;

     HttpStateMachine *state_machine;

     Arena arena;

     HttpConfigParams *http_config_param;
     CacheLookupInfo cache_info;
     bool force_dns;
     DNSLookupInfo dns_info;
     RedirectInfo redirect_info;
     unsigned int updated_server_version;
     bool is_revalidation_necessary;     //Added to check if revalidation is necessary - YTS Team, yamsat
     bool request_will_not_selfloop;     // To determine if process done - YTS Team, yamsat
     HttpConnectionAttributes client_info;
     HttpConnectionAttributes icp_info;
     HttpConnectionAttributes parent_info;
     HttpConnectionAttributes server_info;
     // ConnectionAttributes     router_info;

     Source_t source;
     Source_t pre_transform_source;
     HttpRequestFlavor_t req_flavor;

     CurrentInfo current;
     HeaderInfo hdr_info;
     SquidLogInfo squid_codes;
     HttpApiInfo api_info;
     // To handle parent proxy case, we need to be
     //  able to defer some work in building the request
     TransactFunc_t pending_work;

     // Sandbox of Variables
     StateMachineAction_t cdn_saved_next_action;
     void (*cdn_saved_transact_return_point) (State* s);
     bool cdn_remap_complete;
     bool first_dns_lookup;

     ParentConfigParams *parent_params;
     ParentResult parent_result;
     HttpRequestData request_data;
     CacheControlResult cache_control;
     CacheLookupResult_t cache_lookup_result;
     // FilterResult             content_control;
     bool backdoor_request;      // internal
     bool cop_test_page;         // internal

     StateMachineAction_t next_action;   // out
     StateMachineAction_t api_next_action;       // out
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
    // HostDBInfo host_db_info;    // in
     int cause_of_death_errno;   // in

     ink_time_t client_request_time;     // internal
     ink_time_t request_sent_time;       // internal
     ink_time_t response_received_time;  // internal
     ink_time_t plugin_set_expire_time;

     char via_string[MAX_VIA_INDICES + 1];

     int64_t state_machine_id;

     //HttpAuthParams auth_params;

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
     remap_plugin_info::_tsremap_os_response *fp_tsremap_os_response;
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
     bool api_req_cacheable;
     bool api_resp_cacheable;
     bool api_server_addr_set;
     UpdateCachedObject_t api_update_cached_object;
     LockUrl_t api_lock_url;
     StateMachineAction_t saved_update_next_action;
     CacheAction_t saved_update_cache_action;
     bool stale_icp_lookup;

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
     RangeSetup_t range_setup;
     int64_t num_range_fields;
     int64_t range_output_cl;
     RangeRecord *ranges;

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

#endif /* HTTPTRANSACTSTATE_H_ */
