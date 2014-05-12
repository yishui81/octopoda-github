/*
 * connector.cpp
 *
 *  Created on: 2014年5月4日
 *      Author: jacky
 */
#include "Connector.h"

Connector::Connector():m_request(NULL)
{

}

Connector::~Connector()
{

}

uint32_t Connector::read()
{
	char buffer[4096];
	int ret = 0;
	while((ret = m_stream.Recv(buffer,4096)) == 0){
		//TODO append buffer into  Request buffer
		return 0;
	}
	if(ret){
		//TODO
		LeaveWorkEnv(ret);
	}
	return 0;
}

uint32_t Connector::write()
{
//	int ret = 0;
//	while((ret = m_stream.Send(m_request->getOutput())) == 0){
//
//	}
	return 0;
}

int Connector::handle_open(const URE_Msg &msg)
{
	return 0;
}

int Connector::handle_close(UWorkEnv * orign_uwe, long retcode)
{
    return 0;
}

int Connector::handle_input(URE_Handle h)
{
    return 0;
}

