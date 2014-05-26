/*
 * HttpTunnel.h
 *
 *  Created on: 2014年5月13日
 *      Author: jacky
 */

#ifndef HTTPTUNNEL_H_
#define HTTPTUNNEL_H_

#ifndef _HTTP_TUNNEL_H_
#define _HTTP_TUNNEL_H_

#include "libts.h"
#include "BaseARE/UTaskObj.h"
#include "I_IOBuffer.h"
//#include "P_EventSystem.h"

static const int max_chunked_ahead_blocks = 128;
static const int min_block_transfer_bytes = 256;
static char const * const CHUNK_HEADER_FMT = "%" PRIx64"\r\n";
// This should be as small as possible because it will only hold the
// header and trailer per chunk - the chunk body will be a reference to
// a block in the input stream.
static int const CHUNK_IOBUFFER_SIZE_INDEX = MIN_IOBUFFER_SIZE;

// Get rid of any previous definition first... /leif
#ifdef MAX_PRODUCERS
#undef MAX_PRODUCERS
#endif
#ifdef MAX_CONSUMERS
#undef MAX_CONSUMERS
#endif
#define MAX_PRODUCERS   2
#define MAX_CONSUMERS   4

#define HTTP_TUNNEL_EVENT_DONE             (HTTP_TUNNEL_EVENTS_START + 1)
#define HTTP_TUNNEL_EVENT_PRECOMPLETE      (HTTP_TUNNEL_EVENTS_START + 2)
#define HTTP_TUNNEL_EVENT_CONSUMER_DETACH  (HTTP_TUNNEL_EVENTS_START + 3)

#define HTTP_TUNNEL_STATIC_PRODUCER  (VConnection*)!0

//YTS Team, yamsat Plugin
#define ALLOCATE_AND_WRITE_TO_BUF 1
#define WRITE_TO_BUF 2

struct HttpTunnelProducer;
class HttpStateMachine;
class HttpPagesHandler;
typedef int (HttpStateMachine::*HttpSMHandler) (int event, void *data);

struct HttpTunnelConsumer;
struct HttpTunnelProducer;
typedef int (HttpStateMachine::*HttpProducerHandler) (int event, HttpTunnelProducer * p);
typedef int (HttpStateMachine::*HttpConsumerHandler) (int event, HttpTunnelConsumer * c);

enum HttpTunnelType_t
{
	  HT_HTTP_SERVER,
	  HT_HTTP_CLIENT,
	  HT_CACHE_READ,
	  HT_CACHE_WRITE,
	  HT_TRANSFORM,
	  HT_STATIC
};




class PostDataBuffers
{
public:
	PostDataBuffers()
		: postdata_producer_buffer(NULL),
		postdata_copy_buffer(NULL),
		postdata_producer_reader(NULL),
		postdata_copy_buffer_start(NULL),
		ua_buffer_reader(NULL)
	{
		Debug("http_redirect", "[PostDataBuffers::PostDataBuffers]");
	}

	MIOBuffer *postdata_producer_buffer;
	MIOBuffer *postdata_copy_buffer;
	IOBufferReader *postdata_producer_reader;
	IOBufferReader *postdata_copy_buffer_start;
	IOBufferReader *ua_buffer_reader;
};

class HttpTunnel: public UTaskObj
{
	friend class HttpPagesHandler;
	friend class CoreUtils;

	/** Data for implementing flow control across a tunnel.

	  The goal is to bound the amount of data buffered for a
	  transaction flowing through the tunnel to (roughly) between the
	  @a high_water and @a low_water water marks. Due to the chunky nater of data
	  flow this always approximate.
	*/
	struct FlowControl {
		// Default value for high and low water marks.
		static uint64_t const DEFAULT_WATER_MARK = 1<<16;

		uint64_t high_water; ///< Buffered data limit - throttle if more than this.
		uint64_t low_water; ///< Unthrottle if less than this buffered.
		bool enabled_p; ///< Flow control state (@c false means disabled).

		/// Default constructor.
		FlowControl();
	};

public:
	HttpTunnel();

	void init(HttpStateMachine * sm_arg, ProxyMutex * amutex);
	void reset();

	void kill_tunnel();

	bool is_tunnel_active() {
		return active;
	}

	bool is_tunnel_alive();
	bool has_cache_writer();

	// YTS Team, yamsat Plugin
	void copy_partial_post_data();
	void allocate_redirect_postdata_producer_buffer();
	void allocate_redirect_postdata_buffers(IOBufferReader * ua_reader);
	void deallocate_redirect_postdata_buffers();

	HttpTunnelProducer *add_producer(VConnection * vc,
							   int64_t nbytes,
							   IOBufferReader * reader_start,
							   HttpProducerHandler sm_handler,
							   HttpTunnelType_t vc_type,
							   const char *name);

	void set_producer_chunking_action(HttpTunnelProducer * p,
			int64_t skip_bytes, TunnelChunkingAction_t action);
	/// Set the maximum (preferred) chunk @a size of chunked output for @a producer.
	void set_producer_chunking_size(HttpTunnelProducer* producer, int64_t size);

	HttpTunnelConsumer *add_consumer(VConnection * vc,
							   VConnection * producer,
							   HttpConsumerHandler sm_handler,
							   HttpTunnelType_t vc_type,
							   const char *name,
							   int64_t skip_bytes = 0);

	int deallocate_buffers();
	DLL<HttpTunnelConsumer> *get_consumers(VConnection * vc);
	HttpTunnelProducer *get_producer(VConnection * vc);
	HttpTunnelConsumer *get_consumer(VConnection * vc);
	void tunnel_run(HttpTunnelProducer * p = NULL);

	int main_handler(int event, void *data);
	bool consumer_reenable(HttpTunnelConsumer* c);
	bool consumer_handler(int event, HttpTunnelConsumer * c);
	bool producer_handler(int event, HttpTunnelProducer * p);
	int producer_handler_dechunked(int event, HttpTunnelProducer * p);
	int producer_handler_chunked(int event, HttpTunnelProducer * p);
	void local_finish_all(HttpTunnelProducer * p);
	void chain_finish_all(HttpTunnelProducer * p);
	void chain_abort_cache_write(HttpTunnelProducer * p);
	void chain_abort_all(HttpTunnelProducer * p);
	void abort_cache_write_finish_others(HttpTunnelProducer * p);
	void append_message_to_producer_buffer(HttpTunnelProducer * p, const char *msg, int64_t msg_len);

	/** Mark a producer and consumer as the same underlying object.

	This is use to chain producer/consumer pairs together to
	indicate the data flows through them sequentially. The primary
	example is a transform which serves as a consumer on the server
	side and a producer on the cache/client side.
	*/
	void chain(
		 HttpTunnelConsumer* c,  ///< Flow goes in here
		 HttpTunnelProducer* p   ///< Flow comes back out here
	 );

	void close_vc(HttpTunnelProducer * p);
	void close_vc(HttpTunnelConsumer * c);

private:

	void internal_error();
	void finish_all_internal(HttpTunnelProducer * p, bool chain);
	void update_stats_after_abort(HttpTunnelType_t t);
	void producer_run(HttpTunnelProducer * p);

	HttpTunnelProducer *get_producer(VIO * vio);
	HttpTunnelConsumer *get_consumer(VIO * vio);

	HttpTunnelProducer *alloc_producer();
	HttpTunnelConsumer *alloc_consumer();

private:
	int num_producers;
	int num_consumers;
	HttpTunnelConsumer consumers[MAX_CONSUMERS];
	HttpTunnelProducer producers[MAX_PRODUCERS];
	HttpStateMachine *sm;
	bool active;
	/// State data about flow control.
	FlowControl flow_state;

public:
  	  PostDataBuffers * postbuf;
};



#endif /* HTTPTUNNEL_H_ */
