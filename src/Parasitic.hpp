/*
 * Parasitic.hpp
 *
 *  Created on: Oct 1, 2014
 *      Author: hrkalona
 */

#ifndef PARASITIC_HPP_
#define PARASITIC_HPP_

#include <string>
#include <map>
#include "Node.hpp"
#include <set>

typedef enum
{
	PORT = 0, TAP = 1, INTERNAL = 2
} P_NODE_TYPE;

typedef struct _p_node
{
	string name;
	double capacitance;

	/* graph node points to a node in the graph. if it is a null pointer then the _p_node is a node in the parasitic tree */
	Node* graph_node;

	/* _p_edge has similar functionality as the graph edge */
	vector<struct _p_edge *> adjacent;

	/* the type of the specific node */
	P_NODE_TYPE type;

	bool visited;

	double downstream_cap;
	double downstream_tap_fall_cap;
	double downstream_tap_rise_cap;

	double downstream_product_fall;
	double downstream_product_rise;

	double delay_fall;
	double delay_rise;

	_p_node() :
			capacitance( 0 ), graph_node( NULL ), visited( false ), downstream_cap( 0 ), downstream_tap_fall_cap( 0 ), downstream_tap_rise_cap( 0 ), downstream_product_fall(
					0 ), downstream_product_rise( 0 ), delay_fall( 0 ), delay_rise( 0 ), type( INTERNAL )
	{
	}

} p_node;

typedef struct _p_edge
{
	double resistance;

	struct _p_node * connected_to_a;
	struct _p_node * connected_to_b;

	bool visited;

	_p_edge() :
			resistance( 0 ), connected_to_a( NULL ), connected_to_b( NULL ), visited( false )
	{
	}

} p_edge;

/* sort edges with respect to their resistance value */
struct sort_edge
{
	bool operator()(const p_edge* lhs, const p_edge* rhs) const
	{
		if (lhs->resistance == rhs->resistance)
			return lhs->resistance > rhs->resistance;
		else
			return lhs->resistance > rhs->resistance;
	}
};

typedef map<string, p_node *> p_node_map;
typedef pair<string, p_node *> p_node_entry;

class Parasitic
{
private:
	p_node_map parasitic_nodes;
	multiset<p_edge *, sort_edge> edges;  // A sorted list of p_edges

	p_node * start_point;  // points to the begining of the wire/net
	vector<p_node *> end_points;  // a list of the end points of the wire/net

	/* original name is the net's name as it is found in Verilog
	 * while mapped_name is the a name alias of the original name
	 * as it is found (if any) in the SPEF file */

	string original_name;
	string mapped_name;

	double total_capacitance;

	/* Helper variable, we only want the parasitic errors to be printed only for the first pass */
	bool show_errors;

public:
	Parasitic(string);
	~Parasitic();

	void setStartPoint(Node *);
	void addEndPoint(Node *);
	p_node* addPoint(string, p_node *);

	void setOriginalName(string);
	string getOriginalName();

	void setMappedName(string);
	string getMappedName();

	double getTotalCapacitance();

	void elmoreDelayCalculation();
	void bFactorCalculation();

	p_node * getStartPoint();
	vector<p_node *>& getEndPoints();

	bool hasStartPoint();
	bool hasEndPoints();

	void addEdge(p_edge *);

private:

	void connectionCheck();

	void plotErroneousNet(string);

	void maximumSpanningTree();

	void reIninitialize();

	int recursiveCapacitanceCalculation(p_node *);
	int recursiveCacitanceDelayProductCalculation(p_node *);

};

#endif /* PARASITIC_HPP_ */
