/*
 * HttpTunnelConsumer1.h
 *
 *  Created on: 2014年5月13日
 *      Author: jacky
 */

#ifndef HTTPTUNNELCONSUMER1_H_
#define HTTPTUNNELCONSUMER1_H_

/*
 *
 */
class HttpTunnelConsumer
{
public:
	HttpTunnelConsumer();
	virtual ~HttpTunnelConsumer();

public:
	//LINK(HttpTunnelConsumer, link);
	HttpTunnelProducer *producer;
	HttpTunnelProducer *self_producer;

	HttpTunnelType_t vc_type;
	Connector *vc;
	IOBufferReader *buffer_reader;
	HttpConsumerHandler vc_handler;
	VIO *write_vio;

	int64_t skip_bytes;               // bytes to skip at beginning of stream
	int64_t bytes_written;            // total bytes written to the vc
	int handler_state;              // state used the handlers

	bool alive;
	bool write_success;
	const char *name;

	/** Check if this consumer is downstream from @a vc.
	  @return @c true if any producer in the tunnel eventually feeds
	  data to this consumer.
	*/
	bool is_downstream_from(Connector* vc);
	/** Check if this is a sink (final data destination).
	  @return @c true if data exits the ATS process at this consumer.
	*/
	bool is_sink() const;
};

#endif /* HTTPTUNNELCONSUMER1_H_ */
