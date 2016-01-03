/*
 * Cell.hpp
 *
 *  Created on: May 5, 2014
 *      Author: hrkalona
 */

#ifndef CELL_HPP_
#define CELL_HPP_

#include "Pin.hpp"
#include <string>
#include <vector>
#include "types.hpp"

using namespace std;

typedef map<string, NODE_TYPE> io_map;
typedef pair<string, NODE_TYPE> io_entry;

class Cell
{
private:
	string name;
	string prefix;
	vector<Pin *> input_pins;
	vector<Pin *> output_pins;
	io_map io_pins;
	double drive_strength;

public:
	Cell(string);
	~Cell();

	string getName();
	string getPrefix();

	NODE_TYPE getPinType(string);
	Pin * getOutput(string);
	Pin * getInput(string);
	io_map& getIOPins();
	void logicalEffortExtraction(double);
	vector<Pin *>& getInputs();
	vector<Pin *>& getOutputs();
	void setDriveStrength(double);
	double getDriveStrenth();
	void inputCapacitanceCheck();
	void errorCheck(string);

private:
	void addInputPin(Pin *);
	void addOutputPin(Pin *);
	void addIOPin(string, NODE_TYPE);
};

typedef pair<string, Cell*> cell_entry;
typedef map<string, Cell*> cell_map;

#endif /* CELL_HPP_ */
