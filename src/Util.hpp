/*
 * Util.hpp
 *
 *  Created on: Sep 22, 2014
 *      Author: hrkalona
 */

#ifndef UTIL_HPP_
#define UTIL_HPP_

#include "types.hpp"
#include "Node.hpp"
#include "Timing.hpp"
#include "OutputPin.hpp"
#include "Cell.hpp"
#include "Cellinst.hpp"
#include "Parasitic.hpp"
#include "Path.hpp"

using namespace std;

class Util
{
public:
	static INTERP_VARIABLE variable_1;
	static INTERP_VARIABLE variable_2;

public:

	static void DrawProgressBar(int, double);
	static double bilinearInterpolation(tables *, double, double);
	static double calculateOutputCapacitance(Node *, SLOPE);
	static Timing * getTiming(Pin *, Pin *);
#if RESIZE_FIRST_CELL_ON_PATH
	static void resize_first_cell(int, Path&, cell_map &, int&, int&, int&, vector<Cellinst *>&, set<Parasitic *>&);
#endif
	static bool resize(cell_map&, Cellinst *, string, double, SLOPE);
};



#endif /* UTIL_HPP_ */
