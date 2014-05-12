/*
 * Connection.h
 *
 *  Created on: 2014年5月5日
 *      Author: jacky
 */

#ifndef CONNECTION_H_
#define CONNECTION_H_
#include <BaseARE/UTaskObj.h>
#include "IOStream.h"

/*
 *
 */
class Connection : public UTaskObj, IOStream {
public:
	Connection();
	virtual ~Connection();
};

#endif /* CONNECTION_H_ */
