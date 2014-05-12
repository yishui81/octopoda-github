/*
 * oc_core.h
 *
 *  Created on: 2014年5月4日
 *      Author: jacky
 */

#ifndef OC_CORE_H_
#define OC_CORE_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <BaseARE/UTaskObj.h>
#include <BaseARE/SockAcceptor.h>
#include <BaseARE/UREData.h>
#include <BaseARE/UWorkEnv.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>


#define LF     (u_char) 10
#define CR     (u_char) 13
#define CRLF   "\x0d\x0a"


#define  OC_OK          0
#define  OC_ERROR      -1
#define  OC_AGAIN      -2
#define  OC_BUSY       -3
#define  OC_DONE       -4
#define  OC_DECLINED   -5
#define  OC_ABORT      -6


typedef struct oc_str_t{
	u_char* data;
	size_t    len;
}oc_str_t;

#endif /* OC_CORE_H_ */
