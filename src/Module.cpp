/*
 * Module.cpp
 *
 *  Created on: Jun 11, 2014
 *      Author: hrkalona
 */
#include "Module.hpp"

void Module::setName(string name)
{
	this->name = name;
}

string Module::getName()
{
	return name;
}

void Module::setFileName(string file_name)
{
	this->file_name = file_name;
}

string Module::getFileName()
{
	return file_name;
}

void Module::addArgument(string arg)
{
	arguments.push_back( arg );
}

void Module::addCode(string code)
{
	this->code.push_back( code );
}

void Module::addLine(int line)
{
	lines.push_back( line );
}

vector<string> & Module::getArguments()
{
	return arguments;
}

vector<string> & Module::getCode()
{
	return code;
}

vector<int> & Module::getLines()
{
	return lines;
}

vector<NODE_TYPE> & Module::getPortType()
{
	return port_type;
}

vector<int> & Module::getWireType()
{
	return wire_type;
}

