/*
 * HttpStatus.h
 *
 *  Created on: 2014年5月5日
 *      Author: jacky
 */

#ifndef HTTPSTATUS_H_
#define HTTPSTATUS_H_

/*
 *
 */
class HttpStatus {
public:
	HttpStatus();
	virtual ~HttpStatus();
private:
    int           http_version;
    int            code;
    int            count;
    u_char      *start;
    u_char      *end;
};

#endif /* HTTPSTATUS_H_ */
