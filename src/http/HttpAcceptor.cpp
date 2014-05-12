/*
 * HttpAcceptor.cpp
 *
 *  Created on: 2014年5月6日
 *      Author: jacky
 */

#include "HttpAcceptor.h"
#include "HttpConnector.h"
#include "Acceptor.h"
#include "Connector.h"
#include <sys/types.h>
#include <BaseARE/SockBase.h>


//extern oc_configure_t conf;

HttpAcceptor::HttpAcceptor() {

}

HttpAcceptor::~HttpAcceptor() {

}


int HttpAcceptor::handle_input(URE_Handle h)
{
	if( h == m_acceptor.GetHandle() ) {

		SockAddr addr;
		sockfd_t  fd = INVALID_SOCKET_HANDLE;

		while( ( fd = m_acceptor.Accept( &addr )) != INVALID_SOCKET_HANDLE ) {
				 Connector* conn = new HttpConnector();
				 conn->Attach(fd);
				 conn->EnterWorkEnv(this->GetWorkEnv());
		}
	}
	return 0;
}
