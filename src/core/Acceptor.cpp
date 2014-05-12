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

Acceptor::Acceptor():ip("0.0.0.0"),
		port(8080),
		backlog(8),
		tcpDefferAccept(false),
		tcpDefferTimeout(0)
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
    int32_t iRet = 0;

    SockAddr local(ip.c_str(), port);

    iRet = m_acceptor.Open(local, backlog);
    ///CHECK_GO(iRet >= 0, 0, ERROR_EXIT, "listen %s:%hu failed.", local.GetIpStr().c_str(), local.GetPort());
   if(tcpDefferAccept){
	   m_acceptor.DefferAccept(this->tcpDefferTimeout);
   }
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



