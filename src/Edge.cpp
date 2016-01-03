/*
 * Edge.cpp
 *
 *  Created on: May 8, 2014
 *      Author: hrkalona
 */
#include "Edge.hpp"

Edge::Edge(string name, double dr, double df)
{
	this->name = name;
	this->dr = dr;
	this->df = df;
	connected_to = NULL;
	blocked = false;
	opposite_edge = NULL;

}

void Edge::setConnectedTo(Node * node)
{
	connected_to = node;
}

string Edge::getName()
{
	return name;
}

Node * Edge::getConnectedTo()
{
	return connected_to;
}

void Edge::setDelayRise(double dr)
{
	this->dr = dr;
}

void Edge::setDelayFall(double df)
{
	this->df = df;
}

double Edge::getDelayRise()
{
	return dr;
}

double Edge::getDelayFall()
{
	return df;
}

void Edge::setOppositeEdge(Edge * opposite_edge)
{
	this->opposite_edge = opposite_edge;
}

Edge * Edge::getOppositeEdge()
{
	return opposite_edge;
}

bool Edge::isBlocked()
{
	return blocked;
}

void Edge::setBlocked(bool opt)
{
	blocked = opt;
}

