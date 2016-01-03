/*
 * types.hpp
 *
 *  Created on: Nov 11, 2013
 *      Author: babis
 */

#ifndef TYPES_HPP_
#define TYPES_HPP_

#include <string>
#include <vector>
#include <map>
#include <set>

#include "si2dr_liberty.h"

#define GRAPHING 0
#define HISTOGRAM 0
#define DEBUG 0
#define LIB_VERIFICATION 0
#define ALL_PATHS -1
#define BENCHMARKING 1
#define CRIT_PATH_UPPER_BOUND 0
#define INVALID_LEVEL 100000
#define TESTING 0
#define RESIZE_FIRST_CELL_ON_PATH 0

#if RESIZE_FIRST_CELL_ON_PATH
#define PATH_LIMIT 10
#endif

using namespace std;

typedef enum
{
	ERROR_CODE = -1, INPUT = 0, OUTPUT = 1, UNUSED = 2
} NODE_TYPE;

typedef enum
{
	POSITIVE = 0, NEGATIVE = 1, NON = 2
} UNATE;

typedef enum
{
	NO_TYPE = 0, CLEAR = 1, PRESET = 2, RISING_EDGE = 3, FALLING_EDGE = 4, COMBINATIONAL = 5
} ARC_TYPE;

typedef enum
{
	 NO_SLOPE = -1, RISE = 0, FALL = 1
} SLOPE;

typedef enum
{
	CELL_EDGE = 0, NET_EDGE = 1
} NET_TYPE;

typedef enum
{
	NO_ACTION = 0, MIN_SIZE = 1
} RESIZE_SEQUENTIAL_ACTION;

typedef enum
{
	OUT = 0, ALL = 1, REG = 2
} PATH_TYPE;

typedef enum
{
	UNDEFINED = -1, TOTAL_OUTPUT_NET_CAPACITANCE = 0, INPUT_NET_TRANSITION = 1
} INTERP_VARIABLE;

#define UNIT_INVALID_VAL -1000

typedef struct liberty_value_data tables;


#endif /* TYPES_HPP_ */
