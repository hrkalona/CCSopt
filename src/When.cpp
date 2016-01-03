/*
 * When.cpp
 *
 *  Created on: May 5, 2014
 *      Author: hrkalona
 */
#include "When.hpp"
#include <cstdlib>

When::When()
{
	timing_sense = NON;
	cell_fall = NULL;
	cell_rise = NULL;
	fall_transition = NULL;
	rise_transition = NULL;
	timing_type = NO_TYPE;
}

When::~When()
{
	if (cell_fall != NULL)
	{
		free( cell_fall->dim_sizes );
		for (unsigned int i = 0; i < cell_fall->dimensions; i++)
		{
			free( cell_fall->index_info[i] );
		}
		free( cell_fall->values );
		free( cell_fall );
	}

	if (cell_rise != NULL)
	{
		free( cell_rise->dim_sizes );
		for (unsigned int i = 0; i < cell_rise->dimensions; i++)
		{
			free( cell_rise->index_info[i] );
		}
		free( cell_rise->values );
		free( cell_rise );
	}

	if (fall_transition != NULL)
	{
		free( fall_transition->dim_sizes );
		for (unsigned int i = 0; i < fall_transition->dimensions; i++)
		{
			free( fall_transition->index_info[i] );
		}
		free( fall_transition->values );
		free( fall_transition );
	}

	if (rise_transition != NULL)
	{
		free( rise_transition->dim_sizes );
		for (unsigned int i = 0; i < rise_transition->dimensions; i++)
		{
			free( rise_transition->index_info[i] );
		}
		free( rise_transition->values );
		free( rise_transition );
	}
}

void When::setTimingSense(UNATE timing_sense)
{
	this->timing_sense = timing_sense;
}

void When::setCellFall(tables * cell_fall)
{
	this->cell_fall = cell_fall;
}

void When::setCellRise(tables * cell_rise)
{
	this->cell_rise = cell_rise;
}

void When::setFallTransition(tables * fall_transition)
{
	this->fall_transition = fall_transition;
}

void When::setRiseTransition(tables * rise_transition)
{
	this->rise_transition = rise_transition;
}

tables * When::getCellFall()
{
	return cell_fall;
}

tables * When::getCellRise()
{
	return cell_rise;
}

tables * When::getFallTransition()
{
	return fall_transition;
}

tables * When::getRiseTransition()
{
	return rise_transition;
}

UNATE When::getTimingSense()
{
	return timing_sense;
}

void When::setWhen(string when)
{
	this->when = when;
}

ARC_TYPE When::getTimingType()
{
	return timing_type;
}

void When::setTimingType(ARC_TYPE type)
{
	timing_type = type;
}

