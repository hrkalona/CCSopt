/*
 * Edge.hpp
 *
 *  Created on: May 8, 2014
 *      Author: hrkalona
 */

#ifndef EDGE_HPP_
#define EDGE_HPP_

#include <string>
#include "types.hpp"

class Node;

using namespace std;

class Edge
{
private:
	string name;
	Node * connected_to; // the end point of the directed edge
	Edge * opposite_edge; 	// 	points to the opposite direction edge.

	double dr; // delay rise
	double df; // delay fall
	bool blocked; // TODO: What's that ??

public:
	Edge(string, double, double);
	virtual ~Edge() {};
	string getName();
	virtual NET_TYPE getNetType() = 0;
	void setConnectedTo(Node * node);
	Node * getConnectedTo();
	void setDelayRise(double);
	void setDelayFall(double);
	double getDelayRise();
	double getDelayFall();
	void setOppositeEdge(Edge *);
	Edge * getOppositeEdge();
	bool isBlocked();
	void setBlocked(bool);

};

#endif /* EDGE_HPP_ */
