/*
 * Module.h
 *
 *  Created on: 2014年5月11日
 *      Author: jacky
 */

#ifndef MODULE_H_
#define MODULE_H_

/*
 *
 */
class Module {
public:
	Module();
	virtual ~Module();
private:
	 char* name;
	 char* parent;


};

#endif /* MODULE_H_ */
