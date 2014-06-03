/*
 * connector.h
 *
 *  Created on: 2014年5月4日
 *      Author: jacky
 */

#ifndef CONNECTOR_H_
#define CONNECTOR_H_

#include <BaseARE/UTaskObj.h>
#include <BaseARE/SockStream.h>
#include "IOStream.h"
#include "Request.h"

#define EVENT_NONE                0       // 0
#define EVENT_IMMEDIATE       1
#define EVENT_INTERVAL          2
#define EVENT_ERROR              3
#define EVENT_CALL                 4     // used internally in state machines
#define EVENT_POLL                 5     // negative event; activated on poll or epoll

// Event callback return functions

#define EVENT_DONE                0     // 0
#define EVENT_CONT                1     // 1
#define EVENT_RETURN              5
#define EVENT_RESTART             6
#define EVENT_RESTART_DELAYED     7



#define CON_EVENT_NONE                  	0
#define CON_EVENT_IMMEDIATE               EVENT_IMMEDIATE

#define CON_EVENT_EVENTS_START            100
#define CON_EVENT_READ_READY              CON_EVENT_EVENTS_START

/**
  Any data in the accociated buffer *will be written* when the
  Continuation returns.

*/
#define	CON_EVENT_WRITE_READY             (CON_EVENT_READ_READY+1)
#define	CON_EVENT_READ_COMPLETE           (CON_EVENT_READ_READY+2)
#define	CON_EVENT_WRITE_COMPLETE          (CON_EVENT_READ_READY+3)

/**
  No more data (end of stream). It should be interpreted by a
  protocol engine as either a COMPLETE or ERROR.

*/
#define	CON_EVENT_EOS                     (CON_EVENT_READ_READY+4)

#define	CON_EVENT_ERROR                   EVENT_ERROR

/**
  VC_EVENT_INACTIVITY_TIMEOUT indiates that the operation (read or write) has:
    -# been enabled for more than the inactivity timeout period
       (for a read, there has been space in the buffer)
       (for a write, there has been data in the buffer)
    -# no progress has been made
       (for a read, no data has been read from the connection)
       (for a write, no data has been written to the connection)

*/
#define	CON_EVENT_INACTIVITY_TIMEOUT      (CON_EVENT_EVENTS_START+5)

/**
  Total time for some operation has been exeeded, regardless of any
  intermediate progress.

*/
#define	CON_EVENT_ACTIVE_TIMEOUT          (CON_EVENT_EVENTS_START+6)

#define	CON_EVENT_OOB_COMPLETE            (CON_EVENT_EVENTS_START+7)


enum SHUTDOWN_TYPE
{
	IO_SHUTDOWN_READ = 0,
	IO_SHUTDOWN_WRITE,
	IO_SHUTDOWN_READWRITE
};


enum ip_protocol_t {
	USE_TCP, ///< TCP protocol.
	USE_UDP ///< UDP protocol.
};

enum addr_bind_style {
	ANY_ADDR, ///< Bind to any available local address (don't care, default).
	INTF_ADDR, ///< Bind to interface address in @a local_addr.
	FOREIGN_ADDR ///< Bind to foreign address in @a local_addr.
};



class ConnectorOptions {

public:
	static uint32_t const SOCK_OPT_NO_DELAY = 1;
	static uint32_t const SOCK_OPT_KEEP_ALIVE = 2;


public:
	ConnectorOptions() {
		reset();
	}

	~ConnectorOptions(){

	}

private:

	SockAddr			local_ip;
	ip_protocol_t 		ip_proto;
	addr_bind_style 	addr_binding;

	bool			f_blocking;
	bool			f_blocking_connect;

	uint16_t       ip_family;
	uint16_t 		local_port;
	int32_t			socket_recv_bufsize;
	int32_t			socket_send_bufsize;
	uint32_t 		sockopt_flags;
	uint32_t 		packet_mark;
	uint32_t 		packet_tos;

public:

	void
	ConnectorOptions::reset()
	{
		ip_proto = USE_TCP;
		ip_family = AF_INET;
		//local_ip.invalidate();
		local_port = 0;
		addr_binding = ANY_ADDR;
		f_blocking = false;
		f_blocking_connect = false;
		socket_recv_bufsize = 0;
		socket_send_bufsize = 0;
		sockopt_flags = 0;
		packet_mark = 0;
		packet_tos = 0;
	}

	inline void
	set_sock_param(int _recv_bufsize, int _send_bufsize, unsigned long _opt_flags,
	                             unsigned long _packet_mark, unsigned long _packet_tos)
	{
		socket_recv_bufsize = _recv_bufsize;
		socket_send_bufsize = _send_bufsize;
		sockopt_flags = _opt_flags;
		packet_mark = _packet_mark;
		packet_tos = _packet_tos;
	}

	inline void
	set_recv_bufsize(int32_t recv_bufsize){
		socket_recv_bufsize = recv_bufsize;
	}

	inline int32_t get_recv_bufsize(){
		return socket_recv_bufsize;
	}

	inline void
	set_send_bufsize(int32_t send_bufsize){
		socket_send_bufsize = send_bufsize;
	}

	inline int32_t
	get_send_bufsize(){
		return socket_send_bufsize;
	}

	inline void
	set_socket_option_flags(uint64_t flags){
		sockopt_flags = flags;
	}

	inline uint64_t get_sock_option_flags(){
		return sockopt_flags;
	}

	inline void
	set_packet_mark(uint64_t  _packet_mark){
		packet_mark = _packet_mark;
	}

	inline uint64_t get_packet_mark(){
		return packet_mark;
	}

	inline void
	set_packet_tos(uint64_t _packet_tos){
		packet_tos = _packet_tos;
	}

	inline uint64_t get_packet_tos(){
		return packet_tos;
	}

	inline void
	set_ip_protocol(ip_protocol_t _protocol){
		ip_proto = _protocol;
	}

	inline ip_protocol_t
	get_ip_protocol(){
		return ip_proto;
	}

	inline void
	set_ip_family(uint16_t _ip_family){
		ip_family = _ip_family;
	}

	inline uint16_t
	get_ip_family(){
		return ip_family;
	}

	inline void
	set_local_ip(SockAddr& _local_ip){
		local_ip = _local_ip;
	}

	inline SockAddr&
	get_local_ip(){
		return local_ip;
	}

	inline void
	set_local_port(uint16_t _local_port){
		local_port = _local_port;
	}

	inline uint16_t
	get_local_port(){
		return local_port;
	}

	inline void
	set_addr_bind_systle(addr_bind_style _binding_style){
		addr_binding = _binding_style;
	}

	inline addr_bind_style
	get_addr_bind_systle(){
		return addr_binding;
	}
	inline void
	set_blocking_flag(bool _blocking){
		f_blocking = _blocking;
	}

	inline void
	set_blocking_connect(bool _blocking_connect){
		f_blocking_connect = _blocking_connect;
	}

	inline bool
	get_blocking_connect(){
		return f_blocking_connect;
	}

};

class Connector : public UTaskObj{

public:
	Connector();
	virtual ~Connector();

public:

	virtual int handle_open  (const URE_Msg& msg);
	virtual int handle_close (UWorkEnv * orign_uwe, long retcode);
	virtual int handle_input (URE_Handle h);
	virtual int handle_output(URE_Handle h);
	virtual int handle_message(URE_Msg& msg);

	virtual int32_t 	do_io_read(UTaskObj* obj, int64_t nbytes, MIOBuffer *buf);
	virtual int32_t 	do_io_write(UTaskObj* obj, int64_t nbytes, IOBufferReader *buf, bool owner = false);
	virtual int32_t  	do_io_close(int32_t lerrno = -1);
	virtual int32_t  	do_io_shutdown(int32_t howto);
	virtual int32_t 	set_tcp_init_cwnd(int32_t  desired_cwnd);
	virtual int32_t 	apply_options();
	/////////////////////////////////////////////////////////////////////////////////////
	//                SET THE TIMEOUT ASSOCIATED WITH THIS CONNECTION.              //
	//---------------------------------------------------------------------------------//
	//  active_timeout : 																 //
	//     	 the total elasped time of   the connection.                                        	 //			                            //
	//  inactivity_timeout: 															 //
	//        the elapsed time from the time that a read or a write was scheduled    //
	// during which the  connection  was unable to sink/provide data.                    //
	//                               												    //
	// --------------------------------------------------------------------------------//
	//  NOTES:                                                                                                             //
	//        calling these functions repeatedly resets the timeout.                            //
	//        These functions are NOT THREAD-SAFE, and may only be called when  //
	//  handing an  event from this NetVConnection,   or the Connector creation	 //
	//   callback.                                                                                                          //
	////////////////////////////////////////////////////////////////////////////////////
	virtual void 		set_active_timeout(int64_t timeout_in){this->active_timeout_in = timeout_in;}
	virtual void 		set_inactivity_timeout(int64_t timeout_in){this->inactivity_timeout_in = timeout_in;}
	virtual void 		cancel_active_timeout(){}
	virtual void 		cancel_inactivity_timeout(){}

	virtual int64_t 	get_inactivity_timeout(){return inactivity_timeout_in;}
	virtual int64_t 	get_active_timeout(){return active_timeout_in;}

	virtual void 		set_local_addr(SockAddr& sock){this->m_local_addr = sock; }
	virtual SockAddr  get_local_addr(SockAddr& sock) { return m_local_addr; }
	virtual void 		set_remote_addr(SockAddr& sock) { m_remote_addr = sock; }
	virtual SockAddr  get_remote_addr(){ return m_remote_addr;}

	virtual int64_t 	get_socket(){ return m_stream.GetHandle();}
	virtual int32_t   Attach(int fd){ return m_stream.Attach(fd);}

	virtual int32_t   disable_read();
	virtual int32_t   disable_write();

	virtual int64_t   readv(int32_t  fd, struct iovec *vector, size_t count);
	virtual int64_t   writev(int32_t fd, struct iovec *vector, size_t count);

	virtual int64_t 	load_buffer_and_write(int64_t towrite, int64_t &wattempted, int64_t &total_wrote, MIOBufferAccessor & buf, int &needs);


	///////////////////////////////////////////////////////////////////////////
	//                       HTTP SSL 相关设置                   //
	///////////////////////////////////////////////////////////////////////////
	virtual void setSSLHandshakeWantsRead(bool ) { return; }
	virtual bool getSSLHandshakeWantsRead() { return false; }
	virtual void setSSLHandshakeWantsWrite(bool ) { return; }
	virtual bool getSSLHandshakeWantsWrite() { return false; }
	virtual bool sslStartHandShake(int event, int &err) { return 0;}
	virtual bool getSSLHandShakeComplete() {return (true); }
	virtual bool getSSLClientConnection(){ return (false);}
	virtual void setSSLClientConnection(bool state){ (void) state;}

	//event
	inline int32_t read_signal_and_update(int event);
	inline int32_t write_signal_and_update(int event);
	inline int32_t read_signal_done(int event);
	inline int32_t write_signal_done(int event);
	inline int32_t read_signal_error(int lerrno);
	inline int32_t write_signal_error(int lerrno);

	//
	inline int32_t write_reschedule();
	inline int32_t read_reschedule();
	inline int32_t net_activity();

protected:
	volatile int closed;
	VIO read;
	VIO write;
	bool read_enabled;
	bool write_enabled;


	int64_t		inactivity_timeout_in;
	int64_t		active_timeout_in;
	int64_t		next_inactivity_timeout_at;
	int32_t		recursion;
	int64_t		submit_time;
	uint32_t	 id;

	union
	{
		unsigned int flags;
#define NET_VC_SHUTDOWN_READ  1
#define NET_VC_SHUTDOWN_WRITE 2
		struct
		{
			unsigned int got_local_addr:1;
			unsigned int shutdown:2;
		} f;
	};

	ConnectorOptions    options;
	SockStream 		     m_stream;
	SockAddr     		  m_remote_addr;
	SockAddr     		  m_local_addr;
};


extern ClassAllocator<Connector> netVCAllocator;

#endif /* CONNECTOR_H_ */
