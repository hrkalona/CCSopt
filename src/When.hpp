/*
 * When.hpp
 *
 *  Created on: May 5, 2014
 *      Author: hrkalona
 */

#ifndef WHEN_HPP_
#define WHEN_HPP_

#include <string>
#include "types.hpp"

using namespace std;

struct When
{
private:
	string when;
	UNATE timing_sense;\
	ARC_TYPE timing_type;
	tables * cell_fall;
	tables * cell_rise;
	tables * fall_transition;
	tables * rise_transition;
public:
	When();
	~When();
	void setTimingSense(UNATE);
    void setCellFall(tables *);
    void setCellRise(tables *);
    void setFallTransition(tables *);
    void setRiseTransition(tables *);
    tables * getCellFall();
    tables * getCellRise();
    tables * getFallTransition();
    tables * getRiseTransition();
    UNATE getTimingSense();

	ARC_TYPE getTimingType();
	void setTimingType(ARC_TYPE);

    void setWhen(string);

};

#endif /* WHEN_HPP_ */
