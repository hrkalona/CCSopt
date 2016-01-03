/*
 * Net.hpp
 *
 *  Created on: May 6, 2014
 *      Author: hrkalona
 */

#ifndef NET_HPP_
#define NET_HPP_

#include <set>
#include "types.hpp"
#include "Parasitic.hpp"

using namespace std;

class Node;

class Net
{
private:
	string name;

	/* a virtual net is net carrying a const bit.
	 * For example a 1'b1 or 1'b0 */
	bool virtual_net;

	/* a net can have alias nets.
	 * A Verilog file may contain assign statements. For example "assign wire_A = wire_B".
	 * In that case wire_B points to wire_A */
	vector<Net *> assigned_to;

	/* In the previous example and assuming that the net object (this) corresponds to wire_A
	 * the assigned value field will have the value wire_B */
	string assigned_value;

	/* parasitic info */
	Parasitic * parasitic_net;

public:
	Net(string, bool);
	~Net();
	string getName();
	bool isVirtual();
	void addAssignedNet(Net *);
	vector<Net *> & getAssignedTo();
	string getAssignedValue();
	void setAssignedValue(string);
	Parasitic * getParasitic();
	void setParasitic(Parasitic *);

};

typedef map<string, Net *> net_map;
typedef pair<string, Net *> net_entry;

#endif /* NET_HPP_ */

