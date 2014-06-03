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

#ifndef __HTTP_H__
#define __HTTP_H__


#include <assert.h>
#include "Arena.h"
#include "INK_MD5.h"
#include "MIME.h"
#include "URL.h"
#include "HttpVersion.h"
#include "HttpCacheAlt.h"
#include "HttpHeader.h"
#include "HttpInfo.h"
#include "HttpUrlPrintHack.h"
#include "HttpAcceptor.h"
#include "HttpCacheSM.h"
#include "HttpChunk.h"
#include "HttpClientSession.h"
#include "HttpCompat.h"
#include "HttpParser.h"
#include "HttpRequest.h"
//#include "HttpResponse.h"
#include "HttpStateMachine.h"
#include "HttpStatus.h"
#include "HttpTransact.h"
#include "HttpTransactCache.h"
#include "HttpTransactState.h"
#include "HttpTunnel.h"
#include "HttpTunnelProducer.h"
#include "HttpTunnelConsumer.h"
#include "HttpVariable.h"
#include "HttpConnectionAttributes.h"
#include "HttpTransact.h"
#include "Connector.h"

#include "ink_apidefs.h"

#define MAX_PRODUCERS   2
#define MAX_CONSUMERS   4


#define HTTP_EVENT_NONE							 CON_EVENT_NONE
#define HTTP_EVENT_IMMEDIATE					 CON_EVENT_IMMEDIATE

#define HTTP_EVENT_EVENTS_START            CON_EVENT_EVENTS_START
#define HTTP_EVENT_READ_READY              CON_EVENT_READ_READY


#define	HTTP_EVENT_WRITE_READY             CON_EVENT_WRITE_READY
#define	HTTP_EVENT_READ_COMPLETE           CON_EVENT_READ_COMPLETE
#define	HTTP_EVENT_WRITE_COMPLETE          CON_EVENT_WRITE_COMPLETE

#define	HTTP_EVENT_EOS                     CON_EVENT_EOS
#define	HTTP_EVENT_ERROR                   CON_EVENT_ERROR

#define	HTTP_EVENT_INACTIVITY_TIMEOUT      CON_EVENT_INACTIVITY_TIMEOUT
#define	HTTP_EVENT_ACTIVE_TIMEOUT          CON_EVENT_ACTIVE_TIMEOUT

#define	HTTP_EVENT_OOB_COMPLETE            CON_EVENT_OOB_COMPLETE


#define CON_EVENT_EVENTS_START                    100
#define NET_EVENT_EVENTS_START                    200
#define DISK_EVENT_EVENTS_START                   300
#define CLUSTER_EVENT_EVENTS_START                400
#define HOSTDB_EVENT_EVENTS_START                 500
#define DNS_EVENT_EVENTS_START                    600
#define CONFIG_EVENT_EVENTS_START                 800
#define LOG_EVENT_EVENTS_START	                  900
#define MULTI_CACHE_EVENT_EVENTS_START            1000
#define CACHE_EVENT_EVENTS_START                  1100
#define CACHE_DIRECTORY_EVENT_EVENTS_START        1200
#define CACHE_DB_EVENT_EVENTS_START               1300
#define HTTP_NET_CONNECTION_EVENT_EVENTS_START    1400
#define HTTP_NET_VCONNECTION_EVENT_EVENTS_START   1500
#define GC_EVENT_EVENTS_START                     1600
#define ICP_EVENT_EVENTS_START                    1800
#define TRANSFORM_EVENTS_START                    2000
#define STAT_PAGES_EVENTS_START                   2100
#define HTTP_SESSION_EVENTS_START                 2200
#define HTTP_TUNNEL_EVENTS_START                  2300
#define HTTP_SCH_UPDATE_EVENTS_START              2400
#define NT_ASYNC_CONNECT_EVENT_EVENTS_START       3000
#define NT_ASYNC_IO_EVENT_EVENTS_START            3100
#define RAFT_EVENT_EVENTS_START                   3200
#define SIMPLE_EVENT_EVENTS_START                 3300
#define UPDATE_EVENT_EVENTS_START                 3500
#define LOG_COLLATION_EVENT_EVENTS_START          3800
#define AIO_EVENT_EVENTS_START                    3900
#define BLOCK_CACHE_EVENT_EVENTS_START            4000
#define UTILS_EVENT_EVENTS_START                  5000
#define CONGESTION_EVENT_EVENTS_START             5100
#define INK_API_EVENT_EVENTS_START                60000
#define SRV_EVENT_EVENTS_START	                 62000
#define REMAP_EVENT_EVENTS_START                  63000

#define HTTP_TUNNEL_EVENT_DONE             (HTTP_TUNNEL_EVENTS_START + 1)
#define HTTP_TUNNEL_EVENT_PRECOMPLETE      (HTTP_TUNNEL_EVENTS_START + 2)
#define HTTP_TUNNEL_EVENT_CONSUMER_DETACH  (HTTP_TUNNEL_EVENTS_START + 3)

#define HTTP


enum HTTPStatus
{
  HTTP_STATUS_NONE = 0,

  HTTP_STATUS_CONTINUE = 100,
  HTTP_STATUS_SWITCHING_PROTOCOL = 101,

  HTTP_STATUS_OK = 200,
  HTTP_STATUS_CREATED = 201,
  HTTP_STATUS_ACCEPTED = 202,
  HTTP_STATUS_NON_AUTHORITATIVE_INFORMATION = 203,
  HTTP_STATUS_NO_CONTENT = 204,
  HTTP_STATUS_RESET_CONTENT = 205,
  HTTP_STATUS_PARTIAL_CONTENT = 206,

  HTTP_STATUS_MULTIPLE_CHOICES = 300,
  HTTP_STATUS_MOVED_PERMANENTLY = 301,
  HTTP_STATUS_MOVED_TEMPORARILY = 302,
  HTTP_STATUS_SEE_OTHER = 303,
  HTTP_STATUS_NOT_MODIFIED = 304,
  HTTP_STATUS_USE_PROXY = 305,
  HTTP_STATUS_TEMPORARY_REDIRECT = 307,

  HTTP_STATUS_BAD_REQUEST = 400,
  HTTP_STATUS_UNAUTHORIZED = 401,
  HTTP_STATUS_PAYMENT_REQUIRED = 402,
  HTTP_STATUS_FORBIDDEN = 403,
  HTTP_STATUS_NOT_FOUND = 404,
  HTTP_STATUS_METHOD_NOT_ALLOWED = 405,
  HTTP_STATUS_NOT_ACCEPTABLE = 406,
  HTTP_STATUS_PROXY_AUTHENTICATION_REQUIRED = 407,
  HTTP_STATUS_REQUEST_TIMEOUT = 408,
  HTTP_STATUS_CONFLICT = 409,
  HTTP_STATUS_GONE = 410,
  HTTP_STATUS_LENGTH_REQUIRED = 411,
  HTTP_STATUS_PRECONDITION_FAILED = 412,
  HTTP_STATUS_REQUEST_ENTITY_TOO_LARGE = 413,
  HTTP_STATUS_REQUEST_URI_TOO_LONG = 414,
  HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE = 415,
  HTTP_STATUS_RANGE_NOT_SATISFIABLE = 416,

  HTTP_STATUS_INTERNAL_SERVER_ERROR = 500,
  HTTP_STATUS_NOT_IMPLEMENTED = 501,
  HTTP_STATUS_BAD_GATEWAY = 502,
  HTTP_STATUS_SERVICE_UNAVAILABLE = 503,
  HTTP_STATUS_GATEWAY_TIMEOUT = 504,
  HTTP_STATUS_HTTPVER_NOT_SUPPORTED = 505
};

enum HTTPKeepAlive
{
  HTTP_KEEPALIVE_UNDEFINED = 0,
  HTTP_NO_KEEPALIVE,
  HTTP_KEEPALIVE,
  HTTP_PIPELINE
};

enum HTTPWarningCode
{
  HTTP_WARNING_CODE_NONE = 0,

  HTTP_WARNING_CODE_RESPONSE_STALE = 110,
  HTTP_WARNING_CODE_REVALIDATION_FAILED = 111,
  HTTP_WARNING_CODE_DISCONNECTED_OPERATION = 112,
  HTTP_WARNING_CODE_HERUISTIC_EXPIRATION = 113,
  HTTP_WARNING_CODE_TRANSFORMATION_APPLIED = 114,
  HTTP_WARNING_CODE_MISC_WARNING = 199
};

/* squild log codes */
enum SquidLogCode
{
  SQUID_LOG_EMPTY = '0',
  SQUID_LOG_TCP_HIT = '1',
  SQUID_LOG_TCP_DISK_HIT = '2',
  SQUID_LOG_TCP_MEM_HIT = '.',  // Don't want to change others codes
  SQUID_LOG_TCP_MISS = '3',
  SQUID_LOG_TCP_EXPIRED_MISS = '4',
  SQUID_LOG_TCP_REFRESH_HIT = '5',
  SQUID_LOG_TCP_REF_FAIL_HIT = '6',
  SQUID_LOG_TCP_REFRESH_MISS = '7',
  SQUID_LOG_TCP_CLIENT_REFRESH = '8',
  SQUID_LOG_TCP_IMS_HIT = '9',
  SQUID_LOG_TCP_IMS_MISS = 'a',
  SQUID_LOG_TCP_SWAPFAIL = 'b',
  SQUID_LOG_TCP_DENIED = 'c',
  SQUID_LOG_TCP_WEBFETCH_MISS = 'd',
  SQUID_LOG_TCP_FUTURE_2 = 'f',
  SQUID_LOG_TCP_HIT_REDIRECT = '[',       // standard redirect
  SQUID_LOG_TCP_MISS_REDIRECT = ']',      // standard redirect
  SQUID_LOG_TCP_HIT_X_REDIRECT = '<',     // extended redirect
  SQUID_LOG_TCP_MISS_X_REDIRECT = '>',    // extended redirect
  SQUID_LOG_UDP_HIT = 'g',
  SQUID_LOG_UDP_WEAK_HIT = 'h',
  SQUID_LOG_UDP_HIT_OBJ = 'i',
  SQUID_LOG_UDP_MISS = 'j',
  SQUID_LOG_UDP_DENIED = 'k',
  SQUID_LOG_UDP_INVALID = 'l',
  SQUID_LOG_UDP_RELOADING = 'm',
  SQUID_LOG_UDP_FUTURE_1 = 'n',
  SQUID_LOG_UDP_FUTURE_2 = 'o',
  SQUID_LOG_ERR_READ_TIMEOUT = 'p',
  SQUID_LOG_ERR_LIFETIME_EXP = 'q',
  SQUID_LOG_ERR_NO_CLIENTS_BIG_OBJ = 'r',
  SQUID_LOG_ERR_READ_ERROR = 's',
  SQUID_LOG_ERR_CLIENT_ABORT = 't',
  SQUID_LOG_ERR_CONNECT_FAIL = 'u',
  SQUID_LOG_ERR_INVALID_REQ = 'v',
  SQUID_LOG_ERR_UNSUP_REQ = 'w',
  SQUID_LOG_ERR_INVALID_URL = 'x',
  SQUID_LOG_ERR_NO_FDS = 'y',
  SQUID_LOG_ERR_DNS_FAIL = 'z',
  SQUID_LOG_ERR_NOT_IMPLEMENTED = 'A',
  SQUID_LOG_ERR_CANNOT_FETCH = 'B',
  SQUID_LOG_ERR_NO_RELAY = 'C',
  SQUID_LOG_ERR_DISK_IO = 'D',
  SQUID_LOG_ERR_ZERO_SIZE_OBJECT = 'E',
  SQUID_LOG_ERR_PROXY_DENIED = 'G',
  SQUID_LOG_ERR_WEBFETCH_DETECTED = 'H',
  SQUID_LOG_ERR_FUTURE_1 = 'I',
  SQUID_LOG_ERR_UNKNOWN = 'Z'
};

/* squid hieratchy codes */
enum SquidHierarchyCode
{
  SQUID_HIER_EMPTY = '0',
  SQUID_HIER_NONE = '1',
  SQUID_HIER_DIRECT = '2',
  SQUID_HIER_SIBLING_HIT = '3',
  SQUID_HIER_PARENT_HIT = '4',
  SQUID_HIER_DEFAULT_PARENT = '5',
  SQUID_HIER_SINGLE_PARENT = '6',
  SQUID_HIER_FIRST_UP_PARENT = '7',
  SQUID_HIER_NO_PARENT_DIRECT = '8',
  SQUID_HIER_FIRST_PARENT_MISS = '9',
  SQUID_HIER_LOCAL_IP_DIRECT = 'a',
  SQUID_HIER_FIREWALL_IP_DIRECT = 'b',
  SQUID_HIER_NO_DIRECT_FAIL = 'c',
  SQUID_HIER_SOURCE_FASTEST = 'd',
  SQUID_HIER_SIBLING_UDP_HIT_OBJ = 'e',
  SQUID_HIER_PARENT_UDP_HIT_OBJ = 'f',
  SQUID_HIER_PASSTHROUGH_PARENT = 'g',
  SQUID_HIER_SSL_PARENT_MISS = 'h',
  SQUID_HIER_INVALID_CODE = 'i',
  SQUID_HIER_TIMEOUT_DIRECT = 'j',
  SQUID_HIER_TIMEOUT_SIBLING_HIT = 'k',
  SQUID_HIER_TIMEOUT_PARENT_HIT = 'l',
  SQUID_HIER_TIMEOUT_DEFAULT_PARENT = 'm',
  SQUID_HIER_TIMEOUT_SINGLE_PARENT = 'n',
  SQUID_HIER_TIMEOUT_FIRST_UP_PARENT = 'o',
  SQUID_HIER_TIMEOUT_NO_PARENT_DIRECT = 'p',
  SQUID_HIER_TIMEOUT_FIRST_PARENT_MISS = 'q',
  SQUID_HIER_TIMEOUT_LOCAL_IP_DIRECT = 'r',
  SQUID_HIER_TIMEOUT_FIREWALL_IP_DIRECT = 's',
  SQUID_HIER_TIMEOUT_NO_DIRECT_FAIL = 't',
  SQUID_HIER_TIMEOUT_SOURCE_FASTEST = 'u',
  SQUID_HIER_TIMEOUT_SIBLING_UDP_HIT_OBJ = 'v',
  SQUID_HIER_TIMEOUT_PARENT_UDP_HIT_OBJ = 'w',
  SQUID_HIER_TIMEOUT_PASSTHROUGH_PARENT = 'x',
  SQUID_HIER_TIMEOUT_TIMEOUT_SSL_PARENT_MISS = 'y',
  SQUID_HIER_INVALID_ASSIGNED_CODE = 'z'
};

/* squid hit/miss codes */
enum SquidHitMissCode
{
  SQUID_HIT_RESERVED = '0',
  SQUID_HIT_LEVEL_1 = '1',
  SQUID_HIT_LEVEL_2 = '2',
  SQUID_HIT_LEVEL_3 = '3',
  SQUID_HIT_LEVEL_4 = '4',
  SQUID_HIT_LEVEL_5 = '5',
  SQUID_HIT_LEVEL_6 = '6',
  SQUID_HIT_LEVEL_7 = '7',
  SQUID_HIT_LEVEL_8 = '8',
  SQUID_HIT_LEVEl_9 = '9',
  SQUID_MISS_NONE = '1',
  SQUID_MISS_ICP_AUTH = '2',
  SQUID_MISS_HTTP_NON_CACHE = '3',
  SQUID_MISS_ICP_STOPLIST = '4',
  SQUID_MISS_HTTP_NO_DLE = '5',
  SQUID_MISS_HTTP_NO_LE = '6',
  SQUID_MISS_HTTP_CONTENT = '7',
  SQUID_MISS_PRAGMA_NOCACHE = '8',
  SQUID_MISS_PASS = '9',
  SQUID_MISS_PRE_EXPIRED = 'a',
  SQUID_MISS_ERROR = 'b',
  SQUID_MISS_CACHE_BYPASS = 'c',
  SQUID_HIT_MISS_INVALID_ASSIGNED_CODE = 'z'
};

/// Type of transport on the connection.
enum TransportType {
  TRANSPORT_DEFAULT = 0, ///< Default (normal HTTP).
  TRANSPORT_COMPRESSED, ///< Compressed HTTP.
  TRANSPORT_BLIND_TUNNEL, ///< Blind tunnel (no processing).
  TRANSPORT_SSL, ///< SSL connection.
  TRANSPORT_PLUGIN /// < Protocol plugin connection
};

enum HTTPType
{
  HTTP_TYPE_UNKNOWN,
  HTTP_TYPE_REQUEST,
  HTTP_TYPE_RESPONSE
};


struct HTTPValAccept
{
  char *type;
  char *subtype;
  double qvalue;
};


struct HTTPValAcceptCharset
{
  char *charset;
  double qvalue;
};


struct HTTPValAcceptEncoding
{
  char *encoding;
  double qvalue;
};


struct HTTPValAcceptLanguage
{
  char *language;
  double qvalue;
};


struct HTTPValFieldList
{
  char *name;
  HTTPValFieldList *next;
};


struct HTTPValCacheControl
{
  const char *directive;

  union
  {
    int delta_seconds;
    HTTPValFieldList *field_names;
  } u;
};


struct HTTPValRange
{
  int start;
  int end;
  HTTPValRange *next;
};


struct HTTPValTE
{
  char *encoding;
  double qvalue;
};


struct HTTPParser
{
	  bool m_parsing_http;
	  MIMEParser m_mime_parser;
};


extern const char *HTTP_METHOD_CONNECT;
extern const char *HTTP_METHOD_DELETE;
extern const char *HTTP_METHOD_GET;
extern const char *HTTP_METHOD_HEAD;
extern const char *HTTP_METHOD_ICP_QUERY;
extern const char *HTTP_METHOD_OPTIONS;
extern const char *HTTP_METHOD_POST;
extern const char *HTTP_METHOD_PURGE;
extern const char *HTTP_METHOD_PUT;
extern const char *HTTP_METHOD_TRACE;
extern const char *HTTP_METHOD_PUSH;

extern int HTTP_WKSIDX_CONNECT;
extern int HTTP_WKSIDX_DELETE;
extern int HTTP_WKSIDX_GET;
extern int HTTP_WKSIDX_HEAD;
extern int HTTP_WKSIDX_ICP_QUERY;
extern int HTTP_WKSIDX_OPTIONS;
extern int HTTP_WKSIDX_POST;
extern int HTTP_WKSIDX_PURGE;
extern int HTTP_WKSIDX_PUT;
extern int HTTP_WKSIDX_TRACE;
extern int HTTP_WKSIDX_PUSH;
extern int HTTP_WKSIDX_METHODS_CNT;


extern int HTTP_LEN_CONNECT;
extern int HTTP_LEN_DELETE;
extern int HTTP_LEN_GET;
extern int HTTP_LEN_HEAD;
extern int HTTP_LEN_ICP_QUERY;
extern int HTTP_LEN_OPTIONS;
extern int HTTP_LEN_POST;
extern int HTTP_LEN_PURGE;
extern int HTTP_LEN_PUT;
extern int HTTP_LEN_TRACE;
extern int HTTP_LEN_PUSH;

extern const char *HTTP_VALUE_BYTES;
extern const char *HTTP_VALUE_CHUNKED;
extern const char *HTTP_VALUE_CLOSE;
extern const char *HTTP_VALUE_COMPRESS;
extern const char *HTTP_VALUE_DEFLATE;
extern const char *HTTP_VALUE_GZIP;
extern const char *HTTP_VALUE_IDENTITY;
extern const char *HTTP_VALUE_KEEP_ALIVE;
extern const char *HTTP_VALUE_MAX_AGE;
extern const char *HTTP_VALUE_MAX_STALE;
extern const char *HTTP_VALUE_MIN_FRESH;
extern const char *HTTP_VALUE_MUST_REVALIDATE;
extern const char *HTTP_VALUE_NONE;
extern const char *HTTP_VALUE_NO_CACHE;
extern const char *HTTP_VALUE_NO_STORE;
extern const char *HTTP_VALUE_NO_TRANSFORM;
extern const char *HTTP_VALUE_ONLY_IF_CACHED;
extern const char *HTTP_VALUE_PRIVATE;
extern const char *HTTP_VALUE_PROXY_REVALIDATE;
extern const char *HTTP_VALUE_PUBLIC;
extern const char *HTTP_VALUE_S_MAXAGE;
extern const char *HTTP_VALUE_NEED_REVALIDATE_ONCE;

extern int HTTP_LEN_BYTES;
extern int HTTP_LEN_CHUNKED;
extern int HTTP_LEN_CLOSE;
extern int HTTP_LEN_COMPRESS;
extern int HTTP_LEN_DEFLATE;
extern int HTTP_LEN_GZIP;
extern int HTTP_LEN_IDENTITY;
extern int HTTP_LEN_KEEP_ALIVE;
extern int HTTP_LEN_MAX_AGE;
extern int HTTP_LEN_MAX_STALE;
extern int HTTP_LEN_MIN_FRESH;
extern int HTTP_LEN_MUST_REVALIDATE;
extern int HTTP_LEN_NONE;
extern int HTTP_LEN_NO_CACHE;
extern int HTTP_LEN_NO_STORE;
extern int HTTP_LEN_NO_TRANSFORM;
extern int HTTP_LEN_ONLY_IF_CACHED;
extern int HTTP_LEN_PRIVATE;
extern int HTTP_LEN_PROXY_REVALIDATE;
extern int HTTP_LEN_PUBLIC;
extern int HTTP_LEN_S_MAXAGE;
extern int HTTP_LEN_NEED_REVALIDATE_ONCE;

/* Private */
void http_hdr_adjust(HttpHeaderImpl *hdrp, int32_t offset, int32_t length, int32_t delta);

/* Public */
void http_init();

inkcoreapi HttpHeaderImpl *http_hdr_create(HdrHeap *heap, HTTPType polarity);
void http_hdr_init(HdrHeap *heap, HttpHeaderImpl *hh, HTTPType polarity);
HttpHeaderImpl *http_hdr_clone(HttpHeaderImpl *s_hh, HdrHeap *s_heap, HdrHeap *d_heap);
void http_hdr_copy_onto(HttpHeaderImpl *s_hh, HdrHeap *s_heap, HttpHeaderImpl *d_hh, HdrHeap *d_heap, bool inherit_strs);

inkcoreapi int http_hdr_print(HdrHeap *heap, HttpHeaderImpl *hh, char *buf, int bufsize, int *bufindex, int *dumpoffset);

void http_hdr_describe(HdrHeapObjImpl *obj, bool recurse = true);

int http_hdr_length_get(HttpHeaderImpl *hh);
// HTTPType               http_hdr_type_get (HttpHeaderImpl *hh);

// int32_t                  http_hdr_version_get (HttpHeaderImpl *hh);
inkcoreapi void http_hdr_version_set(HttpHeaderImpl *hh, int32_t ver);

const char *http_hdr_method_get(HttpHeaderImpl *hh, int *length);
inkcoreapi void http_hdr_method_set(HdrHeap *heap, HttpHeaderImpl *hh,
                                    const char *method, int16_t method_wks_idx, int method_length, bool must_copy);

void http_hdr_url_set(HdrHeap *heap, HttpHeaderImpl *hh, URLImpl *url);

// HTTPStatus             http_hdr_status_get (HttpHeaderImpl *hh);
void http_hdr_status_set(HttpHeaderImpl *hh, HTTPStatus status);
const char *http_hdr_reason_get(HttpHeaderImpl *hh, int *length);
void http_hdr_reason_set(HdrHeap *heap, HttpHeaderImpl *hh, const char *value, int length, bool must_copy);
const char *http_hdr_reason_lookup(unsigned status);

void http_parser_init(HTTPParser *parser);
void http_parser_clear(HTTPParser *parser);
MIMEParseResult http_parser_parse_req(HTTPParser *parser, HdrHeap *heap,
                                      HttpHeaderImpl *hh, const char **start,
                                      const char *end, bool must_copy_strings, bool eof);
MIMEParseResult http_parser_parse_resp(HTTPParser *parser, HdrHeap *heap,
                                       HttpHeaderImpl *hh, const char **start,
                                       const char *end, bool must_copy_strings, bool eof);
HTTPStatus http_parse_status(const char *start, const char *end);
int32_t http_parse_version(const char *start, const char *end);


HTTPValTE *http_parse_te(const char *buf, int len, Arena *arena);


#endif /* __HTTP_H__ */
