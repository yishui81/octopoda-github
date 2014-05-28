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


#if !defined (_HttpTransact_h_)
#define _HttpTransact_h_

#include "libts.h"
#include "HttpConfig.h"
#include "Http.h"
#include "HttpTransactCache.h"
//#include "ink_stat_t.h"
//#include "StatSystem.h"
//#include "ControlMatcher.h"
//#include "CacheControl.h"
//#include "ParentSelection.h"
//#include "ProxyConfig.h"
//#include "Transform.h"
//#include "HttpAuthParams.h"
//#include "api/ts/remap.h"
//#include "RemapPluginInfo.h"
//#include "UrlMapping.h"
//#include <records/I_RecHttp.h>

//#include "congest/Congestion.h"

#define MAX_DNS_LOOKUPS 2
#define NUM_SECONDS_IN_ONE_YEAR (31536000)      // (365L * 24L * 3600L)

#define HTTP_RELEASE_ASSERT(X) ink_release_assert(X)
// #define ink_cluster_time(X) time(X)

#define ACQUIRE_PRINT_LOCK()    //ink_mutex_acquire(&print_lock);
#define RELEASE_PRINT_LOCK()    //ink_mutex_release(&print_lock);

#define DUMP_HEADER(T,H,I,S) \
{ \
    if (diags->on(T)) { \
    ACQUIRE_PRINT_LOCK() \
    fprintf(stderr, "+++++++++ %s +++++++++\n", S); \
    fprintf(stderr, "-- State Machine Id: %" PRId64 "\n", I); \
        char b[4096]; \
        int used, tmp, offset; \
        int done; \
        offset = 0; \
    if ((H)->valid()) { \
          do { \
              used = 0; \
              tmp = offset; \
              done = (H)->print (b, 4095, &used, &tmp); \
              offset += used; \
              b[used] = '\0'; \
              fprintf (stderr, "%s", b); \
          } while (!done); \
        } \
    RELEASE_PRINT_LOCK() \
    } \
}


#define TRANSACT_RETURN(n, r)  \
s->next_action = n; \
s->transact_return_point = r; \
DebugSpecific((s->state_machine && s->state_machine->debug_on), "http_trans", "Next action %s; %s", #n, #r); \
return; \

#define SET_UNPREPARE_CACHE_ACTION(C) \
{ \
    if (C.action == HttpTransact::CACHE_PREPARE_TO_DELETE) { \
    C.action = HttpTransact::CACHE_DO_DELETE; \
    } else if (C.action == HttpTransact::CACHE_PREPARE_TO_UPDATE) { \
    C.action = HttpTransact::CACHE_DO_UPDATE; \
    } else { \
    C.action = HttpTransact::CACHE_DO_WRITE; \
    } \
}

typedef time_t ink_time_t;

struct HttpConfigParams;
struct MimeTableEntry;
class HttpStateMachine;

#include "InkErrno.h"
#define UNKNOWN_INTERNAL_ERROR           (INK_START_ERRNO - 1)

enum ViaStringIndex_t
{
  //
  // General information
  VIA_CLIENT = 0,
  VIA_CLIENT_REQUEST,
  VIA_CACHE,
  VIA_CACHE_RESULT,
  VIA_SERVER,
  VIA_SERVER_RESULT,
  VIA_CACHE_FILL,
  VIA_CACHE_FILL_ACTION,
  VIA_PROXY,
  VIA_PROXY_RESULT,
  VIA_ERROR,
  VIA_ERROR_TYPE,
  //
  // State Machine specific details
  VIA_DETAIL_SEPARATOR,
  VIA_DETAIL_TUNNEL_DESCRIPTOR,
  VIA_DETAIL_TUNNEL,
  VIA_DETAIL_CACHE_DESCRIPTOR,
  VIA_DETAIL_CACHE_TYPE,
  VIA_DETAIL_CACHE_LOOKUP,
  VIA_DETAIL_ICP_DESCRIPTOR,
  VIA_DETAIL_ICP_CONNECT,
  VIA_DETAIL_PP_DESCRIPTOR,
  VIA_DETAIL_PP_CONNECT,
  VIA_DETAIL_SERVER_DESCRIPTOR,
  VIA_DETAIL_SERVER_CONNECT,
  //
  // Total
  MAX_VIA_INDICES
};

enum ViaString_t
{
  // client stuff
  VIA_CLIENT_STRING = 'u',
  VIA_CLIENT_ERROR = 'E',
  VIA_CLIENT_IMS = 'I',
  VIA_CLIENT_NO_CACHE = 'N',
  VIA_CLIENT_COOKIE = 'C',
  VIA_CLIENT_SIMPLE = 'S',
  // cache lookup stuff
  VIA_CACHE_STRING = 'c',
  VIA_CACHE_MISS = 'M',
  VIA_IN_CACHE_NOT_ACCEPTABLE = 'A',
  VIA_IN_CACHE_STALE = 'S',
  VIA_IN_CACHE_FRESH = 'H',
  VIA_IN_RAM_CACHE_FRESH = 'R',
  // server stuff
  VIA_SERVER_STRING = 's',
  VIA_SERVER_ERROR = 'E',
  VIA_SERVER_NOT_MODIFIED = 'N',
  VIA_SERVER_SERVED = 'S',
  // cache fill stuff
  VIA_CACHE_FILL_STRING = 'f',
  VIA_CACHE_DELETED = 'D',
  VIA_CACHE_WRITTEN = 'W',
  VIA_CACHE_UPDATED = 'U',
  // proxy stuff
  VIA_PROXY_STRING = 'p',
  VIA_PROXY_NOT_MODIFIED = 'N',
  VIA_PROXY_SERVED = 'S',
  VIA_PROXY_SERVER_REVALIDATED = 'R',
  // errors
  VIA_ERROR_STRING = 'e',
  VIA_ERROR_NO_ERROR = 'N',
  VIA_ERROR_AUTHORIZATION = 'A',
  VIA_ERROR_CONNECTION = 'C',
  VIA_ERROR_DNS_FAILURE = 'D',
  VIA_ERROR_FORBIDDEN = 'F',
  VIA_ERROR_HEADER_SYNTAX = 'H',
  VIA_ERROR_SERVER = 'S',
  VIA_ERROR_TIMEOUT = 'T',
  VIA_ERROR_CACHE_READ = 'R',
  //
  // Now the detailed stuff
  //
  VIA_DETAIL_SEPARATOR_STRING = ':',
  // tunnelling
  VIA_DETAIL_TUNNEL_DESCRIPTOR_STRING = 't',
  VIA_DETAIL_TUNNEL_HEADER_FIELD = 'F',
  VIA_DETAIL_TUNNEL_METHOD = 'M',
  VIA_DETAIL_TUNNEL_CACHE_OFF = 'O',
  VIA_DETAIL_TUNNEL_URL = 'U',
  VIA_DETAIL_TUNNEL_NO_FORWARD = 'N',
  VIA_DETAIL_TUNNEL_AUTHORIZATION = 'A',
  // cache type
  VIA_DETAIL_CACHE_DESCRIPTOR_STRING = 'c',
  VIA_DETAIL_CACHE = 'C',
  VIA_DETAIL_CLUSTER = 'L',
  VIA_DETAIL_ICP = 'I',
  VIA_DETAIL_PARENT = 'P',
  VIA_DETAIL_SERVER = 'S',
  // result of cache lookup
  VIA_DETAIL_HIT_CONDITIONAL = 'N',
  VIA_DETAIL_HIT_SERVED = 'H',
  VIA_DETAIL_MISS_CONDITIONAL = 'I',
  VIA_DETAIL_MISS_NOT_CACHED = 'M',
  VIA_DETAIL_MISS_EXPIRED = 'S',
  VIA_DETAIL_MISS_CONFIG = 'C',
  VIA_DETAIL_MISS_CLIENT = 'U',
  VIA_DETAIL_MISS_METHOD = 'D',
  VIA_DETAIL_MISS_COOKIE = 'K',
  // result of icp suggested host lookup
  VIA_DETAIL_ICP_DESCRIPTOR_STRING = 'i',
  VIA_DETAIL_ICP_SUCCESS = 'S',
  VIA_DETAIL_ICP_FAILURE = 'F',
  // result of pp suggested host lookup
  VIA_DETAIL_PP_DESCRIPTOR_STRING = 'p',
  VIA_DETAIL_PP_SUCCESS = 'S',
  VIA_DETAIL_PP_FAILURE = 'F',
  // result of server suggested host lookup
  VIA_DETAIL_SERVER_DESCRIPTOR_STRING = 's',
  VIA_DETAIL_SERVER_SUCCESS = 'S',
  VIA_DETAIL_SERVER_FAILURE = 'F'
};

typedef struct _HttpApiInfo
{
  char *parent_proxy_name;
  int parent_proxy_port;
  bool cache_untransformed;
  bool cache_transformed;
  bool logging_enabled;
  bool retry_intercept_failures;

  _HttpApiInfo()
  : parent_proxy_name(NULL),
    parent_proxy_port(-1),
    cache_untransformed(false), cache_transformed(true), logging_enabled(true), retry_intercept_failures(false)
  { }
} HttpApiInfo;

enum
{
  HTTP_UNDEFINED_CL = -1
};

enum UrlRemapMode_t
 {
   URL_REMAP_DEFAULT = 0,      // which is the same as URL_REMAP_ALL
   URL_REMAP_ALL,
   URL_REMAP_FOR_OS
 };

 enum AbortState_t
 {
   ABORT_UNDEFINED = 0,
   DIDNOT_ABORT,
   MAYBE_ABORTED,
   ABORTED
 };

 enum Authentication_t
 {
   AUTHENTICATION_SUCCESS = 0,
   AUTHENTICATION_MUST_REVALIDATE,
   AUTHENTICATION_MUST_PROXY,
   AUTHENTICATION_CACHE_AUTH
 };

 enum CacheAction_t
 {
   CACHE_DO_UNDEFINED = 0,
   CACHE_DO_NO_ACTION,
   CACHE_DO_DELETE,
   CACHE_DO_LOOKUP,
   CACHE_DO_REPLACE,
   CACHE_DO_SERVE,
   CACHE_DO_SERVE_AND_DELETE,
   CACHE_DO_SERVE_AND_UPDATE,
   CACHE_DO_UPDATE,
   CACHE_DO_WRITE,
   CACHE_PREPARE_TO_DELETE,
   CACHE_PREPARE_TO_UPDATE,
   CACHE_PREPARE_TO_WRITE,
   TOTAL_CACHE_ACTION_TYPES
 };

 enum CacheWriteLock_t
 {
   CACHE_WL_INIT,
   CACHE_WL_SUCCESS,
   CACHE_WL_FAIL,
   CACHE_WL_READ_RETRY
 };

 enum ClientTransactionResult_t
 {
   CLIENT_TRANSACTION_RESULT_UNDEFINED,
   CLIENT_TRANSACTION_RESULT_HIT_FRESH,
   CLIENT_TRANSACTION_RESULT_HIT_REVALIDATED,
   CLIENT_TRANSACTION_RESULT_MISS_COLD,
   CLIENT_TRANSACTION_RESULT_MISS_CHANGED,
   CLIENT_TRANSACTION_RESULT_MISS_CLIENT_NO_CACHE,
   CLIENT_TRANSACTION_RESULT_MISS_UNCACHABLE,
   CLIENT_TRANSACTION_RESULT_ERROR_ABORT,
   CLIENT_TRANSACTION_RESULT_ERROR_POSSIBLE_ABORT,
   CLIENT_TRANSACTION_RESULT_ERROR_CONNECT_FAIL,
   CLIENT_TRANSACTION_RESULT_ERROR_OTHER
 };

 enum Freshness_t
 {
   FRESHNESS_FRESH = 0,        // Fresh enough, serve it
   FRESHNESS_WARNING,          // Stale, but client says OK
   FRESHNESS_STALE             // Stale, don't use
 };

 enum HostNameExpansionError_t
 {
   RETRY_EXPANDED_NAME,
   EXPANSION_FAILED,
   EXPANSION_NOT_ALLOWED,
   DNS_ATTEMPTS_EXHAUSTED,
   NO_PARENT_PROXY_EXPANSION,
   TOTAL_HOST_NAME_EXPANSION_TYPES
 };

 enum HttpTransactMagic_t
 {
   HTTP_TRANSACT_MAGIC_ALIVE = 0x00001234,
   HTTP_TRANSACT_MAGIC_DEAD = 0xDEAD1234,
   HTTP_TRANSACT_MAGIC_SEPARATOR = 0x12345678
 };

 enum LookingUp_t
 {
   ORIGIN_SERVER,
   UNDEFINED_LOOKUP,
   ICP_SUGGESTED_HOST,
   PARENT_PROXY,
   INCOMING_ROUTER,
   HOST_NONE
 };

 enum ProxyMode_t
 {
   UNDEFINED_MODE,
   GENERIC_PROXY,
   TUNNELLING_PROXY
 };

 enum RequestError_t
 {
   NO_REQUEST_HEADER_ERROR,
   BAD_HTTP_HEADER_SYNTAX,
   BAD_CONNECT_PORT,
   FAILED_PROXY_AUTHORIZATION,
   METHOD_NOT_SUPPORTED,
   MISSING_HOST_FIELD,
   NO_POST_CONTENT_LENGTH,
   NO_REQUEST_SCHEME,
   NON_EXISTANT_REQUEST_HEADER,
   SCHEME_NOT_SUPPORTED,
   UNACCEPTABLE_TE_REQUIRED,
   TOTAL_REQUEST_ERROR_TYPES
 };

 enum ResponseError_t
 {
   NO_RESPONSE_HEADER_ERROR,
   BOGUS_OR_NO_DATE_IN_RESPONSE,
   CONNECTION_OPEN_FAILED,
   MISSING_REASON_PHRASE,
   MISSING_STATUS_CODE,
   NON_EXISTANT_RESPONSE_HEADER,
   NOT_A_RESPONSE_HEADER,
   STATUS_CODE_SERVER_ERROR,
   TOTAL_RESPONSE_ERROR_TYPES
 };

 // Please do not forget to fix TSServerState (ts/ts.h)
 // in case of any modifications in ServerState_t
 enum ServerState_t
 {
   STATE_UNDEFINED = 0,
   ACTIVE_TIMEOUT,
   BAD_INCOMING_RESPONSE,
   CONNECTION_ALIVE,
   CONNECTION_CLOSED,
   CONNECTION_ERROR,
   INACTIVE_TIMEOUT,
   OPEN_RAW_ERROR,
   PARSE_ERROR,
   TRANSACTION_COMPLETE,
   CONGEST_CONTROL_CONGESTED_ON_F,
   CONGEST_CONTROL_CONGESTED_ON_M
 };

 enum CacheWriteStatus_t
 {
   NO_CACHE_WRITE = 0,
   CACHE_WRITE_LOCK_MISS,
   CACHE_WRITE_IN_PROGRESS,
   CACHE_WRITE_ERROR,
   CACHE_WRITE_COMPLETE
 };

 enum HttpRequestFlavor_t
 {
   REQ_FLAVOR_INTERCEPTED = 0,
   REQ_FLAVOR_REVPROXY = 1,
   REQ_FLAVOR_FWDPROXY = 2,
   REQ_FLAVOR_SCHEDULED_UPDATE = 3
 };

 ////////////
 // source //
 ////////////
 enum Source_t
 {
   SOURCE_NONE = 0,
   SOURCE_HTTP_ORIGIN_SERVER,
   SOURCE_RAW_ORIGIN_SERVER,
   SOURCE_CACHE,
   SOURCE_TRANSFORM,
   SOURCE_INTERNAL             // generated from text buffer
 };

 ////////////////////////////////////////////////
 // HttpTransact fills a StateMachineAction_t  //
 // to tell the state machine what to do next. //
 ////////////////////////////////////////////////
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


 enum Variability_t
 {
   VARIABILITY_NONE = 0,
   VARIABILITY_SOME,
   VARIABILITY_ALL
 };

 struct StatRecord_t
 {
   uint16_t index;
   int64_t  increment;
 };

 enum CacheLookupResult_t
 {
   CACHE_LOOKUP_NONE,
   CACHE_LOOKUP_MISS,
   CACHE_LOOKUP_DOC_BUSY,
   CACHE_LOOKUP_HIT_STALE,
   CACHE_LOOKUP_HIT_WARNING,
   CACHE_LOOKUP_HIT_FRESH,
   CACHE_LOOKUP_SKIPPED
 };

 enum UpdateCachedObject_t
 {
   UPDATE_CACHED_OBJECT_NONE,
   UPDATE_CACHED_OBJECT_PREPARE,
   UPDATE_CACHED_OBJECT_CONTINUE,
   UPDATE_CACHED_OBJECT_ERROR,
   UPDATE_CACHED_OBJECT_SUCCEED,
   UPDATE_CACHED_OBJECT_FAIL
 };

 enum LockUrl_t
 {
   LOCK_URL_FIRST = 0,
   LOCK_URL_SECOND,
   LOCK_URL_ORIGINAL,
   LOCK_URL_DONE,
   LOCK_URL_QUIT
 };

 enum RangeSetup_t
 {
   RANGE_NONE = 0,
   RANGE_REQUESTED,
   RANGE_NOT_SATISFIABLE,
   RANGE_NOT_HANDLED,
   RANGE_NOT_TRANSFORM_REQUESTED
 };

 enum CacheAuth_t
 {
   CACHE_AUTH_NONE = 0,
   //CACHE_AUTH_TRUE,
   CACHE_AUTH_FRESH,
   CACHE_AUTH_STALE,
   CACHE_AUTH_SERVE
 };

#define StatBlockEntries 28

 struct StatBlock
 {
   StatBlock()
   {
     init();
     memset(&stats, 0, sizeof(stats));
   };

   void init()
   {
     next = NULL;
     next_insert = 0;
   };

   StatRecord_t stats[StatBlockEntries];
   StatBlock *next;
   uint16_t next_insert;
 };
typedef struct _CacheDirectives
{
  bool does_client_permit_lookup;
  bool does_client_permit_storing;
  bool does_client_permit_dns_storing;
  bool does_config_permit_lookup;
  bool does_config_permit_storing;
  bool does_server_permit_lookup;
  bool does_server_permit_storing;

  _CacheDirectives()
    : does_client_permit_lookup(true),
      does_client_permit_storing(true),
      does_client_permit_dns_storing(true),
      does_config_permit_lookup(true),
      does_config_permit_storing(true), does_server_permit_lookup(true), does_server_permit_storing(true)
  { }
} CacheDirectives;

typedef struct _CacheLookupInfo
{
  CacheAction_t action;
  CacheAction_t transform_action;

  CacheWriteStatus_t write_status;
  CacheWriteStatus_t transform_write_status;

  URL *lookup_url;
  URL lookup_url_storage;
  URL original_url;
  HttpInfo *object_read;
  HttpInfo *second_object_read;
  HttpInfo object_store;
  HttpInfo transform_store;
  CacheLookupHttpConfig config;
  CacheDirectives directives;
  int open_read_retries;
  int open_write_retries;
  CacheWriteLock_t write_lock_state;
  int lookup_count;
  bool is_ram_cache_hit;

  _CacheLookupInfo()
    : action(CACHE_DO_UNDEFINED),
      transform_action(CACHE_DO_UNDEFINED),
      write_status(NO_CACHE_WRITE),
      transform_write_status(NO_CACHE_WRITE),
      lookup_url(NULL),
      lookup_url_storage(),
      original_url(),
      object_read(NULL),
      second_object_read(NULL),
      object_store(),
      transform_store(),
      config(),
      directives(),
      open_read_retries(0),
      open_write_retries(0),
    write_lock_state(CACHE_WL_INIT),
    lookup_count(0),
    is_ram_cache_hit(false)
  { }
} CacheLookupInfo;

typedef struct _RedirectInfo
{
  bool redirect_in_process;
  URL original_url;
  URL redirect_url;

  _RedirectInfo()
    : redirect_in_process(false), original_url(), redirect_url()
  { }
} RedirectInfo;



typedef struct _CurrentInfo
{
  ProxyMode_t mode;
  LookingUp_t request_to;
  HttpConnectionAttributes *server;
  ink_time_t now;
  ServerState_t state;
  int attempts;

  _CurrentInfo()
    : mode(UNDEFINED_MODE),
      request_to(UNDEFINED_LOOKUP), server(NULL), now(0), state(STATE_UNDEFINED), attempts(1)
  { };
} CurrentInfo;

typedef struct _DNSLookupInfo
{
  int attempts;
  /** Origin server address source selection.

	If config says to use CTA (client target addr) state is
	OS_ADDR_TRY_CLIENT, otherwise it remains the default. If the
	connect fails then we switch to a USE. We go to USE_HOSTDB if
	(1) the HostDB lookup is successful and (2) some address other
	than the CTA is available to try. Otherwise we keep retrying
	on the CTA (USE_CLIENT) up to the max retry value.  In essence
	we try to treat the CTA as if it were another RR value in the
	HostDB record.
   */
  enum {
    OS_ADDR_TRY_DEFAULT, ///< Initial state, use what config says.
    OS_ADDR_TRY_HOSTDB, ///< Try HostDB data.
    OS_ADDR_TRY_CLIENT, ///< Try client target addr.
    OS_ADDR_USE_HOSTDB, ///< Force use of HostDB target address.
    OS_ADDR_USE_CLIENT ///< Use client target addr, no fallback.
  } os_addr_style;

  bool lookup_success;
  char *lookup_name;
  char srv_hostname[MAXDNAME];
  LookingUp_t looking_up;
  bool srv_lookup_success;
  short srv_port;
  //HostDBApplicationInfo srv_app;

  _DNSLookupInfo()
  : attempts(0), os_addr_style(OS_ADDR_TRY_DEFAULT),
      lookup_success(false), lookup_name(NULL), looking_up(UNDEFINED_LOOKUP),
      srv_lookup_success(false), srv_port(0)
  {
    srv_hostname[0] = '\0';
//      srv_app.allotment.application1 = 0;
//      srv_app.allotment.application2 = 0;
  }
} DNSLookupInfo;

typedef struct _HeaderInfo
{
  HttpHeader client_request;
  HttpHeader client_response;
  HttpHeader server_request;
  HttpHeader server_response;
  HttpHeader transform_response;
  HttpHeader cache_response;
  int64_t request_content_length;
  int64_t response_content_length;
  int64_t transform_request_cl;
  int64_t transform_response_cl;
  bool client_req_is_server_style;
  bool trust_response_cl;
  ResponseError_t response_error;
  bool extension_method;
  bool request_body_start;

  _HeaderInfo()
    : client_request(),
      client_response(),
      server_request(),
      server_response(),
      transform_response(),
      cache_response(),
      request_content_length(HTTP_UNDEFINED_CL),
      response_content_length(HTTP_UNDEFINED_CL),
      transform_request_cl(HTTP_UNDEFINED_CL),
      transform_response_cl(HTTP_UNDEFINED_CL),
      client_req_is_server_style(false),
      trust_response_cl(false),
      response_error(0), extension_method(false), request_body_start(false)
  { }
} HeaderInfo;

typedef struct _SquidLogInfo
{
  SquidLogCode log_code;
  SquidHierarchyCode hier_code;
  SquidHitMissCode hit_miss_code;

  _SquidLogInfo()
    : log_code(SQUID_LOG_ERR_UNKNOWN), hier_code(SQUID_HIER_EMPTY),
      hit_miss_code(SQUID_MISS_NONE)
  { }
} SquidLogInfo;



//////////////////////////////////////////////////////////////////////////////
//
//  HttpTransact
//
//  The HttpTransact class is purely used for scoping and should
//  not be instantiated.
//
//////////////////////////////////////////////////////////////////////////////
#define SET_VIA_STRING(I,S) s->via_string[I]=S;
#define GET_VIA_STRING(I) (s->via_string[I])

class HttpTransact
{
public:


  struct State;
  typedef void (*TransactFunc_t) (HttpTransact::State *);



#define HTTP_TRANSACT_STATE_MAX_XBUF_SIZE  (1024*2)     /* max size of plugin exchange buffer */


  static void free_internal_msg_buffer(char *buffer, int64_t size);

  static void HandleBlindTunnel(State* s);
  static void StartRemapRequest(State* s);
  static void RemapRequest(State* s);
  static void EndRemapRequest(State* s);
  static void PerformRemap(State* s);
  static void ModifyRequest(State* s);
  static void HandleRequest(State* s);
  static bool handleIfRedirect(State* s);

  static void StartAccessControl(State* s);
  static void StartAuth(State* s);
  static void HandleRequestAuthorized(State* s);
  static void BadRequest(State* s);
  static void HandleFiltering(State* s);
  static void DecideCacheLookup(State* s);
  static void LookupSkipOpenServer(State* s);

  static void CallOSDNSLookup(State* s);
  static void OSDNSLookup(State* s);
  static void ReDNSRoundRobin(State* s);
  static void PPDNSLookup(State* s);
  static void HandleAuth(State* s);
  static void HandleAuthFailed(State* s);
  static void OriginServerRawOpen(State* s);
  static void HandleCacheOpenRead(State* s);
  static void HandleCacheOpenReadHitFreshness(State* s);
  static void HandleCacheOpenReadHit(State* s);
  static void HandleCacheOpenReadMiss(State* s);
  static void build_response_from_cache(State* s, HTTPWarningCode warning_code);
  static void handle_cache_write_lock(State* s);
  static void HandleICPLookup(State* s);
  static void HandleResponse(State* s);
  static void HandleUpdateCachedObject(State* s);
  static void HandleUpdateCachedObjectContinue(State* s);
  static void HandleStatPage(State* s);
  static void handle_100_continue_response(State* s);
  static void handle_transform_ready(State* s);
  static void handle_transform_cache_write(State* s);
  static void handle_response_from_icp_suggested_host(State* s);
  static void handle_response_from_parent(State* s);
  static void handle_response_from_server(State* s);
  static void delete_server_rr_entry(State* s, int max_retries);
  static void retry_server_connection_not_open(State* s, ServerState_t conn_state, int max_retries);
  static void handle_server_connection_not_open(State* s);
  static void handle_forward_server_connection_open(State* s);
  static void handle_cache_operation_on_forward_server_response(State* s);
  static void handle_no_cache_operation_on_forward_server_response(State* s);
  static void merge_and_update_headers_for_cache_update(State* s);
  static void set_headers_for_cache_write(State* s, HttpInfo* cache_info, HttpHeader* request, HttpHeader* response);
  static void set_header_for_transform(State* s, HttpHeader* base_header);
  static void merge_response_header_with_cached_header(HttpHeader* cached_header, HttpHeader* response_header);
  static void merge_warning_header(HttpHeader* cached_header, HttpHeader* response_header);
  static void SetCacheFreshnessLimit(State* s);
  static void HandleApiErrorJump(State *);

  static void HandleCacheOpenReadPush(State* s, bool read_successful);
  static void HandlePushResponseHdr(State* s);
  static void HandlePushCacheWrite(State* s);
  static void HandlePushTunnelSuccess(State* s);
  static void HandlePushTunnelFailure(State* s);
  static void HandlePushError(State *s, const char *reason);
  static void HandleBadPushRespHdr(State* s);

  // Utility Methods
  static void issue_revalidate(State* s);
  static void get_ka_info_from_host_db(State* s, HttpConnectionAttributes* server_info, HttpConnectionAttributes* client_info,
                                       HostDBInfo* host_db_info);
  static bool service_transaction_in_proxy_only_mode(State* s);
  static void setup_plugin_request_intercept(State* s);
  static void handle_msie_reload_badness(State* s, HttpHeader* client_request);
  static void add_client_ip_to_outgoing_request(State* s, HttpHeader* request);
  static RequestError_t check_request_validity(State* s, HttpHeader* incoming_hdr);
  static ResponseError_t check_response_validity(State* s, HttpHeader* incoming_hdr);
  static bool delete_all_document_alternates_and_return(State* s, bool cache_hit);
  static bool did_forward_server_send_0_9_response(State* s);
//  static bool does_client_request_permit_cached_response(const OverridableHttpConfigParams *p, CacheControlResult *c,
//                                                         HttpHeader *h, char *via_string);
//  static bool does_client_request_permit_dns_caching(CacheControlResult* c, HttpHeader* h);
//  static bool does_client_request_permit_storing(CacheControlResult* c, HttpHeader* h);
  static bool handle_internal_request(State* s, HttpHeader* incoming_hdr);
  static bool handle_trace_and_options_requests(State* s, HttpHeader* incoming_hdr);
  static void bootstrap_state_variables_from_request(State* s, HttpHeader* incoming_request);
  static void initialize_state_variables_for_origin_server(State* s, HttpHeader* incoming_request, bool second_time);
  static void initialize_state_variables_from_request(State* s, HttpHeader* obsolete_incoming_request);
  static void initialize_state_variables_from_response(State* s, HttpHeader* incoming_response);
  static bool is_server_negative_cached(State* s);
  static bool is_cache_response_returnable(State* s);
  static bool is_stale_cache_response_returnable(State* s);
  static bool need_to_revalidate(State* s);
  static bool url_looks_dynamic(URL* url);
  static bool is_request_cache_lookupable(State* s);
  static bool is_request_valid(State* s, HttpHeader* incoming_request);
  static bool is_request_retryable(State* s);
  static bool is_response_cacheable(State* s, HttpHeader* request, HttpHeader* response);
  static bool is_response_valid(State* s, HttpHeader* incoming_response);

  static void process_quick_http_filter(State* s, int method);
  static bool perform_accept_encoding_filtering(State* s);

  static HostNameExpansionError_t try_to_expand_host_name(State* s);

  static bool setup_auth_lookup(State* s);
  static bool will_this_request_self_loop(State* s);
  static bool is_request_likely_cacheable(State* s, HttpHeader* request);

  static void build_request(State* s, HttpHeader* base_request, HttpHeader* outgoing_request, HTTPVersion outgoing_version);
  static void build_response(State* s, HttpHeader* base_response, HttpHeader* outgoing_response, HTTPVersion outgoing_version,
                             HTTPStatus status_code, const char *reason_phrase = NULL);
  static void build_response(State* s, HttpHeader* base_response, HttpHeader* outgoing_response, HTTPVersion outgoing_version);
  static void build_response(State* s, HttpHeader* outgoing_response, HTTPVersion outgoing_version, HTTPStatus status_code,
                             const char *reason_phrase = NULL);

  static void build_response_copy(State* s, HttpHeader* base_response, HttpHeader* outgoing_response, HTTPVersion outgoing_version);
  static void handle_content_length_header(State* s, HttpHeader* header, HttpHeader* base);
  static void change_response_header_because_of_range_request(State* s, HttpHeader* header);

  static void handle_request_keep_alive_headers(State *s, HTTPVersion ver, HttpHeader *heads);
  static void handle_response_keep_alive_headers(State *s, HTTPVersion ver, HttpHeader *heads);
  static int calculate_document_freshness_limit(State *s, HttpHeader *response, time_t response_date, bool *heuristic);
  static int calculate_freshness_fuzz(State *s, int fresh_limit);
  static Freshness_t what_is_document_freshness(State *s, HttpHeader *client_request, HttpHeader *cached_obj_response);
  static Authentication_t AuthenticationNeeded(const OverridableHttpConfigParams *p, HttpHeader *client_request, HttpHeader *obj_response);
  static void handle_parent_died(State* s);
  static void handle_server_died(State* s);
  static void build_error_response(State *s, HTTPStatus status_code, const char *reason_phrase_or_null, const char *error_body_type,
                                   const char *format, ...);
  static void build_redirect_response(State* s);
  static const char *get_error_string(int erno);

  // the stat functions
//  static void update_stat(State* s, int stat, ink_statval_t increment);
  static void update_size_and_time_stats(State* s, ink_hrtime total_time, ink_hrtime user_agent_write_time,
                                         ink_hrtime origin_server_read_time, int user_agent_request_header_size,
                                         int64_t user_agent_request_body_size, int user_agent_response_header_size,
                                         int64_t user_agent_response_body_size, int origin_server_request_header_size,
                                         int64_t origin_server_request_body_size, int origin_server_response_header_size,
                                         int64_t origin_server_response_body_size, int pushed_response_header_size,
                                         int64_t pushed_response_body_size);
  static void histogram_request_document_size(State* s, int64_t size);
  static void histogram_response_document_size(State* s, int64_t size);
  static void user_agent_connection_speed(State* s, ink_hrtime transfer_time, int64_t nbytes);
  static void origin_server_connection_speed(State* s, ink_hrtime transfer_time, int64_t nbytes);
  static void client_result_stat(State* s, ink_hrtime total_time, ink_hrtime request_process_time);
  static void add_new_stat_block(State* s);
  static void delete_warning_value(HttpHeader* to_warn, HTTPWarningCode warning_code);
  static bool is_connection_collapse_checks_success(State* s); //YTS Team, yamsat
};

typedef void (*TransactEntryFunc_t) (HttpTransact::State* s);

inline void
HttpTransact::free_internal_msg_buffer(char *buffer, int64_t size)
{
  ink_assert(buffer);
  if (size >= 0) {
    ioBufAllocator[size].free_void(buffer);
  } else {
    ats_free(buffer);
  }
}

inline const char*
conn_state_enum_to_str(ServerState_t the_state)
{
  switch (the_state) {
  case STATE_UNDEFINED:
    return "STATE_UNDEFINED";
  case ACTIVE_TIMEOUT:
    return "ACTIVE_TIMEOUT";
  case BAD_INCOMING_RESPONSE:
    return "BAD_INCOMING_RESPONSE";
  case CONNECTION_ALIVE:
    return "CONNECTION_ALIVE";
  case CONNECTION_CLOSED:
    return "CONNECTION_CLOSED";
  case CONNECTION_ERROR:
    return "CONNECTION_ERROR";
  case INACTIVE_TIMEOUT:
    return "INACTIVE_TIMEOUT";
  case OPEN_RAW_ERROR:
    return "OPEN_RAW_ERROR";
  case PARSE_ERROR:
    return "PARSE_ERROR";
  case TRANSACTION_COMPLETE:
    return "TRANSACTION_COMPLETE";
  default:
    return "BOGUG_STATE";
  }
}

inline bool
is_response_body_precluded(HTTPStatus status_code, int method)
{

  ////////////////////////////////////////////////////////
  // the spec says about message body the following:    //
  // All responses to the HEAD request method MUST NOT  //
  // include a message-body, even though the presence   //
  // of entity-header fields might lead one to believe  //
  // they do. All 1xx (informational), 204 (no content),//
  // and 304 (not modified) responses MUST NOT include  //
  // a message-body.                                    //
  ////////////////////////////////////////////////////////

  if (((status_code != HTTP_STATUS_OK) &&
       ((status_code == HTTP_STATUS_NOT_MODIFIED) ||
        ((status_code<HTTP_STATUS_OK) && (status_code>= HTTP_STATUS_CONTINUE)) ||
        (status_code == 204))) || (method == HTTP_WKSIDX_HEAD)) {
    return true;
  } else {
    return false;
  }
}

inkcoreapi extern ink_time_t ink_cluster_time(void);

//inline void
//HttpTransact::update_stat(State* s, int stat, ink_statval_t increment)
//{
//  if (s->current_stats->next_insert >= StatBlockEntries) {
//    // This a rare operation and we want to avoid the
//    //   code bloat of inlining it everywhere so
//    //   it's a function call
//    add_new_stat_block(s);
//  }
//
//  uint16_t *next_insert = &s->current_stats->next_insert;
//  s->current_stats->stats[*next_insert].index = stat;
//  s->current_stats->stats[*next_insert].increment = increment;
//  (*next_insert)++;
//}

#endif
