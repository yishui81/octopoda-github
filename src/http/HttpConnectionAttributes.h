/*
 * HttpConnectionAttributes.h
 *
 *  Created on: 2014年5月27日
 *      Author: jacky
 */

#ifndef HTTPCONNECTIONATTRIBUTES_H_
#define HTTPCONNECTIONATTRIBUTES_H_
#include "SockBase.h"
#include "Http.h"

class HttpConnectionAttributes
{

public:
	 HttpConnectionAttributes();
	 virtual ~HttpConnectionAttributes();

public:

	inline bool had_connect_fail() const
	{
		return 0 != connect_result;
	}

	inline void clear_connect_fail()
	{
		connect_result = 0;
	}

	inline void set_connect_fail(int e)
	{
		connect_result = e;
	}

private:

	HTTPVersion http_version;
	HTTPKeepAlive keep_alive;

	// The following variable is true if the client expects to
	// received a chunked response.
	bool receive_chunked_response;
	bool pipeline_possible;
	bool proxy_connect_hdr;
	/// @c errno from the most recent attempt to connect.
	/// zero means no failure (not attempted, succeeded).
	int connect_result;
	char *name;
	bool dns_round_robin;
	TransferEncoding_t transfer_encoding;

	SockAddr addr;    // replaces 'ip' field

	// port to connect to, except for client
	// connection where it is port on proxy
	// that client connected to.
	// This field is managed separately from the port
	// part of 'addr' above as in various cases the two
	// are set/manipulated independently and things are
	// clearer this way.
	uint16_t port; // host order.
	//ServerState_t state;
	//AbortState_t abort;
	 TransportType port_attribute;

	/// @c true if the connection is transparent.
	bool is_transparent;
};
#endif /* HTTPCONNECTIONATTRIBUTES_H_ */
