/*
 * Path.hpp
 *
 *  Created on: Jun 2, 2014
 *      Author: hrkalona
 */

#ifndef PATH_HPP_
#define PATH_HPP_

#include <vector>
#include <math.h>
#include "Node.hpp"
#include "Cellinst.hpp"
#include "Cell.hpp"
#include "Pin.hpp"
#include "Parasitic.hpp"

using namespace std;

class Path
{
private:
	/* The nodes that compose the path */
	vector<Node *> nodes;

	/* The slope @each node of the path */
	vector<SLOPE> slopes;

	/* The total delay of the path */
	double delay;


public:
	Path();
	~Path()
	{
	}
	;
	void addNode(Node *);
	void setNodes(vector<Node *>);
	void setSlopes(vector<SLOPE>);
	void addSlope(SLOPE);
	vector<SLOPE> getSlopesConst() const;
	vector<Node *> & getPath();
	vector<Node *> getPathConst() const;
	double getDelay() const;
	void setDelay(double);
	bool unifiedLogicalEffort(cell_map&, set<Parasitic *>&, vector<Cellinst *>&, double, bool, int &, int &, int&, int, double);
	void reset();

	bool operator<(const Path&) const;
	bool operator==(const Path&) const;

};

#endif /* PATH_HPP_ */
