/*
 * CellEdge.hpp
 *
 *  Created on: May 8, 2014
 *      Author: hrkalona
 */

#ifndef CELLEDGE_HPP_
#define CELLEDGE_HPP_

#include <string>
#include "types.hpp"
#include "Edge.hpp"
#include "Timing.hpp"

using namespace std;

class CellEdge : public Edge
{
private:
    UNATE timing_sense_rise;
    UNATE timing_sense_fall;
    Timing * timing;
public:
	CellEdge(string);
	~CellEdge() {};
	UNATE getFallTimingSense();
	UNATE getRiseTimingSense();
	void setFallTimingSense(UNATE);
	void setRiseTimingSense(UNATE);
	NET_TYPE getNetType();

	void setTiming(Timing *);
	Timing * getTiming();

};

#endif /* CELLEDGE_HPP_ */
