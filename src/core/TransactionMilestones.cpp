/*
 * TransactionMilestones.cpp
 *
 *  Created on: 2014年5月15日
 *      Author: jacky
 */

#include "TransactionMilestones.h"

TransactionMilestones::TransactionMilestones()
		: user_agent_begin(0),
		user_agent_read_header_done(0),
		user_agent_begin_write(0),
		user_agent_close(0),
		server_first_connect(0),
		server_connect(0),
		server_connect_end(0),
		server_begin_write(0),
		server_first_read(0),
		server_read_header_done(0),
		server_close(0),
		cache_open_read_begin(0),
		cache_open_read_end(0),
		cache_open_write_begin(0),
		cache_open_write_end(0),
		dns_lookup_begin(0),
		dns_lookup_end(0),
		state_machine_start(0),
		state_machine_finish(0)
{

}

TransactionMilestones::~TransactionMilestones()
{

}

