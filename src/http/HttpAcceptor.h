/*
 * HttpAcceptor.h
 *
 *  Created on: 2014年5月6日
 *      Author: jacky
 */

#ifndef HTTPACCEPTOR_H_
#define HTTPACCEPTOR_H_
#include <BaseARE/UTaskObj.h>
#include "Acceptor.h"
/*
 *
 */
class HttpAcceptor : public Acceptor{
public:
		HttpAcceptor();
		virtual ~HttpAcceptor();
		virtual int handle_input (URE_Handle h);

};

#endif /* HTTPACCEPTOR_H_ */
