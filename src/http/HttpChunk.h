/*
 * HttpChunk.h
 *
 *  Created on: 2014年5月5日
 *      Author: jacky
 */

#ifndef HTTPCHUNK_H_
#define HTTPCHUNK_H_
#include "ts/I_IOBuffer.h"
/*
 *
 */
enum TunnelChunkingAction_t
{
  TCA_CHUNK_CONTENT,
  TCA_DECHUNK_CONTENT,
  TCA_PASSTHRU_CHUNKED_CONTENT,
  TCA_PASSTHRU_DECHUNKED_CONTENT
};

class ChunkedHandler
{
	enum ChunkedState
	{
		CHUNK_READ_CHUNK = 0,
		CHUNK_READ_SIZE_START,
		CHUNK_READ_SIZE,
		CHUNK_READ_SIZE_CRLF,
		CHUNK_READ_TRAILER_BLANK,
		CHUNK_READ_TRAILER_CR,
		CHUNK_READ_TRAILER_LINE,
		CHUNK_READ_ERROR,
		CHUNK_READ_DONE,
		CHUNK_WRITE_CHUNK,
		CHUNK_WRITE_DONE,
		CHUNK_FLOW_CONTROL
	};

public:
	ChunkedHandler();

	void init(IOBufferReader * buffer_in, HttpTunnelProducer * p);

	/// Set the max chunk @a size.
	/// If @a size is zero it is set to @c DEFAULT_MAX_CHUNK_SIZE.
	void set_max_chunk_size(int64_t size);

	// Returns true if complete, false otherwise
	bool process_chunked_content();
	bool generate_chunked_content();

	static int const DEFAULT_MAX_CHUNK_SIZE = 4096;


public:
	IOBufferReader *chunked_reader;
	MIOBuffer *dechunked_buffer;
	int64_t dechunked_size;

	IOBufferReader *dechunked_reader;
	MIOBuffer *chunked_buffer;
	int64_t chunked_size;

	bool truncation;
	int64_t skip_bytes;

	ChunkedState state;
	int64_t cur_chunk_size;
	int64_t bytes_left;
	int last_server_event;

	// Parsing Info
	int running_sum;
	int num_digits;

	/// @name Output data.
	//@{
	/// The maximum chunk size.
	/// This is the preferred size as well, used whenever possible.
	int64_t max_chunk_size;
	/// Caching members to avoid using printf on every chunk.
	/// It holds the header for a maximal sized chunk which will cover
	/// almost all output chunks.
	char max_chunk_header[16];
	int max_chunk_header_len;


private:
	void read_size();
	void read_chunk();
	void read_trailer();
	int64_t transfer_bytes();
};
#endif /* HTTPCHUNK_H_ */
