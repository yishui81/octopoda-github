/*
 * Handle.h
 *
 *  Created on: 2014年5月20日
 *      Author: jacky
 */

#ifndef HANDLE_H_
#define HANDLE_H_

/*
 *
 */


#define set_handler(_h) \
  (handler = ((Handler)_h))

typedef int32_t (Handle::*Handler) (int event, void *data);
class Handle {
public:
	Handle(){handler = 0;}
	virtual ~Handle(){}
public:

	virtual int32_t do_handle(int event, void* data){
		return (this->*handler) (event, data);
	}

protected:
	Handler handler;

};


#endif /* HANDLE_H_ */
