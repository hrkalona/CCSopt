/*
 * Module.hpp
 *
 *  Created on: Jun 11, 2014
 *      Author: hrkalona
 */

#ifndef MODULE_HPP_
#define MODULE_HPP_

#include <string>
#include <vector>
#include <map>
#include "types.hpp"

using namespace std;

class Module
{
private:
	string name;
	vector<string> arguments;
	vector<NODE_TYPE> port_type;
	vector<int> wire_type;
	vector<string> code;
	vector<int> lines;
	string file_name;
public:
	Module() {}
	~Module() {}
	string getName();
	void setName(string);
	void setFileName(string);
	string getFileName();
	void addArgument(string);
	void addCode(string);
	void addLine(int);

	vector<string> & getArguments();
	vector<string> & getCode();
	vector<int> & getLines();
	vector<NODE_TYPE> & getPortType();
	vector<int> & getWireType();

};

typedef map<string, Module *> module_map;
typedef pair<string, Module *> module_entry;

#endif /* MODULE_HPP_ */
