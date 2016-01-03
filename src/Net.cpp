/*
 * Net.cpp
 *
 *  Created on: May 6, 2014
 *      Author: hrkalona
 */

#include "Net.hpp"

Net::Net(string name, bool virtual_net)
{
	this->name = name;
	this->virtual_net = virtual_net;

	parasitic_net = new Parasitic(name);
}

Net::~Net()
{
	if(assigned_value.empty())
	{
		delete parasitic_net;
	}
}

string Net::getName()
{
	return name;
}

bool Net::isVirtual()
{
	return virtual_net;
}

void Net::addAssignedNet(Net * assigned_net)
{
	assigned_to.push_back( assigned_net);
}

vector<Net *> & Net::getAssignedTo()
{
	return assigned_to;
}

string Net::getAssignedValue()
{
	return assigned_value;
}

void Net::setAssignedValue(string value)
{
	assigned_value = value;
}

Parasitic* Net::getParasitic()
{
	return parasitic_net;
}

void Net::setParasitic(Parasitic * parasitic_net)
{
	this->parasitic_net = parasitic_net;
}

