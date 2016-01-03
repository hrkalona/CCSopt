/*
 * Node.hpp
 *
 *  Created on: May 8, 2014
 *      Author: hrkalona
 */

#ifndef NODE_HPP_
#define NODE_HPP_

#include <string>
#include "types.hpp"
#include "Edge.hpp"
#include "Pin.hpp"

class Cellinst;
class Net;

using namespace std;

class Node
{
private:
	string name;
	Cellinst *cell; // points to Cellinst that belongs to
	Pin *pin;
	Net *net; // points to Net that belongs to
	vector<Edge *> forward; // forward edges
	vector<Edge *> backward; // backward edges
	//bool visited;

	double at_late_rise;
	double at_late_fall;

	double Co_r;
	double Co_f;

	double tr_late_f;
	double tr_late_r;

	double user_capacitance_fall;
	double user_capacitance_rise;

	bool used_as_terminal;

	Node * previous_rise;
	bool inverted_rise;

	Node * previous_fall;
	bool inverted_fall;

public:
	Node(string, Pin*);
	~Node();
	string getName();
	Pin* getPin();
	void setName(string);
	void setPin(Pin *);
	Net* getNet();
	void addForwardEdge(Edge *);
	void addBackwardEdge(Edge *);
	vector<Edge *>& getForwardEdges();
	vector<Edge *>& getBackwardEdges();
	void setCellInst(Cellinst *);
	Cellinst * getCellInst();
	void setNet(Net *);
	//void setVisited(bool);
	//bool isVisited();
	double getCoRise();
	double getCoFall();
	double getTrLateRise();
	double getTrLateFall();
	double getAtLateRise();
	double getAtLateFall();
	void setCoRise(double);
	void setCoFall(double);
	void setTrLateRise(double);
	void setTrLateFall(double);
	void setAtLateRise(double);
	void setAtLateFall(double);
	void setUserCapacitanceFall(double);
	double getUserCapacitanceFall();
	void setUserCapacitanceRise(double);
	double getUserCapacitanceRise();

	void setUsedAsTerminal(bool);
	bool usedAsTerminal();

	void setPreviousRise(Node *);
	void setPreviousFall(Node *);
	void setInvertedRise(bool);
	void setInvertedFall(bool);

	Node * getPreviousRise();
	Node * getPreviousFall();
	bool getInvertedRise();
	bool getInvertedFall();

};

typedef map<string, Node *> node_map;
typedef pair<string, Node *> node_entry;

#endif /* NODE_HPP_ */
