/*
 * acceptor.cpp
 *
 *  Created on: 2014年5月4日
 *      Author: jacky
 */


#include "Acceptor.h"
#include "Connector.h"
#include <sys/types.h>
#include <BaseARE/SockBase.h>

Acceptor::Acceptor()
{
	 // PR_DEBUG("listener uto ctor!");
	conns.clear();
}

Acceptor::~Acceptor()
{
	m_acceptor.Close();
}

int32_t Acceptor::handle_open(const URE_Msg &msg)
{

    return 0;
}

int32_t Acceptor::handle_close(UWorkEnv * orign_uwe, long retcode)
{
	return 0;
}

int32_t Acceptor::handle_input(URE_Handle h)
{

    return 0;
}



