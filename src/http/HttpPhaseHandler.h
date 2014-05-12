/*
 * HttpPhaseHandler.h
 *
 *  Created on: 2014年5月9日
 *      Author: jacky
 */

#ifndef HTTPPHASEHANDLER_H_
#define HTTPPHASEHANDLER_H_

/*
 *
 */
class HttpPhaseHandler {
public:
	HttpPhaseHandler();
	virtual ~HttpPhaseHandler();
private:
	uint32_t m_step;
};

#endif /* HTTPPHASEHANDLER_H_ */
