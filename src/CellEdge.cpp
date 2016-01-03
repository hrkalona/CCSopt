/*
 * CellEdge.cpp
 *
 *  Created on: May 8, 2014
 *      Author: hrkalona
 */
#include "CellEdge.hpp"
#include <limits>

CellEdge::CellEdge(string name) :
		Edge( name, -numeric_limits<double>::max(), -numeric_limits<double>::max() )
{
	timing_sense_fall = NON;
	timing_sense_rise = NON;
	timing = NULL;
}

NET_TYPE CellEdge::getNetType()
{
	return CELL_EDGE;
}

UNATE CellEdge::getFallTimingSense()
{
	return timing_sense_fall;
}

UNATE CellEdge::getRiseTimingSense()
{
	return timing_sense_rise;
}

void CellEdge::setFallTimingSense(UNATE timing_sense_fall)
{
	this->timing_sense_fall = timing_sense_fall;
}

void CellEdge::setRiseTimingSense(UNATE timing_sense_rise)
{
	this->timing_sense_rise = timing_sense_rise;
}

void CellEdge::setTiming(Timing * timing)
{
	this->timing = timing;
}

Timing * CellEdge::getTiming()
{
	return timing;
}

