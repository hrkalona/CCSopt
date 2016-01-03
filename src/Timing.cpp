/*
 * Timing.cpp
 *
 *  Created on: May 5, 2014
 *      Author: hrkalona
 */
#include "Timing.hpp"

Timing::Timing(Pin * related_pin)
{
	this->related_pin = related_pin;
}

Timing::~Timing()
{
	for (unsigned int i = 0; i < when.size(); i++)
	{
		delete when[i];
	}
}

void Timing::addTimingWhen(When * timing_when)
{
	when.push_back( timing_when );
}

Pin * Timing::getRelatedPin()
{
	return related_pin;
}

vector<When *>& Timing::getWhen()
{
	return when;
}
