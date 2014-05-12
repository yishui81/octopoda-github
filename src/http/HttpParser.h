/*
 * HttpParse.h
 *
 *  Created on: 2014年5月5日
 *      Author: jacky
 */

#ifndef HTTPPARSE_H_
#define HTTPPARSE_H_

/*
 *
 */

#include "HttpRequest.h"
#include <oc_core.h>
#include <StringUtils.h>
#include <vector>

using namespace std;
class HttpParser{
public:
	HttpParser(HttpRequest* request);
	virtual ~HttpParser();
public:
	int32_t http_arg(HttpRequest *r, u_char *name, size_t len, oc_str_t *value);
	int32_t http_parse_chunked(HttpRequest *r, oc_buf_t *b,  HttpChunkContext *ctx);
	int32_t http_parse_complex_uri(HttpRequest *r, uint32_t  merge_slashes);
	int32_t http_parse_header_line(HttpRequest *r, oc_buf_t *b, int allow_underscores);
	int32_t http_parse_multi_header_lines(vector<char*> *headers, oc_str_t *name,     oc_str_t *value);
	int32_t http_parse_status_line(HttpRequest *r, oc_buf_t *b, HttpStatus *status);
	int32_t http_parse_unsafe_uri(HttpRequest *r, oc_str_t *uri,  oc_str_t *args, uint32_t *flags);
	int32_t http_parse_uri(HttpRequest *r);
	void    http_split_args(HttpRequest *r, oc_str_t *uri, oc_str_t *args);
	int32_t parseRequstLine(HttpRequest *r, std::string& buffer);

private:
	HttpRequest*  m_request;
};

#endif /* HTTPPARSE_H_ */
