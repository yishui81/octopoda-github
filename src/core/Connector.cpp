/*
 * connector.cpp
 *
 *  Created on: 2014年5月4日
 *      Author: jacky
 */
#include "Connector.h"
#include <sys/socket.h>
#include "ink_sock.h"
#include "I_IOBuffer.h"
#include <bits/uio.h>
#include "BaseARE/UTaskObj.h"

enum{
	OC_NET_EVENT_IN = 5000, //读数据
	OC_NET_EVENT_OUT,      	//写数据
	OC_NET_EVENT_CLOSE, 		//关闭网络
	OC_CONN_CLOSE  			//销毁Connector
}

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
	//this->m_stream.Close();
}

int32_t
Connector::do_io_read(UTaskObj* obj, int64_t nbytes, MIOBuffer *buf)
{
	ink_assert(!closed);

	read.op = VIO::READ;
	//read.vio.mutex = c->mutex;
	read._cont = obj;
	read.nbytes = nbytes;
	read.ndone = 0;
	read.vc_server = (VConnection *) this;

	if (buf) {

		read.buffer.writer_for(buf);
		if (!read_enabled){
			read.reenable();
		}

	} else {
		read.buffer.clear();
		disable_read();
	}
	return 0;
}

int32_t
Connector::do_io_write(UTaskObj* obj,int64_t nbytes, IOBufferReader *reader, bool owner = false)
{
	int32_t  ret = 0;
	write.op = VIO::WRITE;
	write._cont = obj;
	write.nbytes = nbytes;
	write.ndone = 0;
	write.vc_server = (Connector *) this;

	if (reader) {

		ink_assert(!owner);
		write.buffer.reader_for(reader);
		if (nbytes && !write_enabled){
			write.reenable();
		}

	} else {

		disable_write();
	}
	return ret;
}

int
Connector::handle_open(const URE_Msg &msg)
{
	//TODO
	return 0;
}

int
Connector::handle_close(UWorkEnv * orign_uwe, long retcode)
{
	//TODO
	return 0;
}

int
Connector::handle_input(URE_Handle h)
{
	MIOBufferAccessor & buf =read.buffer;

	if ( read.op != VIO::READ) {
		disable_read();
		return 0;
	}

	ink_assert(buf.writer());

	// if there is nothing to do, disable connection
	int64_t ntodo = read.ntodo();
	if (ntodo <= 0) {
		disable_read();
		return 0;
	}

	int64_t toread = buf.writer()->write_avail();
	if (toread > ntodo){
		toread = ntodo;
	}

	if(toread == 0){
		goto end;
	}

	// read data
	int64_t rattempted = 0;
	int64_t total_read = 0;

	int64_t  niov = 0;
	int64_t  r = 0;

	struct iovec  tiovec[UIO_MAXIOV];

	IOBufferBlock *b = buf.writer()->first_write_block();

	do {

		niov = 0;
		rattempted = 0;

		while (b && niov < UIO_MAXIOV) {

			int64_t a = b->write_avail();
			if (a > 0) {

				tiovec[niov].iov_base = b->_end;
				int64_t togo = toread - total_read - rattempted;
				if (a > togo){
					a = togo;
				}

				tiovec[niov].iov_len = a;
				rattempted += a;
				niov++;

				if (a >= togo){
					break;
				}

			}
			b = b->next;
		}

		if (niov == 1) {

			r = m_stream.Send((char*) tiovec[0].iov_base, tiovec[0].iov_len);

		} else {

			r = writev((int32_t)m_stream.GetHandle(), tiovec, niov);
		}

		total_read += rattempted;

	} while (r == rattempted && total_read < toread);


	// if we have already moved some bytes successfully, summarize in r
	if (total_read != rattempted) {

		if (r <= 0){
			r = total_read - rattempted;
		}else{
			r = total_read - rattempted + r;
		}
	}

	// check for errors
	if (r <= 0) {

		if (r == -EAGAIN || r == -ENOTCONN) {
			//NET_DEBUG_COUNT_DYN_STAT(net_calls_to_read_nodata_stat, 1);
			//vc->read.triggered = 0;
			//nh->read_ready_list.remove(vc);
			return 0;
		}

		if (!r || r == -ECONNRESET) {
			read_signal_done(CONN_EVENT_EOS);
			return 0;

		}

		read_signal_error((int)-r);
		return 0;
	}
	//  NET_SUM_DYN_STAT(net_read_bytes_stat, r);

	// Add data to buffer and signal continuation.
	buf.writer()->fill(r);
	read.ndone += r;
	//net_activity();

	// Signal read ready, check if user is not done

	// If there are no more bytes to read, signal read complete
	ink_assert(ntodo >= 0);
	if (read.ntodo() <= 0) {
	    read_signal_done(CON_EVENT_READ_COMPLETE);
		 Debug("iocore_net", "read_from_net, read finished - signal done");
		 return 0;

	} else {
		  if (read_signal_and_update(CON_EVENT_READ_READY) != EVENT_CONT){
			  return 0;
		  }

		  // change of lock... don't look at shared variables!
//		  if (lock.m.m_ptr != s->vio.mutex.m_ptr) {
			read_reschedule();
			return 0;
//		  }
	}


end:
	// If here are is no more room, or nothing to do, disable the connection
	if (read.ntodo() <= 0 || !read_enabled || !buf.writer()->write_avail()) {
		disable_read();
		return 0;
	}

	read_reschedule();
//	read_reschedule(nh, vc);
	return 0;
}

int
Connector::handle_output(URE_Handle h)
{
	// If it is not enabled,add to WaitList.
	if (!write_enabled || write.op != VIO::WRITE) {
		disable_write();
		return 0;
	}
	// If there is nothing to do, disable
	int64_t ntodo = write.ntodo();
	if (ntodo <= 0) {
		disable_write();
		return 0;
	}

	MIOBufferAccessor & buf = write.buffer;
	ink_assert(buf.writer());

	// Calculate amount to write
	int64_t towrite = buf.reader()->read_avail();
	if (towrite > ntodo){
		towrite = ntodo;
	}

	int signalled = 0;

	  // signal write ready to allow user to fill the buffer
	if (towrite != ntodo && buf.writer()->write_avail()) {

//		notify_int64(read._cont->GetUTOID(), CON_EVENT_WRITE_READY , 0,  NULL);
		if (write_signal_and_update(CON_EVENT_WRITE_READY) != EVENT_CONT) {
			return 0;
		}

		ntodo = write.ntodo();
		if (ntodo <= 0) {
			disable_write();
			return 0;
		}

		signalled = 1;
		// Recalculate amount to write
		towrite = buf.reader()->read_avail();
		if (towrite > ntodo){
			towrite = ntodo;
		}
	}

	// if there is nothing to do, disable
	ink_assert(towrite >= 0);
	if (towrite <= 0) {
		disable_write();
		return 0;
	}

	int64_t total_wrote = 0, wattempted = 0;
	int needs = 0;
	int64_t r = load_buffer_and_write(towrite, wattempted, total_wrote, buf, needs);

	// if we have already moved some bytes successfully, summarize in r
	if (total_wrote != wattempted) {

		if (r <= 0){
			r = total_wrote - wattempted;
		}else{
			r = total_wrote - wattempted + r;
		}

	}


	// check for errors
	if (r <= 0) {
		// if the socket was not ready,add to WaitList
		if (r == -EAGAIN || r == -ENOTCONN) {

			//NET_DEBUG_COUNT_DYN_STAT(net_calls_to_write_nodata_stat, 1);
			if((needs & WRITE_MASK) == WRITE_MASK) {
				write_reschedule();
			}

			if((needs & READ_MASK) == READ_MASK) {
				read_reschedule();
			}

			return 0;
		}

		if (!r || r == -ECONNRESET) {

			notify_int64(read._cont->GetUTOID(), CON_EVENT_EOS , 0,  NULL);
			return 0;
		}

		//vc->write.triggered = 0;
		write_signal_error((int)-r);
		return 0;

	} else {

		// Remove data from the buffer and signal continuation.
		ink_assert(buf.reader()->read_avail() >= r);
		buf.reader()->consume(r);
		ink_assert(buf.reader()->read_avail() >= 0);
		write.ndone += r;

		net_activity();
		// If there are no more bytes to write, signal write complete,
		ink_assert(ntodo >= 0);
		if (write.ntodo() <= 0) {

			notify_int64(read._cont->GetUTOID(), CON_EVENT_WRITE_COMPLETE , 0,  NULL);
			//write_signal_done(VC_EVENT_WRITE_COMPLETE, nh, vc);
			return 0
					;
		} else if (!signalled) {

			if (write_signal_and_update(CON_EVENT_WRITE_READY) != EVENT_CONT) {
				return 0;
			}

			// change of lock... don't look at shared variables!
			//if (lock.m.m_ptr != s->vio.mutex.m_ptr) {
				write_reschedule();
				return 0;
			//}
		}

		if (!buf.reader()->read_avail()) {
			disable_write();
			return 0;;
		}

		if((needs & WRITE_MASK) == WRITE_MASK) {
			write_reschedule();
		}
		if((needs & READ_MASK) == READ_MASK) {
			read_reschedule();
		}
		return 0;
	}
}

int
Connector::handle_message(URE_Msg& msg)
{
	int64_t event = msg.GetInt64();
	int64_t msg_type = msg.GetType();

	switch(event){
	case OC_NET_EVENT_IN:
		return handle_input((URE_Handle)m_stream.GetHandle());
	case OC_NET_EVENT_OUT:
		return handle_output((URE_Handle)m_stream.GetHandle());
	case OC_NET_EVENT_CLOSE:
		return do_io_close();
	case OC_CONN_CLOSE:
		return handle_close(GetWorkEnv(), 0);
	default:
		//TODO
	}
	return 0;
}

int32_t
Connector::do_io_shutdown(int32_t  howto)
{


	int32_t fd = m_stream.GetHandle();
	int32_t  ret = 0;

	closed = true;
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

int32_t
Connector::do_io_close(int32_t err_code)
{
	return do_io_shutdown(IO_SHUTDOWN_READWRITE);
}

int32_t
Connector::apply_options(){

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

int32_t
Connector::set_tcp_init_cwnd(int32_t desired_cwnd){
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


int32_t
Connector::disable_read()
{
	read_enabled = false;
	return remove_handler((URE_Handle)m_stream.GetHandle(), READ_MASK);
}

int32_t
Connector::disable_write()
{
	write_enabled = false;
	return remove_handler((URE_Handle)m_stream.GetHandle(), WRITE_MASK);
}

int64_t
Connector::readv(int fd, struct iovec *vector, size_t count)
{
	  int64_t result  = 0;
	  do {
		// coverity[tainted_data_argument]
		if (likely((result =::readv(fd, vector, count)) >= 0)){
			break;
		}
		result = -errno;
	  } while (errno == EINTR);

	  return result;
}

// This code was pulled out of write_to_net so
// I could overwrite it for the SSL implementation
// (SSL read does not support overlapped i/o)
// without duplicating all the code in write_to_net.
int64_t
Connector::load_buffer_and_write(int64_t towrite, int64_t &wattempted, int64_t &total_wrote, MIOBufferAccessor & buf, int &needs)
{
	int64_t result = 0;

	// XXX Rather than dealing with the block directly, we should use the IOBufferReader API.
	int64_t offset = buf.reader()->start_offset;
	IOBufferBlock *b = buf.reader()->block;

	do {

		struct iovec  tiovec[UIO_MAXIOV];
		int niov = 0;
		int64_t total_wrote_last = total_wrote;

		while (b && niov < UIO_MAXIOV) {

			// check if we have done this block
			int64_t avail = b->read_avail();
			avail -= offset;
			if (avail <= 0) {
				offset = -avail;
				b = b->next;
				continue;
			}

			// check if to amount to write exceeds that in this buffer
			int64_t wavail = towrite - total_wrote;
			if (avail > wavail){
				avail = wavail;
			}

			if (!avail){
				break;
			}

			total_wrote += avail;
			// build an iov entry
			tiovec[niov].iov_len = avail;
			tiovec[niov].iov_base = b->start() + offset;
			niov++;
			// on to the next block
			offset = 0;
			b = b->next;
		}

		wattempted = total_wrote - total_wrote_last;
		if (niov == 1){
			result = m_stream.Send((char*)tiovec[0].iov_base, tiovec[0].iov_len);
		} else{
			result = this->writev((int)m_stream.GetHandle(), &tiovec[0], niov);
		}

	} while (result == wattempted && total_wrote < towrite);

	needs |= WRITE_MASK;

	return (result);
}


inline int32_t
Connector::read_signal_and_update(int event)
{
  recursion++;
  notify_int64(read._cont->GetUTOID(), 0 , event , NULL);
  if (!recursion && closed) {
    return EVENT_DONE;
  } else {
    return EVENT_CONT;
  }
}


inline int32_t
Connector::write_signal_and_update(int event)
{
  recursion++;
  notify_int64(write._cont->GetUTOID(), 0, event, NULL);
  if (!recursion && closed) {
    return EVENT_DONE;
  } else {
    return EVENT_CONT;
  }
}

inline int32_t
Connector::read_signal_done(int event)
{
  read_enabled = 0;
  if (read_signal_and_update(event) == EVENT_DONE) {
    return EVENT_DONE;
  } else {
    read_reschedule();
    return EVENT_CONT;
  }
}

inline int32_t
Connector::write_signal_done(int event)
{
  write_enabled = 0;
  if (write_signal_and_update(event) == EVENT_DONE) {
    return EVENT_DONE;
  } else {
    write_reschedule();
    return EVENT_CONT;
  }
}

inline int32_t
Connector::read_signal_error(int lerrno)
{
  return read_signal_done(CON_EVENT_ERROR);
}

inline int32_t
Connector::write_signal_error(int lerrno)
{
  return write_signal_done(CON_EVENT_ERROR);
}

inline int32_t
Connector::write_reschedule()
{
	return notify_int64(GetUTOID(), OC_NET_EVENT_OUT, 0, NULL);
}

inline int32_t
Connector::read_reschedule()
{
	return notify_int64(GetUTOID(), OC_NET_EVENT_IN, 0, NULL);
}

inline int32_t
Connector::net_activity()
{
	return 0;
}
