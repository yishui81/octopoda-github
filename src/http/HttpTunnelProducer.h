/*
 * HttpTunnelProducer.h
 *
 *  Created on: 2014年5月13日
 *      Author: jacky
 */

#ifndef HTTPTUNNELPRODUCER_H_
#define HTTPTUNNELPRODUCER_H_

/*
 *
 */

class HttpTunnelProducer
{
public:
	HttpTunnelProducer();
	virtual ~HttpTunnelProducer();

public:
	DLL<HttpTunnelConsumer> consumer_list;
	HttpTunnelConsumer *self_consumer;
	VConnection *vc;
	HttpProducerHandler vc_handler;
	VIO *read_vio;
	MIOBuffer *read_buffer;
	IOBufferReader *buffer_start;
	HttpTunnelType_t vc_type;

	ChunkedHandler chunked_handler;
	TunnelChunkingAction_t chunking_action;

	bool do_chunking;
	bool do_dechunking;
	bool do_chunked_passthru;

	int64_t init_bytes_done;          // bytes passed in buffer
	int64_t nbytes;                   // total bytes (client's perspective)
	int64_t ntodo;                    // what this vc needs to do
	int64_t bytes_read;               // total bytes read from the vc
	int handler_state;              // state used the handlers
	int last_event;                   ///< Tracking for flow control restarts.

	int num_consumers;

	bool alive;
	bool read_success;
	/// Flag and pointer for active flow control throttling.
	/// If this is set, it points at the source producer that is under flow control.
	/// If @c NULL then data flow is not being throttled.
	HttpTunnelProducer* flow_control_source;
	const char *name;

	/** Get the largest number of bytes any consumer has not consumed.
	  Use @a limit if you only need to check if the backlog is at least @a limit.
	  @return The actual backlog or a number at least @a limit.
	*/
	uint64_t backlog( uint64_t limit = INTU64_MAX );///< More than this is irrelevant
	/// Check if producer is original (to ATS) source of data.
	/// @return @c true if this producer is the source of bytes from outside ATS.
	bool is_source() const;
	/// Throttle the flow.
	void throttle();
	/// Unthrottle the flow.
	void unthrottle();
	/// Check throttled state.
	bool is_throttled() const;

	/** Set the flow control source producer for the flow.
	  This sets the value for this producer and all downstream producers.
	  @note This is the implementation for @c throttle and @c unthrottle.
	  @see throttle
	  @see unthrottle
	*/
	void set_throttle_src(
			HttpTunnelProducer* srcp ///< Source producer of flow.
			);
};
#endif /* HTTPTUNNELPRODUCER_H_ */
