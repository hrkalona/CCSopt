/*
 * Util.cpp
 *
 *  Created on: Sep 22, 2014
 *      Author: hrkalona
 */
#include "Util.hpp"
#include "InputPin.hpp"
#include "Edge.hpp"
#include "NetEdge.hpp"
#include <limits>
#include <cmath>
#include <iostream>
#include "Net.hpp"
#include "types.hpp"

INTERP_VARIABLE Util::variable_1 = UNDEFINED;
INTERP_VARIABLE Util::variable_2 = UNDEFINED;

double Util::bilinearInterpolation(tables * timing_tab, double t, double c)
{
	if (timing_tab->dimensions == 1)
	{
		return timing_tab->values[0];
	}

	double x_0;
	double y_0;

	if (variable_1 == INPUT_NET_TRANSITION)
	{
		x_0 = t;
		y_0 = c;
	}
	else
	{
		x_0 = c;
		y_0 = t;
	}

	if (timing_tab->dim_sizes[0] > 1 && timing_tab->dim_sizes[1] > 1)
	{
		unsigned int x1, y1, x2, y2;

		for (x2 = 0; x2 < timing_tab->dim_sizes[0]; x2++)  // maybe binary search
		{
			if (timing_tab->index_info[0][x2] >= x_0)
			{
				break;
			}
		}

		if (x2 == 0)
		{
			x2 = 1;
		}
		else if (x2 == timing_tab->dim_sizes[0])
		{
			x2 = timing_tab->dim_sizes[0] - 1;
		}

		x1 = x2 - 1;

		for (y2 = 0; y2 < timing_tab->dim_sizes[1]; y2++)  // maybe binary search
		{
			if (timing_tab->index_info[1][y2] >= y_0)
			{
				break;
			}
		}

		if (y2 == 0)
		{
			y2 = 1;
		}
		else if (y2 == timing_tab->dim_sizes[1])
		{
			y2 = timing_tab->dim_sizes[1] - 1;
		}

		y1 = y2 - 1;

		double T_11 = timing_tab->values[x1 * timing_tab->dim_sizes[1] + y1];
		double T_12 = timing_tab->values[x1 * timing_tab->dim_sizes[1] + y2];
		double T_21 = timing_tab->values[x2 * timing_tab->dim_sizes[1] + y1];
		double T_22 = timing_tab->values[x2 * timing_tab->dim_sizes[1] + y2];

		double x_1 = timing_tab->index_info[0][x1];
		double x_2 = timing_tab->index_info[0][x2];
		double y_1 = timing_tab->index_info[1][y1];
		double y_2 = timing_tab->index_info[1][y2];

		double temp1 = x_2 - x_1;
		double temp2 = y_2 - y_1;

		double x_01 = (x_0 - x_1) / temp1;
		double x_20 = (x_2 - x_0) / temp1;
		double y_01 = (y_0 - y_1) / temp2;
		double y_20 = (y_2 - y_0) / temp2;

		return x_20 * y_20 * T_11 + x_20 * y_01 * T_12 + x_01 * y_20 * T_21 + x_01 * y_01 * T_22;
	}
	else if (timing_tab->dim_sizes[0] == 1 && timing_tab->dim_sizes[1] > 1)
	{
		/*unsigned int y1, y2;

		 for (y2 = 0; y2 < timing_tab->dim_sizes[1]; y2++)  // maybe binary search
		 {
		 if (timing_tab->index_info[1][y2] >= y_0)
		 {
		 break;
		 }
		 }

		 if (y2 == 0)
		 {
		 y2 = 1;
		 }
		 else if (y2 == timing_tab->dim_sizes[1])
		 {
		 y2 = timing_tab->dim_sizes[1] - 1;
		 }

		 y1 = y2 - 1;

		 double T_11 = timing_tab->values[y1];
		 double T_12 = timing_tab->values[y2];

		 double y_1 = timing_tab->index_info[1][y1];
		 double y_2 = timing_tab->index_info[1][y2];

		 double temp2 = y_2 - y_1;

		 double y_01 = (y_0 - y_1) / temp2;
		 double y_20 = (y_2 - y_0) / temp2;

		 return y_20 * T_11 + y_01 * T_12;*/
		cout << "Untested 1xm timing table.\n";
		cout << "Terminating.\n";
		exit( -1 );
	}
	else if (timing_tab->dim_sizes[1] == 1 && timing_tab->dim_sizes[0] > 1)
	{
		/*unsigned int x1, x2;

		 for (x2 = 0; x2 < timing_tab->dim_sizes[0]; x2++)  // maybe binary search
		 {
		 if (timing_tab->index_info[0][x2] >= x_0)
		 {
		 break;
		 }
		 }

		 if (x2 == 0)
		 {
		 x2 = 1;
		 }
		 else if (x2 == timing_tab->dim_sizes[0])
		 {
		 x2 = timing_tab->dim_sizes[0] - 1;
		 }

		 x1 = x2 - 1;

		 double T_11 = timing_tab->values[x1 * timing_tab->dim_sizes[1]];
		 double T_21 = timing_tab->values[x2 * timing_tab->dim_sizes[1]];

		 double x_1 = timing_tab->index_info[0][x1];
		 double x_2 = timing_tab->index_info[0][x2];

		 double temp1 = x_2 - x_1;

		 double x_01 = (x_0 - x_1) / temp1;
		 double x_20 = (x_2 - x_0) / temp1;

		 return x_20 * T_11 + x_01 * T_21;*/
		cout << "Untested mx1 timing table.\n";
		cout << "Terminating.\n";
		exit( -1 );
	}

}

double Util::calculateOutputCapacitance(Node * output_pin, SLOPE slope)
{
	double Co = 0;

	/* Calculate the output capacitance based on the slope */

	for (unsigned int i = 0; i < output_pin->getForwardEdges().size(); i++)
	{
		Edge * next_edge = output_pin->getForwardEdges()[i];
		Node * next_node = next_edge->getConnectedTo();
		if (next_node->getPin() == NULL)  // We reached a HELPER NODE
		{
			if (slope == RISE)
			{
				Co += next_node->getUserCapacitanceRise();
			}
			else
			{
				Co += next_node->getUserCapacitanceFall();
			}
		}
		else  // check the inputs of the fanout cells to gather the capacitance
		{
			if (slope == RISE)
			{
				Co += (( InputPin * ) (next_node->getPin()))->getCrise();
			}
			else
			{
				Co += (( InputPin * ) (next_node->getPin()))->getCfall();
			}
		}

		Co += (( NetEdge * ) next_edge)->getCapacitance();
	}

	return Co;
}

Timing * Util::getTiming(Pin * input, Pin * output)
{

	timing_map::iterator it = (( OutputPin * ) output)->getTimings().find( input->getName() );

	if (it == (( OutputPin * ) output)->getTimings().end())
	{
		cout << "Pin " << output->getName() << " does not have " << input->getName() << " as related pin.\n";
		cout << "Terminating.\n";
		exit( -1 );
	}

	return it->second;

}

bool Util::resize(cell_map& cells, Cellinst * cell_to_be_resized, string pin_name, double Cin, SLOPE slope)
{
	cell_map::iterator it;

	string prefix = cell_to_be_resized->getCell()->getPrefix();

	double diff = numeric_limits<double>::max();
	Cell* replacement = NULL;

	for (it = cells.lower_bound( prefix ); it != cells.end() && it->second->getPrefix().compare( prefix ) == 0; it++)
	{
		Pin * temp = it->second->getInput( pin_name );

		double temp_Cin = slope == FALL ? (( InputPin * ) temp)->getCfall() : (( InputPin * ) temp)->getCrise();

		double temp_diff = abs( temp_Cin - Cin );

		if (temp_diff < diff)
		{
			diff = temp_diff;
			replacement = it->second;
		}
	}

	if (replacement != NULL && replacement != cell_to_be_resized->getCell())  // && cell_to_be_resized->isExamined() == false)
	{
#if DEBUG
		cout << "Replacing " << cell_to_be_resized->getCell()->getName() << " with " << replacement->getName() << ", of instance " << cell_to_be_resized->getInstanceName() <<"\n";
		cout << "Cin diff: " << diff << "\n";
#endif
		cell_to_be_resized->substitute( replacement );

		return true;
	}
#if RESIZE_FIRST_CELL_ON_PATH
	else
	{
		cell_to_be_resized->setLocked( true );
#if DEBUG
		cout << "Cannot replace " << cell_to_be_resized->getCell()->getName() << ", of instance " << cell_to_be_resized->getInstanceName() <<"\n";
#endif
	}
#endif

	return false;
}

#if RESIZE_FIRST_CELL_ON_PATH
void Util::resize_first_cell(int path_counter, Path& sub_path, cell_map & cells, int& cells_examined, int& cells_resized,
		int& cells_potentially_resized, vector<Cellinst *>& changed_cells, set<Parasitic *>& changed_nets)
{
	Cellinst * temp_cell = sub_path.getPath()[sub_path.getPath().size() - 1]->getCellInst();

	if (path_counter > PATH_LIMIT)
	{
		if (temp_cell->isSequential() == false && temp_cell->isLocked() == false && temp_cell->isResized() == false)
		{
			Pin * temp_pin = sub_path.getPath()[sub_path.getPath().size() - 1]->getPin();

			if (temp_pin->getType() == INPUT)
			{
				SLOPE temp_slope = sub_path.getSlopesConst()[sub_path.getSlopesConst().size() - 1];
				double cap = temp_slope == FALL ? (( InputPin * ) temp_pin)->getCfall() : (( InputPin * ) temp_pin)->getCrise();

				bool changed = false;

#if DEBUG
				cout << "\nFirst cell on path resizing, for the cell " << temp_cell->getCell()->getName() << ", of instance " << temp_cell->getInstanceName() <<"\n";
#endif

				cells_potentially_resized++;

				changed = resize( cells, temp_cell, temp_pin->getName(), cap * 0.5, temp_slope );

				if (temp_cell->isExamined() == false)
				{
					temp_cell->setExamined( true );
					cells_examined++;
				}

				if (temp_cell->isResized() == false && changed)
				{
					temp_cell->setResized( true );
					cells_resized++;

					changed_cells.push_back( temp_cell );

					for (unsigned int k = 0; k < temp_cell->getInputs().size(); k++)
					{
						Node * a = temp_cell->getInputs()[k];

						if (a->getNet() != NULL)
						{
							changed_nets.insert( a->getNet()->getParasitic() );

#if DEBUG
							cout << "Affected Net " << a->getNet()->getParasitic()->getMappedName() << " "
							<< a->getNet()->getParasitic()->getOriginalName() << "\n";
#endif

						}
					}

					Pin * temp_pin2 = sub_path.getPath()[sub_path.getPath().size() - 1]->getPin();

					double new_cell_cin = (temp_slope == FALL ? (( InputPin * ) temp_pin2)->getCfall() : (( InputPin * ) temp_pin2)->getCrise());

					temp_cell->setScaleFactor( new_cell_cin / cap );
				}

			}
		}
		else
		{
			if (temp_cell->isSequential() == false)
			{
				temp_cell->setLocked( true );
			}
		}

	}
}
#endif

void Util::DrawProgressBar(int len, double percent)
{
//cout << "\x1B[2K";  // Erase the entire current line.
//cout << "\x1B[0E";  // Move to the beginning of the current line.
	cout << "\r";
	string progress;
	for (int i = 0; i < len; ++i)
	{
		if (i < static_cast<int>( len * percent ))
		{
			progress += "=";
		}
		else
		{
			progress += " ";
		}
	}
	cout << "Examined Cells: [" << progress << "] " << (( unsigned int ) (100 * percent)) << "%";
	flush( cout );  // Required.
}
