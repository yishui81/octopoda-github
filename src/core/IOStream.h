/*
 * IOStream.h
 *
 *  Created on: 2014年5月9日
 *      Author: jacky
 */

#ifndef IOSTREAM_H_
#define IOSTREAM_H_

#include "Request.h"
/*
 *
 */
class IOStream {
public:
	IOStream();
	virtual ~IOStream();
public:
	virtual uint32_t read(Request* request) = 0;
	virtual uint32_t write(Request* request) = 0;
};

#endif /* IOSTREAM_H_ */
