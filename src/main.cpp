/*
 * main.c
 *
 *  Created on: Nov 11, 2013
 *      Author: babis
 */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sys/time.h>
#include <time.h>

#include <string>

#include "types.hpp"
#include "System.hpp"
#include "Graph.hpp"

using namespace std;

int main(int argc, char* argv[])
{
	/* Tile */
	cout << "NanoTrim ~ Transistor Continuous Width Resizing Tool \n\n";

#if BENCHMARKING
	struct timeval calc_start, calc_end;
#endif

#if BENCHMARKING
	gettimeofday( &calc_start, NULL );
#endif

	System system;

	system.inputHandler( argc, argv );

	system.engine();

	system.outputHandler();

#if BENCHMARKING
	gettimeofday( &calc_end, NULL );

	/* !!!for benchmarking!!! */
	cout << "\n\nDone in, " << (calc_end.tv_sec * 1000 + calc_end.tv_usec / 1000) - (calc_start.tv_sec * 1000 + calc_start.tv_usec / 1000)
			<< " ms.\n\n";
#endif

	return 0;
}

