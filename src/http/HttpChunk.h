/*
 * HttpChunk.h
 *
 *  Created on: 2014年5月5日
 *      Author: jacky
 */

#ifndef HTTPCHUNK_H_
#define HTTPCHUNK_H_

/*
 *
 */
class HttpChunkContext {
public:
	HttpChunkContext();
	virtual ~HttpChunkContext();
private:
    int          	   state;
    off_t                size;
    off_t                length;
};

#endif /* HTTPCHUNK_H_ */
