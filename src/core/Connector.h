/*
 * connector.h
 *
 *  Created on: 2014年5月4日
 *      Author: jacky
 */

#ifndef CONNECTOR_H_
#define CONNECTOR_H_
#include <BaseARE/UTaskObj.h>
#include <BaseARE/SockStream.h>
#include "IOStream.h"

class Connector : public UTaskObj, IOStream{
public:
	Connector();
    ~Connector();

public:
    virtual uint32_t read(Request* r);
    virtual uint32_t write(Request* r);
    virtual int handle_open  (const URE_Msg& msg);
    virtual int handle_close (UWorkEnv * orign_uwe, long retcode);
    virtual int handle_input (URE_Handle h);
    uint32_t  Attach(int fd){ return m_stream.Attach(fd);}

private:
    SockStream m_stream;
};



#endif /* CONNECTOR_H_ */
