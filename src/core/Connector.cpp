/*
 * connector.cpp
 *
 *  Created on: 2014年5月4日
 *      Author: jacky
 */
#include "Connector.h"

Connector::Connector(){

}

Connector::~Connector()
{

}

uint32_t Connector::read(Request* r)
{
	u_char buffer[4096];
	int ret = 0;
	while((ret = m_stream.Recv(buffer,4096)) == 0){
		//TODO append buffer into  Request buffer
		return 0;
	}
	if(ret){
		//TODO
		LeaveWorkEnv(ret);
	}
}

uint32_t Connector::write(Request* r)
{
	int ret = 0;
	while((ret = m_stream.Send(r->getOutput())) == 0){

	}
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

