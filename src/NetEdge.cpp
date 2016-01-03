/*
 * NetEdge.cpp
 *
 *  Created on: May 8, 2014
 *      Author: hrkalona
 */
#include "NetEdge.hpp"
#include "InputPin.hpp"
#include "Node.hpp"
#include <math.h>

NetEdge::NetEdge(string name) :
		Edge( name, 0, 0 )
{
	capacitance = 0;
	resistance = 0;
	Sot_hat_fall_sqr = 0;
	Sot_hat_rise_sqr = 0;
}

NET_TYPE NetEdge::getNetType()
{
	return NET_EDGE;
}

void NetEdge::setCapacitance(double capacitance)
{
	this->capacitance = capacitance;
}

double NetEdge::getCapacitance()
{
	return capacitance;
}

void NetEdge::setResistance(double resistance)
{
	this->resistance = resistance;
}

double NetEdge::getResistance()
{
	return resistance;
}

void NetEdge::setBFactorFall(double b_factor_fall)
{
	Sot_hat_fall_sqr = 2 * b_factor_fall - getDelayFall() * getDelayFall();
}

void NetEdge::setBFactorRise(double b_factor_rise)
{
	Sot_hat_rise_sqr = 2 * b_factor_rise - getDelayRise() * getDelayRise();
}

double NetEdge::getOutputTransitionFall(double input_transition)
{
	return sqrt( input_transition * input_transition + Sot_hat_fall_sqr );
}

double NetEdge::getOutputTransitionRise(double input_transition)
{
	return sqrt( input_transition * input_transition + Sot_hat_rise_sqr );
}

