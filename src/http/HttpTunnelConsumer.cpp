/*
 * HttpTunnelConsumer1.cpp
 *
 *  Created on: 2014年5月13日
 *      Author: jacky
 */

#include "HttpTunnelConsumer.h"
#include <sys/types.h>

HttpTunnelConsumer::HttpTunnelConsumer()
		: link(),
		producer(NULL),
		self_producer(NULL),
		vc_type(HT_HTTP_CLIENT),
		vc(NULL),
		buffer_reader(NULL),
		vc_handler(NULL),
		write_vio(NULL),
		skip_bytes(0),
		bytes_written(0),
		handler_state(0),
		alive(false),
		write_success(false),
		name(NULL)
{

}



