/*
 * ConfHandler.h
 *
 *  Created on: 2014年5月12日
 *      Author: jacky
 */

#ifndef CONFHANDLER_H_
#define CONFHANDLER_H_

/*
 *
 */
class ConfHandler {
public:
	ConfHandler();
	virtual ~ConfHandler();

	virtual int32_t preConfiguration() = 0;
	virtual int32_t postConfiguration() = 0;
	virtual int32_t createMainConf() = 0;
	virtual int32_t initMainConf() = 0;
	virtual int32_t createSrvConf() = 0;
	virtual int32_t createLocConf() = 0;
	virtual int32_t mergeLocConf() = 0;

};

#endif /* CONFHANDLER_H_ */
