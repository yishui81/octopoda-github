/*
 * HttpMethod.h
 *
 *  Created on: 2014年5月5日
 *      Author: jacky
 */

#ifndef HTTPMETHOD_H_
#define HTTPMETHOD_H_

/*
 *
 */
class HttpMethod {
const  int HTTP_GET = 1;
const int HTTP_POST = 2;
const int HTTP_DELETE = 3;

public:
	HttpMethod();
	virtual ~HttpMethod();
private:

};

#endif /* HTTPMETHOD_H_ */
