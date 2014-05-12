/*
 * oc_config.h
 *
 *  Created on: 2014年5月4日
 *      Author: jacky
 */

#ifndef OC_CONFIG_H_
#define OC_CONFIG_H_

#define LINUX 1

#define OC_INT32_LEN   (sizeof("-2147483648") - 1)
#define OC_INT64_LEN   (sizeof("-9223372036854775808") - 1)
#define OC_INT_T_LEN   OC_INT64_LEN

#define OC_MAX_UINT32_VALUE  (uint32_t) 0xffffffff
#define OC_MAX_INT32_VALUE   (uint32_t) 0x7fffffff


#endif /* OC_CONFIG_H_ */
