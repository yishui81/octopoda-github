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


  typedef enum
  {
    OC_HTTP_READ_REQUEST_HDR_HOOK,
    OC_HTTP_OS_DNS_HOOK,
    OC_HTTP_SEND_REQUEST_HDR_HOOK,
    OC_HTTP_READ_CACHE_HDR_HOOK,
    OC_HTTP_READ_RESPONSE_HDR_HOOK,
    OC_HTTP_SEND_RESPONSE_HDR_HOOK,
    OC_HTTP_REQUEST_TRANSFORM_HOOK,
    OC_HTTP_RESPONSE_TRANSFORM_HOOK,
    OC_HTTP_SELECT_ALT_HOOK,
    OC_HTTP_TXN_START_HOOK,
    OC_HTTP_TXN_CLOSE_HOOK,
    OC_HTTP_SSN_START_HOOK,
    OC_HTTP_SSN_CLOSE_HOOK,
    OC_HTTP_CACHE_LOOKUP_COMPLETE_HOOK,
    OC_HTTP_PRE_REMAP_HOOK,
    OC_HTTP_POST_REMAP_HOOK,
    OC_HTTP_RESPONSE_CLIENT_HOOK,
    OC_HTTP_LAST_HOOK
  } OC_HTTP_MSG_ID;

typedef struct oc_str_t{
	u_char* data;
	size_t    len;
}oc_str_t;

#endif /* OC_CORE_H_ */
