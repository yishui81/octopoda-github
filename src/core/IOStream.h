/*
 * IOStream.h
 *
 *  Created on: 2014年5月9日
 *      Author: jacky
 */

#ifndef IOSTREAM_H_
#define IOSTREAM_H_
#include <stdint.h>
/*
 *
 */
class IOStream {
public:
	IOStream(){};
	virtual ~IOStream(){};
public:
	virtual int32_t read() = 0;
	virtual int32_t write() = 0;
};

#endif /* IOSTREAM_H_ */
