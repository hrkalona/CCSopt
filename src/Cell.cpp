/*
 * Cell.cpp
 *
 *  Created on: May 5, 2014
 *      Author: hrkalona
 */
#include "Cell.hpp"
#include "InputPin.hpp"
#include "OutputPin.hpp"
#include "Timing.hpp"
#include <iostream>
#include <boost/regex.hpp>

using namespace boost;

Cell::Cell(string name)
{
	this->name = name;
	drive_strength = 0;
}

Cell::~Cell()
{
	for (unsigned int i = 0; i < input_pins.size(); i++)
	{
		delete input_pins[i];
	}

	for (unsigned int i = 0; i < output_pins.size(); i++)
	{
		delete output_pins[i];
	}
}

void Cell::addInputPin(Pin * input_pin)
{
	input_pins.push_back( input_pin );
}

void Cell::addOutputPin(Pin * output_pin)
{
	output_pins.push_back( output_pin );
}

string Cell::getName()
{
	return name;
}

string Cell::getPrefix()
{
	return prefix;
}

void Cell::setDriveStrength(double drive_strength)
{
	this->drive_strength = drive_strength;
}

double Cell::getDriveStrenth()
{
	return drive_strength;
}

void Cell::addIOPin(string name, NODE_TYPE type)
{
	io_pins.insert( io_entry( name, type ) );
}

NODE_TYPE Cell::getPinType(string name)
{
	io_map::iterator it = io_pins.find( name );

	if (it == io_pins.end())
	{
		return ERROR_CODE;
	}

	return it->second;
}

Pin * Cell::getOutput(string name)
{
	for (unsigned int i = 0; i < output_pins.size(); i++)
	{
		if (output_pins[i]->getName().compare( name ) == 0)
		{
			return output_pins[i];
		}
	}

	/* Create a new output pin */
	OutputPin * new_out_pin = new OutputPin( name );

	addIOPin( name, OUTPUT );

	/* Add newly allocated pin in cell's output pins list */
	addOutputPin( new_out_pin );

	return new_out_pin;
}

Pin * Cell::getInput(string name)
{
	for (unsigned int i = 0; i < input_pins.size(); i++)
	{
		if (input_pins[i]->getName().compare( name ) == 0)
		{
			return input_pins[i];
		}
	}

	/* Create a new input pin */
	InputPin * new_in_pin = new InputPin( name );

	addIOPin( name, INPUT );

	/* Add newly allocated pin in cell's input pins list */
	addInputPin( new_in_pin );

	return new_in_pin;
}

io_map& Cell::getIOPins()
{
	return io_pins;
}

vector<Pin *>& Cell::getInputs()
{
	return input_pins;
}

vector<Pin *>& Cell::getOutputs()
{
	return output_pins;
}

void Cell::inputCapacitanceCheck()
{
	for (unsigned int j = 0; j < input_pins.size(); ++j)
	{
		if ((( InputPin * ) input_pins[j])->getCfall() == 0)
		{
			double c = (( InputPin * ) input_pins[j])->getC();

			if (c != 0)
			{
				(( InputPin * ) input_pins[j])->setCfall( c );
			}
			else
			{
				cout << "No input capacitance found for the pin " << input_pins[j]->getName() << " of the cell " << name << "\n";
			}
		}

		if ((( InputPin * ) input_pins[j])->getCrise() == 0)
		{
			double c = (( InputPin * ) input_pins[j])->getC();

			if (c != 0)
			{
				(( InputPin * ) input_pins[j])->setCrise( c );
			}
			else
			{
				cout << "No input capacitance found for the pin " << input_pins[j]->getName() << " of the cell " << name << "\n";
			}
		}
	}
}

void Cell::errorCheck(string delimiters)
{

	if (input_pins.empty())
	{
		cout << "Warning, no input pins were found in the cell " << name << "\n";
		//cout << "Terminating.\n";
		//exit( -1 );
	}

	if (output_pins.empty())
	{
		cout << "Warning, no outputs pins were found in the cell " << name << "\n";
		//cout << "Terminating.\n";
		//exit( -1 );
	}

	unsigned int best = 0;

	for (unsigned int i = 0; i < delimiters.size(); i++)
	{
		int index = name.find_last_of( delimiters[i] );

		if (index >= 0 && index < name.size())
		{
			string temp_str = name.substr( 0, index + 1 );

			if (temp_str.size() > prefix.size())
			{
				prefix = temp_str;
				best = i;
			}
		}

	}

	if (prefix.empty())
	{
		cout << "No delimiter was found in the name of the cell " << name << "\n";
		cout << "Terminating.\n";
		exit( -1 );
	}

	if (drive_strength == 0)
	{
		string sub_str = name.substr( name.find_last_of( delimiters[best] ) + 1, name.size() );

		if (regex_match( sub_str, regex( "([-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?)" ) ))  //export the drive strength from the name, if any
		{
			drive_strength = stod( sub_str );
		}

		if (drive_strength == 0)
		{
			cout << "The cell " << name << " has zero drive strength.\n";
			cout << "Terminating.\n";
			exit( -1 );
		}
	}

	for (unsigned int j = 0; j < output_pins.size(); ++j)
	{
		OutputPin *out_pin = ( OutputPin * ) output_pins[j];
		timing_map::iterator it;
		for (it = out_pin->getTimings().begin(); it != out_pin->getTimings().end(); it++)
		{
			Timing *timing_info = it->second;

			for (unsigned int i = 0; i < timing_info->getWhen().size(); i++)
			{
				if (timing_info->getWhen()[i]->getTimingSense() == NON
						&& (timing_info->getWhen()[i]->getTimingType() != RISING_EDGE && timing_info->getWhen()[i]->getTimingType() != FALLING_EDGE
								&& timing_info->getWhen()[i]->getTimingType() != COMBINATIONAL))
				{
					cout << "The timing arc " << out_pin->getName() << " -> " << timing_info->getRelatedPin()->getName() << " for the cell " << name
							<< " is not unate.\n";
					cout << "The timing type must be either falling edge or rising edge or combinational.\n";
					cout << "Terminating.\n";
					exit( -1 );
				}
				/*if(timing_info->getWhen()[i]->getTimingType() == NORMAL)
				 {
				 if(timing_info->getWhen()[i]->getCellFall() == NULL || timing_info->getWhen()[i]->getCellRise() == NULL || timing_info->getWhen()[i]->getFallTransition() == NULL || timing_info->getWhen()[i]->getRiseTransition() == NULL)
				 {
				 cout << "Missing timing tables for the arc " << out_pin->getName() << "->" << timing_info->getRelatedPin()->getName() << "\n";
				 cout << "Terminating.\n";
				 exit(-1);
				 }
				 }*/
			}

		}
	}

}

/* g = (Ci / Co) / x */
void Cell::logicalEffortExtraction(double C0)
{

	double temp = C0 * drive_strength;

	for (unsigned int j = 0; j < input_pins.size(); ++j)
	{
		InputPin *in_pin = ( InputPin * ) input_pins[j];

		in_pin->setGfall( in_pin->getCfall() / temp );
		in_pin->setGrise( in_pin->getCrise() / temp );
	}

}

