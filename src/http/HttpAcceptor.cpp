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
#include "BaseARE/SockBase.h"
#include "HttpConnector.h"
#include "HttpClientSession.h"


inline HttpAcceptOptions::HttpAcceptOptions()
				: transport_type(0)
				, listen_port(0)
				, transparent(false)
				, transparent_passthrough(false)
				, backdoor(false)
				, backlog(8)
				, tcpDefferTimeout(0)
				, keepalive(true)
				, keepIdle(0)
				, keepInterval(0)
				, keepProbeCount(0)
				, sendBufSize(0)
				, recvBufSize(0)
				, sockReuse(false)
{
	//memcpy(host_res_preference, host_res_default_preference_order, sizeof(host_res_preference));
}

inline HttpAcceptOptions&
HttpAcceptOptions::setTransportType(int type)
{
	transport_type =  type;
	return *this;
}

inline HttpAcceptOptions&
HttpAcceptOptions::setListenIp(SockAddr& ip)
{
	if (!ip.IsIPv6()){
		listen_ip4 = ip;
	}else {
		listen_ip6 = ip;
	}
	return *this;
}

inline HttpAcceptOptions&
HttpAcceptOptions::setListenPort(uint16_t port)
{
	listen_port = port;
	return *this;
}

inline HttpAcceptOptions&
HttpAcceptOptions::setTransparent(bool flag)
{
	transparent = flag;
	return *this;
}

inline HttpAcceptOptions&
HttpAcceptOptions::setTransparentPassthrough(bool flag)
{
	transparent_passthrough = flag;
	return *this;
}

inline HttpAcceptOptions&
HttpAcceptOptions::setBackdoor(bool flag)
{
	backdoor = flag;
	return *this;
}

//inline HttpAcceptOptions&
//HttpAcceptOptions::setHostResPreference(HostResPreferenceOrder const order)
//{
//	memcpy(host_res_preference, order, sizeof(host_res_preference));
//	return *this;
//}

inline HttpAcceptOptions&
HttpAcceptOptions::setDefferAccept(int32_t timeout)
{
	tcpDefferTimeout = timeout;
	return *this;
}

inline HttpAcceptOptions&
HttpAcceptOptions::setBacklog(int32_t backlog)
{
	this->backlog = backlog;
	return *this;
}

inline HttpAcceptOptions&
HttpAcceptOptions::setKeepAlive(bool keepalive)
{
	this->keepalive = keepalive;
	return *this;
}

inline HttpAcceptOptions&
HttpAcceptOptions::setKeepAlive(int64_t idle, int64_t interval, int32_t count)
{
	this->keepIdle = idle;
	this->keepInterval = interval;
	this->keepProbeCount = count;
	return *this;
}

inline HttpAcceptOptions&
HttpAcceptOptions::setSendBufSize(int64_t size)
{
	this->sendBufSize = size;
	return *this;
}

inline HttpAcceptOptions&
HttpAcceptOptions::setRecvBufSize(int64_t size)
{
	this->recvBufSize = size;
	return *this;
}


HttpAcceptor::HttpAcceptor()
{

}

HttpAcceptor::~HttpAcceptor()
{

}

int32_t
HttpAcceptor::handle_open(const URE_Msg &msg)
{
    int32_t iRet = 0;

    iRet = m_acceptor.Open(this->listen_ip4, backlog);

   if(tcpDefferTimeout){
	   m_acceptor.DefferAccept(this->tcpDefferTimeout);
   }

    register_handler(m_acceptor.GetHandle(), READ_MASK);
    return 0;
}

int32_t
HttpAcceptor::handle_close(UWorkEnv * orign_uwe, long retcode)
{
	remove_handler(m_acceptor.GetHandle(), READ_MASK);
	leave_uwe(retcode);
    	return 0;
}

int32_t
HttpAcceptor::handle_input(URE_Handle h)
{
	if( h == m_acceptor.GetHandle() )
	{
		SockAddr addr;
		sockfd_t  fd = INVALID_SOCKET_HANDLE;

		while( ( fd = m_acceptor.Accept( &addr )) != INVALID_SOCKET_HANDLE ) {

			//(1) 设置连接的基本参数
			//TODO allocatte HttpConnector for mempool
			 Connector* conn = new HttpConnector();
			 conn->set_remote_addr(addr);
			 conn->Attach(fd);

			////////////////////////////////////////////////////
			// if client address forbidden, close immediately //
			////////////////////////////////////////////////////
			 //(2) client address forbidden
			 //TODO
			 //if(add.client_ip  is forbidden){
			 //		conn.close();
			 //}

			 //(3) copy over session related data
			HttpClientSession *new_session = HttpClientSession::allocate();

			new_session->outbound_transparent = transparent;
			new_session->transparent_passthrough = transparent_passthrough;
			new_session->outbound_ip4 = this->listen_ip4;
			new_session->outbound_ip6 = this->listen_ip6;
			new_session->outbound_port = this->listen_port;

			//new_session->host_res_style = ats_host_res_from(client_ip->sa_family, host_res_preference);
			//new_session->acl_method_mask = acl_method_mask;
			new_session->attach_connector(conn, backdoor);

			//(4) Connection Task进入UV
			conn->EnterWorkEnv(this->GetWorkEnv());
		}
	}
	return 0;
}
