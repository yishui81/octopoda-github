/*
 * HttpChunk.cpp
 *
 *  Created on: 2014年5月5日
 *      Author: jacky
 */

#include "HttpChunk.h"
#include "Connector.h"
#include <stdio.h>

ChunkedHandler::ChunkedHandler()
		  : chunked_reader(NULL),
			dechunked_buffer(NULL),
			dechunked_size(0),
			dechunked_reader(NULL),
			chunked_buffer(NULL),
			chunked_size(0),
			truncation(false),
			skip_bytes(0),
			state(CHUNK_READ_CHUNK),
			cur_chunk_size(0),
			bytes_left(0),
			last_server_event(CON_EVENT_NONE),
			running_sum(0),
			num_digits(0),
			max_chunk_size(DEFAULT_MAX_CHUNK_SIZE),
			max_chunk_header_len(0)
{

}

void
ChunkedHandler::init(IOBufferReader * buffer_in, HttpTunnelProducer * p)
{
	running_sum = 0;
	num_digits = 0;
	cur_chunk_size = 0;
	bytes_left = 0;
	truncation = false;

	if (p->do_chunking) {

		dechunked_reader = buffer_in->mbuf->clone_reader(buffer_in);
		dechunked_reader->mbuf->water_mark = min_block_transfer_bytes;
		chunked_buffer = new_MIOBuffer(CHUNK_IOBUFFER_SIZE_INDEX);
		chunked_size = 0;

	} else {

		ink_assert(p->do_dechunking || p->do_chunked_passthru);
		chunked_reader = buffer_in->mbuf->clone_reader(buffer_in);

		if (p->do_dechunking) {
		  // This is the min_block_transfer_bytes value.
		  dechunked_buffer = new_MIOBuffer(BUFFER_SIZE_INDEX_256);
		  dechunked_size = 0;
		}
	}
}

void
ChunkedHandler::set_max_chunk_size(int64_t size)
{
  max_chunk_size = size ? size : DEFAULT_MAX_CHUNK_SIZE;
  max_chunk_header_len = snprintf(max_chunk_header, sizeof(max_chunk_header), CHUNK_HEADER_FMT, max_chunk_size);
}

void
ChunkedHandler::read_size()
{
	int64_t bytes_used;
	bool done = false;

	while (chunked_reader->read_avail() > 0 && !done) {
		const char *tmp = chunked_reader->start();
		int64_t data_size = chunked_reader->block_read_avail();

		ink_assert(data_size > 0);
		bytes_used = 0;

		while (data_size > 0) {
			bytes_used++;
			if (state == CHUNK_READ_SIZE) {
				// The http spec says the chunked size is always in hex
				if (ParseRules::is_hex(*tmp)) {
					num_digits++;
					running_sum *= 16;

					if (ParseRules::is_digit(*tmp)) {
						running_sum += *tmp - '0';
					} else {
						running_sum += ParseRules::ink_tolower(*tmp) - 'a' + 10;
					}
				} else {
					// We are done parsing size
					if (num_digits == 0 || running_sum < 0) {
						// Bogus chunk size
						state = CHUNK_READ_ERROR;
						done = true;
						break;
					} else {
						state = CHUNK_READ_SIZE_CRLF;       // now look for CRLF
					}
				}
			} else if (state == CHUNK_READ_SIZE_CRLF) {       // Scan for a linefeed
				if (ParseRules::is_lf(*tmp)) {
					Debug("http_chunk", "read chunk size of %d bytes", running_sum);
					bytes_left = (cur_chunk_size = running_sum);
					state = (running_sum == 0) ? CHUNK_READ_TRAILER_BLANK : CHUNK_READ_CHUNK;
					done = true;
					break;
				}
			} else if (state == CHUNK_READ_SIZE_START) {
				if (ParseRules::is_lf(*tmp)) {
					running_sum = 0;
					num_digits = 0;
					state = CHUNK_READ_SIZE;
				}
			}
			tmp++;
			data_size--;
		}
		chunked_reader->consume(bytes_used);
	}
}

// int ChunkedHandler::transfer_bytes()
//
//   Transfer bytes from chunked_reader to dechunked buffer
//   Use block reference method when there is a sufficient
//   size to move.  Otherwise, uses memcpy method
//
int64_t
ChunkedHandler::transfer_bytes()
{
	int64_t block_read_avail, moved, to_move, total_moved = 0;

	// Handle the case where we are doing chunked passthrough.
	if (!dechunked_buffer) {
		moved = MIN(bytes_left, chunked_reader->read_avail());
		chunked_reader->consume(moved);
		bytes_left = bytes_left - moved;
		return moved;
	}

	while (bytes_left > 0) {
		block_read_avail = chunked_reader->block_read_avail();
		moved = 0;

		to_move = MIN(bytes_left, block_read_avail);
		if (to_move <= 0){
			break;
		}

		if (to_move >= min_block_transfer_bytes) {
			moved = dechunked_buffer->write(chunked_reader, bytes_left);
		} else {
			// Small amount of data available.  We want to copy the
			// data rather than block reference to prevent the buildup
			// of too many small blocks which leads to stack overflow
			// on deallocation
			moved = dechunked_buffer->write(chunked_reader->start(), to_move);
		}

		if (moved > 0) {
			chunked_reader->consume(moved);
			bytes_left = bytes_left - moved;
			dechunked_size += moved;
			total_moved += moved;
		} else {
			break;
		}
	}
	return total_moved;
}

void
ChunkedHandler::read_chunk()
{
	int64_t b = transfer_bytes();

	ink_assert(bytes_left >= 0);

	if (bytes_left == 0) {

		Debug("http_chunk", "completed read of chunk of %" PRId64" bytes", cur_chunk_size);
		state = CHUNK_READ_SIZE_START;

	} else if (bytes_left > 0) {

		Debug("http_chunk", "read %" PRId64" bytes of an %" PRId64" chunk", b, cur_chunk_size);

	}
}

void
ChunkedHandler::read_trailer()
{
	int64_t bytes_used;
	bool done = false;

	while (chunked_reader->is_read_avail_more_than(0) && !done) {

		const char *tmp = chunked_reader->start();
		int64_t data_size = chunked_reader->block_read_avail();

		ink_assert(data_size > 0);

		for (bytes_used = 0; data_size > 0; data_size--) {

		  bytes_used++;

		  if (ParseRules::is_cr(*tmp)) {
				// For a CR to signal we are almost done, the preceding
				//  part of the line must be blank and next character
				//  must a LF
				state = (state == CHUNK_READ_TRAILER_BLANK) ? CHUNK_READ_TRAILER_CR : CHUNK_READ_TRAILER_LINE;
		  } else if (ParseRules::is_lf(*tmp)) {
				// For a LF to signal we are done reading the
				//   trailer, the line must have either been blank
				//   or must have have only had a CR on it
				if (state == CHUNK_READ_TRAILER_CR || state == CHUNK_READ_TRAILER_BLANK) {
				  state = CHUNK_READ_DONE;
				  Debug("http_chunk", "completed read of trailers");
				  done = true;
				  break;
				} else {
				  // A LF that does not terminate the trailer
				  //  indicates a new line
				  state = CHUNK_READ_TRAILER_BLANK;
				}
		  } else {
				// A character that is not a CR or LF indicates
				//  the we are parsing a line of the trailer
				state = CHUNK_READ_TRAILER_LINE;
		  }
		  tmp++;
		}
		chunked_reader->consume(bytes_used);
	}
}

bool ChunkedHandler::process_chunked_content()
{
	while (chunked_reader->is_read_avail_more_than(0) && state != CHUNK_READ_DONE && state != CHUNK_READ_ERROR) {
		switch (state) {
		case CHUNK_READ_SIZE:
		case CHUNK_READ_SIZE_CRLF:
		case CHUNK_READ_SIZE_START:
			  read_size();
			  break;
		case CHUNK_READ_CHUNK:
			  read_chunk();
			  break;
		case CHUNK_READ_TRAILER_BLANK:
		case CHUNK_READ_TRAILER_CR:
		case CHUNK_READ_TRAILER_LINE:
			  read_trailer();
			  break;
		case CHUNK_FLOW_CONTROL:
			  return false;
		default:
			  ink_release_assert(0);
			  break;
	}
}
  return (state == CHUNK_READ_DONE || state == CHUNK_READ_ERROR);
}

bool ChunkedHandler::generate_chunked_content()
{
	char tmp[16];
	bool server_done = false;
	int64_t r_avail;

	ink_assert(max_chunk_header_len);

	switch (last_server_event) {
		case CON_EVENT_EOS:
		case CON_EVENT_READ_COMPLETE:
		case HTTP_TUNNEL_EVENT_PRECOMPLETE:
			server_done = true;
			break;
	}

	while ((r_avail = dechunked_reader->read_avail()) > 0
				&& state != CHUNK_WRITE_DONE) {
		int64_t write_val = MIN(max_chunk_size, r_avail);

		state = CHUNK_WRITE_CHUNK;
		Debug("http_chunk", "creating a chunk of size %" PRId64 " bytes", write_val);

		// Output the chunk size.
		if (write_val != max_chunk_size) {
			  int len = snprintf(tmp, sizeof(tmp), CHUNK_HEADER_FMT, write_val);
			  chunked_buffer->write(tmp, len);
			  chunked_size += len;
		} else {
			  chunked_buffer->write(max_chunk_header, max_chunk_header_len);
			  chunked_size += max_chunk_header_len;
		}

		// Output the chunk itself.
		//
		// BZ# 54395 Note - we really should only do a
		//   block transfer if there is sizable amount of
		//   data (like we do for the case where we are
		//   removing chunked encoding in ChunkedHandler::transfer_bytes()
		//   However, I want to do this fix with as small a risk
		//   as possible so I'm leaving this issue alone for
		//   now
		//
		chunked_buffer->write(dechunked_reader, write_val);
		chunked_size += write_val;
		dechunked_reader->consume(write_val);

		// Output the trailing CRLF.
		chunked_buffer->write("\r\n", 2);
		chunked_size += 2;
	}

	if (server_done) {
		state = CHUNK_WRITE_DONE;

		// Add the chunked transfer coding trailer.
		chunked_buffer->write("0\r\n\r\n", 5);
		chunked_size += 5;
		return true;
	}
	return false;
}


