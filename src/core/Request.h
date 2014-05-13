/*
 * Request.h
 *
 *  Created on: 2014年5月9日
 *      Author: jacky
 */

#ifndef REQUEST_H_
#define REQUEST_H_
#include "Connector.h"
#include <BaseARE/UTaskObj.h>
/*
 *
 */
class Connector;
class Request : public UTaskObj {
public:
	Request(Connector* stream);
	virtual ~Request();

	virtual int32_t Parse(){return 0;}
	virtual int32_t AppendUnparsedBuffer(const char* buf, uint32_t length){return 0;}

protected:
	Connector* m_connector;
};

#endif /* REQUEST_H_ */
