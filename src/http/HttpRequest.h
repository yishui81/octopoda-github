/*
 * HttpRequest.h
 *
 *  Created on: 2014年5月5日
 *      Author: jacky
 */

#ifndef HTTPREQUEST_H_
#define HTTPREQUEST_H_

/*
 *
 */

#define OC_HTTP_MAX_URI_CHANGES           10
#define OC_HTTP_MAX_SUBREQUESTS           200

/* must be 2^n */
#define OC_HTTP_LC_HEADER_LEN             32


#define OC_HTTP_DISCARD_BUFFER_SIZE       4096
#define OC_HTTP_LINGERING_BUFFER_SIZE     4096


#define OC_HTTP_VERSION_9                 9
#define OC_HTTP_VERSION_10                1000
#define OC_HTTP_VERSION_11                1001

#define OC_HTTP_UNKNOWN                   0x0001
#define OC_HTTP_GET                       0x0002
#define OC_HTTP_HEAD                      0x0004
#define OC_HTTP_POST                      0x0008
#define OC_HTTP_PUT                       0x0010
#define OC_HTTP_DELETE                    0x0020
#define OC_HTTP_MKCOL                     0x0040
#define OC_HTTP_COPY                      0x0080
#define OC_HTTP_MOVE                      0x0100
#define OC_HTTP_OPTIONS                   0x0200
#define OC_HTTP_PROPFIND                  0x0400
#define OC_HTTP_PROPPATCH                 0x0800
#define OC_HTTP_LOCK                      0x1000
#define OC_HTTP_UNLOCK                    0x2000
#define OC_HTTP_PATCH                     0x4000
#define OC_HTTP_TRACE                     0x8000

#define OC_HTTP_CONNECTION_CLOSE          1
#define OC_HTTP_CONNECTION_KEEP_ALIVE     2


#define OC_NONE                           1


#define OC_HTTP_PARSE_HEADER_DONE         1

#define OC_HTTP_CLIENT_ERROR              10
#define OC_HTTP_PARSE_INVALID_METHOD      10
#define OC_HTTP_PARSE_INVALID_REQUEST     11
#define OC_HTTP_PARSE_INVALID_09_METHOD   12

#define OC_HTTP_PARSE_INVALID_HEADER      13


/* unused                                  1 */
#define OC_HTTP_SUBREQUEST_IN_MEMORY      2
#define OC_HTTP_SUBREQUEST_WAITED         4
#define OC_HTTP_LOG_UNSAFE                8


#define OC_HTTP_CONTINUE                  100
#define OC_HTTP_SWITCHING_PROTOCOLS       101
#define OC_HTTP_PROCESSING                102

#define OC_HTTP_OK                        200
#define OC_HTTP_CREATED                   201
#define OC_HTTP_ACCEPTED                  202
#define OC_HTTP_NO_CONTENT                204
#define OC_HTTP_PARTIAL_CONTENT           206

#define OC_HTTP_SPECIAL_RESPONSE          300
#define OC_HTTP_MOVED_PERMANENTLY         301
#define OC_HTTP_MOVED_TEMPORARILY         302
#define OC_HTTP_SEE_OTHER                 303
#define OC_HTTP_NOT_MODIFIED              304
#define OC_HTTP_TEMPORARY_REDIRECT        307

#define OC_HTTP_BAD_REQUEST               400
#define OC_HTTP_UNAUTHORIZED              401
#define OC_HTTP_FORBIDDEN                 403
#define OC_HTTP_NOT_FOUND                 404
#define OC_HTTP_NOT_ALLOWED               405
#define OC_HTTP_REQUEST_TIME_OUT          408
#define OC_HTTP_CONFLICT                  409
#define OC_HTTP_LENGTH_REQUIRED           411
#define OC_HTTP_PRECONDITION_FAILED       412
#define OC_HTTP_REQUEST_ENTITY_TOO_LARGE  413
#define OC_HTTP_REQUEST_URI_TOO_LARGE     414
#define OC_HTTP_UNSUPPORTED_MEDIA_TYPE    415
#define OC_HTTP_RANGE_NOT_SATISFIABLE     416


/* Our own HTTP codes */

/* The special code to close connection without any response */
#define OC_HTTP_CLOSE                     444

#define OC_HTTP_NGINX_CODES               494

#define OC_HTTP_REQUEST_HEADER_TOO_LARGE  494

#define OC_HTTPS_CERT_ERROR               495
#define OC_HTTPS_NO_CERT                  496

/*
 * We use the special code for the plain HTTP requests that are sent to
 * HTTPS port to distinguish it from 4XX in an error page redirection
 */
#define OC_HTTP_TO_HTTPS                  497

/* 498 is the canceled code for the requests with invalid host name */

/*
 * HTTP does not define the code for the case when a client closed
 * the connection while we are processing its request so we introduce
 * own code to log such situation when a client has closed the connection
 * before we even try to send the HTTP header to it
 */
#define OC_HTTP_CLIENT_CLOSED_REQUEST     499


#define OC_HTTP_INTERNAL_SERVER_ERROR     500
#define OC_HTTP_NOT_IMPLEMENTED           501
#define OC_HTTP_BAD_GATEWAY               502
#define OC_HTTP_SERVICE_UNAVAILABLE       503
#define OC_HTTP_GATEWAY_TIME_OUT          504
#define OC_HTTP_INSUFFICIENT_STORAGE      507


#define OC_HTTP_LOWLEVEL_BUFFERED         0xf0
#define OC_HTTP_WRITE_BUFFERED            0x10
#define OC_HTTP_GZIP_BUFFERED             0x20
#define OC_HTTP_SSI_BUFFERED              0x01
#define OC_HTTP_SUB_BUFFERED              0x02
#define OC_HTTP_COPY_BUFFERED             0x04


typedef enum {
    OC_HTTP_UNKOWN_PHASE = 0,
    OC_HTTP_POST_READ_PHASE = 1,      /* 读取请求 */
    OC_HTTP_SERVER_REWRITE_PHASE,   /* server级别的rewrite */

    OC_HTTP_FIND_CONFIG_PHASE,      /* 根据uri查找location */
    OC_HTTP_REWRITE_PHASE,          /* localtion级别的rewrite */
    OC_HTTP_POST_REWRITE_PHASE,     /* server、location级别的rewrite都是在这个phase进行收尾工作的 */

    OC_HTTP_PREACCESS_PHASE,        /* 粗粒度的access */

    OC_HTTP_ACCESS_PHASE,           /* 细粒度的access，比如权限验证、存取控制 */
    OC_HTTP_POST_ACCESS_PHASE,      /* 根据上述两个phase得到access code进行操作 */

    OC_HTTP_TRY_FILES_PHASE,        /* 实现try_files指令 */
    OC_HTTP_CONTENT_PHASE,          /* 生成http响应 */

    OC_HTTP_LOG_PHASE               /* log模块 */
} HTTP_REQUEST_PROCESS_PHASE;


class HttpRequest : public UTaskObj{
public:
	HttpRequest(Connector* connector);
	virtual ~HttpRequest();

public:
	int32_t Read();
	int32_t Write();
	int32_t handle_request();

    virtual int handle_open  (const URE_Msg& msg);
    virtual int handle_close (UWorkEnv * orign_uwe, long retcode);

    //timer
    virtual	int handle_timeout( const TimeValue & origts, long time_id, const void * act );

   //message
    virtual int handle_message( const URE_Msg & msg ) { return -1; }
    virtual int handle_failed_message( const URE_Msg & msg ) ;

   //aio
    virtual int handle_read( URE_AioBlock * aib ) { return 0; }
    virtual int handle_write( URE_AioBlock * aib ) { return 0; }

private :
     // HttpHandler* readhandler;

	int32_t  process_http_uri();
	int32_t  process_http_header();
	int32_t  process_http_body();

public:
	HttpParser*  	 	   m_parser;
	Connector*   	  	   m_connector;
	HTTP_REQUEST_PROCESS_PHASE			   m_phase;
	 uint32_t                   m_signature;         /* "HTTP" */

//	    ngx_connection_t                 *connection;

//	    void                            **ctx;
//	    void                            **main_conf;
//	    void                            **srv_conf;
//	    void                            **loc_conf;

	    ngx_http_event_handler_pt         read_event_handler;
	    ngx_http_event_handler_pt         write_event_handler;

	#if (NGX_HTTP_CACHE)
	    ngx_http_cache_t                 *cache;
	#endif

//	    ngx_http_upstream_t              *upstream;
//	    ngx_array_t                      *upstream_states;
//	                                         /* of ngx_http_upstream_state_t */

//	    ngx_pool_t                       *pool;
//	    ngx_buf_t                        *header_in;

	    ngx_http_headers_in_t             headers_in;
	    ngx_http_headers_out_t            headers_out;

	    ngx_http_request_body_t          *request_body;

	    time_t                            lingering_time;
	    time_t                            start_sec;
	    //msec_t                        start_msec;

	    uint32_t                        method;
	    uint32_t                        http_version;

	    oc_str_t                         request_line;
	    oc_str_t                         uri;
	    oc_str_t                         args;
	    oc_str_t                         exten;
	    oc_str_t                         unparsed_uri;

	    oc_str_t                         method_name;
	    oc_str_t                         http_protocol;

//	    ngx_chain_t                      *out;
	    ngx_http_request_t               *main;
	    ngx_http_request_t               *parent;
	    ngx_http_postponed_request_t     *postponed;
	    ngx_http_post_subrequest_t       *post_subrequest;
	    ngx_http_posted_request_t        *posted_requests;

	    int32_t                         phase_handler;
	    ngx_http_handler_pt               content_handler;
	    uint32_t                        access_code;

	    ngx_http_variable_value_t        *variables;

	#if (NGX_PCRE)
	    ngx_uint_t                        ncaptures;
	    int                              *captures;
	    u_char                           *captures_data;
	#endif

	    size_t                            limit_rate;
	    size_t                            limit_rate_after;

	    /* used to learn the Apache compatible response length without a header */
	    size_t                            header_size;

	    off_t                             request_length;

	    uint32_t                        err_status;

	    ngx_http_connection_t            *http_connection;
	#if (NGX_HTTP_SPDY)
	    ngx_http_spdy_stream_t           *spdy_stream;
	#endif

	    ngx_http_log_handler_pt           log_handler;

	    ngx_http_cleanup_t               *cleanup;

	    unsigned                          subrequests:8;
	    unsigned                          count:8;
	    unsigned                          blocked:8;

	    unsigned                          aio:1;

	    unsigned                          http_state:4;

	    /* URI with "/." and on Win32 with "//" */
	    unsigned                          complex_uri:1;

	    /* URI with "%" */
	    unsigned                          quoted_uri:1;

	    /* URI with "+" */
	    unsigned                          plus_in_uri:1;

	    /* URI with " " */
	    unsigned                          space_in_uri:1;

	    unsigned                          invalid_header:1;

	    unsigned                          add_uri_to_alias:1;
	    unsigned                          valid_location:1;
	    unsigned                          valid_unparsed_uri:1;
	    unsigned                          uri_changed:1;
	    unsigned                          uri_changes:4;

	    unsigned                          request_body_in_single_buf:1;
	    unsigned                          request_body_in_file_only:1;
	    unsigned                          request_body_in_persistent_file:1;
	    unsigned                          request_body_in_clean_file:1;
	    unsigned                          request_body_file_group_access:1;
	    unsigned                          request_body_file_log_level:3;

	    unsigned                          subrequest_in_memory:1;
	    unsigned                          waited:1;

	#if (NGX_HTTP_CACHE)
	    unsigned                          cached:1;
	#endif

	#if (NGX_HTTP_GZIP)
	    unsigned                          gzip_tested:1;
	    unsigned                          gzip_ok:1;
	    unsigned                          gzip_vary:1;
	#endif

	    unsigned                          proxy:1;
	    unsigned                          bypass_cache:1;
	    unsigned                          no_cache:1;

	    /*
	     * instead of using the request context data in
	     * ngx_http_limit_conn_module and ngx_http_limit_req_module
	     * we use the single bits in the request structure
	     */
	    unsigned                          limit_conn_set:1;
	    unsigned                          limit_req_set:1;

	#if 0
	    unsigned                          cacheable:1;
	#endif

	    unsigned                          pipeline:1;
	    unsigned                          chunked:1;
	    unsigned                          header_only:1;
	    unsigned                          keepalive:1;
	    unsigned                          lingering_close:1;
	    unsigned                          discard_body:1;
	    unsigned                          internal:1;
	    unsigned                          error_page:1;
	    unsigned                          ignore_content_encoding:1;
	    unsigned                          filter_finalize:1;
	    unsigned                          post_action:1;
	    unsigned                          request_complete:1;
	    unsigned                          request_output:1;
	    unsigned                          header_sent:1;
	    unsigned                          expect_tested:1;
	    unsigned                          root_tested:1;
	    unsigned                          done:1;
	    unsigned                          logged:1;

	    unsigned                          buffered:4;

	    unsigned                          main_filter_need_in_memory:1;
	    unsigned                          filter_need_in_memory:1;
	    unsigned                          filter_need_temporary:1;
	    unsigned                          allow_ranges:1;

	#if (NGX_STAT_STUB)
	    unsigned                          stat_reading:1;
	    unsigned                          stat_writing:1;
	#endif

	    /* used to parse HTTP headers */

	    uint32_t                        state;

	    uint32_t                        header_hash;
	    uint32_t                        lowcase_index;
	    //u_char                           lowcase_header[NGX_HTTP_LC_HEADER_LEN];

	    u_char                           *header_name_start;
	    u_char                           *header_name_end;
	    u_char                           *header_start;
	    u_char                           *header_end;

	    /*
	     * a memory that can be reused after parsing a request line
	     * via ngx_http_ephemeral_t
	     */

	    u_char                           *uri_start;
	    u_char                           *uri_end;
	    u_char                           *uri_ext;
	    u_char                           *args_start;
	    u_char                           *request_start;
	    u_char                           *request_end;
	    u_char                           *method_end;
	    u_char                           *schema_start;
	    u_char                           *schema_end;
	    u_char                           *host_start;
	    u_char                           *host_end;
	    u_char                           *port_start;
	    u_char                           *port_end;

	    unsigned                       http_minor:16;
	    unsigned                        http_major:16;


};

#endif /* HTTPREQUEST_H_ */
