/*
 * HttpAcceptor.h
 *
 *  Created on: 2014年5月6日
 *      Author: jacky
 */

#ifndef HTTPACCEPTOR_H_
#define HTTPACCEPTOR_H_
#include <BaseARE/UTaskObj.h>
#include <core/Acceptor.h>
/*
 *
 */
class HttpAcceptor : public Acceptor{
public:
	HttpAcceptor();
	virtual ~HttpAcceptor();

    virtual int handle_open  (const URE_Msg& msg);
    virtual int handle_close (UWorkEnv * orign_uwe, long retcode);
    virtual int handle_input (URE_Handle h);

};

#endif /* HTTPACCEPTOR_H_ */
