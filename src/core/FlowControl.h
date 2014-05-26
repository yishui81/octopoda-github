/*
 * FlowControl.h
 *
 *  Created on: 2014年5月14日
 *      Author: jacky
 */

#ifndef FLOWCONTROL_H_
#define FLOWCONTROL_H_

/*
 *
 */
class FlowControl {
public:
	FlowControl();
	virtual ~FlowControl();

private:
	uint64_t high_water; ///< Buffered data limit - throttle if more than this.
	uint64_t low_water; ///< Unthrottle if less than this buffered.
	bool enabled_p; ///< Flow control state (@c false means disabled).

public:
	// Default value for high and low water marks.
	static uint64_t const DEFAULT_WATER_MARK = 1<<16;
};

#endif /* FLOWCONTROL_H_ */
