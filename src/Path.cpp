/*
 * Path.cpp
 *
 *  Created on: Jun 2, 2014
 *      Author: hrkalona
 */

#include "Path.hpp"
#include "OutputPin.hpp"
#include "InputPin.hpp"
#include "types.hpp"
#include <iostream>
#include <boost/algorithm/string/predicate.hpp>
#include "NetEdge.hpp"
#include "Util.hpp"
#include "Net.hpp"

using namespace boost;

Path::Path()
{
	delay = 0;
}

void Path::reset()
{
	delay = 0;

	nodes.clear();
	slopes.clear();
}

bool Path::operator<(const Path& rhs) const
{
	return delay > rhs.delay;
}

bool Path::operator==(const Path& rhs) const
{
	return delay == rhs.delay;
}

void Path::setDelay(double delay)
{
	this->delay = delay;
}

void Path::addNode(Node * node)
{
	nodes.push_back( node );
}

void Path::setNodes(vector<Node *> path)
{
	nodes = path;
}

void Path::setSlopes(vector<SLOPE> slopes)
{
	this->slopes = slopes;
}

vector<Node *> & Path::getPath()
{
	return nodes;
}

vector<Node *> Path::getPathConst() const
{
	return nodes;
}

vector<SLOPE> Path::getSlopesConst() const
{
	return slopes;
}

double Path::getDelay() const
{
	return delay;
}

void Path::addSlope(SLOPE slope)
{
	slopes.push_back( slope );
}

bool Path::unifiedLogicalEffort(cell_map& cells, set<Parasitic *>& changed_nets, vector<Cellinst *>& changed_cells, double tau, bool upsizing,
		int & cells_examined, int & cells_resized, int & cells_potential_resized, int max_iterations, double tolerance)
{

	vector<double> initial_cell_cap;
	vector<double> output_cap;
	vector<double> current_cap;
	vector<double> g;
	vector<Cellinst *> cell_instances;

	double on_path_cap = 0;

	for (unsigned int i = 0; i < nodes.size(); i++)
	{
		if (nodes[i]->getPin() != NULL)
		{
			if (nodes[i]->getPin()->getType() == INPUT)
			{
				on_path_cap = slopes[i] == FALL ? (( InputPin * ) nodes[i]->getPin())->getCfall() : (( InputPin * ) nodes[i]->getPin())->getCrise();
				initial_cell_cap.push_back( on_path_cap );

				if (slopes[i] == FALL)
				{
					g.push_back( (( InputPin * ) nodes[i]->getPin())->getGfall() );
				}
				else
				{
					g.push_back( (( InputPin * ) nodes[i]->getPin())->getGrise() );
				}
			}
			else if (nodes[i]->getPin()->getType() == OUTPUT)
			{
				//output_cap.push_back( Util::calculateOutputCapacitance( nodes[i], slopes[i] ) - on_path_cap );
				if (slopes[i] == FALL)
				{
					output_cap.push_back( nodes[i]->getCoFall() - on_path_cap );
				}
				else
				{
					output_cap.push_back( nodes[i]->getCoRise() - on_path_cap );
				}

				cell_instances.push_back( nodes[i]->getCellInst() );
			}
		}
	}

	current_cap = initial_cell_cap;

	/* #stages of the path */
	int stages = initial_cell_cap.size();

#if DEBUG
	cout << "\nLogical Effort method:\n";
#endif

	int iteration = 0;

	bool converged = true;

	if (stages > 1)
	{

		do
		{
			double a = 0;
			int cell_counter = 0;

			double C_total_out_i = 0;
			double gi = 0;
			double gi_1 = 0;
			double Rwi_1 = 0;
			double Ci_1 = 0;
			double old_Ci = 0;

			converged = true;

			iteration++;
#if DEBUG
			cout << "Iteration: " << iteration << "\n";
#endif

			for (unsigned int i = 0; i < nodes.size(); i++)  // UNIFIED LOGICAL EFFORT ALGORITHM
			{
				if (cell_counter == stages - 1)
				{
					break;
				}

				if (nodes[i]->getPin() != NULL && nodes[i]->getPin()->getType() == INPUT)  // On path cell
				{
					Edge * temp_edge;
					for (unsigned int k = 0; k < nodes[i]->getBackwardEdges().size(); k++)
					{
						if (nodes[i]->getBackwardEdges()[k]->getConnectedTo() == nodes[i + 1])
						{
							temp_edge = nodes[i]->getBackwardEdges()[k];
							break;
						}
					}

					Rwi_1 = (( NetEdge * ) temp_edge)->getResistance();

					Ci_1 = current_cap[cell_counter + 1];
					gi = g[cell_counter];
					gi_1 = g[cell_counter + 1];

					on_path_cap = cell_counter == 0 ? 0 : current_cap[cell_counter - 1];

					C_total_out_i = output_cap[cell_counter] + on_path_cap;

					double Ci = sqrt( (gi * Ci_1 * (C_total_out_i)) / (gi_1 + (Rwi_1 * Ci_1) / tau) );

					old_Ci = current_cap[cell_counter];

					current_cap[cell_counter] = Ci;

#if DEBUG

					double a = Ci / initial_cell_cap[cell_counter];

#endif

					if ((abs( Ci - old_Ci ) / abs( Ci )) > tolerance)
					{
						converged = false;
					}

					cell_counter++;

#if DEBUG
					cout << nodes[i]->getName() << " : gi = " << gi << "\n";
					cout << nodes[i]->getName() << " : gi_1 = " << gi_1 << "\n";
					cout << nodes[i]->getName() << " : Rwi_1 = " << Rwi_1 << "\n";
					cout << nodes[i]->getName() << " : Ci_1 = " << Ci_1 << "\n";
					cout << nodes[i]->getName() << " : C_total_out_i = " << C_total_out_i << "\n";
					cout << nodes[i]->getName() << " : old_Ci = " << old_Ci << "\n";
					cout << nodes[i]->getName() << " : Ci = " << Ci << "\n";
					cout << nodes[i]->getName() << " : a = " << a << "\n\n";

#endif

				}
			}

		} while (!converged && iteration < max_iterations);

		if (iteration >= max_iterations)
		{
			cout << "\nULE failed to converged in " << max_iterations << " iterations.\n";
			exit( -1 );
		}

#if 1
		/* Upsize checking */
		if (upsizing == false)
		{
			int cell_counter = 0;

			for (; cell_counter < initial_cell_cap.size(); cell_counter++)
			{
				if (current_cap[cell_counter] > initial_cell_cap[cell_counter])
				{
					if (cell_instances[cell_counter]->isExamined() == false)
					{
						cells_examined++;
						cell_instances[cell_counter]->setExamined( true );
					}

#if DEBUG
					cout << "Cannot resize the cell " << cell_instances[cell_counter]->getCell()->getName() << " higher than its original size.\n";
#endif
					return false;
				}
			}
		}
		/*****************/
#endif

	}

	int cell_counter = 0;
	for (unsigned int i = 0; i < nodes.size(); i++)  //Mark onpath + offpath
	{
		if (nodes[i]->getPin() != NULL)
		{
			if (nodes[i]->getPin()->getType() == OUTPUT && cell_counter < stages - 1) // Dont do the offpath marking for the first cell
			{
				//cout << "Marking " << nodes[i]->getCellInst()->getCell()->getName() << "<" << nodes[i]->getCellInst()->getInstanceName() << ">\n";

				for (unsigned j = 0; j < nodes[i]->getForwardEdges().size(); j++)
				{
					Node * t = nodes[i]->getForwardEdges()[j]->getConnectedTo();

					if (t->getCellInst() != NULL && t->getCellInst()->isExamined() == false)
					{
						t->getCellInst()->setExamined( true );
						cells_examined++;
						//cout << "Marking " << t->getCellInst()->getCell()->getName() << "<" << t->getCellInst()->getInstanceName() << ">\n";
					}

				}

			}
			else if (nodes[i]->getPin()->getType() == INPUT)
			{
				bool changed = false;

				if (current_cap[cell_counter] != initial_cell_cap[cell_counter])
				{
					cells_potential_resized++;
					changed = Util::resize( cells, nodes[i]->getCellInst(), nodes[i]->getPin()->getName(), current_cap[cell_counter], slopes[i] );
				}
#if RESIZE_FIRST_CELL_ON_PATH
				else
				{
					nodes[i]->getCellInst()->setLocked( true );
				}
#endif

				if (nodes[i]->getCellInst()->isExamined() == false)
				{
					nodes[i]->getCellInst()->setExamined( true );
					cells_examined++;
				}

				if (nodes[i]->getCellInst()->isResized() == false && changed)
				{
					nodes[i]->getCellInst()->setResized( true );
					cells_resized++;

					changed_cells.push_back( nodes[i]->getCellInst() );

					for (unsigned int k = 0; k < nodes[i]->getCellInst()->getInputs().size(); k++)
					{
						Node * a = nodes[i]->getCellInst()->getInputs()[k];

						if (a->getNet() != NULL)
						{
							changed_nets.insert( a->getNet()->getParasitic() );

#if DEBUG
							cout << "Affected Net " << a->getNet()->getParasitic()->getMappedName() << " "
							<< a->getNet()->getParasitic()->getOriginalName() << "\n";
#endif

						}
					}

					double new_cell_cin = (
							slopes[i] == FALL ? (( InputPin * ) nodes[i]->getPin())->getCfall() : (( InputPin * ) nodes[i]->getPin())->getCrise());

					nodes[i]->getCellInst()->setScaleFactor( new_cell_cin / initial_cell_cap[cell_counter] );
				}

				cell_counter++;
			}
		}
	}

	return true;

}

