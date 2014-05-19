/*
 * connector.cpp
 *
 *  Created on: 2014年5月4日
 *      Author: jacky
 */
#include "Connector.h"
#include <sys/socket.h>
#include "ink_sock.h"

Connector::Connector()
		: inactivity_timeout_in(0),
		  active_timeout_in(0),
		  next_inactivity_timeout_at(0),
		  recursion(0),
		  submit_time(0),
		  id(0),
		  options(),
		  m_stream(),
		  closed(0)
{
	  memset(&m_local_addr, 0, sizeof m_local_addr);
	  memset(&m_remote_addr, 0, sizeof m_remote_addr);
}

Connector::~Connector()
{
	this->m_stream.Close();
}

int32_t Connector::read(int64_t nbytes, MIOBuffer *buf)
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

int32_t Connector::write(int64_t nbytes, IOBufferReader *buf, bool owner = false)
{
	int32_t  ret = 0;
//	while((ret = m_stream.Send(m_request->getOutput())) == 0){
//
//	}

//	  ink_assert(!closed);
//	  write.vio.op = VIO::WRITE;
//	  write.vio.mutex = c->mutex;
//	  write.vio._cont = c;
//	  write.vio.nbytes = nbytes;
//	  write.vio.ndone = 0;
//	  write.vio.vc_server = (VConnection *) this;
//	  if (reader) {
//	    ink_assert(!owner);
//	    write.vio.buffer.reader_for(reader);
//	    if (nbytes && !write.enabled)
//	      write.vio.reenable();
//	  } else {
//	    disable_write(this);
//	  }
//	  return &write.vio;
	return ret;
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


int32_t Connector::shutdown(int32_t  howto)
{

	int32_t fd = m_stream.GetHandle();
	int32_t  ret = 0;
	switch (howto) {

	case IO_SHUTDOWN_READ:
		ret = ::shutdown(fd, SHUT_RD);
		break;

	case IO_SHUTDOWN_WRITE:
		ret = ::shutdown(fd, SHUT_WR);
		break;

	case IO_SHUTDOWN_READWRITE:
		ret = ::shutdown(fd, SHUT_RDWR);
		break;

	default:
		//ink_assert(!"not reached");
	}

	return ret;

}

int32_t Connector::close(int32_t err_code)
{
	return this->shutdown(IO_SHUTDOWN_READWRITE);
}

int32_t Connector::apply_options(){

	int32_t fd = m_stream.GetHandle();

	if(options.sockopt_flags & ConnectorOptions::SOCK_OPT_NO_DELAY ){
		safe_setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, SOCKOPT_ON, sizeof(int));
			//TODO log
	}

	if (options.sockopt_flags & ConnectorOptions::SOCK_OPT_KEEP_ALIVE) {
		safe_setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, SOCKOPT_ON, sizeof(int));
		//Debug("socket", "::open: setsockopt() SO_KEEPALIVE on socket");
	}


//	#if TS_HAS_SO_MARK
	uint32_t mark = options.packet_mark;
	if(mark){
		safe_setsockopt(fd, SOL_SOCKET, SO_MARK, reinterpret_cast<char *>(&mark), sizeof(uint32_t));
		//DEBUG TODO
	}
//	#endif

//	#if TS_HAS_IP_TOS
	uint32_t tos = options.packet_tos;
	if(tos){
		safe_setsockopt(fd, IPPROTO_IP, IP_TOS, reinterpret_cast<char *>(&tos), sizeof(uint32_t));
		//DEBUG TODO
	}
//	#endif
	return 0;
}

int32_t Connector::set_tcp_init_cwnd(int32_t desired_cwnd){
#ifdef TCP_INIT_CWND
	int32_t   rv = 0;
	uint32_t val = desired_cwnd;
	rv = setsockopt(con.fd, IPPROTO_TCP, TCP_INIT_CWND, &val, sizeof(val));
	//Debug("socket", "Setting TCP initial congestion window (%d) -> %d", init_cwnd, rv);
	//TODO log
	return rv;
#else
	//FIXME
 // Debug("socket", "Setting TCP initial congestion window %d -> unsupported", init_cwnd);
  return -1;
#endif
}

