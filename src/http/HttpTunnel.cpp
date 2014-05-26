/*
 * HttpTunnel.cpp
 *
 *  Created on: 2014年5月13日
 *      Author: jacky
 */

#include "HttpTunnel.h"
#include "BaseARE/UTaskObj.h"

/** @file

  A brief file description

  @section license License

  Licensed to the Apache Software Foundation (ASF) under one
  or more contributor license agreements.  See the NOTICE file
  distributed with this work for additional information
  regarding copyright ownership.  The ASF licenses this file
  to you under the Apache License, Version 2.0 (the
  "License"); you may not use this file except in compliance
  with the License.  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 */

/****************************************************************************

   HttpTunnel.cc

   Description:


****************************************************************************/

#include "ink_config.h"
#include "HttpConfig.h"
#include "HttpTunnel.h"
#include "HttpStateMachine.h"
//#include "HttpDebugNames.h"
#include "ParseRules.h"
#include "stdio.h"

static const int max_chunked_ahead_blocks = 128;
static const int min_block_transfer_bytes = 256;
static char const * const CHUNK_HEADER_FMT = "%" PRIx64"\r\n";
// This should be as small as possible because it will only hold the
// header and trailer per chunk - the chunk body will be a reference to
// a block in the input stream.
static int const CHUNK_IOBUFFER_SIZE_INDEX = MIN_IOBUFFER_SIZE;

char
VcTypeCode(HttpTunnelType_t t) {

  char zret = ' ';
  switch (t) {
	  case HT_HTTP_CLIENT:
		  zret = 'U';
		  break;
	  case HT_HTTP_SERVER:
		  zret = 'S';
		  break;
	  case HT_TRANSFORM:
		  zret = 'T';
		  break;
	  case HT_CACHE_READ:
		  zret = 'R';
		  break;
	  case HT_CACHE_WRITE:
		  zret = 'W';
		  break;
	  default:
		  break;
  }

  return zret;
}




HttpTunnel::HttpTunnel()
		:num_producers(0),
		num_consumers(0),
		sm(NULL),
		active(false)
{

}


// void HttpTunnel::abort_cache_write_finish_others
//
//    Abort all downstream cache writes and finsish
//      all other local consumers
//
inline void
HttpTunnel::abort_cache_write_finish_others(HttpTunnelProducer * p)
{
	chain_abort_cache_write(p);
	local_finish_all(p);
}

// void HttpTunnel::local_finish_all(HttpTunnelProducer* p)
//
//   After the producer has finished, causes direct consumers
//      to finish their writes
//
inline void
HttpTunnel::local_finish_all(HttpTunnelProducer * p)
{
	finish_all_internal(p, false);
}

// void HttpTunnel::chain_finish_all(HttpTunnelProducer* p)
//
//   After the producer has finished, cause everyone
//    downstream in the tunnel to send everything
//    that producer has placed in the buffer
//
inline void
HttpTunnel::chain_finish_all(HttpTunnelProducer * p)
{
	finish_all_internal(p, true);
}

inline bool
HttpTunnel::is_tunnel_alive()
{
	bool tunnel_alive = false;

	for (int i = 0; i < MAX_PRODUCERS; i++) {

		if (producers[i].alive == true) {
			tunnel_alive = true;
			break;
		}
	}

	if (!tunnel_alive) {

		for (int i = 0; i < MAX_CONSUMERS; i++) {

			if (consumers[i].alive == true) {
				tunnel_alive = true;
				break;
			}

		}
	}

	return tunnel_alive;
}

inline HttpTunnelProducer *
HttpTunnel::get_producer(VConnection * vc)
{
	for (int i = 0; i < MAX_PRODUCERS; i++) {

		if (producers[i].vc == vc) {
			return producers + i;
		}

	}
	return NULL;
}

inline HttpTunnelConsumer *
HttpTunnel::get_consumer(VConnection * vc)
{
	for (int i = 0; i < MAX_CONSUMERS; i++) {

		if (consumers[i].vc == vc) {
			return consumers + i;
		}

	}
	return NULL;
}

inline HttpTunnelProducer *
HttpTunnel::get_producer(VIO * vio)
{
	for (int i = 0; i < MAX_PRODUCERS; i++) {

		if (producers[i].read_vio == vio) {
			return producers + i;
		}

	}

	return NULL;
}

inline HttpTunnelConsumer *
HttpTunnel::get_consumer(VIO * vio)
{
	for (int i = 0; i < MAX_CONSUMERS; i++) {

		if (consumers[i].write_vio == vio) {
			return consumers + i;
		}

	}
	return NULL;
}

inline void
HttpTunnel::append_message_to_producer_buffer(HttpTunnelProducer * p, const char *msg, int64_t msg_len)
{
	if (p == NULL || p->read_buffer == NULL){
		return;
	}

	p->read_buffer->write(msg, msg_len);
	p->nbytes += msg_len;
	p->bytes_read += msg_len;
}

inline bool
HttpTunnel::has_cache_writer()
{
	for (int i = 0; i < MAX_CONSUMERS; i++) {
		if (consumers[i].vc_type == HT_CACHE_WRITE
				&& consumers[i].vc != NULL) {
			return true;
		}
	}
	return false;
}

inline bool
HttpTunnelConsumer::is_downstream_from(VConnection *vc)
{
	HttpTunnelProducer* p = producer;
	HttpTunnelConsumer* c;

	while (p) {

		if (p->vc == vc) {
			return true;
		}

		// The producer / consumer chain can contain a cycle in the case
		// of a blind tunnel so give up if we find ourself (the original
		// consumer).
		c = p->self_consumer;
		p = (c && c != this) ? c->producer : 0;

	}
	return false;
}

inline bool
HttpTunnelConsumer::is_sink() const
{
  return HT_HTTP_CLIENT == vc_type || HT_CACHE_WRITE == vc_type;
}

inline bool
HttpTunnelProducer::is_source() const
{
  // If a producer is marked as a client, then it's part of a bidirectional tunnel
  // and so is an actual source of data.
  return HT_HTTP_SERVER == vc_type || HT_CACHE_READ == vc_type || HT_HTTP_CLIENT == vc_type;
}

inline bool
HttpTunnelProducer::is_throttled() const
{
	  return (0 != flow_control_source);
}

inline void
HttpTunnelProducer::throttle()
{
	if (!this->is_throttled()){
		this->set_throttle_src(this);
	}
}

inline void
HttpTunnelProducer::unthrottle()
{
	if (this->is_throttled()){
		this->set_throttle_src(0);
	}
}

inline
HttpTunnel::FlowControl::FlowControl()
	  : high_water(DEFAULT_WATER_MARK)
	  , low_water(DEFAULT_WATER_MARK)
	  , enabled_p(false)
{

}

void
HttpTunnel::init(HttpStateMachine * sm_arg, ProxyMutex * amutex)
{
	  HttpConfigParams* params = sm_arg->t_state.http_config_param;
	  sm = sm_arg;
	  active = false;
	  mutex = amutex;

	  SET_HANDLER(&HttpTunnel::main_handler);
	  flow_state.enabled_p = params->oride.flow_control_enabled;
	  if (params->oride.flow_low_water_mark > 0){
		flow_state.low_water = params->oride.flow_low_water_mark;
	  }
	  if (params->oride.flow_high_water_mark > 0){
		flow_state.high_water = params->oride.flow_high_water_mark;
	  }
	  // This should always be true, we handled default cases back in HttpConfig::reconfigure()
	  ink_assert(flow_state.low_water <= flow_state.high_water);
}

void
HttpTunnel::reset()
{
  ink_assert(active == false);
#ifdef DEBUG
  for (int i = 0; i < MAX_PRODUCERS; ++i) {
    ink_assert(producers[i].alive == false);
  }
  for (int j = 0; j < MAX_CONSUMERS; ++j) {
    ink_assert(consumers[j].alive == false);
  }
#endif

  num_producers = 0;
  num_consumers = 0;
  memset(consumers, 0, sizeof(consumers));
  memset(producers, 0, sizeof(producers));
}

void
HttpTunnel::kill_tunnel()
{
	for (int i = 0; i < MAX_PRODUCERS; ++i) {
		if (producers[i].vc != NULL) {
		  chain_abort_all(&producers[i]);
		}
		ink_assert(producers[i].alive == false);
	}
	active = false;
	this->deallocate_buffers();
	this->deallocate_redirect_postdata_buffers();
	this->reset();
}

HttpTunnelProducer *
HttpTunnel::alloc_producer()
{
	for (int i = 0; i < MAX_PRODUCERS; ++i) {
		if (producers[i].vc == NULL) {
			  num_producers++;
			  ink_assert(num_producers <= MAX_PRODUCERS);
			  return producers + i;
		}
	}
	ink_release_assert(0);
	return NULL;
}

HttpTunnelConsumer *
HttpTunnel::alloc_consumer()
{
	for (int i = 0; i < MAX_CONSUMERS; i++) {
		if (consumers[i].vc == NULL) {
			num_consumers++;
			ink_assert(num_consumers <= MAX_CONSUMERS);
			return consumers + i;
		}
	}
	ink_release_assert(0);
	return NULL;
}

int
HttpTunnel::deallocate_buffers()
{
	int num = 0;
	ink_release_assert(active == false);

	for (int i = 0; i < MAX_PRODUCERS; ++i) {
		if (producers[i].read_buffer != NULL) {
			ink_assert(producers[i].vc != NULL);
			free_MIOBuffer(producers[i].read_buffer);
			producers[i].read_buffer = NULL;
			producers[i].buffer_start = NULL;
			num++;
		}

		if (producers[i].chunked_handler.dechunked_buffer != NULL) {
			ink_assert(producers[i].vc != NULL);
			free_MIOBuffer(producers[i].chunked_handler.dechunked_buffer);
			producers[i].chunked_handler.dechunked_buffer = NULL;
			num++;
		}

		if (producers[i].chunked_handler.chunked_buffer != NULL) {
			ink_assert(producers[i].vc != NULL);
			free_MIOBuffer(producers[i].chunked_handler.chunked_buffer);
			producers[i].chunked_handler.chunked_buffer = NULL;
			num++;
		}

		producers[i].chunked_handler.max_chunk_header_len = 0;
	}
	return num;
}

void
HttpTunnel::set_producer_chunking_action(HttpTunnelProducer * p, int64_t skip_bytes, TunnelChunkingAction_t action)
{
	p->chunked_handler.skip_bytes = skip_bytes;
	p->chunking_action = action;

	switch (action) {
	case TCA_CHUNK_CONTENT:
		p->chunked_handler.state = p->chunked_handler.CHUNK_WRITE_CHUNK;
		break;
	case TCA_DECHUNK_CONTENT:
	case TCA_PASSTHRU_CHUNKED_CONTENT:
		p->chunked_handler.state = p->chunked_handler.CHUNK_READ_SIZE;
		break;
	default:
		break;
	}
}

void
HttpTunnel::set_producer_chunking_size(HttpTunnelProducer* p, int64_t size)
{
	p->chunked_handler.set_max_chunk_size(size);
}

// HttpTunnelProducer* HttpTunnel::add_producer
//
//   Adds a new producer to the tunnel
//
HttpTunnelProducer *
HttpTunnel::add_producer(VConnection * vc,
                         int64_t nbytes_arg,
                         IOBufferReader * reader_start,
                         HttpProducerHandler sm_handler,
                         HttpTunnelType_t vc_type,
                         const char *name_arg)
{
	HttpTunnelProducer *p;

	Debug("http_tunnel", "[%" PRId64 "] adding producer '%s'", sm->sm_id, name_arg);

	ink_assert(reader_start->mbuf);
	if ((p = alloc_producer()) != NULL) {
		p->vc = vc;
		p->nbytes = nbytes_arg;
		p->buffer_start = reader_start;
		p->read_buffer = reader_start->mbuf;
		p->vc_handler = sm_handler;
		p->vc_type = vc_type;
		p->name = name_arg;
		p->chunking_action = TCA_PASSTHRU_DECHUNKED_CONTENT;

		p->do_chunking = false;
		p->do_dechunking = false;
		p->do_chunked_passthru = false;

		p->init_bytes_done = reader_start->read_avail();
		if (p->nbytes < 0) {
			p->ntodo = p->nbytes;
		} else {                    // The byte count given us includes bytes
			//  that alread may be in the buffer.
			//  ntodo represents the number of bytes
			//  the tunneling mechanism needs to read
			//  for the producer
			p->ntodo = p->nbytes - p->init_bytes_done;
			ink_assert(p->ntodo >= 0);
		}

		// We are static, the producer is never "alive"
		//   It just has data in the buffer
		if (vc == HTTP_TUNNEL_STATIC_PRODUCER) {
			ink_assert(p->ntodo == 0);
			p->alive = false;
			p->read_success = true;
		} else {
			p->alive = true;
		}
	}
	return p;
}

// void HttpTunnel::add_consumer
//
//    Adds a new consumer to the tunnel.  The producer must
//    be specified and already added to the tunnel.  Attaches
//    the new consumer to the entry for the existing producer
//
//    Returns true if the consumer successfully added.  Returns
//    false if the consumer was not added because the source failed
//
HttpTunnelConsumer *
HttpTunnel::add_consumer(VConnection * vc,
                         VConnection * producer,
                         HttpConsumerHandler sm_handler,
                         HttpTunnelType_t vc_type,
                         const char *name_arg,
                         int64_t skip_bytes)
{
	Debug("http_tunnel", "[%" PRId64 "] adding consumer '%s'", sm->sm_id, name_arg);

	// Find the producer entry
	HttpTunnelProducer *p = get_producer(producer);
	ink_release_assert(p);

	// Check to see if the producer terminated
	//  without sending all of its data
	if (p->alive == false &&
			p->read_success == false) {
		Debug("http_tunnel", "[%" PRId64 "] consumer '%s' not added due to producer failure", sm->sm_id, name_arg);
		return NULL;
	}
	// Initialize the consumer structure
	HttpTunnelConsumer *c = alloc_consumer();
	c->producer = p;
	c->vc = vc;
	c->alive = true;
	c->skip_bytes = skip_bytes;
	c->vc_handler = sm_handler;
	c->vc_type = vc_type;
	c->name = name_arg;

	// Register the consumer with the producer
	p->consumer_list.push(c);
	p->num_consumers++;

	return c;
}

void
HttpTunnel::chain(HttpTunnelConsumer* c, HttpTunnelProducer* p)
{
	p->self_consumer = c;
	c->self_producer = p;
	// If the flow is already throttled update the chained producer.
	if (c->producer->is_throttled()){
		p->set_throttle_src(c->producer->flow_control_source);
	}
}

// void HttpTunnel::tunnel_run()
//
//    Makes the tunnel go
//
void
HttpTunnel::tunnel_run(HttpTunnelProducer * p_arg)
{
	Debug("http_tunnel", "tunnel_run started, p_arg is %s", p_arg ? "provided" : "NULL");

	if (p_arg) {
		producer_run(p_arg);
	} else {
		HttpTunnelProducer *p;

		ink_assert(active == false);

		for (int i = 0 ; i < MAX_PRODUCERS ; ++i) {
			p = producers + i;
			if (p->vc != NULL) {
				producer_run(p);
			}
		}
	}

	// It is possible that there was nothing to do
	//   due to a all transfers being zero length
	//   If that is the case, call the state machine
	//   back to say we are done
	if (!is_tunnel_alive()) {
		active = false;
		sm->handleEvent(HTTP_TUNNEL_EVENT_DONE, this);
	}
}

void
HttpTunnel::producer_run(HttpTunnelProducer * p)
{
	// Determine whether the producer has a cache-write consumer,
	// since all chunked content read by the producer gets dechunked
	// prior to being written into the cache.
	HttpTunnelConsumer *c, *cache_write_consumer = NULL;
	bool transform_consumer = false;

	for (c = p->consumer_list.head; c; c = c->link.next) {
		if (c->vc_type == HT_CACHE_WRITE) {
			cache_write_consumer = c;
			break;
		}
	}

	// bz57413
	for (c = p->consumer_list.head; c; c = c->link.next) {
		if (c->vc_type == HT_TRANSFORM) {
			transform_consumer = true;
			break;
		}
	}

	// Determine whether the producer is to perform chunking,
	// dechunking, or chunked-passthough of the incoming response.
	TunnelChunkingAction_t action = p->chunking_action;

	// [bug 2579251] static producers won't have handler set
	if (p->vc != HTTP_TUNNEL_STATIC_PRODUCER) {
		if (action == TCA_CHUNK_CONTENT){
			p->do_chunking = true;
		}else if (action == TCA_DECHUNK_CONTENT){
			p->do_dechunking = true;
		}else if (action == TCA_PASSTHRU_CHUNKED_CONTENT) {
			p->do_chunked_passthru = true;

			// Dechunk the chunked content into the cache.
			if (cache_write_consumer != NULL)
			p->do_dechunking = true;
		}
	}

	int64_t consumer_n;
	int64_t producer_n;

	ink_assert(p->vc != NULL);
	active = true;

	IOBufferReader *chunked_buffer_start = NULL, *dechunked_buffer_start = NULL;
	if (p->do_chunking || p->do_dechunking || p->do_chunked_passthru) {
		producer_n = (consumer_n = INT64_MAX);
		p->chunked_handler.init(p->buffer_start, p);

		// Copy the header into the chunked/dechunked buffers.
		if (p->do_chunking) {

			// initialize a reader to chunked buffer start before writing to keep ref count
			chunked_buffer_start = p->chunked_handler.chunked_buffer->alloc_reader();
			p->chunked_handler.chunked_buffer->write(p->buffer_start, p->chunked_handler.skip_bytes);

		} else if (p->do_dechunking) {

			Debug("http_tunnel", "[producer_run] do_dechunking p->chunked_handler.chunked_reader->read_avail() = %" PRId64"",
			p->chunked_handler.chunked_reader->read_avail());

			// initialize a reader to dechunked buffer start before writing to keep ref count
			dechunked_buffer_start = p->chunked_handler.dechunked_buffer->alloc_reader();

			// If there is no transformation then add the header to the buffer, else the
			// client already has got the header from us, no need for it in the buffer.
			if (!transform_consumer) {
				p->chunked_handler.dechunked_buffer->write(p->buffer_start, p->chunked_handler.skip_bytes);
				Debug("http_tunnel", "[producer_run] do_dechunking::Copied header of size %" PRId64"", p->chunked_handler.skip_bytes);
			}
		}
	}

	int64_t read_start_pos = 0;
	if (p->vc_type == HT_CACHE_READ &&
			sm->t_state.range_setup == HttpTransact::RANGE_NOT_TRANSFORM_REQUESTED) {
		ink_assert(sm->t_state.num_range_fields == 1); // we current just support only one range entry
		read_start_pos = sm->t_state.ranges[0]._start;
		producer_n = (sm->t_state.ranges[0]._end - sm->t_state.ranges[0]._start)+1;
		consumer_n = (producer_n + sm->client_response_hdr_bytes);
	} else if (p->nbytes >= 0) {
		consumer_n = p->nbytes;
		producer_n = p->ntodo;
	} else {
		consumer_n = (producer_n = INT64_MAX);
	}

	// Do the IO on the consumers first so
	//  data doesn't disappear out from
	//  under the tunnel
	ink_release_assert(p->num_consumers > 0);
	for (c = p->consumer_list.head; c;) {
		// Create a reader for each consumer.  The reader allows
		// us to implement skip bytes
		if (c->vc_type == HT_CACHE_WRITE) {
			switch (action) {
				case TCA_CHUNK_CONTENT:
				case TCA_PASSTHRU_DECHUNKED_CONTENT:
					c->buffer_reader = p->read_buffer->clone_reader(p->buffer_start);
					break;
				case TCA_DECHUNK_CONTENT:
				case TCA_PASSTHRU_CHUNKED_CONTENT:
					c->buffer_reader = p->chunked_handler.dechunked_buffer->clone_reader(dechunked_buffer_start);
					break;
				default:
					break;
			}
		}
		// Non-cache consumers.
		else if (action == TCA_CHUNK_CONTENT) {
			c->buffer_reader = p->chunked_handler.chunked_buffer->clone_reader(chunked_buffer_start);
		} else if (action == TCA_DECHUNK_CONTENT) {
			c->buffer_reader = p->chunked_handler.dechunked_buffer->clone_reader(dechunked_buffer_start);
		} else {
			c->buffer_reader = p->read_buffer->clone_reader(p->buffer_start);
		}

		// Consume bytes of the reader if we skipping bytes
		if (c->skip_bytes > 0) {
			ink_assert(c->skip_bytes <= c->buffer_reader->read_avail());
			c->buffer_reader->consume(c->skip_bytes);
		}
		int64_t c_write = consumer_n;

		// INKqa05109 - if we don't know the length leave it at
		//  INT64_MAX or else the cache may bounce the write
		//  because it thinks the document is too big.  INT64_MAX
		//  is a special case for the max document size code
		//  in the cache
		if (c_write != INT64_MAX) {
			c_write -= c->skip_bytes;
		}
		// Fix for problems with not chunked content being chunked and
		// not sending the entire data.  The content length grows when
		// it is being chunked.
		if (p->do_chunking == true) {
			c_write = INT64_MAX;
		}

		if (c_write == 0) {
			// Nothing to do, call back the cleanup handlers
			c->write_vio = NULL;
			consumer_handler(VC_EVENT_WRITE_COMPLETE, c);
		} else {
			c->write_vio = c->vc->do_io_write(this, c_write, c->buffer_reader);
			ink_assert(c_write > 0);
		}

		c = c->link.next;
	}

	//YTS Team, yamsat Plugin
	// Allocate and copy partial POST data to buffers. Check for the various parameters
	// including the maximum configured post data size
	if (p->alive && sm->t_state.method == HTTP_WKSIDX_POST
			&& sm->enable_redirection
			&& sm->redirection_tries == 0
			&& (p->vc_type == HT_HTTP_CLIENT)) {

		Debug("http_redirect", "[HttpTunnel::producer_run] client post: %" PRId64" max size: %" PRId64"",
		p->buffer_start->read_avail(), HttpConfig::m_master.post_copy_size);

		// (note that since we are not dechunking POST, this is the chunked size if chunked)
		if (p->buffer_start->read_avail() > HttpConfig::m_master.post_copy_size) {
			Debug("http_redirect", "[HttpTunnel::producer_handler] post exceeds buffer limit, buffer_avail=%" PRId64" limit=%" PRId64"",
			p->buffer_start->read_avail(), HttpConfig::m_master.post_copy_size);
			sm->enable_redirection = false;
		} else {
			// allocate post buffers with a new reader. The reader will be freed when p->read_buffer is freed
			allocate_redirect_postdata_buffers(p->read_buffer->clone_reader(p->buffer_start));
			copy_partial_post_data();
		}
	}                             //end of added logic for partial POST

	if (p->do_chunking) {

		// remove the chunked reader marker so that it doesn't act like a buffer guard
		p->chunked_handler.chunked_buffer->dealloc_reader(chunked_buffer_start);
		p->chunked_handler.dechunked_reader->consume(p->chunked_handler.skip_bytes);

		// If there is data to process in the buffer, do it now
		if (p->chunked_handler.dechunked_reader->read_avail()){
			producer_handler(VC_EVENT_READ_READY, p);
		}

	} else if (p->do_dechunking || p->do_chunked_passthru) {

		// remove the dechunked reader marker so that it doesn't act like a buffer guard
		if (p->do_dechunking)
		p->chunked_handler.dechunked_buffer->dealloc_reader(dechunked_buffer_start);

		// bz57413
		// If there is no transformation plugin, then we didn't add the header, hence no need to consume it
		Debug("http_tunnel", "[producer_run] do_dechunking p->chunked_handler.chunked_reader->read_avail() = %" PRId64"",
		p->chunked_handler.chunked_reader->read_avail());

		if (!transform_consumer
				&& (p->chunked_handler.chunked_reader->read_avail() >= p->chunked_handler.skip_bytes)) {
			p->chunked_handler.chunked_reader->consume(p->chunked_handler.skip_bytes);
			Debug("http_tunnel", "[producer_run] do_dechunking p->chunked_handler.skip_bytes = %" PRId64"",
			p->chunked_handler.skip_bytes);
		}

		//if(p->chunked_handler.chunked_reader->read_avail() > 0)
		//p->chunked_handler.chunked_reader->consume(
		//p->chunked_handler.skip_bytes);

		if (p->chunked_handler.chunked_reader->read_avail()) {
			producer_handler(VC_EVENT_READ_READY, p);
		} else if (sm->redirection_tries > 0 && p->vc_type == HT_HTTP_CLIENT) {
			// read_avail() == 0
			// [bug 2579251]
			// Ugh, this is horrible but in the redirect case they are running a the tunnel again with the
			// now closed/empty producer to trigger PRECOMPLETE.  If the POST was chunked, producer_n is set
			// (incorrectly) to INT64_MAX.  It needs to be set to 0 to prevent triggering another read.
			producer_n = 0;
		}
	}

	if (p->alive) {

		ink_assert(producer_n >= 0);

		if (producer_n == 0) {

			// Everything is already in the buffer so mark the producer as done.  We need to notify
			// state machine that everything is done.  We use a special event to say the producers is
			// done but we didn't do anything
			p->alive = false;
			p->read_success = true;
			Debug("http_tunnel", "[%" PRId64 "] [tunnel_run] producer already done", sm->sm_id);
			producer_handler(HTTP_TUNNEL_EVENT_PRECOMPLETE, p);

		} else {

			if (read_start_pos > 0) {
				p->read_vio = ((CacheVC*)p->vc)->do_io_pread(this, producer_n, p->read_buffer, read_start_pos);
			}else {
				p->read_vio = p->vc->do_io_read(this, producer_n, p->read_buffer);
			}

		}
	}

	// Now that the tunnel has started, we must remove producer's reader so
	// that it doesn't act like a buffer guard
	p->read_buffer->dealloc_reader(p->buffer_start);
	p->buffer_start = NULL;
}

int
HttpTunnel::producer_handler_dechunked(int event, HttpTunnelProducer * p)
{
	ink_assert(p->do_chunking);

	Debug("http_tunnel", "[%" PRId64 "] producer_handler_dechunked [%s %s]", sm->sm_id, p->name, HttpDebugNames::get_event_name(event));

	// We only interested in translating certain events
	switch (event) {
		case VC_EVENT_READ_READY:
		case VC_EVENT_READ_COMPLETE:
		case HTTP_TUNNEL_EVENT_PRECOMPLETE:
		case VC_EVENT_EOS:
			p->last_event =
			  p->chunked_handler.last_server_event = event;
			// TODO: Should we check the return code?
			p->chunked_handler.generate_chunked_content();
			break;
	}

	// Since we will consume all the data if the server is actually finished
	//   we don't have to translate events like we do in the
	//   case producer_handler_chunked()
	return event;
}

// int HttpTunnel::producer_handler_chunked(int event, HttpTunnelProducer* p)
//
//   Handles events from chunked producers.  It calls the chunking handlers
//    if appropriate and then translates the event we got into a suitable
//    event to represent the unchunked state, and does chunked bookeeping
//
int
HttpTunnel::producer_handler_chunked(int event, HttpTunnelProducer * p)
{
	ink_assert(p->do_dechunking || p->do_chunked_passthru);

	Debug("http_tunnel", "[%" PRId64 "] producer_handler_chunked [%s %s]", sm->sm_id, p->name, HttpDebugNames::get_event_name(event));

	// We only interested in translating certain events
	switch (event) {
		case VC_EVENT_READ_READY:
		case VC_EVENT_READ_COMPLETE:
		case HTTP_TUNNEL_EVENT_PRECOMPLETE:
		case VC_EVENT_EOS:
			break;
		default:
			return event;
	}

	p->last_event =
			p->chunked_handler.last_server_event = event;
	bool done = p->chunked_handler.process_chunked_content();

	// If we couldn't understand the encoding, return
	//   an error
	if (p->chunked_handler.state == ChunkedHandler::CHUNK_READ_ERROR) {
		Debug("http_tunnel", "[%" PRId64 "] producer_handler_chunked [%s chunk decoding error]", sm->sm_id, p->name);
		p->chunked_handler.truncation = true;
		// FIX ME: we return EOS here since it will cause the
		//  the client to be reenabled.  ERROR makes more
		//  sense but no reenables follow
		return VC_EVENT_EOS;
	}

	switch (event) {
		case VC_EVENT_READ_READY:
			if (done) {
				return VC_EVENT_READ_COMPLETE;
			}
			break;
		case HTTP_TUNNEL_EVENT_PRECOMPLETE:
		case VC_EVENT_EOS:
		case VC_EVENT_READ_COMPLETE:
			if (!done) {
				p->chunked_handler.truncation = true;
			}
			break;
	}

  return event;
}

//
// bool HttpTunnel::producer_handler(int event, HttpTunnelProducer* p)
//
//   Handles events from producers.
//
//   If the event is interesting only to the tunnel, this
//    handler takes all necessary actions and returns false
//    If the event is interesting to the state_machine,
//    it calls back the state machine and returns true
//
//
bool HttpTunnel::producer_handler(int event, HttpTunnelProducer * p)
{
	HttpTunnelConsumer *c;
	HttpProducerHandler jump_point;
	bool sm_callback = false;

	Debug("http_tunnel", "[%" PRId64 "] producer_handler [%s %s]", sm->sm_id, p->name, HttpDebugNames::get_event_name(event));

	// Handle chunking/dechunking/chunked-passthrough if necessary.
	if (p->do_chunking) {
		event = producer_handler_dechunked(event, p);

		// If we were in PRECOMPLETE when this function was called
		// and we are doing chunking, then we just wrote the last
		// chunk in the the function call above.  We are done with the
		// tunnel.
		if (event == HTTP_TUNNEL_EVENT_PRECOMPLETE) {
		  event = VC_EVENT_EOS;
		}

	} else if (p->do_dechunking || p->do_chunked_passthru) {
		event = producer_handler_chunked(event, p);
	} else {
		p->last_event = event;
	}

	//YTS Team, yamsat Plugin
	//Copy partial POST data to buffers. Check for the various parameters including
	//the maximum configured post data size
	if (sm->t_state.method == HTTP_WKSIDX_POST
			&& sm->enable_redirection
			&&(event == VC_EVENT_READ_READY || event == VC_EVENT_READ_COMPLETE)
			&& (p->vc_type == HT_HTTP_CLIENT)) {

		Debug("http_redirect", "[HttpTunnel::producer_handler] [%s %s]", p->name, HttpDebugNames::get_event_name(event));

		c = p->consumer_list.head;
		if ((postbuf->postdata_copy_buffer_start->read_avail() + postbuf->ua_buffer_reader->read_avail())
			> HttpConfig::m_master.post_copy_size) {
			  Debug("http_redirect",
					"[HttpTunnel::producer_handler] post exceeds buffer limit, buffer_avail=%" PRId64" reader_avail=%" PRId64" limit=%" PRId64"",
					postbuf->postdata_copy_buffer_start->read_avail(), postbuf->ua_buffer_reader->read_avail(),
					HttpConfig::m_master.post_copy_size);

			  deallocate_redirect_postdata_buffers();
			  sm->enable_redirection = false;
		} else {
			copy_partial_post_data();
		}
	}                             //end of added logic for partial copy of POST

	Debug("http_redirect", "[HttpTunnel::producer_handler] enable_redirection: [%d %d %d] event: %d",
		p->alive == true, sm->enable_redirection, (p->self_consumer && p->self_consumer->alive == true), event);
	ink_assert(p->alive == true || event == HTTP_TUNNEL_EVENT_PRECOMPLETE || event == VC_EVENT_EOS ||
			 sm->enable_redirection || (p->self_consumer && p->self_consumer->alive == true));

	switch (event) {
		case VC_EVENT_READ_READY:
			// Data read from producer, reenable consumers
			for (c = p->consumer_list.head; c; c = c->link.next) {
			  if (c->alive) {
				c->write_vio->reenable();
			  }
			}
			break;

		case HTTP_TUNNEL_EVENT_PRECOMPLETE:

			// the producer had finished before the tunnel
			//  started so just call the state machine back
			//  We don't need to reenable since the consumers
			//  were just activated.  Likewise, we can't be
			//  done because the consumer couldn't have
			//  called us back yet

			p->bytes_read = 0;
			jump_point = p->vc_handler;
			(sm->*jump_point) (event, p);
			sm_callback = true;
			break;

		case VC_EVENT_READ_COMPLETE:
		case VC_EVENT_EOS:

			// The producer completed
			p->alive = false;
			if (p->read_vio) {
				p->bytes_read = p->read_vio->ndone;
			} else {
				  // If we are chunked, we can receive the whole document
				  //   along with the header without knowing it (due to
				  //   the message length being a property of the encoding)
				  //   In that case, we won't have done a do_io so there
				  //   will not be vio
				  p->bytes_read = 0;
			}

			// callback the SM to notify of completion
			//  Note: we need to callback the SM before
			//  reenabling the consumers as the reenable may
			//  make the data visible to the consumer and
			//  initiate async I/O operation.  The SM needs to
			//  set how much I/O to do before async I/O is
			//  initiated
			jump_point = p->vc_handler;
			(sm->*jump_point) (event, p);
			sm_callback = true;

			// Data read from producer, reenable consumers
			for (c = p->consumer_list.head; c; c = c->link.next) {
				  if (c->alive) {
					  c->write_vio->reenable();
				  }
			}
			break;

		case VC_EVENT_ERROR:
		case VC_EVENT_ACTIVE_TIMEOUT:
		case VC_EVENT_INACTIVITY_TIMEOUT:
		case HTTP_TUNNEL_EVENT_CONSUMER_DETACH:
			p->alive = false;
			p->bytes_read = p->read_vio->ndone;
			// Interesting tunnel event, call SM
			jump_point = p->vc_handler;
			(sm->*jump_point) (event, p);
			sm_callback = true;
			break;

		case VC_EVENT_WRITE_READY:
		case VC_EVENT_WRITE_COMPLETE:
		default:
			// Producers should not get these events
			ink_release_assert(0);
			break;
		}

	return sm_callback;
}

bool
HttpTunnel::consumer_reenable(HttpTunnelConsumer* c)
{
  HttpTunnelProducer* p = c->producer;
  HttpTunnelProducer* srcp = p->flow_control_source;
  if (p->alive
#ifndef LAZY_BUF_ALLOC
      && p->read_buffer->write_avail() > 0
#endif
    ) {
    // Only do flow control if enabled and the producer is an external
    // source.  Otherwise disable by making the backlog zero. Because
    // the backlog short cuts quit when the value is equal (or
    // greater) to the target, we use strict comparison only for
    // checking low water, otherwise the flow control can stall out.
    uint64_t backlog = (flow_state.enabled_p && p->is_source())
      ? p->backlog(flow_state.high_water)
      : 0;

    if (backlog >= flow_state.high_water) {
      if (is_debug_tag_set("http_tunnel"))
        Debug("http_tunnel", "Throttle   %p %" PRId64 " / %" PRId64, p, backlog, p->backlog());
      p->throttle(); // p becomes srcp for future calls to this method
    } else {
      if (srcp && c->is_sink()) {
        // Check if backlog is below low water - note we need to check
        // against the source producer, not necessarily the producer
        // for this consumer. We don't have to recompute the backlog
        // if they are the same because we know low water <= high
        // water so the value is sufficiently accurate.
        if (srcp != p)
          backlog = srcp->backlog(flow_state.low_water);
        if (backlog < flow_state.low_water) {
          if (is_debug_tag_set("http_tunnel"))
            Debug("http_tunnel", "Unthrottle %p %" PRId64 " / %" PRId64, p, backlog, p->backlog());
          srcp->unthrottle();
          srcp->read_vio->reenable();
          // Kick source producer to get flow ... well, flowing.
          this->producer_handler(VC_EVENT_READ_READY, srcp);
        }
      }
      p->read_vio->reenable();
    }
  }
  return p->is_throttled();
}

//
// bool HttpTunnel::consumer_handler(int event, HttpTunnelConsumer* p)
//
//   Handles events from consumers.
//
//   If the event is interesting only to the tunnel, this
//    handler takes all necessary actions and returns false
//    If the event is interesting to the state_machine,
//    it calls back the state machine and returns true
//
//
bool HttpTunnel::consumer_handler(int event, HttpTunnelConsumer * c)
{
	bool sm_callback = false;
	HttpConsumerHandler jump_point;
	HttpTunnelProducer* p = c->producer;

	Debug("http_tunnel", "[%" PRId64 "] consumer_handler [%s %s]", sm->sm_id, c->name, HttpDebugNames::get_event_name(event));

	ink_assert(c->alive == true);

	switch (event) {
	case VC_EVENT_WRITE_READY:
		this->consumer_reenable(c);
		break;

	case VC_EVENT_WRITE_COMPLETE:
	case VC_EVENT_EOS:
	case VC_EVENT_ERROR:
	case VC_EVENT_ACTIVE_TIMEOUT:
	case VC_EVENT_INACTIVITY_TIMEOUT:
		ink_assert(c->alive);
		ink_assert(c->buffer_reader);
		c->alive = false;

		c->bytes_written = c->write_vio ? c->write_vio->ndone : 0;

		// Interesting tunnel event, call SM
		jump_point = c->vc_handler;
		(sm->*jump_point) (event, c);
		sm_callback = true;

		// Deallocate the reader after calling back the sm
		//  because buffer problems are easier to debug
		//  in the sm when the reader is still valid
		c->buffer_reader->mbuf->dealloc_reader(c->buffer_reader);
		c->buffer_reader = NULL;

		// Since we removed a consumer, it may now be
		//   possbile to put more stuff in the buffer
		// Note: we reenable only after calling back
		//    the SM since the reenabling has the side effect
		//    updating the buffer state for the VConnection
		//    that is being reenabled
		if (p->alive && p->read_vio
			#ifndef LAZY_BUF_ALLOC
			&& p->read_buffer->write_avail() > 0
			#endif
		) {
			  if (p->is_throttled()){
				  this->consumer_reenable(c);
			  }else{
				p->read_vio->reenable();
			  }
		}
		// [amc] I don't think this happens but we'll leave a debug trap
		// here just in case.
		if (p->is_throttled()){
			Debug("http_tunnel", "Special event %s on %p with flow control on", HttpDebugNames::get_event_name(event), p);
		}
		break;

	case VC_EVENT_READ_READY:
	case VC_EVENT_READ_COMPLETE:
	default:
		// Consumers should not get these events
		ink_release_assert(0);
		break;
	}

	return sm_callback;
}


// void HttpTunnel::chain_abort_all(HttpTunnelProducer* p)
//
//    Abort the producer and everyone still alive
//     downstream of the producer
//
void
HttpTunnel::chain_abort_all(HttpTunnelProducer * p)
{
	HttpTunnelConsumer *c = p->consumer_list.head;

	while (c) {
		if (c->alive) {
			  c->alive = false;
			  c->write_vio = NULL;
			  c->vc->do_io_close(EHTTP_ERROR);
			  update_stats_after_abort(c->vc_type);
		}

		if (c->self_producer) {
			  chain_abort_all(c->self_producer);
		}

		c = c->link.next;
	}

	if (p->alive) {
		p->alive = false;
		p->bytes_read = p->read_vio->ndone;
		if (p->self_consumer) {
			p->self_consumer->alive = false;
		}
		p->read_vio = NULL;
		p->vc->do_io_close(EHTTP_ERROR);
		update_stats_after_abort(p->vc_type);
	}
}

// void HttpTunnel::chain_finish_internal(HttpTunnelProducer* p)
//
//    Internal function for finishing all consumers.  Takes
//       chain argument about where to finish just immediate
//       consumer or all those downstream
//
void
HttpTunnel::finish_all_internal(HttpTunnelProducer * p, bool chain)
{
	ink_assert(p->alive == false);
	HttpTunnelConsumer *c = p->consumer_list.head;
	int64_t total_bytes = 0;
	TunnelChunkingAction_t action = p->chunking_action;

	while (c) {

		if (c->alive) {

			if (c->vc_type == HT_CACHE_WRITE) {
				switch (action) {
				case TCA_CHUNK_CONTENT:
				case TCA_PASSTHRU_DECHUNKED_CONTENT:
					  total_bytes = p->bytes_read + p->init_bytes_done;
					  break;
				case TCA_DECHUNK_CONTENT:
				case TCA_PASSTHRU_CHUNKED_CONTENT:
					  total_bytes = p->chunked_handler.skip_bytes + p->chunked_handler.dechunked_size;
					  break;
				default:
					break;
				}

			} else if (action == TCA_CHUNK_CONTENT) {
				total_bytes = p->chunked_handler.skip_bytes + p->chunked_handler.chunked_size;
			} else if (action == TCA_DECHUNK_CONTENT) {
				total_bytes = p->chunked_handler.skip_bytes + p->chunked_handler.dechunked_size;
			} else{
				total_bytes = p->bytes_read + p->init_bytes_done;
			}

			c->write_vio->nbytes = total_bytes - c->skip_bytes;
			ink_assert(c->write_vio->nbytes >= 0);

			if (c->write_vio->nbytes < 0) {
				// TODO: Wtf, printf?
				fprintf(stderr,
						"[HttpTunnel::finish_all_internal] ERROR: Incorrect total_bytes - c->skip_bytes = %" PRId64 "\n",
						(int64_t) (total_bytes - c->skip_bytes));
			}

			if (chain == true && c->self_producer) {
				chain_finish_all(c->self_producer);
			}
			// The IO Core will not call us back if there
			//   is nothing to do.  Check to see if there is
			//   nothing to do and take the appripriate
			//   action
			if (c->write_vio->nbytes == c->write_vio->ndone) {
				consumer_handler(VC_EVENT_WRITE_COMPLETE, c);
			}
		}

		c = c->link.next;
	}
}

// void HttpTunnel::chain_abort_cache_write(HttpProducer* p)
//
//    Terminates all cache writes.  Used to prevent truncated
//     documents from being stored in the cache
//
void
HttpTunnel::chain_abort_cache_write(HttpTunnelProducer * p)
{
	HttpTunnelConsumer *c = p->consumer_list.head;

	while (c) {

		if (c->alive) {

			  if (c->vc_type == HT_CACHE_WRITE) {

				ink_assert(c->self_producer == NULL);
				c->write_vio = NULL;
				c->vc->do_io_close(EHTTP_ERROR);
				c->alive = false;
				HTTP_DECREMENT_DYN_STAT(http_current_cache_connections_stat);

			  } else if (c->self_producer) {
				chain_abort_cache_write(c->self_producer);
			  }
		}

		c = c->link.next;
	}
}

// void HttpTunnel::close_vc(HttpTunnelProducer* p)
//
//    Closes the vc associated with the producer and
//      updates the state of the self_consumer
//
void
HttpTunnel::close_vc(HttpTunnelProducer * p)
{
	ink_assert(p->alive == false);
	HttpTunnelConsumer *c = p->self_consumer;

	if (c && c->alive) {
		c->alive = false;
		if (c->write_vio) {
		  c->bytes_written = c->write_vio->ndone;
		}
	}

	p->vc->do_io_close();
}

// void HttpTunnel::close_vc(HttpTunnelConsumer* c)
//
//    Closes the vc associated with the consumer and
//      updates the state of the self_producer
//
void
HttpTunnel::close_vc(HttpTunnelConsumer * c)
{
	ink_assert(c->alive == false);
	HttpTunnelProducer *p = c->self_producer;

	if (p && p->alive) {
		p->alive = false;
		if (p->read_vio) {
			  p->bytes_read = p->read_vio->ndone;
		}
	}

	c->vc->do_io_close();
}

// int HttpTunnel::main_handler(int event, void* data)
//
//   Main handler for the tunnel.  Vectors events
//   based on whether they are from consumers or
//   producers
//
int
HttpTunnel::main_handler(int event, void *data)
{
	HttpTunnelProducer *p = NULL;
	HttpTunnelConsumer *c = NULL;
	bool sm_callback = false;

	ink_assert(sm->magic == HTTP_SM_MAGIC_ALIVE);

	// Find the appropriate entry
	if ((p = get_producer((VIO *) data)) != 0) {
		sm_callback = producer_handler(event, p);
	} else {
		if ((c = get_consumer((VIO *) data)) != 0) {
			ink_assert(c->write_vio == (VIO *) data);
			sm_callback = consumer_handler(event, c);
		} else {
			internal_error();         // do nothing
		}
	}

	// We called a vc handler, the tunnel might be
	//  finished.  Check to see if there are any remaining
	//  VConnections alive.  If not, notifiy the state machine
	//
	if (sm_callback && !is_tunnel_alive()) {
		active = false;
		sm->handleEvent(HTTP_TUNNEL_EVENT_DONE, this);
		return EVENT_DONE;
	}
	return EVENT_CONT;
}

void
HttpTunnel::update_stats_after_abort(HttpTunnelType_t t)
{
	switch (t) {
	case HT_CACHE_READ:
	case HT_CACHE_WRITE:
		HTTP_DECREMENT_DYN_STAT(http_current_cache_connections_stat);
		break;
	default:
		// Handled here:
		// HT_HTTP_SERVER, HT_HTTP_CLIENT,
		// HT_TRANSFORM, HT_STATIC
		break;
	};
}

void
HttpTunnel::internal_error()
{
}


//YTS Team, yamsat Plugin
//Function to copy the partial Post data while tunnelling
void
HttpTunnel::copy_partial_post_data()
{
	postbuf->postdata_copy_buffer->write(postbuf->ua_buffer_reader);
	Debug("http_redirect", "[HttpTunnel::copy_partial_post_data] wrote %" PRId64" bytes to buffers %" PRId64"",
		postbuf->ua_buffer_reader->read_avail(), postbuf->postdata_copy_buffer_start->read_avail());
	postbuf->ua_buffer_reader->consume(postbuf->ua_buffer_reader->read_avail());
}

//YTS Team, yamsat Plugin
//Allocate a new buffer for static producers
void
HttpTunnel::allocate_redirect_postdata_producer_buffer()
{
	int64_t alloc_index = buffer_size_to_index(sm->t_state.hdr_info.request_content_length);

	ink_release_assert(postbuf->postdata_producer_buffer == NULL);

	postbuf->postdata_producer_buffer = new_MIOBuffer(alloc_index);
	postbuf->postdata_producer_reader = postbuf->postdata_producer_buffer->alloc_reader();
}

//YTS Team, yamsat Plugin
//Allocating the post data buffers
void
HttpTunnel::allocate_redirect_postdata_buffers(IOBufferReader * ua_reader)
{
	int64_t alloc_index = buffer_size_to_index(sm->t_state.hdr_info.request_content_length);

	Debug("http_redirect", "[HttpTunnel::allocate_postdata_buffers]");

	// TODO: This is uncool, shouldn't this use the class allocator or proxy allocator ?
	// If fixed, obviously also fix the deallocator.
	if (postbuf == NULL) {
		postbuf = new PostDataBuffers();
	}
	postbuf->ua_buffer_reader = ua_reader;
	postbuf->postdata_copy_buffer = new_MIOBuffer(alloc_index);
	postbuf->postdata_copy_buffer_start = postbuf->postdata_copy_buffer->alloc_reader();
	allocate_redirect_postdata_producer_buffer();
}


//YTS Team, yamsat Plugin
//Deallocating the post data buffers
void
HttpTunnel::deallocate_redirect_postdata_buffers()
{
	  Debug("http_redirect", "[HttpTunnel::deallocate_postdata_copy_buffers]");

	  if (postbuf != NULL) {
			if (postbuf->postdata_producer_buffer != NULL) {
				  free_MIOBuffer(postbuf->postdata_producer_buffer);
				  postbuf->postdata_producer_buffer = NULL;
				  postbuf->postdata_producer_reader = NULL; //deallocated by the buffer
			}
			if (postbuf->postdata_copy_buffer != NULL) {
				  free_MIOBuffer(postbuf->postdata_copy_buffer);
				  postbuf->postdata_copy_buffer = NULL;
				  postbuf->postdata_copy_buffer_start = NULL;       //deallocated by the buffer
			}
			delete postbuf;
			postbuf = NULL;
	  }
}

