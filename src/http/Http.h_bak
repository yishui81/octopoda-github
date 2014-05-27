/*
 * Http.h
 *
 *  Created on: 2014年5月5日
 *      Author: jacky
 */

#ifndef HTTP_H_
#define HTTP_H_
#include <ngx_http_variables.h>
#include <ngx_http_config.h>
#include <ngx_http_request.h>
#include <ngx_http_script.h>
#include <ngx_http_upstream.h>
#include <ngx_http_upstream_round_robin.h>
#include <ngx_http_busy_lock.h>
#include <ngx_http_core_module.h>

#include "HttpChunk.h"
#include "HttpStatus.h"

//struct ngx_http_chunked_s {
//    ngx_uint_t           state;
//    off_t                size;
//    off_t                length;
//};

//typedef struct HttpChunk {
//    ngx_uint_t           state;
//    off_t                size;
//    off_t                length;
//}HttpChunk;

class Http{

public:
	void http_add_location(ngx_conf_t *cf, ngx_queue_t **locations,
		    ngx_http_core_loc_conf_t *clcf);
	void http_add_listener();
	void http_init_connection(Connection *c);
	void http_close_connection(Connection *c);
	int   http_parse_request_line(ngx_http_request_t *r, ngx_buf_t *b);
	int    http_parse_uri(ngx_http_request_t *r);
	int    http_parse_complex_uri(ngx_http_request_t *r,  ngx_uint_t merge_slashes);
	int    http_parse_status_line(ngx_http_request_t *r, ngx_buf_t *b,
	    ngx_http_status_t *status);
	int    http_parse_unsafe_uri(ngx_http_request_t *r, ngx_str_t *uri,
	    ngx_str_t *args, ngx_uint_t *flags);
	int    http_parse_header_line(ngx_http_request_t *r, ngx_buf_t *b,
	    ngx_uint_t allow_underscores);
	int    http_parse_multi_header_lines(ngx_array_t *headers,
	    ngx_str_t *name, ngx_str_t *value);
	int    http_arg(ngx_http_request_t *r, u_char *name, size_t len,
	    ngx_str_t *value);
	void http_split_args(ngx_http_request_t *r, ngx_str_t *uri,
	    ngx_str_t *args);
	int    http_parse_chunked(ngx_http_request_t *r, ngx_buf_t *b,
	    ngx_http_chunked_t *ctx);


	ngx_http_request_t *http_create_request(ngx_connection_t *c);
	int  http_process_request_uri(ngx_http_request_t *r);
	int  http_process_request_header(ngx_http_request_t *r);
	void 	http_process_request(ngx_http_request_t *r);
	void 	http_update_location_config(ngx_http_request_t *r);
	void 	http_handler(ngx_http_request_t *r);
	void 	http_run_posted_requests(ngx_connection_t *c);
	int   http_post_request(ngx_http_request_t *r,
	    ngx_http_posted_request_t *pr);
	void http_finalize_request(ngx_http_request_t *r, ngx_int_t rc);
	void http_free_request(ngx_http_request_t *r, ngx_int_t rc);

	void http_empty_handler(ngx_event_t *wev);
	void http_request_empty_handler(ngx_http_request_t *r);

	gx_int_t ngx_http_send_special(ngx_http_request_t *r, ngx_uint_t flags);


	ngx_int_t ngx_http_read_client_request_body(ngx_http_request_t *r,
	    ngx_http_client_body_handler_pt post_handler);

	ngx_int_t ngx_http_send_header(ngx_http_request_t *r);
	ngx_int_t ngx_http_special_response_handler(ngx_http_request_t *r,
	    ngx_int_t error);
	ngx_int_t ngx_http_filter_finalize_request(ngx_http_request_t *r,
	    ngx_module_t *m, ngx_int_t error);
	void ngx_http_clean_header(ngx_http_request_t *r);


	time_t ngx_http_parse_time(u_char *value, size_t len);
	size_t ngx_http_get_time(char *buf, time_t t);



	ngx_int_t ngx_http_discard_request_body(ngx_http_request_t *r);
	void ngx_http_discarded_request_body_handler(ngx_http_request_t *r);
	void ngx_http_block_reading(ngx_http_request_t *r);
	void ngx_http_test_reading(ngx_http_request_t *r);


	char *ngx_http_types_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
	char *ngx_http_merge_types(ngx_conf_t *cf, ngx_array_t **keys,
	    ngx_hash_t *types_hash, ngx_array_t **prev_keys,
	    ngx_hash_t *prev_types_hash, ngx_str_t *default_types);
	ngx_int_t ngx_http_set_default_types(ngx_conf_t *cf, ngx_array_t **types,
	    ngx_str_t *default_type);

	ngx_int_t ngx_http_send_special(ngx_http_request_t *r, ngx_uint_t flags);


	ngx_int_t ngx_http_read_client_request_body(ngx_http_request_t *r,
	    ngx_http_client_body_handler_pt post_handler);

	ngx_int_t ngx_http_send_header(ngx_http_request_t *r);
	ngx_int_t ngx_http_special_response_handler(ngx_http_request_t *r,
	    ngx_int_t error);
	ngx_int_t ngx_http_filter_finalize_request(ngx_http_request_t *r,
	    ngx_module_t *m, ngx_int_t error);
	void ngx_http_clean_header(ngx_http_request_t *r);


	time_t ngx_http_parse_time(u_char *value, size_t len);
	size_t ngx_http_get_time(char *buf, time_t t);



	ngx_int_t ngx_http_discard_request_body(ngx_http_request_t *r);
	void ngx_http_discarded_request_body_handler(ngx_http_request_t *r);
	void ngx_http_block_reading(ngx_http_request_t *r);
	void ngx_http_test_reading(ngx_http_request_t *r);


	char *ngx_http_types_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
	char *ngx_http_merge_types(ngx_conf_t *cf, ngx_array_t **keys,
	    ngx_hash_t *types_hash, ngx_array_t **prev_keys,
	    ngx_hash_t *prev_types_hash, ngx_str_t *default_types);
	ngx_int_t ngx_http_set_default_types(ngx_conf_t *cf, ngx_array_t **types,
	    ngx_str_t *default_type);



private:
	HttpRequest request;
	HttpChuck  chunked;
	HttpUpstream upstream;
	HttpCache   cache;
	HttpStatusCode status;


};
typedef struct ngx_http_request_s     ngx_http_request_t;
typedef struct ngx_http_upstream_s    ngx_http_upstream_t;
typedef struct ngx_http_cache_s       ngx_http_cache_t;
typedef struct ngx_http_file_cache_s  ngx_http_file_cache_t;
typedef struct ngx_http_log_ctx_s     ngx_http_log_ctx_t;
typedef struct ngx_http_chunked_s     ngx_http_chunked_t;

#if (NGX_HTTP_SPDY)
typedef struct ngx_http_spdy_stream_s  ngx_http_spdy_stream_t;
#endif

typedef ngx_int_t (*ngx_http_header_handler_pt)(ngx_http_request_t *r,
    ngx_table_elt_t *h, ngx_uint_t offset);
typedef u_char *(*ngx_http_log_handler_pt)(ngx_http_request_t *r,
    ngx_http_request_t *sr, u_char *buf, size_t len);




#if (NGX_HTTP_SPDY)
#include <ngx_http_spdy.h>
#endif
#if (NGX_HTTP_CACHE)
#include <ngx_http_cache.h>
#endif
#if (NGX_HTTP_SSI)
#include <ngx_http_ssi_filter_module.h>
#endif
#if (NGX_HTTP_SSL)
#include <ngx_http_ssl_module.h>
#endif


struct ngx_http_log_ctx_s {
    ngx_connection_t    *connection;
    ngx_http_request_t  *request;
    ngx_http_request_t  *current_request;
};


struct ngx_http_chunked_s {
    ngx_uint_t           state;
    off_t                size;
    off_t                length;
};





#define ngx_http_get_module_ctx(r, module)  (r)->ctx[module.ctx_index]
#define ngx_http_set_ctx(r, c, module)      r->ctx[module.ctx_index] = c;


ngx_int_t ngx_http_add_location(ngx_conf_t *cf, ngx_queue_t **locations,
    ngx_http_core_loc_conf_t *clcf);
ngx_int_t ngx_http_add_listen(ngx_conf_t *cf, ngx_http_core_srv_conf_t *cscf,
    ngx_http_listen_opt_t *lsopt);


void ngx_http_init_connection(ngx_connection_t *c);
void ngx_http_close_connection(ngx_connection_t *c);

#if (NGX_HTTP_SSL && defined SSL_CTRL_SET_TLSEXT_HOSTNAME)
int ngx_http_ssl_servername(ngx_ssl_conn_t *ssl_conn, int *ad, void *arg);
#endif

ngx_int_t ngx_http_parse_request_line(ngx_http_request_t *r, ngx_buf_t *b);
ngx_int_t ngx_http_parse_uri(ngx_http_request_t *r);
ngx_int_t ngx_http_parse_complex_uri(ngx_http_request_t *r,
    ngx_uint_t merge_slashes);
ngx_int_t ngx_http_parse_status_line(ngx_http_request_t *r, ngx_buf_t *b,
    ngx_http_status_t *status);
ngx_int_t ngx_http_parse_unsafe_uri(ngx_http_request_t *r, ngx_str_t *uri,
    ngx_str_t *args, ngx_uint_t *flags);
ngx_int_t ngx_http_parse_header_line(ngx_http_request_t *r, ngx_buf_t *b,
    ngx_uint_t allow_underscores);
ngx_int_t ngx_http_parse_multi_header_lines(ngx_array_t *headers,
    ngx_str_t *name, ngx_str_t *value);
ngx_int_t ngx_http_arg(ngx_http_request_t *r, u_char *name, size_t len,
    ngx_str_t *value);
void ngx_http_split_args(ngx_http_request_t *r, ngx_str_t *uri,
    ngx_str_t *args);
ngx_int_t ngx_http_parse_chunked(ngx_http_request_t *r, ngx_buf_t *b,
    ngx_http_chunked_t *ctx);


ngx_http_request_t *ngx_http_create_request(ngx_connection_t *c);
ngx_int_t ngx_http_process_request_uri(ngx_http_request_t *r);
ngx_int_t ngx_http_process_request_header(ngx_http_request_t *r);
void ngx_http_process_request(ngx_http_request_t *r);
void ngx_http_update_location_config(ngx_http_request_t *r);
void ngx_http_handler(ngx_http_request_t *r);
void ngx_http_run_posted_requests(ngx_connection_t *c);
ngx_int_t ngx_http_post_request(ngx_http_request_t *r,
    ngx_http_posted_request_t *pr);
void ngx_http_finalize_request(ngx_http_request_t *r, ngx_int_t rc);
void ngx_http_free_request(ngx_http_request_t *r, ngx_int_t rc);

void ngx_http_empty_handler(ngx_event_t *wev);
void ngx_http_request_empty_handler(ngx_http_request_t *r);


#define ngx_http_ephemeral(r)  (void *) (&r->uri_start)


#define NGX_HTTP_LAST   1
#define NGX_HTTP_FLUSH  2



#if (NGX_HTTP_DEGRADATION)
ngx_uint_t  ngx_http_degraded(ngx_http_request_t *);
#endif


extern ngx_module_t  ngx_http_module;

extern ngx_str_t  ngx_http_html_default_types[];


extern ngx_http_output_header_filter_pt  ngx_http_top_header_filter;
extern ngx_http_output_body_filter_pt    ngx_http_top_body_filter;


#endif /* HTTP_H_ */
