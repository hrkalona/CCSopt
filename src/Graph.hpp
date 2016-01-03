/*
 * Graph.hpp
 *
 *  Created on: May 8, 2014
 *      Author: hrkalona
 */

#ifndef GRAPH_HPP_
#define GRAPH_HPP_

#include "Path.hpp"
#include <string>
#include "NetEdge.hpp"
#include <vector>
#include <queue>
#include <fstream>

#include "Parasitic.hpp"


using namespace std;

class Path;

typedef struct _t_node
{
	Node * terminal;
	int level;

	_t_node() :
			terminal( NULL ), level( -1 )
	{
	}

	bool operator<(const _t_node& rhs) const
	{
		return level > rhs.level;
	}

	bool operator==(const _t_node& rhs) const
	{
		return level == rhs.level;
	}
} T_node;

class Graph
{
private:

	/* Top module name */
	string name;

	/* internal nodes.
	 * Each node in the graph corresponde to a pin in the circuit */
	node_map int_nodes;

	/* The nodes below correspond to the
	 * primary inputs (PIs) of the circuit */
	node_map inputs;

	/* The nodes below correspond to the
	 * primary output (POs) of the circuit */
	node_map outputs;

	/* a virtual node is a PI from where a virtual net begins.
	 * See virtual net definition in Net.hpp */
	node_map virtual_nodes;

	/* The nodes below correspond to the nodes from which
	 * critical path tracing initiates */

	multiset<T_node> terminal_nodes;
	multiset<T_node>::iterator critical_path_node;

	Path critical_path;

	int total_levels;
	vector<int> cells_at_level;
	Cellinst *** cells_hierarchy;

	bool report_timing;
	int num_paths;
	PATH_TYPE pt;
	bool preset_clear;
	bool upsizing;

	int cells_no_design;
	int cells_examined;
	int cells_resized;
	int cells_potentially_resized;

	int performed_stas;
	int iteration;

#if TESTING
	bool first_path_found;
#endif

#if RESIZE_FIRST_CELL_ON_PATH
	long int path_counter;
#endif

public:
	Graph();
	~Graph();
	node_map& getInputNodes();
	node_map& getOutputNodes();
	node_map& getInternalNodes();
	node_map& getVirtualNodes();
	Node * addInputNode(string, Pin *);
	Node * addOutputNode(string, Pin*);
	Node * addInternalNode(string, Pin*);
	Node * addVirtualNode(string, Pin*);
	void setName(string);
	string getName();
	bool isInput(string);
	bool isOutput(string);
	void resizer(cell_map &, cellinst_map&, vector<Parasitic *>&, double, int, int, int, int, int, double);
	void setReportTiming(bool);
	void setNumPaths(int);
	void setPathType(PATH_TYPE);
	bool getReportTiming();
	void setPresetClear(bool);
	void setUpsizingAction(bool);

private:
	void levelization(cellinst_map&);
	void sta(vector<Parasitic *>&);
	void staIncremental(vector<Parasitic *>&, vector<Cellinst *>&);
	int findMinimunLevel(vector<Cellinst *>&, set<Node *>&);
	void findCriticalPath();
	void findSubPaths(cell_map &, vector<Parasitic *>&, vector<Cellinst *>&, double, int, double);
	void reIninitialize();
	void reIninitializeIncremental();
	void initializeTerminalNodes();
	void addNewTerminalNodes();
	void findCriticalPaths(Node *, multiset<Path>&);
	void pathTokenizer(Path, vector<Path>&);
	void plotGraph();

};

#endif /* GRAPH_HPP_ */
