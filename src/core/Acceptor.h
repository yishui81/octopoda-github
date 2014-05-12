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



class Acceptor : public UTaskObj
{
public:
    Acceptor();
    virtual ~Acceptor();

public:
    virtual int32_t handle_open  (const URE_Msg& msg);
    virtual int32_t handle_close (UWorkEnv * orign_uwe, long retcode);
    virtual int32_t handle_input (URE_Handle h);
    sockfd_t  Accept( SockAddr* remote){return m_acceptor.Accept(remote);};


private:
    SockAcceptor m_acceptor;
    ConfigSet*  m_conf;

};


#endif /* ACCEPT_TASK_H_ */
