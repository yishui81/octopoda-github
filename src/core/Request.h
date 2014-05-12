/*
 * Request.h
 *
 *  Created on: 2014年5月9日
 *      Author: jacky
 */

#ifndef REQUEST_H_
#define REQUEST_H_
#include "IOStream.h"
/*
 *
 */
class Request : public UTaskObj {
public:
	Request(IOStream* stream);
	virtual ~Request();

private:
	IOStream* m_stream;
};

#endif /* REQUEST_H_ */
