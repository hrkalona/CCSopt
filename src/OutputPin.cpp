/*
 * OutputPin.cpp
 *
 *  Created on: May 5, 2014
 *      Author: hrkalona
 */
#include "OutputPin.hpp"
#include "types.hpp"
#include "Timing.hpp"

OutputPin::OutputPin(string name) :
		Pin( name )
{

}

OutputPin::~OutputPin()
{
	timing_map::iterator t;
	for (t = timings.begin(); t != timings.end(); t++)
	{
		delete t->second;
	}
}



Timing * OutputPin::addTiming(Pin * pin)
{
	timing_map::iterator find_it = timings.lower_bound( pin->getName() );

	Timing * new_timing;

	if (find_it != timings.end() && find_it->second->getRelatedPin()->getName().compare( pin->getName() ) == 0)
	{
		new_timing = find_it->second;
	}
	else
	{
		new_timing = new Timing( pin );
		timings.insert( find_it, timing_entry( pin->getName(), new_timing ) );
	}

	return new_timing;
}

timing_map& OutputPin::getTimings()
{
	return timings;
}

NODE_TYPE OutputPin::getType()
{
	return OUTPUT;
}


