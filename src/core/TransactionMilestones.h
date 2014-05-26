/*
 * TransactionMilestones.h
 *
 *  Created on: 2014年5月15日
 *      Author: jacky
 */

#ifndef TRANSACTIONMILESTONES_H_
#define TRANSACTIONMILESTONES_H_

/*
 *
 */
class TransactionMilestones {

public:

	TransactionMilestones();
	virtual ~TransactionMilestones();

	////////////////////////////////////////////////////////
	// user agent:                                        //
	// user_agent_begin represents the time this          //
	// transaction started. If this is the first          //
	// transaction in a connection, then user_agent_begin //
	// is set to accept time. otherwise it is set to      //
	// first read time.                                   //
	////////////////////////////////////////////////////////
	int64_t user_agent_begin;
	int64_t user_agent_read_header_done;
	int64_t user_agent_begin_write;
	int64_t user_agent_close;

	////////////////////////////////////////////////////////
	// server (origin_server , parent, blind tunnnel //
	////////////////////////////////////////////////////////
	int64_t server_first_connect;
	int64_t server_connect;
	int64_t server_connect_end;
	int64_t server_begin_write;            //  http only
	int64_t server_first_read; //  http only
	int64_t server_read_header_done;   //  http only
	int64_t server_close;

	int64_t cache_open_read_begin;
	int64_t cache_open_read_end;
	int64_t cache_open_write_begin;
	int64_t cache_open_write_end;

	int64_t dns_lookup_begin;
	int64_t dns_lookup_end;

	///////////////////
	// state machine //
	///////////////////
	int64_t state_machine_start;
	int64_t state_machine_finish;

	// TODO: Should we instrument these at some point?
	// int64_t  cache_read_begin;
	// int64_t  cache_read_end;
	// int64_t  cache_write_begin;
	// int64_t  cache_write_end;
};


#endif /* TRANSACTIONMILESTONES_H_ */
