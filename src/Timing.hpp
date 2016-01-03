/*
 * Timing.hpp
 *
 *  Created on: May 5, 2014
 *      Author: hrkalona
 */

#ifndef TIMING_HPP_
#define TIMING_HPP_

#include <string>
#include <vector>
#include "When.hpp"

class Pin;

using namespace std;

class Timing
{
private:
	Pin * related_pin;
	vector<When *> when;

public:
	Timing(Pin *);
	~Timing();
	void addTimingWhen(When *);
	Pin * getRelatedPin();
	vector<When *>& getWhen();

};

typedef pair<string, Timing*> timing_entry;
typedef map<string, Timing*> timing_map;

#endif /* TIMING_HPP_ */
