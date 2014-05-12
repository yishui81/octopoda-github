/*
 * HttpAcceptor.cpp
 *
 *  Created on: 2014年5月6日
 *      Author: jacky
 */

#include "HttpAcceptor.h"
#include "Acceptor.h"
#include "Connector.h"
#include "octopoda.h"
#include <sys/types.h>
#include <BaseARE/SockBase.h>
extern oc_configure_t conf;

HttpAcceptor::HttpAcceptor() {

}

HttpAcceptor::~HttpAcceptor() {

}


int HttpAcceptor::handle_open(const URE_Msg &msg)
{
    int32_t iRet = 0;

    SockAddr local(conf.ip.c_str(), conf.port);

    iRet = m_acceptor.Open(local, 100000);
    ///CHECK_GO(iRet >= 0, 0, ERROR_EXIT, "listen %s:%hu failed.", local.GetIpStr().c_str(), local.GetPort());

    register_handler(m_acceptor.GetHandle(), READ_MASK);

    return 0;
}

int HttpAcceptor::handle_close(UWorkEnv * orign_uwe, long retcode)
{
    return leave_uwe(0);
}

int HttpAcceptor::handle_input(URE_Handle h)
{
	if( h == m_acceptor.GetHandle() ) {

		SockAddr addr;
		sockfd_t  fd = INVALID_SOCKET_HANDLE;

		while( ( fd = m_acceptor.Accept( &addr )) != INVALID_SOCKET_HANDLE ) {
				 HttpConnector* conn = new HttpConnector();
				 conn->Attach(fd);
				 conn->EnterWorkEnv(GetWorkEnv());
		}
	}
	return 0;
}
