/*
 * InputPin.hpp
 *
 *  Created on: May 5, 2014
 *      Author: hrkalona
 */

#ifndef INPUTPIN_HPP_
#define INPUTPIN_HPP_

#include <string>
#include "Pin.hpp"
#include "types.hpp"

using namespace std;

class InputPin: public Pin
{
private:
	double Cfall;
	double Crise;
	double C;
	bool clock_pin;

	double gfall;
	double grise;
public:
	InputPin(string);
	~InputPin()
	{
	}
	void setCfall(double);
	void setCrise(double);
	void setC(double);
	double getC(void);
	double getCfall();
	double getCrise();
	NODE_TYPE getType();
	bool isClockPin();
	void setClockPin(bool);
	void setGfall(double);
	double getGfall();
	void setGrise(double);
	double getGrise();

};

#endif /* INPUTPIN_HPP_ */
