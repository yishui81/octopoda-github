/*
 * Request.h
 *
 *  Created on: 2014年5月9日
 *      Author: jacky
 */

#ifndef REQUEST_H_
#define REQUEST_H_
#include "IOStream.h"
#include <BaseARE/UTaskObj.h>
/*
 *
 */
class Request : public UTaskObj {
public:
	Request(IOStream* stream);
	virtual ~Request();

	virtual int32_t Parse(){return 0;};
	virtual int32_t AppendUnparsedBuffer(const u_char* buf, uint32_t length){return 0;};

private:
	IOStream* m_stream;
};

#endif /* REQUEST_H_ */
