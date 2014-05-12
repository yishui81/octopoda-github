/*
 * oc_cpu.h
 *
 *  Created on: 2014年5月4日
 *      Author: jacky
 */

#ifndef OC_CPU_H_
#define OC_CPU_H_
#include <unistd.h>

int get_cpu_num(){
	return  sysconf(_SC_NPROCESSORS_ONLN);
}


#endif /* OC_CPU_H_ */
