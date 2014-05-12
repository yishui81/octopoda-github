/*
 * acceptor.cpp
 *
 *  Created on: 2014年5月4日
 *      Author: jacky
 */


#include "Acceptor.h"
#include "Connector.h"
#include "Octopoda.h"
#include <sys/types.h>
#include <BaseARE/SockBase.h>

Acceptor::Acceptor()
{
   // PR_DEBUG("listener uto ctor!");
}

Acceptor::~Acceptor()
{
	m_acceptor.Close();
}

int32_t Acceptor::handle_open(const URE_Msg &msg)
{
    int32_t iRet = 0;

    SockAddr local(conf.ip.c_str(), conf.port);

    iRet = m_acceptor.Open(local, 100000);
    ///CHECK_GO(iRet >= 0, 0, ERROR_EXIT, "listen %s:%hu failed.", local.GetIpStr().c_str(), local.GetPort());

    register_handler(m_acceptor.GetHandle(), READ_MASK);

    return 0;
}

int32_t Acceptor::handle_close(UWorkEnv * orign_uwe, long retcode)
{
	remove_handler(m_acceptor.GetHandle(), READ_MASK);
	leave_uwe(retcode);
    	return 0;
}

int32_t Acceptor::handle_input(URE_Handle h)
{
    SockAddr scAddr;
    sockfd_t   confd = -1;


	if( h == m_acceptor.GetHandle() ) {

		SockAddr addr;
		sockfd_t  fd = INVALID_SOCKET_HANDLE;

		while( ( fd = m_acceptor.Accept( &addr )) != INVALID_SOCKET_HANDLE ) {
				 Connector* conn = new Connector();
				 conn->Attach(fd);
				 conn->EnterWorkEnv(GetWorkEnv());
		}
	}

    return 0;
}



