/*
 * InputPin.cpp
 *
 *  Created on: May 5, 2014
 *      Author: hrkalona
 */
#include "InputPin.hpp"

InputPin::InputPin(string name) :
		Pin( name )
{
	Cfall = 0;
	Crise = 0;
	C = 0;
	clock_pin = false;
	gfall = 0;
	grise = 0;
}

void InputPin::setCfall(double Cfall)
{
	this->Cfall = Cfall;
}

void InputPin::setCrise(double Crise)
{
	this->Crise = Crise;
}

double InputPin::getCfall()
{
	return Cfall;
}

double InputPin::getCrise()
{
	return Crise;
}

NODE_TYPE InputPin::getType()
{
	return INPUT;
}

bool InputPin::isClockPin()
{
	return clock_pin;
}

void InputPin::setClockPin(bool opt)
{
	clock_pin = opt;
}

void InputPin::setC(double C)
{
	this->C = C;
}

double InputPin::getC()
{
	return C;
}

void InputPin::setGfall(double g)
{
	gfall = g;
}

double InputPin::getGfall()
{
	return gfall;
}

void InputPin::setGrise(double g)
{
	grise = g;
}

double InputPin::getGrise()
{
	return grise;
}

