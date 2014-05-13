/*
 * HttpAcceptor.h
 *
 *  Created on: 2014年5月6日
 *      Author: jacky
 */

#ifndef HTTPACCEPTOR_H_
#define HTTPACCEPTOR_H_
#include <BaseARE/UTaskObj.h>
#include "Acceptor.h"
#include "ts/ink_inet.h"
#include "ts/ink_resolver.h"
/*
 *
 */
  class HttpAcceptOptions
 {
  public:
		HttpAcceptOptions();

		HttpAcceptOptions& setTransportType(int);

		HttpAcceptOptions& setListenIp(SockAddr & ip);

		HttpAcceptOptions& setListenPort(uint16_t);

		HttpAcceptOptions& setTransparent(bool);

		HttpAcceptOptions& setTransparentPassthrough(bool);

		HttpAcceptOptions& setBackdoor(bool);

		HttpAcceptOptions& setDefferAccept(int32_t timeout);

		HttpAcceptOptions& setBacklog(int32_t);

		HttpAcceptOptions& setKeepAlive(bool);

		HttpAcceptOptions& setKeepAlive(int64_t idle, int64_t interval, int32_t count);

		HttpAcceptOptions& setSendBufSize(int64_t size);

		HttpAcceptOptions& setRecvBufSize(int64_t size);

		HttpAcceptOptions& setSockReuse(bool resuse);

protected:
		//proxy type
		int32_t 	 transport_type;
		SockAddr	 listen_ip4;
		SockAddr 	 listen_ip6;
		uint16_t 	 listen_port;
		bool  		 transparent;
		bool 		 transparent_passthrough;
		bool 		 backdoor;
		int32_t        backlog;
		int32_t        tcpDefferTimeout;
		bool 		 keepalive;
		int64_t        keepIdle;
		int64_t        keepInterval;
		int32_t        keepProbeCount;
		int64_t        sendBufSize;
		int64_t        recvBufSize;
		bool           sockReuse;
  };


class HttpAcceptor : public Acceptor, private HttpAcceptOptions{
public:
		HttpAcceptor();
		virtual ~HttpAcceptor();

		virtual int32_t handle_open  (const URE_Msg& msg);
		virtual int32_t handle_close (UWorkEnv * orign_uwe, long retcode);
		virtual int32_t handle_input (URE_Handle h);

};

#endif /* HTTPACCEPTOR_H_ */
