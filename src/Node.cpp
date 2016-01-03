/*
 * Node.cpp
 *
 *  Created on: May 8, 2014
 *      Author: hrkalona
 */

#include "Node.hpp"
#include <limits>

Node::Node(string name, Pin* pin)
{
	this->name = name;
	this->pin = pin;
	cell = NULL;
	net = NULL;
	Co_f = -1;
	Co_r = -1;

	user_capacitance_fall = -1;
	user_capacitance_rise = -1;

	tr_late_f = -numeric_limits<double>::max();
	tr_late_r = -numeric_limits<double>::max();

	at_late_fall = -numeric_limits<double>::max();
	at_late_rise = -numeric_limits<double>::max();
	//visited = false;

	used_as_terminal = false;

	previous_rise = NULL;
	inverted_rise = false;

	previous_fall = NULL;
	inverted_fall = false;

}

void Node::setName(string name)
{
	this->name = name;
}

Node::~Node()
{
	for (unsigned int i = 0; i < forward.size(); i++)
	{
		delete forward[i];
	}

	for (unsigned int i = 0; i < backward.size(); i++)
	{
		delete backward[i];
	}
}

Pin* Node::getPin()
{
	return pin;
}

void Node::setPin(Pin * pin)
{
	this->pin = pin;
}

Net* Node::getNet()
{
	return net;
}

void Node::addForwardEdge(Edge * edge)
{
	forward.push_back( edge );
}

void Node::addBackwardEdge(Edge * edge)
{
	backward.push_back( edge );
}

vector<Edge *>& Node::getForwardEdges()
{
	return forward;
}

vector<Edge *>& Node::getBackwardEdges()
{
	return backward;
}

string Node::getName()
{
	return name;
}

void Node::setCellInst(Cellinst *cell)
{
	this->cell = cell;
}

void Node::setNet(Net *net)
{
	this->net = net;
}

/*void Node::setVisited(bool visited)
{
	this->visited = visited;
}

bool Node::isVisited()
{
	return visited;
}*/

double Node::getCoRise()
{
	return Co_r;
}

double Node::getCoFall()
{
	return Co_f;
}

double Node::getTrLateRise()
{
	return tr_late_r;
}

double Node::getTrLateFall()
{
	return tr_late_f;
}

double Node::getAtLateRise()
{
	return at_late_rise;
}

double Node::getAtLateFall()
{
	return at_late_fall;
}

void Node::setCoRise(double Co_r)
{
	this->Co_r = Co_r;
}

void Node::setCoFall(double Co_f)
{
	this->Co_f = Co_f;
}

void Node::setTrLateRise(double tr_late_r)
{
	this->tr_late_r = tr_late_r;
}

void Node::setTrLateFall(double tr_late_f)
{
	this->tr_late_f = tr_late_f;
}

void Node::setAtLateRise(double at_late_rise)
{
	this->at_late_rise = at_late_rise;
}

void Node::setAtLateFall(double at_late_fall)
{
	this->at_late_fall = at_late_fall;
}

Cellinst * Node::getCellInst()
{
	return cell;
}

void Node::setUserCapacitanceFall(double cap)
{
	user_capacitance_fall = cap;
}

double Node::getUserCapacitanceFall()
{
	return user_capacitance_fall;
}

void Node::setUserCapacitanceRise(double cap)
{
	user_capacitance_rise = cap;
}

double Node::getUserCapacitanceRise()
{
	return user_capacitance_rise;
}

void Node::setUsedAsTerminal(bool used_as_terminal)
{
	this->used_as_terminal = used_as_terminal;
}

bool Node::usedAsTerminal()
{
	return used_as_terminal;
}

void Node::setPreviousRise(Node * node)
{
	previous_rise = node;
}

void Node::setPreviousFall(Node * node)
{
	previous_fall = node;
}

void Node::setInvertedRise(bool opt)
{
	inverted_rise = opt;
}

void Node::setInvertedFall(bool opt)
{
	inverted_fall = opt;
}

Node * Node::getPreviousRise()
{
	return previous_rise;
}

Node * Node::getPreviousFall()
{
	return previous_fall;
}

bool Node::getInvertedRise()
{
	return inverted_rise;
}

bool Node::getInvertedFall()
{
	return inverted_fall;
}


