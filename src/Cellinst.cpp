/*
 * Cellinst.cpp
 *
 *  Created on: May 6, 2014
 *      Author: hrkalona
 */

#include "Cellinst.hpp"
#include <iostream>
#include "CellEdge.hpp"
#include "Util.hpp"

Cellinst::Cellinst(Cell * cell, string instance_name)
{
	this->cell = cell;
	this->instance_name = instance_name;
	resized = false;
	examined = false;
	sequential = false;
	scale_factor = 1;
	level = 0;
	visited = false;
	incremental_change = false;

#if RESIZE_FIRST_CELL_ON_PATH
	locked = false;
#endif

}

void Cellinst::addInputPin(Node * input)
{
	inputs.push_back( input );
}

void Cellinst::setSequential(bool seq)
{
	sequential = seq;
}

bool Cellinst::isSequential()
{
	return sequential;
}

void Cellinst::addOutputPin(Node * output)
{
	outputs.push_back( output );
}

Cell * Cellinst::getCell()
{
	return cell;
}

string Cellinst::getInstanceName()
{
	return instance_name;
}

void Cellinst::setCell(Cell * cell)
{
	this->cell = cell;
}

vector<Node *>& Cellinst::getInputs()
{
	return inputs;
}

vector<Node *>& Cellinst::getOutputs()
{
	return outputs;
}

void Cellinst::substitute(Cell *new_cell)
{
	cell = new_cell;

	//string name = cell_name + "<" + instance_name + ">";

	for (unsigned int i = 0; i < outputs.size(); i++)
	{
		outputs[i]->setPin( new_cell->getOutput( outputs[i]->getPin()->getName() ) );
		//outputs[i]->setName(name + ":" + outputs[i]->getPin()->getName());

	}

	for (unsigned int i = 0; i < inputs.size(); i++)
	{
		inputs[i]->setPin( new_cell->getInput( inputs[i]->getPin()->getName() ) );
		//inputs[i]->setName(name + ":" + inputs[i]->getPin()->getName());

		for (unsigned int j = 0; j < inputs[i]->getForwardEdges().size(); j++)
		{
			Edge * e = inputs[i]->getForwardEdges()[j];
			Node * u = e->getConnectedTo();

			if (e->getNetType() == CELL_EDGE)
			{
				Timing * temp = Util::getTiming( inputs[i]->getPin(), u->getPin() );

				(( CellEdge * ) e)->setTiming( temp );

				(( CellEdge * ) e->getOppositeEdge())->setTiming( temp );
			}
		}
	}

}

void Cellinst::setExamined(bool examined)
{
	this->examined = examined;
}

bool Cellinst::isExamined()
{
	return examined;
}

void Cellinst::setResized(bool resized)
{
	this->resized = resized;
}

bool Cellinst::isResized()
{
	return resized;
}

double Cellinst::getScaleFactor()
{
	return scale_factor;
}

void Cellinst::setScaleFactor(double scale_factor)
{
	this->scale_factor = scale_factor;
}

int Cellinst::getLevel()
{
	return level;
}

void Cellinst::setLevel(int level)
{
	this->level = level;
}

bool Cellinst::isVisited()
{
	return visited;
}

void Cellinst::setVisited(bool visited)
{
	this->visited = visited;
}

void Cellinst::setIncrementalChange(bool incremental_change)
{
	this->incremental_change = incremental_change;
}

bool Cellinst::isIncrementalChanged()
{
	return incremental_change;
}

#if RESIZE_FIRST_CELL_ON_PATH
void Cellinst::setLocked(bool locked)
{
	this->locked = locked;
}

bool Cellinst::isLocked()
{
	return locked;
}
#endif

