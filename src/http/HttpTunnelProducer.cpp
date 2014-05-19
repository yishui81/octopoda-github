/*
 * HttpTunnelProducer.cpp
 *
 *  Created on: 2014年5月13日
 *      Author: jacky
 */

#include "HttpTunnelProducer.h"
#include <sys/types.h>

HttpTunnelProducer::HttpTunnelProducer()
		  : consumer_list(),
			self_consumer(NULL),
			vc(NULL),
			vc_handler(NULL),
			read_vio(NULL),
			read_buffer(NULL),
			buffer_start(NULL),
			vc_type(HT_HTTP_SERVER),
			chunking_action(TCA_PASSTHRU_DECHUNKED_CONTENT),
			do_chunking(false),
			do_dechunking(false),
			do_chunked_passthru(false),
			init_bytes_done(0),
			nbytes(0),
			ntodo(0),
			bytes_read(0),
			handler_state(0),
			last_event(0),
			num_consumers(0),
			alive(false),
			read_success(false),
			flow_control_source(0),
			name(NULL)
{

}

uint64_t
HttpTunnelProducer::backlog(uint64_t limit) {
	  uint64_t zret = 0;
	  // Calculate the total backlog, the # of bytes inside ATS for this producer.
	  // We go all the way through each chain to the ending sink and take the maximum
	  // over those paths. Do need to be careful about loops which can occur.
	  for ( HttpTunnelConsumer* c = consumer_list.head ; c ; c = c->link.next ) {

		if (c->alive && c->write_vio) {

			  uint64_t n = 0;
			  if (HT_TRANSFORM == c->vc_type) {
				  n += static_cast<TransformVCChain*>(c->vc)->backlog(limit);
			  } else {
				IOBufferReader* r = c->write_vio->get_reader();

				if (r) {
					n += static_cast<uint64_t>(r->read_avail());
				}
			  }

			  if (n >= limit) {
				  return n;
			  }

			  if (!c->is_sink()) {
				HttpTunnelProducer* dsp = c->self_producer;
				if (dsp) {
					n += dsp->backlog();
				}
			  }

			  if (n >= limit) {
				  return n;
			  }

			  if (n > zret) {
				  zret = n;
			  }
		}
	  }

	if (chunked_handler.chunked_reader) {
		zret += static_cast<uint64_t>(chunked_handler.chunked_reader->read_avail());
	}

	return zret;
}

/*  We set the producers in a flow chain specifically rather than
    using a tunnel level variable in order to handle bi-directional
    tunnels correctly. In such a case the flow control on producers is
    not related so a single value for the tunnel won't work.
*/
void
HttpTunnelProducer::set_throttle_src(HttpTunnelProducer* srcp) {

	HttpTunnelProducer* p = this;
	p->flow_control_source = srcp;

	for ( HttpTunnelConsumer* c = consumer_list.head ; c ; c = c->link.next ) {
		if (!c->is_sink()) {
			p = c->self_producer;
			if (p){
				  p->set_throttle_src(srcp);
			}
		}
	}
}
