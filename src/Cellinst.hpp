/*
 * Cellinst.hpp
 *
 *  Created on: May 6, 2014
 *      Author: hrkalona
 */

#ifndef CELLINST_HPP_
#define CELLINST_HPP_

#include <vector>
#include "types.hpp"
#include "Cell.hpp"
#include "Node.hpp"

using namespace std;

class Cellinst
{
private:
	Cell * cell; // points to the type of the Cellinst
	string instance_name;
	vector<Node *> inputs; // Keeps the graph nodes that serve as inputs to the Cellinst
	vector<Node *> outputs; // Keeps the graph nodes that serve as outputs to the Cellinst
	double scale_factor;
	bool resized; // has been resized?
	bool examined; // has been examined?
	bool sequential; // is it a sequential element?
	int level; // What's the level of the Cellinst?
	bool visited;
	bool incremental_change; // Is it affected by a Cellinst substitution occuring durimg Cellinst resizing?
#if RESIZE_FIRST_CELL_ON_PATH
	bool locked;
#endif

public:
	Cellinst(Cell *, string);

	~Cellinst()
	{
	}
	void addInputPin(Node *);
	void addOutputPin(Node *);
	Cell * getCell();
	void setCell(Cell *);
	string getInstanceName();
	vector<Node *>& getInputs();
	vector<Node *>& getOutputs();

	void substitute(Cell *);

	void setExamined(bool);
	bool isExamined();
	void setResized(bool);
	bool isResized();
	void setSequential(bool);
	bool isSequential();

	double getScaleFactor();
	void setScaleFactor(double);

	int getLevel();
	void setLevel(int);

	bool isVisited();
	void setVisited(bool);

	void setIncrementalChange(bool);
	bool isIncrementalChanged();

#if RESIZE_FIRST_CELL_ON_PATH
	void setLocked(bool);
	bool isLocked();
#endif

};


typedef map<string, Cellinst *> cellinst_map;
typedef pair<string, Cellinst *> cellinst_entry;

#endif /* CELLINST_HPP_ */
