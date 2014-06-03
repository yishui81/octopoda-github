/*
 * HttpVariable.cpp
 *
 *  Created on: 2014年5月5日
 *      Author: jacky
 */

#include "HttpVariable.h"



HttpVariables::HttpVariables()
	: request(NULL)
{

}

HttpVariables::~HttpVariables()
{
	// TODO Auto-generated destructor stub
}


int64_t
HttpVariables::get_http_request_size()
{
	return 0;
}

Str&    HttpVariables::get_http_request_cookies()
{
	return 0;
}

Str&	  HttpVariables::get_http_request_line()
{
	return 0;
}

int32_t HttpVariables::get_http_sent_connection()
{
	return 0;
}

int32_t HttpVariables::get_http_sent_last_modified()
{
	return 0;
}

int32_t HttpVariables::get_http_sent_keep_alive()
{
	return 0;
}

int32_t HttpVariables::get_http_sent_transfer_encoding()
{
	return 0;
}

int32_t HttpVariables::get_http_connection()
{
	return 0;
}

int32_t HttpVariables::get_http_connection_requests()
{
	return 0;
}

int32_t HttpVariables::get_http_hostname()
{
	return 0;
}

int32_t HttpVariables::get_http_version()
{
	return 0;
}

int32_t HttpVariables::get_http_pid()
{
	return 0;
}

int32_t HttpVariables::get_http_time_msec()
{
	return 0;
}

int32_t HttpVariables::get_http_time_iso8601()
{
	return 0;
}

int32_t
HttpVariables::get_http_variable_time_local()
{
	return 0;
}
