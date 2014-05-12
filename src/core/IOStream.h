/*
 * IOStream.h
 *
 *  Created on: 2014年5月9日
 *      Author: jacky
 */

#ifndef IOSTREAM_H_
#define IOSTREAM_H_

/*
 *
 */
class IOStream {
public:
	IOStream(){};
	virtual ~IOStream(){};
public:
	virtual uint32_t read() = 0;
	virtual uint32_t write() = 0;
};

#endif /* IOSTREAM_H_ */
