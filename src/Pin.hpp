/*
 * Pin.hpp
 *
 *  Created on: May 5, 2014
 *      Author: hrkalona
 */

#ifndef PIN_HPP_
#define PIN_HPP_

#include <string>
#include "types.hpp"

using namespace std;

class Pin
{
private:
	string name;
public:
	Pin(string);
	virtual ~Pin() {};
	string getName();
	virtual NODE_TYPE getType() = 0;
};

#endif /* PIN_HPP_ */
