/*
 * HttpConnectionAttributes.cpp
 *
 *  Created on: 2014年5月27日
 *      Author: jacky
 */

#include "HttpConnectionAttributes.h"

HttpConnectionAttributes::HttpConnectionAttributes()
: http_version(),
  keep_alive(HTTP_KEEPALIVE_UNDEFINED),
  receive_chunked_response(false),
  pipeline_possible(false),
  proxy_connect_hdr(false),
  connect_result(0),
  name(NULL),
  dns_round_robin(false),
  transfer_encoding(NO_TRANSFER_ENCODING),
  port(0),
	// state(STATE_UNDEFINED),
	// abort(ABORT_UNDEFINED),
  port_attribute(TRANSPORT_DEFAULT),
  is_transparent(false)
{
	memset(&addr, 0, sizeof(addr));
}

HttpConnectionAttributes::~HttpConnectionAttributes() {
	// TODO Auto-generated destructor stub
}

