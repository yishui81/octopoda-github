/*
 * HttpVariable.h
 *
 *  Created on: 2014年5月5日
 *      Author: jacky
 */

#ifndef HTTPVARIABLE_H_
#define HTTPVARIABLE_H_
#include "Http.h"
/*
 *
 */

#define HTTP_VAR_CHANGEABLE   1
#define HTTP_VAR_NOCACHEABLE  2
#define HTTP_VAR_INDEXED      4
#define HTTP_VAR_NOHASH       8


extern http_variable_value_t  http_variable_null_value;
extern http_variable_value_t  http_variable_true_value;

class HttpVariables {

public:
	HttpVariables();
	virtual ~HttpVariables();

public:

	int64_t get_http_request_size();

	Str&    get_http_request_cookies();

	Str&	  get_http_request_line();

	int32_t get_http_sent_connection();

	int32_t get_http_sent_last_modified();

	int32_t get_http_sent_keep_alive();

	int32_t get_http_sent_transfer_encoding();

	int32_t get_http_connection();

	int32_t get_http_connection_requests();

	int32_t get_http_hostname();

	int32_t get_http_version();

	int32_t get_http_pid();

	int32_t get_http_time_msec();

	int32_t get_http_time_iso8601();

	int32_t get_http_variable_time_local();

private:
	HttpRequest* request;
};

#endif /* HTTPVARIABLE_H_ */
