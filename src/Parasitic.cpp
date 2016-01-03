/*
 * Parasitic.cpp
 *
 *  Created on: Oct 1, 2014
 *      Author: hrkalona
 */
#include "Parasitic.hpp"
#include <iostream>
#include "NetEdge.hpp"
#include "InputPin.hpp"
#include <fstream>

Parasitic::Parasitic(string name)
{
	start_point = NULL;
	total_capacitance = 0;
	original_name = name;
	show_errors = true;
}

void Parasitic::setOriginalName(string name)
{
	this->original_name = name;
}

string Parasitic::getOriginalName()
{
	return original_name;
}

void Parasitic::setMappedName(string name)
{
	this->mapped_name = name;
}

string Parasitic::getMappedName()
{
	return mapped_name;
}

p_node * Parasitic::getStartPoint()
{
	return start_point;
}

vector<p_node *>& Parasitic::getEndPoints()
{
	return end_points;
}

Parasitic::~Parasitic()
{

	if (parasitic_nodes.empty())
	{
		delete start_point;

		for (unsigned int i = 0; i < end_points.size(); i++)
		{
			delete end_points[i];
		}
	}
	else
	{
		p_node_map::iterator p;

		for (p = parasitic_nodes.begin(); p != parasitic_nodes.end(); p++)
		{
			delete p->second;
		}

		multiset<p_edge *, sort_edge>::iterator it;

		for (it = edges.begin(); it != edges.end(); it++)
		{
			delete *it;
		}
	}
}

void Parasitic::setStartPoint(Node * temp_node)
{

	if (start_point == NULL)
	{
		p_node* temp = new p_node();
		temp->type = PORT;
		temp->graph_node = temp_node;
		start_point = temp;
	}

}

void Parasitic::addEndPoint(Node * temp_node)
{

	for (unsigned int i = 0; i < end_points.size(); i++)
	{
		if (end_points[i]->graph_node == temp_node)
		{
			return;
		}
	}

	p_node* temp = new p_node();
	temp->type = TAP;
	temp->graph_node = temp_node;
	end_points.push_back( temp );
}

bool Parasitic::hasStartPoint()
{
	return start_point != NULL;
}

bool Parasitic::hasEndPoints()
{
	return !end_points.empty();
}

p_node* Parasitic::addPoint(string name, p_node* temp_node)
{

	if (temp_node != NULL)
	{
		temp_node->name = name;

		parasitic_nodes.insert( p_node_entry( name, temp_node ) );

		return temp_node;
	}
	else
	{
		p_node_map::iterator it = parasitic_nodes.lower_bound( name );

		if (it != parasitic_nodes.end() && (*it).first.compare( name ) == 0)
		{
			return (*it).second;
		}
		else
		{
			p_node* temp = new p_node();

			temp->name = name;

			parasitic_nodes.insert( it, p_node_entry( name, temp ) );

			return temp;
		}
	}

}

void Parasitic::connectionCheck()
{

	if (show_errors)
	{
		if (parasitic_nodes.empty())
		{
			cout << "Warning net " << original_name << " has no defined parasitic net. Assuming zero wire delay/transition.\n";
		}

		p_node_map::iterator p;
		bool found = false;
		for (p = parasitic_nodes.begin(); p != parasitic_nodes.end(); p++)
		{
			if (p->second->visited == false)
			{
				found = true;
			}
		}

		if (found)
		{
			cout << "Warning net " << mapped_name << " (" << original_name << ") contains disconnected parasitic nodes.\n";
		}
	}

}

double Parasitic::getTotalCapacitance()
{
	return total_capacitance;
}

int Parasitic::recursiveCapacitanceCalculation(p_node * cur)
{

	cur->visited = true;

	cur->downstream_cap += cur->capacitance;

	if (cur->type == TAP)
	{
		if (cur->graph_node->getPin() != NULL)
		{
			cur->downstream_tap_fall_cap += (( InputPin * ) cur->graph_node->getPin())->getCfall();
			cur->downstream_tap_rise_cap += (( InputPin * ) cur->graph_node->getPin())->getCrise();
		}
		else
		{
			cur->downstream_tap_fall_cap += cur->graph_node->getUserCapacitanceFall();
			cur->downstream_tap_rise_cap += cur->graph_node->getUserCapacitanceRise();
		}
	}

	for (unsigned int i = 0; i < cur->adjacent.size(); i++)
	{
		if (cur->adjacent[i]->visited == false)
		{
			cur->adjacent[i]->visited = true;

			if (cur->adjacent[i]->connected_to_a == cur)
			{
				p_node * temp = cur->adjacent[i]->connected_to_b;

				if (temp->visited == false)
				{
					if (recursiveCapacitanceCalculation( temp ) == -1)
					{
						return -1;
					}

					cur->downstream_cap += temp->downstream_cap;
					cur->downstream_tap_fall_cap += temp->downstream_tap_fall_cap;
					cur->downstream_tap_rise_cap += temp->downstream_tap_rise_cap;

				}
				else
				{
					cout << "Warning net " << mapped_name << " (" << original_name << ") contains a resistor loop.\n";
					plotErroneousNet( "erroneous_net" + original_name );
					return -1;
				}
			}
			else
			{
				p_node * temp = cur->adjacent[i]->connected_to_a;

				if (temp->visited == false)
				{
					if (recursiveCapacitanceCalculation( temp ) == -1)
					{
						return -1;
					}

					cur->downstream_cap += temp->downstream_cap;
					cur->downstream_tap_fall_cap += temp->downstream_tap_fall_cap;
					cur->downstream_tap_rise_cap += temp->downstream_tap_rise_cap;

				}
				else
				{
					cout << "Warning net " << mapped_name << " (" << original_name << ") contains a resistor loop.\n";
					plotErroneousNet( "erroneous_net" + original_name );
					return -1;
				}
			}
		}
	}

	return 0;

}

/*
int Parasitic::recursiveCacitanceDelayProductCalculation(p_node * cur)
{

	cur->visited = true;

	cur->downstream_product_fall += cur->capacitance * cur->delay_fall;
	cur->downstream_product_rise += cur->capacitance * cur->delay_rise;

	if (cur->type == TAP)
	{

		if (cur->graph_node->getPin() != NULL)
		{
			cur->downstream_product_fall += (( InputPin * ) cur->graph_node->getPin())->getCfall() * cur->delay_fall;
			cur->downstream_product_rise += (( InputPin * ) cur->graph_node->getPin())->getCrise() * cur->delay_rise;
		}
		else
		{
			cur->downstream_product_fall += cur->graph_node->getUserCapacitanceFall() * cur->delay_fall;
			cur->downstream_product_rise += cur->graph_node->getUserCapacitanceRise() * cur->delay_rise;
		}
	}

	for (unsigned int i = 0; i < cur->adjacent.size(); i++)
	{
		if (cur->adjacent[i]->visited == false)
		{
			cur->adjacent[i]->visited = true;

			if (cur->adjacent[i]->connected_to_a == cur)
			{
				p_node * temp = cur->adjacent[i]->connected_to_b;

				if (temp->visited == false)
				{
					if (recursiveCacitanceDelayProductCalculation( temp ) == -1)
					{
						return -1;
					}

					cur->downstream_product_fall += temp->downstream_product_fall;
					cur->downstream_product_rise += temp->downstream_product_rise;

				}
				else  // Should never execute
				{
					cout << "Warning net " << mapped_name << " (" << original_name << ") contains a resistor loop.\n";
					plotErroneousNet( "erroneous_net" + original_name );
					return -1;
				}
			}
			else
			{
				p_node * temp = cur->adjacent[i]->connected_to_a;

				if (temp->visited == false)
				{
					if (recursiveCacitanceDelayProductCalculation( temp ) == -1)
					{
						return -1;
					}

					cur->downstream_product_fall += temp->downstream_product_fall;
					cur->downstream_product_rise += temp->downstream_product_rise;

				}
				else  // Should never execute
				{
					cout << "Warning net " << mapped_name << " (" << original_name << ") contains a resistor loop.\n";
					plotErroneousNet( "erroneous_net" + original_name );
					return -1;
				}
			}

		}

	}

	return 0;

}*/

typedef struct dfs_elmore
{
	unsigned int route;
	double resistance;
	double b_factor_fall;
	double b_factor_rise;
} dfs;

void Parasitic::elmoreDelayCalculation()
{
	if (start_point == NULL)
	{
		if (show_errors && parasitic_nodes.empty())
		{
			cout << "Warning net " << original_name << " has no defined parasitic net. Assuming zero wire delay/transition.\n";
			show_errors = false;
		}
		return;
	}

	reIninitialize();

	if (recursiveCapacitanceCalculation( start_point ) == -1)
	{
		maximumSpanningTree();
		reIninitialize();

		if (recursiveCapacitanceCalculation( start_point ) == -1)
		{
			cout << "Fatal Error the net " << mapped_name << " still contains a resistor loop.\n";
			cout << "Terminating.\n";
			exit( -1 );
		}
	}

	total_capacitance = start_point->downstream_cap;

	connectionCheck();

	p_node_map::iterator p;
	for (p = parasitic_nodes.begin(); p != parasitic_nodes.end(); p++)
	{
		for (unsigned int i = 0; i < p->second->adjacent.size(); i++)
		{
			p->second->adjacent[i]->visited = false;
		}

		p->second->visited = false;
	}

	vector<dfs_elmore> S;

	vector<p_node *> path;

	dfs_elmore initial_path;
	initial_path.route = 0;
	initial_path.resistance = 0;

	S.push_back( initial_path );
	path.push_back( start_point );

	/* Run Depth First Search and discover all paths */
	while (!S.empty())
	{
		dfs_elmore temp2 = S.back();

		p_node * current = path.back();

		if (!current->visited) // (Delay, Capacitance) Product for b factor
		{
			current->downstream_product_fall += current->capacitance * current->delay_fall;
			current->downstream_product_rise += current->capacitance * current->delay_rise;
		}

		if (current->type == TAP && !current->visited)  // End point
		{
			Edge * temp_edge = NULL;
			for (unsigned int i = 0; i < start_point->graph_node->getForwardEdges().size(); i++)
			{
				if (start_point->graph_node->getForwardEdges()[i]->getConnectedTo() == current->graph_node)
				{
					temp_edge = start_point->graph_node->getForwardEdges()[i];
					break;
				}
			}

			double capacitance = total_capacitance / start_point->graph_node->getForwardEdges().size();

			(( NetEdge * ) temp_edge)->setResistance( temp2.resistance );
			(( NetEdge * ) temp_edge)->setCapacitance( capacitance );
			(( NetEdge * ) temp_edge)->setDelayFall( current->delay_fall );
			(( NetEdge * ) temp_edge)->setDelayRise( current->delay_rise );

			(( NetEdge * ) temp_edge->getOppositeEdge())->setResistance( temp2.resistance );
			(( NetEdge * ) temp_edge->getOppositeEdge())->setCapacitance( capacitance );
			(( NetEdge * ) temp_edge->getOppositeEdge())->setDelayFall( current->delay_fall );
			(( NetEdge * ) temp_edge->getOppositeEdge())->setDelayRise( current->delay_rise );

			if (current->graph_node->getPin() != NULL) // (Delay, Capacitance) Product for b factor
			{
				current->downstream_product_fall += (( InputPin * ) current->graph_node->getPin())->getCfall() * current->delay_fall;
				current->downstream_product_rise += (( InputPin * ) current->graph_node->getPin())->getCrise() * current->delay_rise;
			}
			else
			{
				current->downstream_product_fall += current->graph_node->getUserCapacitanceFall() * current->delay_fall;
				current->downstream_product_rise += current->graph_node->getUserCapacitanceRise() * current->delay_rise;
			}

		}

		current->visited = true;

		if (temp2.route < current->adjacent.size())  // There are more routes from this node, not yet explored
		{
			p_edge *ptr = current->adjacent[temp2.route];

			if (ptr->visited == false)  // this edge is not visited
			{
				ptr->visited = true;

				dfs temp3;

				p_node * next = ptr->connected_to_a == current ? ptr->connected_to_b : ptr->connected_to_a;

				temp3.route = 0;  // always start exploring far left child
				temp3.resistance = temp2.resistance + ptr->resistance;

				next->delay_fall = ptr->resistance * (next->downstream_cap + next->downstream_tap_fall_cap) + current->delay_fall;
				next->delay_rise = ptr->resistance * (next->downstream_cap + next->downstream_tap_rise_cap) + current->delay_rise;

				S.push_back( temp3 );
				path.push_back( next );
			}
			else
			{
				S.back().route++;
			}
		}
		else  //This node is completely explored
		{
			path.pop_back();
			S.pop_back();

			if (!S.empty())
			{
				S.back().route++;

				/* Update my father with my downstream delay * capacitance sum */
				p_node * father = path.back();
				father->downstream_product_fall += current->downstream_product_fall;
				father->downstream_product_rise += current->downstream_product_rise;
			}
		}

	}

}

void Parasitic::bFactorCalculation()
{

	if (start_point == NULL)
	{
		return;
	}

	p_node_map::iterator p;

	/*for (p = parasitic_nodes.begin(); p != parasitic_nodes.end(); p++)
	 {
	 for (unsigned int i = 0; i < p->second->adjacent.size(); i++)
	 {
	 p->second->adjacent[i]->visited = false;
	 }

	 p->second->visited = false;
	 }

	 recursiveCacitanceDelayProductCalculation( start_point );*/

	for (p = parasitic_nodes.begin(); p != parasitic_nodes.end(); p++)
	{
		for (unsigned int i = 0; i < p->second->adjacent.size(); i++)
		{
			p->second->adjacent[i]->visited = false;
		}

		p->second->visited = false;
	}

	vector<dfs_elmore> S;

	vector<p_node *> path;

	dfs_elmore initial_path;
	initial_path.route = 0;
	initial_path.b_factor_fall = 0;
	initial_path.b_factor_rise = 0;

	S.push_back( initial_path );
	path.push_back( start_point );

	/* Run Depth First Search and discover all paths */
	while (!S.empty())
	{
		dfs_elmore temp2 = S.back();

		p_node * current = path.back();

		if (current->type == TAP && !current->visited)  // End point
		{
			Edge * temp_edge = NULL;
			for (unsigned int i = 0; i < start_point->graph_node->getForwardEdges().size(); i++)
			{
				if (start_point->graph_node->getForwardEdges()[i]->getConnectedTo() == current->graph_node)
				{
					temp_edge = start_point->graph_node->getForwardEdges()[i];
					break;
				}
			}

			(( NetEdge * ) temp_edge)->setBFactorFall( temp2.b_factor_fall );
			(( NetEdge * ) temp_edge)->setBFactorRise( temp2.b_factor_rise );

			(( NetEdge * ) temp_edge->getOppositeEdge())->setBFactorFall( temp2.b_factor_fall );
			(( NetEdge * ) temp_edge->getOppositeEdge())->setBFactorRise( temp2.b_factor_rise );

		}

		current->visited = true;

		if (temp2.route < current->adjacent.size())  // There are more routes from this node, not yet explored
		{
			p_edge *ptr = current->adjacent[temp2.route];

			if (ptr->visited == false)  // this edge is not visited
			{
				ptr->visited = true;

				dfs temp3;

				p_node * next = ptr->connected_to_a == current ? ptr->connected_to_b : ptr->connected_to_a;

				temp3.route = 0;  // always start exploring far left child

				temp3.b_factor_fall = ptr->resistance * next->downstream_product_fall + temp2.b_factor_fall;
				temp3.b_factor_rise = ptr->resistance * next->downstream_product_rise + temp2.b_factor_rise;

				S.push_back( temp3 );
				path.push_back( next );
			}
			else
			{
				S.back().route++;
			}
		}
		else  //This node is completely explored
		{
			path.pop_back();
			S.pop_back();

			if (!S.empty())
			{
				S.back().route++;
			}
		}

	}

	show_errors = false;

}

void Parasitic::reIninitialize()
{
	total_capacitance = 0;

	p_node_map::iterator p;

	for (p = parasitic_nodes.begin(); p != parasitic_nodes.end(); p++)
	{
		for (unsigned int i = 0; i < p->second->adjacent.size(); i++)
		{
			p->second->adjacent[i]->visited = false;
		}

		p->second->visited = false;

		p->second->downstream_cap = 0;
		p->second->downstream_tap_fall_cap = 0;
		p->second->downstream_tap_rise_cap = 0;

		p->second->downstream_product_fall = 0;
		p->second->downstream_product_rise = 0;

		p->second->delay_fall = 0;
		p->second->delay_rise = 0;
	}
}

void Parasitic::plotErroneousNet(string name)
{
#if !DEBUG
	return;
#endif
	ofstream graph_file;

	graph_file.open( name.c_str() );

	graph_file << "digraph G {\nrankdir=LR;\nranksep=equally;\nnodesep=1.0;\n";

	graph_file << "subgraph \"cluster_erroneousnet\" {style=bold; color=gray; label=\"Net " << mapped_name << " (" << original_name << ")" << "\"; ";

	p_node_map::iterator p;

	for (p = parasitic_nodes.begin(); p != parasitic_nodes.end(); p++)
	{
		if (p->second == start_point)
		{
			graph_file << "node[shape=circle, color=green, style=bold];\n";
			//graph_file << "subgraph \"cluster_port" << "\" {style=bold; color=black; label=\"Port\"; ";
			graph_file << "\"" << p->first << "\";\n";
			//graph_file << "}\n";
		}
		else if (p->second->graph_node != NULL)
		{
			graph_file << "node[shape=circle, color=red, style=bold];\n";
			//graph_file << "subgraph \"cluster_tap" << "\" {style=bold; color=black; label=\"Taps\"; ";
			graph_file << "\"" << p->first << "\";\n";
			//graph_file << "}\n";
		}
		else
		{
			graph_file << "node[shape=circle, color=black, style=bold];\n";
			graph_file << "\"" << p->first << "\";\n";
		}

	}

	for (p = parasitic_nodes.begin(); p != parasitic_nodes.end(); p++)
	{
		for (unsigned int i = 0; i < p->second->adjacent.size(); i++)
		{
			if (p->second->adjacent[i]->connected_to_a == p->second)
			{
				graph_file << "\"" << p->first << "\"" << " -> " << "\"" << p->second->adjacent[i]->connected_to_b->name
						<< "\" [dir=none, style=bold, color=blue, label=\"" << "\"];\n";
			}
		}

	}

	graph_file << "}\n";

	graph_file << "}\n";

	graph_file.close();

	FILE *pipe = popen( ("xdot " + name + " &").c_str(), "w" );
	fclose( pipe );
}

void Parasitic::addEdge(p_edge * edge)
{
	edges.insert( edge );
}

typedef struct _forest
{
	struct _forest * representative;
	vector<struct _forest *> represented_nodes;

	_forest() :
			representative( NULL )
	{
	}
} forest;

typedef map<p_node *, forest *> forest_map;
typedef pair<p_node *, forest *> forest_entry;

void Parasitic::maximumSpanningTree()
{

	forest_map forests;

	p_node_map::iterator p;

	for (p = parasitic_nodes.begin(); p != parasitic_nodes.end(); p++)
	{
		forest * temp = new forest;
		forests.insert( forest_entry( p->second, temp ) );
	}

	multiset<p_edge *, sort_edge>::iterator it;

	for (it = edges.begin(); it != edges.end(); it++)
	{
		forest_map::iterator pa, pb;

		pa = forests.find( (*it)->connected_to_a );
		pb = forests.find( (*it)->connected_to_b );

		if (pa->second->representative == NULL && pb->second->representative == NULL)
		{
			pa->second->representative = pa->second;
			pa->second->represented_nodes.push_back( pb->second );
			pb->second->representative = pa->second;
		}
		else if (pa->second->representative == NULL && pb->second->representative != NULL)
		{
			pa->second->representative = pb->second->representative;
			pb->second->representative->represented_nodes.push_back( pa->second );
		}
		else if (pa->second->representative != NULL && pb->second->representative == NULL)
		{
			pb->second->representative = pa->second->representative;
			pa->second->representative->represented_nodes.push_back( pb->second );
		}
		else
		{
			if (pb->second->representative == pa->second->representative)  // Trying to union two forests with the same representative, loop detected
			{
				for (unsigned int p = 0; p < (*it)->connected_to_a->adjacent.size(); p++)  // delete the edge
				{
					if ((*it)->connected_to_a->adjacent[p] == *it)
					{
						(*it)->connected_to_a->adjacent.erase( (*it)->connected_to_a->adjacent.begin() + p );
					}
				}

				for (unsigned int p = 0; p < (*it)->connected_to_b->adjacent.size(); p++)  // delete the edge
				{
					if ((*it)->connected_to_b->adjacent[p] == *it)
					{
						(*it)->connected_to_b->adjacent.erase( (*it)->connected_to_b->adjacent.begin() + p );
					}
				}
			}
			else
			{
				forest * a_representative = pa->second->representative;
				forest * b_representative = pb->second->representative;

				for (unsigned int k = 0; k < b_representative->represented_nodes.size(); k++)
				{
					a_representative->represented_nodes.push_back( b_representative->represented_nodes[k] );
					b_representative->represented_nodes[k]->representative = a_representative;
				}
				b_representative->represented_nodes.clear();

				a_representative->represented_nodes.push_back( b_representative );

				b_representative->representative = a_representative;
			}
		}

	}

	forest_map::iterator it2;

	for (it2 = forests.begin(); it2 != forests.end(); it2++)
	{
		delete it2->second;
	}

	plotErroneousNet( "fixed_net" + original_name );

}

