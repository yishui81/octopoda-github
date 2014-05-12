/*
 * GeneralConf.h
 *
 *  Created on: 2014年5月11日
 *      Author: jacky
 */

#ifndef GENERALCONF_H_
#define GENERALCONF_H_

/*
 *
 */
struct ConfigureInfo{
	std::string name;
	std::string value;
	uint32_t    lineno;
};
class GeneralConf {
public:
	GeneralConf();
	virtual ~GeneralConf();
};

#endif /* GENERALCONF_H_ */
