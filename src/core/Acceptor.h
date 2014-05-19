/*
 * accept_task.h
 *
 *  Created on: 2014年5月4日
 *      Author: jacky
 */

#ifndef ACCEPT_TASK_H_
#define ACCEPT_TASK_H_
#include <oc_core.h>
#include <BaseARE/ConfigSet.h>
#include <map>
#include "Connector.h"



class Acceptor : public UTaskObj
{
public:
    Acceptor();
    virtual ~Acceptor();

public:
    virtual int32_t handle_open  (const URE_Msg& msg);
    virtual int32_t handle_close (UWorkEnv * orign_uwe, long retcode);
    virtual int32_t handle_input (URE_Handle h);

   int32_t  AcceptedNum()
   {
	   return m_acceptor.AcceptedNum();
   }

protected:

	SockAcceptor m_acceptor;
	std::map<std::string, Connector*> conns;


};


#endif /* ACCEPT_TASK_H_ */
