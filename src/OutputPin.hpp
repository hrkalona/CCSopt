/*
 * OutputPin.hpp
 *
 *  Created on: May 5, 2014
 *      Author: hrkalona
 */

#ifndef OUTPUTPIN_HPP_
#define OUTPUTPIN_HPP_

#include "types.hpp"

#include "Timing.hpp"
#include "Pin.hpp"

using namespace std;

class OutputPin: public Pin
{
private:
	timing_map timings;
public:
	OutputPin(string);
	~OutputPin();
	Timing * addTiming(Pin* pin);
	timing_map& getTimings();
	NODE_TYPE getType();
};

#endif /* OUTPUTPIN_HPP_ */
