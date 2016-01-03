/*
 * Graph.cpp
 *
 *  Created on: May 8, 2014
 *      Author: hrkalona
 */

#include "Graph.hpp"

#include <fstream>

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <queue>
#include <set>
#include "Timing.hpp"
#include "OutputPin.hpp"
#include "InputPin.hpp"
#include "System.hpp"
#include "Util.hpp"
#include "NetEdge.hpp"
#include "CellEdge.hpp"
#include "types.hpp"

using namespace boost;

Graph::Graph()
{

	report_timing = false;
	num_paths = 0;
	pt = OUT;
	preset_clear = false;
	upsizing = false;

	total_levels = 0;

	cells_no_design = 0;
	cells_resized = 0;
	cells_examined = 0;
	cells_potentially_resized = 0;

	cells_hierarchy = NULL;

	performed_stas = 0;

#if TESTING
	first_path_found = false;
#endif

#if RESIZE_FIRST_CELL_ON_PATH
	path_counter = 0;
#endif

}

void Graph::setName(string name)
{
	this->name = name;
}

string Graph::getName()
{
	return name;
}

void Graph::setReportTiming(bool option)
{
	report_timing = option;
}

void Graph::setNumPaths(int number)
{
	num_paths = number;
}

void Graph::setPathType(PATH_TYPE path_type)
{
	pt = path_type;
}

bool Graph::getReportTiming()
{
	return report_timing;
}

void Graph::setPresetClear(bool opt)
{
	preset_clear = opt;
}

void Graph::setUpsizingAction(bool opt)
{
	upsizing = opt;
}

Graph::~Graph()
{
	node_map::iterator p;

	for (p = int_nodes.begin(); p != int_nodes.end(); ++p)
		delete p->second;

	for (p = inputs.begin(); p != inputs.end(); ++p)
		delete p->second;

	for (p = outputs.begin(); p != outputs.end(); ++p)
		delete p->second;

	for (unsigned int i = 0; i < total_levels; i++)
		delete[] cells_hierarchy[i];

	delete[] cells_hierarchy;

}

/* Adds a PI in the list of PIs */
Node * Graph::addInputNode(string name, Pin* pin)
{
	string full_name = "<input>:" + name;

	node_map::iterator it = inputs.lower_bound( full_name );

	if (it != inputs.end() && (*it).first.compare( full_name ) == 0)
	{
		return (*it).second;
	}
	else
	{
		Node* temp = new Node( name, pin );

		/* insert the previously allocate node */
		inputs.insert( it, node_entry( full_name, temp ) );

		return temp;
	}
}

/* Adds an internal node in the list of internal nodes */
Node * Graph::addInternalNode(string name, Pin* pin)
{
	node_map::iterator it = int_nodes.lower_bound( name );

	if (it != int_nodes.end() && (*it).first.compare( name ) == 0)
		return (*it).second;

	else
	{
		Node* temp = new Node( name, pin );

		int_nodes.insert( it, node_entry( name, temp ) );

		return temp;
	}
}

/* Adds an output node in the list of output nodes */
Node * Graph::addOutputNode(string name, Pin* pin)
{
	string full_name = "<output>:" + name;

	node_map::iterator it = outputs.lower_bound( full_name );

	if (it != outputs.end() && (*it).first.compare( full_name ) == 0)
	{
		return (*it).second;
	}
	else
	{
		Node* temp = new Node( name, pin );

		outputs.insert( it, node_entry( full_name, temp ) );

		return temp;
	}
}

/* Add a virtual node in the list of virtual nodes */
Node * Graph::addVirtualNode(string name, Pin* pin)
{
	string full_name = "<virtual>:" + name;

	node_map::iterator it = virtual_nodes.lower_bound( full_name );

	if (it != virtual_nodes.end() && (*it).first.compare( full_name ) == 0)
		return (*it).second;

	else
	{
		Node* temp = new Node( name, pin );

		virtual_nodes.insert( it, node_entry( full_name, temp ) );

		return temp;
	}
}

bool Graph::isInput(string name)
{
	/* if found an entry */
	if (inputs.find( name ) != inputs.end())
		return true;

	node_map::iterator it = inputs.lower_bound( name + "[" );

	/* if found an entry */
	if (it != inputs.end())
		return it->first.substr( 0, it->first.find_first_of( "[" ) + 1 ).compare( name + "[" ) == 0;

	return false;
}

bool Graph::isOutput(string name)
{
	/* if found an entry */
	if (outputs.find( name ) != outputs.end())
		return true;

	node_map::iterator it = outputs.lower_bound( name + "[" );

	/* if found an entry */
	if (it != outputs.end())
		return it->first.substr( 0, it->first.find_first_of( "[" ) + 1 ).compare( name + "[" ) == 0;

	return false;
}

node_map& Graph::getInputNodes()
{
	return inputs;
}

node_map& Graph::getOutputNodes()
{
	return outputs;
}

node_map& Graph::getInternalNodes()
{
	return int_nodes;
}

node_map& Graph::getVirtualNodes()
{
	return virtual_nodes;
}

/* The method below dumps a file that's compatible with xdot.
 * Thus it can be ploted for debugging reasons */
void Graph::plotGraph()
{
	ofstream graph_file;

	node_map::iterator p;

	graph_file.open( "circuit" );

	graph_file.precision( 8 );
	graph_file << fixed;
	graph_file << "digraph G {\nrankdir=LR;\nranksep=equally;\n";

	graph_file << "subgraph \"cluster_circuit\" {style=bold; color=gray; label=\"Circuit " << name << "\"; ";

	cout << "\nPrimary Outputs:\n";

	for (p = outputs.begin(); p != outputs.end(); ++p)
	{

		graph_file << "node[shape=rect, color=red, style=bold];\n";

		graph_file << "\"" << p->second->getName() << "\\n\\nat_late_rise: " << p->second->getAtLateRise() << "\\nat_late_fall: "
				<< p->second->getAtLateFall() << "\";\n";

		typedef tokenizer<char_separator<char> > tokenizer;
		char_separator<char> sep( ":" );
		tokenizer tokens( p->first, sep );

		tokenizer::iterator tok_iter = tokens.begin();

		graph_file << "node[shape=rect, color=blue, style=bold];\n";

		graph_file << "subgraph \"cluster_" << *tok_iter << "\" {style=bold; color=black; label=\"" << *tok_iter << "\"; ";

		graph_file << "\"" << p->second->getName() << "\\n\\nat_late_rise: " << p->second->getAtLateRise() << "\\nat_late_fall: "
				<< p->second->getAtLateFall() << "\"; ";

		graph_file << "}\n";

		cout << p->second->getName() << "\n";

	}

	cout << "\nPrimary Inputs:\n";

	for (p = inputs.begin(); p != inputs.end(); ++p)
	{

		graph_file << "node[shape=rect, color=green, style=bold];\n";

		graph_file << "\"" << p->second->getName() << "\\n\\nat_late_rise: " << p->second->getAtLateRise() << "\\nat_late_fall: "
				<< p->second->getAtLateFall() << "\";\n";

		graph_file << "node[shape=rect, color=blue, style=bold];\n";

		for (unsigned int i = 0; i < p->second->getForwardEdges().size(); i++)
		{
			if (p->second->getForwardEdges()[i]->getNetType() == CELL_EDGE)
			{
				string temp1;
				string temp2;

				if ((( CellEdge * ) p->second->getForwardEdges()[i])->getFallTimingSense() == POSITIVE)
				{
					temp2 = "Positive";
				}
				else if ((( CellEdge * ) p->second->getForwardEdges()[i])->getFallTimingSense() == NEGATIVE)
				{
					temp2 = "Negative";
				}
				else
				{
					temp2 = "Non";
				}

				if ((( CellEdge * ) p->second->getForwardEdges()[i])->getRiseTimingSense() == POSITIVE)
				{
					temp1 = "Positive";
				}
				else if ((( CellEdge * ) p->second->getForwardEdges()[i])->getRiseTimingSense() == NEGATIVE)
				{
					temp1 = "Negative";
				}
				else
				{
					temp1 = "Non";
				}

				graph_file << "\"" << p->second->getName() << "\\n\\nat_late_rise: " << p->second->getAtLateRise() << "\\nat_late_fall: "
						<< p->second->getAtLateFall() << "\"" << " -> " << "\"" << p->second->getForwardEdges()[i]->getConnectedTo()->getName()
						<< "\\n\\nat_late_rise: " << p->second->getForwardEdges()[i]->getConnectedTo()->getAtLateRise() << "\\nat_late_fall: "
						<< p->second->getForwardEdges()[i]->getConnectedTo()->getAtLateFall() << "\" [style=bold, color=orange, label=\""
						<< p->second->getForwardEdges()[i]->getName() << "delay rise: " << p->second->getForwardEdges()[i]->getDelayRise() << ", "
						<< temp1 << "\\ndelay fall: " << p->second->getForwardEdges()[i]->getDelayFall() << ", " << temp2 << "\"];\n";
			}
			else
			{
				graph_file << "\"" << p->second->getName() << "\\n\\nat_late_rise: " << p->second->getAtLateRise() << "\\nat_late_fall: "
						<< p->second->getAtLateFall() << "\"" << " -> " << "\"" << p->second->getForwardEdges()[i]->getConnectedTo()->getName()
						<< "\\n\\nat_late_rise: " << p->second->getForwardEdges()[i]->getConnectedTo()->getAtLateRise() << "\\nat_late_fall: "
						<< p->second->getForwardEdges()[i]->getConnectedTo()->getAtLateFall() << "\" [style=bold, color=blue, label=\""
						<< p->second->getForwardEdges()[i]->getName() << "\\n" << "delay rise: " << p->second->getForwardEdges()[i]->getDelayRise()
						<< "\\ndelay fall: " << p->second->getForwardEdges()[i]->getDelayFall() << "\"];\n";
			}
		}

		typedef tokenizer<char_separator<char> > tokenizer;
		char_separator<char> sep( ":" );
		tokenizer tokens( p->first, sep );

		tokenizer::iterator tok_iter = tokens.begin();

		graph_file << "node[shape=rect, color=blue, style=bold];\n";

		graph_file << "subgraph \"cluster_" << *tok_iter << "\" {style=bold; color=black; label=\"" << *tok_iter << "\"; ";

		graph_file << "\"" << p->second->getName() << "\\n\\nat_late_rise: " << p->second->getAtLateRise() << "\\nat_late_fall: "
				<< p->second->getAtLateFall() << "\"; ";

		graph_file << "}\n";

		cout << p->second->getName() << "\n";

	}

	cout << "\nVirtual nodes:\n";

	for (p = virtual_nodes.begin(); p != virtual_nodes.end(); ++p)
	{

		graph_file << "node[shape=rect, color=magenta, style=bold];\n";

		graph_file << "\"" << p->second->getName() << "\\n\\nat_late_rise: " << p->second->getAtLateRise() << "\\nat_late_fall: "
				<< p->second->getAtLateFall() << "\";\n";

		graph_file << "node[shape=rect, color=blue, style=bold];\n";

		for (unsigned int i = 0; i < p->second->getForwardEdges().size(); i++)
		{
			if (p->second->getForwardEdges()[i]->getNetType() == CELL_EDGE)
			{
				string temp1;
				string temp2;

				if ((( CellEdge * ) p->second->getForwardEdges()[i])->getFallTimingSense() == POSITIVE)
				{
					temp2 = "Positive";
				}
				else if ((( CellEdge * ) p->second->getForwardEdges()[i])->getFallTimingSense() == NEGATIVE)
				{
					temp2 = "Negative";
				}
				else
				{
					temp2 = "Non";
				}

				if ((( CellEdge * ) p->second->getForwardEdges()[i])->getRiseTimingSense() == POSITIVE)
				{
					temp1 = "Positive";
				}
				else if ((( CellEdge * ) p->second->getForwardEdges()[i])->getRiseTimingSense() == NEGATIVE)
				{
					temp1 = "Negative";
				}
				else
				{
					temp1 = "Non";
				}

				if (p->second->getForwardEdges()[i]->isBlocked())
				{
					graph_file << "\"" << p->second->getName() << "\\n\\nat_late_rise: " << p->second->getAtLateRise() << "\\nat_late_fall: "
							<< p->second->getAtLateFall() << "\"" << " -> " << "\"" << p->second->getForwardEdges()[i]->getConnectedTo()->getName()
							<< "\\n\\nat_late_rise: " << p->second->getForwardEdges()[i]->getConnectedTo()->getAtLateRise() << "\\nat_late_fall: "
							<< p->second->getForwardEdges()[i]->getConnectedTo()->getAtLateFall() << "\" [style=bold, color=brown, label=\""
							<< p->second->getForwardEdges()[i]->getName() << "delay rise: " << p->second->getForwardEdges()[i]->getDelayRise() << ", "
							<< temp1 << "\\ndelay fall: " << p->second->getForwardEdges()[i]->getDelayFall() << ", " << temp2 << "\"];\n";
				}
				else
				{
					graph_file << "\"" << p->second->getName() << "\\n\\nat_late_rise: " << p->second->getAtLateRise() << "\\nat_late_fall: "
							<< p->second->getAtLateFall() << "\"" << " -> " << "\"" << p->second->getForwardEdges()[i]->getConnectedTo()->getName()
							<< "\\n\\nat_late_rise: " << p->second->getForwardEdges()[i]->getConnectedTo()->getAtLateRise() << "\\nat_late_fall: "
							<< p->second->getForwardEdges()[i]->getConnectedTo()->getAtLateFall() << "\" [style=bold, color=orange, label=\""
							<< p->second->getForwardEdges()[i]->getName() << "delay rise: " << p->second->getForwardEdges()[i]->getDelayRise() << ", "
							<< temp1 << "\\ndelay fall: " << p->second->getForwardEdges()[i]->getDelayFall() << ", " << temp2 << "\"];\n";
				}
			}
			else
			{
				graph_file << "\"" << p->second->getName() << "\\n\\nat_late_rise: " << p->second->getAtLateRise() << "\\nat_late_fall: "
						<< p->second->getAtLateFall() << "\"" << " -> " << "\"" << p->second->getForwardEdges()[i]->getConnectedTo()->getName()
						<< "\\n\\nat_late_rise: " << p->second->getForwardEdges()[i]->getConnectedTo()->getAtLateRise() << "\\nat_late_fall: "
						<< p->second->getForwardEdges()[i]->getConnectedTo()->getAtLateFall() << "\" [style=bold, color=blue, label=\""
						<< p->second->getForwardEdges()[i]->getName() << "\\n" << "delay rise: " << p->second->getForwardEdges()[i]->getDelayRise()
						<< "\\ndelay fall: " << p->second->getForwardEdges()[i]->getDelayFall() << "\"];\n";
			}
		}

		typedef tokenizer<char_separator<char> > tokenizer;
		char_separator<char> sep( ":" );
		tokenizer tokens( p->first, sep );

		tokenizer::iterator tok_iter = tokens.begin();

		graph_file << "node[shape=rect, color=blue, style=bold];\n";

		graph_file << "subgraph \"cluster_" << *tok_iter << "\" {style=bold; color=black; label=\"" << *tok_iter << "\"; ";

		graph_file << "\"" << p->second->getName() << "\\n\\nat_late_rise: " << p->second->getAtLateRise() << "\\nat_late_fall: "
				<< p->second->getAtLateFall() << "\"; ";

		graph_file << "}\n";

		cout << p->second->getName() << "\n";

	}

	cout << "\nInternal Nodes:\n";

	for (p = int_nodes.begin(); p != int_nodes.end(); ++p)
	{

		graph_file << "node[shape=rect, color=blue, style=bold];\n";

		graph_file << "\"" << p->second->getName() << "\\n\\nat_late_rise: " << p->second->getAtLateRise() << "\\nat_late_fall: "
				<< p->second->getAtLateFall() << "\";\n";

		for (unsigned int i = 0; i < p->second->getForwardEdges().size(); i++)
		{

			if (p->second->getForwardEdges()[i]->getNetType() == CELL_EDGE)
			{
				string temp1;
				string temp2;

				if ((( CellEdge * ) p->second->getForwardEdges()[i])->getFallTimingSense() == POSITIVE)
				{
					temp2 = "Positive";
				}
				else if ((( CellEdge * ) p->second->getForwardEdges()[i])->getFallTimingSense() == NEGATIVE)
				{
					temp2 = "Negative";
				}
				else
				{
					temp2 = "Non";
				}

				if ((( CellEdge * ) p->second->getForwardEdges()[i])->getRiseTimingSense() == POSITIVE)
				{
					temp1 = "Positive";
				}
				else if ((( CellEdge * ) p->second->getForwardEdges()[i])->getRiseTimingSense() == NEGATIVE)
				{
					temp1 = "Negative";
				}
				else
				{
					temp1 = "Non";
				}

				if (p->second->getForwardEdges()[i]->isBlocked())
				{
					graph_file << "\"" << p->second->getName() << "\\n\\nat_late_rise: " << p->second->getAtLateRise() << "\\nat_late_fall: "
							<< p->second->getAtLateFall() << "\"" << " -> " << "\"" << p->second->getForwardEdges()[i]->getConnectedTo()->getName()
							<< "\\n\\nat_late_rise: " << p->second->getForwardEdges()[i]->getConnectedTo()->getAtLateRise() << "\\nat_late_fall: "
							<< p->second->getForwardEdges()[i]->getConnectedTo()->getAtLateFall() << "\" [style=bold, color=brown, label=\""
							<< p->second->getForwardEdges()[i]->getName() << "delay rise: " << p->second->getForwardEdges()[i]->getDelayRise() << ", "
							<< temp1 << "\\ndelay fall: " << p->second->getForwardEdges()[i]->getDelayFall() << ", " << temp2 << "\"];\n";
				}
				else
				{
					graph_file << "\"" << p->second->getName() << "\\n\\nat_late_rise: " << p->second->getAtLateRise() << "\\nat_late_fall: "
							<< p->second->getAtLateFall() << "\"" << " -> " << "\"" << p->second->getForwardEdges()[i]->getConnectedTo()->getName()
							<< "\\n\\nat_late_rise: " << p->second->getForwardEdges()[i]->getConnectedTo()->getAtLateRise() << "\\nat_late_fall: "
							<< p->second->getForwardEdges()[i]->getConnectedTo()->getAtLateFall() << "\" [style=bold, color=orange, label=\""
							<< p->second->getForwardEdges()[i]->getName() << "delay rise: " << p->second->getForwardEdges()[i]->getDelayRise() << ", "
							<< temp1 << "\\ndelay fall: " << p->second->getForwardEdges()[i]->getDelayFall() << ", " << temp2 << "\"];\n";
				}
			}
			else
			{
				graph_file << "\"" << p->second->getName() << "\\n\\nat_late_rise: " << p->second->getAtLateRise() << "\\nat_late_fall: "
						<< p->second->getAtLateFall() << "\"" << " -> " << "\"" << p->second->getForwardEdges()[i]->getConnectedTo()->getName()
						<< "\\n\\nat_late_rise: " << p->second->getForwardEdges()[i]->getConnectedTo()->getAtLateRise() << "\\nat_late_fall: "
						<< p->second->getForwardEdges()[i]->getConnectedTo()->getAtLateFall() << "\" [style=bold, color=blue, label=\""
						<< p->second->getForwardEdges()[i]->getName() << "\\n" << "delay rise: " << p->second->getForwardEdges()[i]->getDelayRise()
						<< "\\ndelay fall: " << p->second->getForwardEdges()[i]->getDelayFall() << "\"];\n";
			}

		}

		string instance_name = p->second->getCellInst()->getInstanceName();

		typedef tokenizer<char_separator<char> > tokenizer;
		char_separator<char> sep2( "." );
		tokenizer tokens2( instance_name, sep2 );

		int dot_num = count( instance_name.begin(), instance_name.end(), '.' );

		tokenizer::iterator tok_iter2 = tokens2.begin();

		for (unsigned int k = 0; k < dot_num; k++)
		{
			graph_file << "node[shape=rect, color=blue, style=bold];\n";

			graph_file << "subgraph \"cluster_" << *tok_iter2 << "\" {style=bold; color=black; label=\"" << *tok_iter2 << "\"; ";
			tok_iter2++;
		}

		char_separator<char> sep( ":" );
		tokenizer tokens( p->first, sep );

		tokenizer::iterator tok_iter = tokens.begin();

		graph_file << "node[shape=rect, color=blue, style=bold];\n";

		graph_file << "subgraph \"cluster_" << *tok_iter << "\" {style=bold; color=black; label=\"" << *tok_iter << "\\nLevel "
				<< p->second->getCellInst()->getLevel() << "\"; ";

		graph_file << "\"" << p->second->getName() << "\\n\\nat_late_rise: " << p->second->getAtLateRise() << "\\nat_late_fall: "
				<< p->second->getAtLateFall() << "\"; ";

		graph_file << "}\n";

		for (unsigned int k = 0; k < dot_num; k++)
		{
			graph_file << "}\n";
		}

		cout << p->second->getName() << "\n";

	}

	cout << "\n\n\n";

	graph_file << "}\n";

	graph_file << "}";

	graph_file.close();

	FILE *pipe = popen( "xdot circuit &", "w" );
	fclose( pipe );
}

void Graph::resizer(cell_map & cells, cellinst_map& instantiated_cells, vector<Parasitic *>& parasitics, double tau, int cells_no_design,
		int cells_examined, int cells_resized, int cells_potentially_resized, int max_iterations, double tolerance)
{

	cout.precision( 8 );
	cout << fixed;

	if (report_timing)  // A configuration that sets the tool to operate as a SIMPLE TIMER
	{
		/* First step: Assign levels to each instance.
		 * The level of a Cellinst is the maximum level of all Cellinsts that drive the first (Cellinst) + 1*/

		cout << "\n\n";

		levelization( instantiated_cells );

		cout << "Performing timing analysis...\n\n";

		/* Second step: a. Assign delays to timing arcs
		 * 				b. Find AT(arrival time) @all nodes */
		sta( parasitics );  //

		/* Keep critical paths */
		multiset<Path> critical_paths;

		node_map::iterator p;

		/* Loop over all PIs and trace paths beginning from PIs */
		for (p = inputs.begin(); p != inputs.end(); ++p)
		{
			findCriticalPaths( p->second, critical_paths );
		}

		/* enqueue all Virtuals */

		/* Loop over all virtual nodes and trace paths beginning from those nodes */
		for (p = virtual_nodes.begin(); p != virtual_nodes.end(); ++p)
		{
			findCriticalPaths( p->second, critical_paths );
		}

		multiset<Path>::iterator it;
		int counter = 1;

		cout << "\n\nTiming Report:\n\n";

		/* Loop over all critical paths */
		for (it = critical_paths.begin(); it != critical_paths.end(); it++)
		{
			cout << "Critical path " << counter << " [" << it->getDelay() << "]\n";

			/* Loop over the nodes that constitute the current critical path */
			for (unsigned int i = 0; i < it->getPathConst().size(); i++)
			{
				if (it->getSlopesConst()[i] == RISE)
				{
					cout << it->getPathConst()[i]->getName() << " [RISE]\n";
				}
				else
				{
					cout << it->getPathConst()[i]->getName() << " [FALL]\n";
				}

			}
			cout << "\n\n";
			counter++;
		}

#if GRAPHING

		plotGraph();

#endif
	}
	else  // the common case
	{
		this->cells_no_design = cells_no_design;
		this->cells_examined = cells_examined;
		this->cells_resized = cells_resized;
		this->cells_potentially_resized = cells_potentially_resized;

		/* First step: Assign levels to each cell instance.
		 * The level of a Cellinst is the maximum level of all Cellinsts that drive the first (Cellinst) + 1*/
		levelization( instantiated_cells );

		cout << "Performing resizing...\n\n";

		int iteration = 0;

		/* Initialize all terminal nodes used for the critical path extraction */
		initializeTerminalNodes();

		/* Second step: a. Assign delays to all timing arcs ( Cellinst inputs-to-outputs and wires )
		 * 				b. Find AT(arrival time) @all nodes */
		sta( parasitics );

		do
		{

#if GRAPHING
			if (iteration == 0)
			{
				plotGraph();
			}
#endif

			/* Critical path extraction */
			findCriticalPath();

#if CRIT_PATH_UPPER_BOUND
			if (iteration == 0)
			{
				cout << "Critical path upper bound: " << critical_path.getDelay() << "\n\n";
			}
#endif

			/* keep all Cellinsts that have been resized */
			vector<Cellinst *> changed_cells;

			/* tokenize the critical path into subpaths */
			findSubPaths( cells, parasitics, changed_cells, tau, max_iterations, tolerance );

			if (changed_cells.empty())
			{
				/* While none of the cells on the path have been resized AND
				 * there are terminal nodes */
				while (changed_cells.empty() && !terminal_nodes.empty())
				{
					if (!critical_path.getPath().empty())
					{
						/* erase the last node of critical path which also
						 * exist in terminal nodes */

						terminal_nodes.erase( critical_path_node );

						/* Add the new terminal nodes */
						addNewTerminalNodes();
					}

					critical_path.reset();

					/* critical path extraction */
					findCriticalPath();

					findSubPaths( cells, parasitics, changed_cells, tau, max_iterations, tolerance );  // tokenize the critical path
				}
			}

			/* if no terminal nodes have been left */
			if (terminal_nodes.empty())
				break;

			/* Second step: a. Assign delays to timing arcs
			 * 				b. Find AT(arrival time) @all nodes
			 * 				propagating the changes only from the resized cell instances */
			staIncremental( parasitics, changed_cells );

			reIninitializeIncremental();

			iteration++;
		} while (true);

		cout << "\n\nCells resized: " << this->cells_resized << " / " << this->cells_no_design << "\n";
		cout << "\n\nCells potentially resized: " << this->cells_potentially_resized << " / " << this->cells_no_design << "\n";
		cout << "\n\nPerformed " << performed_stas << " static timing analysis iterations.\n";

	}
}

void Graph::addNewTerminalNodes()
{
	/* Loop over all nodes of the critical path */
	for (unsigned int i = 0; i < critical_path.getPath().size(); i++)
	{
		/* if the current node is the output of a Cellinst */
		if (critical_path.getPath()[i]->getPin() != NULL && critical_path.getPath()[i]->getPin()->getType() == OUTPUT)
		{
			/* find the previous nodes connected to this output (in other words the inputs of the Cellinst) */
			for (unsigned int j = 0; j < critical_path.getPath()[i]->getBackwardEdges().size(); j++)
			{
				Node * t = critical_path.getPath()[i]->getBackwardEdges()[j]->getConnectedTo();

				/* if t doesn't belong to the critical path node list */
				if (t != critical_path.getPath()[i + 1] && t->getPin() != NULL && t->getPin()->getType() == INPUT)
				{
					/* for all edges in the backwards order.
					 * In that case we could only get the first entry from
					 * BackwardEdges as it is impossible to reach an input pin two
					 * different nets */
					for (unsigned int k = 0; k < t->getBackwardEdges().size(); k++)
					{
						Node * s = t->getBackwardEdges()[k]->getConnectedTo();

						/* if node s is the output pin of a Cellinst */
						if (s->getPin() != NULL && s->getPin()->getType() == OUTPUT && s->usedAsTerminal() == false)
						{

							T_node temp;
							temp.terminal = s;

							temp.level = s->getCellInst()->getLevel() + 1;

							terminal_nodes.insert( temp );

							s->setUsedAsTerminal( true );
						}
					}
				}
			}
		}
	}
}

void Graph::sta(vector<Parasitic *>& parasitics)
{

#if DEBUG
	cout << "\nNets delay calculation:\n";
#endif

#pragma omp parallel for schedule(dynamic)
	for (unsigned int i = 0; i < parasitics.size(); i++)
	{
#if 0
		cout << "Net " << parasitics[i]->getMappedName() << " (" << parasitics[i]->getOriginalName() << ")" << "\n";
#endif
		parasitics[i]->elmoreDelayCalculation();

		parasitics[i]->bFactorCalculation();
	}

	cout << "\n";

	parasitics.clear();

	node_map::iterator p;

	/* Propage the transitions from level i to level i+1 CELLS */

	/* Loop over PIs */
	for (p = inputs.begin(); p != inputs.end(); ++p)
	{
		Node * t = p->second;

		/* Loop over all edges begining from node t */
		for (unsigned int i = 0; i < t->getForwardEdges().size(); i++)
		{
			/* u <- G.adjancentVertex(t,e) */
			Edge * e = t->getForwardEdges()[i];

			/* get the end point of edge e */
			Node *u = e->getConnectedTo();

			if (e->getNetType() == NET_EDGE)
			{
				/* Intitialize the AT @inputs of cell instance beloging to level 1 */

				u->setTrLateFall( (( NetEdge * ) e)->getOutputTransitionFall( t->getTrLateFall() ) );
				u->setTrLateRise( (( NetEdge * ) e)->getOutputTransitionRise( t->getTrLateRise() ) );

				u->setAtLateFall( t->getAtLateFall() + e->getDelayFall() );
				u->setAtLateRise( t->getAtLateRise() + e->getDelayRise() );

				u->setPreviousRise( t );
				u->setInvertedRise( false );
				u->setPreviousFall( t );
				u->setInvertedFall( false );
			}
		}

	}

	for (p = virtual_nodes.begin(); p != virtual_nodes.end(); ++p)
	{

		Node * t = p->second;

		/* Initialize transitions with 0 */
		t->setTrLateFall( 0 );
		t->setTrLateRise( 0 );

		t->setAtLateFall( 0 );
		t->setAtLateRise( 0 );

		for (unsigned int i = 0; i < t->getForwardEdges().size(); i++)
		{
			/* u <- G.adjancentVertex(t,e) */
			Edge * e = t->getForwardEdges()[i];

			Node *u = e->getConnectedTo();

			if (e->getNetType() == NET_EDGE)
			{
				/* Intitialize the AT @inputs of cell instance beloging to level 1 */

				u->setTrLateFall( (( NetEdge * ) e)->getOutputTransitionFall( t->getTrLateFall() ) );
				u->setTrLateRise( (( NetEdge * ) e)->getOutputTransitionRise( t->getTrLateRise() ) );

				u->setAtLateFall( t->getAtLateFall() + e->getDelayFall() );
				u->setAtLateRise( t->getAtLateRise() + e->getDelayRise() );

				u->setPreviousRise( t );
				u->setInvertedRise( false );
				u->setPreviousFall( t );
				u->setInvertedFall( false );
			}
		}

	}

	double min_val = -numeric_limits<double>::max();

	/* Propage the transitions from level n to level n+1 CELLS */

	/* Loop over Cellinst levels */
#pragma omp parallel
	for (unsigned int i = 0; i < total_levels; i++)
	{

		/* All the same level cells */

		/* Loop over Cellinstances of level i */
#pragma omp for
		for (unsigned int j = 0; j < cells_at_level[i]; j++)
		{
			/* Get cell instances @ level i */
			Cellinst * current_cell = cells_hierarchy[i][j];

			/* For every input pin of that Cellinst */
			for (unsigned int m = 0; m < current_cell->getInputs().size(); m++)
			{
				Node * t = current_cell->getInputs()[m];

				/* for all edges in G.adjacentEdges(t) loop */
				for (unsigned int l = 0; l < t->getForwardEdges().size(); ++l)
				{
					/* u <- G.adjancentVertex(t,e) */
					Edge * e = t->getForwardEdges()[l];

					Node *u = e->getConnectedTo();

					if (e->getNetType() == CELL_EDGE)
					{
						/* if output capacitance has not been initialized */
						if (u->getCoFall() == -1)
						{
							u->setCoRise( Util::calculateOutputCapacitance( u, RISE ) );
							u->setCoFall( Util::calculateOutputCapacitance( u, FALL ) );
						}

						double Co_r = u->getCoRise();
						double Co_f = u->getCoFall();

						/* Get timing info about the current Cellinst */
						Timing * temp_timing = (( CellEdge * ) e)->getTiming();

						double tr_late_f = min_val, tr_late_r = min_val;
						double df = min_val, dr = min_val;

						double at_late_f = min_val, at_late_r = min_val;

						bool inverted_rise = false;
						bool inverted_fall = false;

						UNATE u_fall = NON;
						UNATE u_rise = NON;

						ARC_TYPE timing_type = NO_TYPE;

						for (unsigned int k = 0; k < temp_timing->getWhen().size(); k++)  // for all whens
						{
							if (temp_timing->getWhen()[k]->getTimingSense() == POSITIVE)
							{
								double temp;

								/*****************TRANSITION RISE********************/
								/* the worst transition among all transitions that occur for different when states */
								/* if there exist a timing table for that when state */
								if (temp_timing->getWhen()[k]->getRiseTransition() != NULL && t->getTrLateRise() != min_val)
								{
									temp = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getRiseTransition(), t->getTrLateRise(), Co_r );
									tr_late_r = temp > tr_late_r ? temp : tr_late_r;
								}

								/* do the same as the rise case */

								/*****************TRANSITION FALL********************/
								if (temp_timing->getWhen()[k]->getFallTransition() != NULL && t->getTrLateFall() != min_val)
								{
									temp = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getFallTransition(), t->getTrLateFall(), Co_f );
									tr_late_f = temp > tr_late_f ? temp : tr_late_f;
								}

								/*******************DELAY RISE******************/
								/* if there exits rise delay information for that when case */
								if (temp_timing->getWhen()[k]->getCellRise() != NULL && t->getTrLateRise() != min_val)
								{
									temp = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getCellRise(), t->getTrLateRise(), Co_r );

									/* the worst delay among all delay that occur for different when states */
									if (temp > dr)
									{
										u_rise = POSITIVE;
										dr = temp;
										timing_type = temp_timing->getWhen()[k]->getTimingType();
									}
								}
								else
								{
									if (dr == min_val)
									{
										u_rise = POSITIVE;
										dr = 0;
										timing_type = temp_timing->getWhen()[k]->getTimingType();
									}
								}

								/* do the same as the rise case */

								/*******************DELAY FALL******************/
								if (temp_timing->getWhen()[k]->getCellFall() != NULL && t->getTrLateFall() != min_val)
								{
									temp = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getCellFall(), t->getTrLateFall(), Co_f );
									if (temp > df)
									{
										u_fall = POSITIVE;
										df = temp;
										timing_type = temp_timing->getWhen()[k]->getTimingType();
									}
								}
								else
								{
									if (df == min_val)
									{
										u_fall = POSITIVE;
										df = 0;
										timing_type = temp_timing->getWhen()[k]->getTimingType();
									}
								}

							}

							/* do the symmetric with respect to the POSITIVE unate */

							else if (temp_timing->getWhen()[k]->getTimingSense() == NEGATIVE)
							{
								double temp;
								/*****************TRANSITION RISE********************/
								if (temp_timing->getWhen()[k]->getRiseTransition() != NULL && t->getTrLateFall() != min_val)
								{
									temp = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getRiseTransition(), t->getTrLateFall(), Co_r );

									/*if (t->getCellInst() != NULL && t->getCellInst()->getInstanceName().compare( "p5988A" ) == 0
											&& t->getPin()->getName().compare( "A1" ) == 0)
									{
										cout << "Instance " << t->getCellInst()->getInstanceName() << ", Arc type " << t->getPin()->getName() << "->" << u->getPin()->getName() << ": Negative Unate\n";
										cout << "Transition_Fall @ " << t->getPin()->getName() << ": " << t->getTrLateFall() << " Capacitance Rise @ " << u->getPin()->getName() << ": " << Co_r << "\n";
										cout << "Transition_Rise @ " << u->getPin()->getName() << ": " << temp << "\n\n";
									}*/
									tr_late_r = temp > tr_late_r ? temp : tr_late_r;
								}

								/******************TRANSITION FALL*******************/
								if (temp_timing->getWhen()[k]->getFallTransition() != NULL && t->getTrLateRise() != min_val)
								{
									temp = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getFallTransition(), t->getTrLateRise(), Co_f );

									/*if (t->getCellInst() != NULL && t->getCellInst()->getInstanceName().compare( "p5988A" ) == 0
											&& t->getPin()->getName().compare( "A1" ) == 0)
									{
										cout << "Instance p5988A, Arc type A1->ZN: Negative Unate\n";
										cout << "Transition_Rise @ A1: " << t->getTrLateFall() << " Capacitance Fall @ ZN: " << Co_f << "\n";
										cout << "Transition_Fall @ ZN: " << temp << "\n\n";
									}*/
									tr_late_f = temp > tr_late_f ? temp : tr_late_f;
								}

								/******************DELAY RISE*******************/
								if (temp_timing->getWhen()[k]->getCellRise() != NULL && t->getTrLateFall() != min_val)
								{
									temp = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getCellRise(), t->getTrLateFall(), Co_r );

									/*if (t->getCellInst() != NULL && t->getCellInst()->getInstanceName().compare( "p5988A" ) == 0
											&& t->getPin()->getName().compare( "A1" ) == 0)
									{
										cout << "Instance p5988A, Arc type A1->ZN: Negative Unate\n";
										cout << "Transition_Fall @ A1: " << t->getTrLateFall() << " Capacitance Rise @ ZN: " << Co_r << "\n";
										cout << "Delay_Rise @ ZN: " << temp << "\n\n";
									}*/

									if (temp > dr)
									{
										u_rise = NEGATIVE;
										dr = temp;
										timing_type = temp_timing->getWhen()[k]->getTimingType();
									}
								}
								else
								{
									if (dr == min_val)
									{
										u_rise = NEGATIVE;
										dr = 0;
										timing_type = temp_timing->getWhen()[k]->getTimingType();
									}
								}

								/******************DELAY FALL*******************/
								if (temp_timing->getWhen()[k]->getCellFall() != NULL && t->getTrLateRise() != min_val)
								{
									temp = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getCellFall(), t->getTrLateRise(), Co_f );

									/*if (t->getCellInst() != NULL && t->getCellInst()->getInstanceName().compare( "p5988A" ) == 0
											&& t->getPin()->getName().compare( "A1" ) == 0)
									{
										cout << "Instance p5988A, Arc type A1->ZN: Negative Unate\n";
										cout << "Transition_Rise @ A1: " << t->getTrLateFall() << " Capacitance Fall @ ZN: " << Co_f << "\n";
										cout << "Delay_Fall @ ZN: " << temp << "\n\n";
									}*/

									if (temp > df)
									{
										u_fall = NEGATIVE;
										df = temp;
										timing_type = temp_timing->getWhen()[k]->getTimingType();
									}
								}
								else
								{
									if (df == min_val)
									{
										u_fall = NEGATIVE;
										df = 0;
										timing_type = temp_timing->getWhen()[k]->getTimingType();
									}
								}

							}
							else if (temp_timing->getWhen()[k]->getTimingSense() == NON)
							{
								if (temp_timing->getWhen()[k]->getTimingType() == RISING_EDGE)
								{
									double temp;
									/****************TRANSITION RISE*********************/
									if (temp_timing->getWhen()[k]->getRiseTransition() != NULL && t->getTrLateRise() != min_val)
									{

										temp = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getRiseTransition(), t->getTrLateRise(),
												Co_r );

										tr_late_r = temp > tr_late_r ? temp : tr_late_r;
									}

									/******************TRANSITION FALL*******************/
									if (temp_timing->getWhen()[k]->getFallTransition() != NULL && t->getTrLateRise() != min_val)
									{
										temp = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getFallTransition(), t->getTrLateRise(),
												Co_f );

										tr_late_f = temp > tr_late_f ? temp : tr_late_f;
									}
									/******************DELAY RISE*******************/
									if (temp_timing->getWhen()[k]->getCellRise() != NULL && t->getTrLateRise() != min_val)
									{
										/* Assume that the edge is a POSITIVE UNATE edge */
										temp = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getCellRise(), t->getTrLateRise(), Co_r );

										if (temp > dr)
										{
											u_rise = POSITIVE;
											dr = temp;
											timing_type = temp_timing->getWhen()[k]->getTimingType();
										}
									}
									else
									{
										if (dr == min_val)
										{
											u_rise = POSITIVE;
											dr = 0;
											timing_type = temp_timing->getWhen()[k]->getTimingType();
										}
									}

									/********************DELAY FALL*****************/
									if (temp_timing->getWhen()[k]->getCellFall() != NULL && t->getTrLateRise() != min_val)
									{

										/* Assume that the edge is a  NEGATIVE UNATE edge*/
										temp = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getCellFall(), t->getTrLateRise(), Co_f );

										if (temp > df)
										{
											u_fall = NEGATIVE;
											df = temp;
											timing_type = temp_timing->getWhen()[k]->getTimingType();
										}
									}
									else
									{
										if (df == min_val)
										{
											u_fall = POSITIVE;
											df = 0;
											timing_type = temp_timing->getWhen()[k]->getTimingType();
										}
									}
									/*************************************/
								}
								else if (temp_timing->getWhen()[k]->getTimingType() == FALLING_EDGE)
								{
									double temp;
									/****************TRANSITION RISE*********************/
									if (temp_timing->getWhen()[k]->getRiseTransition() != NULL && t->getTrLateFall() != min_val)
									{

										temp = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getRiseTransition(), t->getTrLateFall(),
												Co_r );

										tr_late_r = temp > tr_late_r ? temp : tr_late_r;
									}

									/******************TRANSITION FALL*******************/
									if (temp_timing->getWhen()[k]->getFallTransition() != NULL && t->getTrLateFall() != min_val)
									{
										temp = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getFallTransition(), t->getTrLateFall(),
												Co_f );

										tr_late_f = temp > tr_late_f ? temp : tr_late_f;
									}
									/******************DELAY RISE*******************/
									if (temp_timing->getWhen()[k]->getCellRise() != NULL && t->getTrLateFall() != min_val)
									{
										/* Assume that the edge is a NEGATIVE UNATE edge */
										temp = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getCellRise(), t->getTrLateFall(), Co_r );

										if (temp > dr)
										{
											u_rise = NEGATIVE;
											dr = temp;
											timing_type = temp_timing->getWhen()[k]->getTimingType();
										}
									}
									else
									{
										if (dr == min_val)
										{
											u_rise = POSITIVE;
											dr = 0;
											timing_type = temp_timing->getWhen()[k]->getTimingType();
										}
									}

									/********************DELAY FALL*****************/
									if (temp_timing->getWhen()[k]->getCellFall() != NULL && t->getTrLateFall() != min_val)
									{

										/* Assume that the edge is a  POSITIVE UNATE edge*/
										temp = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getCellFall(), t->getTrLateFall(), Co_f );

										if (temp > df)
										{
											u_fall = POSITIVE;
											df = temp;
											timing_type = temp_timing->getWhen()[k]->getTimingType();
										}
									}
									else
									{
										if (df == min_val)
										{
											u_fall = POSITIVE;
											df = 0;
											timing_type = temp_timing->getWhen()[k]->getTimingType();
										}
									}
									/*************************************/
								}
								else if (temp_timing->getWhen()[k]->getTimingType() == COMBINATIONAL)
								{
									double temp1, temp2, temp;

									/****************TRANSITION RISE*********************/

									if (temp_timing->getWhen()[k]->getRiseTransition() != NULL && t->getTrLateRise() != min_val
											&& t->getTrLateFall() != min_val)
									{
										temp1 = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getRiseTransition(), t->getTrLateFall(),
												Co_r );
										temp2 = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getRiseTransition(), t->getTrLateRise(),
												Co_r );

										temp = temp1 > temp2 ? temp1 : temp2;
										tr_late_r = temp > tr_late_r ? temp : tr_late_r;
									}

									/******************TRANSITION FALL*******************/
									if (temp_timing->getWhen()[k]->getFallTransition() != NULL && t->getTrLateRise() != min_val
											&& t->getTrLateFall() != min_val)
									{
										temp1 = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getFallTransition(), t->getTrLateRise(),
												Co_f );
										temp2 = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getFallTransition(), t->getTrLateFall(),
												Co_f );

										temp = temp1 > temp2 ? temp1 : temp2;
										tr_late_f = temp > tr_late_f ? temp : tr_late_f;
									}

									/******************DELAY RISE*******************/
									if (temp_timing->getWhen()[k]->getCellRise() != NULL && t->getTrLateRise() != min_val
											&& t->getTrLateFall() != min_val)
									{

										/* Assume that the edge is a  NEGATIVE UNATE edge */
										temp1 = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getCellRise(), t->getTrLateFall(), Co_r );

										/* Assume that the edge is a POSITIVE UNATE edge */
										temp2 = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getCellRise(), t->getTrLateRise(), Co_r );

										UNATE temp_unate;

										if (temp1 > temp2)
										{
											temp = temp1;
											temp_unate = NEGATIVE;
										}
										else
										{
											temp = temp2;
											temp_unate = POSITIVE;
										}

										if (temp > dr)
										{
											u_rise = temp_unate;
											dr = temp;
											timing_type = temp_timing->getWhen()[k]->getTimingType();
										}
									}
									else
									{
										if (dr == min_val)
										{
											u_rise = POSITIVE;
											dr = 0;
											timing_type = temp_timing->getWhen()[k]->getTimingType();
										}
									}

									/********************DELAY FALL*****************/
									if (temp_timing->getWhen()[k]->getCellFall() != NULL && t->getTrLateRise() != min_val
											&& t->getTrLateFall() != min_val)
									{

										/* Assume that the edge is a  NEGATIVE UNATE edge*/
										temp1 = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getCellFall(), t->getTrLateRise(), Co_f );

										/* Assume that the edge is a  POSITIVE UNATE edge*/
										temp2 = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getCellFall(), t->getTrLateFall(), Co_f );

										UNATE temp_unate;

										if (temp1 > temp2)
										{
											temp = temp1;
											temp_unate = NEGATIVE;
										}
										else
										{
											temp = temp2;
											temp_unate = POSITIVE;
										}

										if (temp > df)
										{
											u_fall = temp_unate;
											df = temp;
											timing_type = temp_timing->getWhen()[k]->getTimingType();
										}
									}
									else
									{
										if (df == min_val)
										{
											u_fall = POSITIVE;
											df = 0;
											timing_type = temp_timing->getWhen()[k]->getTimingType();
										}
									}
									/*************************************/
								}
							}
						}

						/* set delay info to forward and backward edges */

						e->setDelayFall( df );
						(( CellEdge * ) e)->setFallTimingSense( u_fall );
						e->setDelayRise( dr );
						(( CellEdge * ) e)->setRiseTimingSense( u_rise );

						e->getOppositeEdge()->setDelayFall( df );
						(( CellEdge * ) e->getOppositeEdge())->setFallTimingSense( u_fall );
						e->getOppositeEdge()->setDelayRise( dr );
						(( CellEdge * ) e->getOppositeEdge())->setRiseTimingSense( u_rise );

						/* if user has disabled the asynchronous arcs consideration and
						 * the type of edge is an asynchronous edge (not normal) */
						if (preset_clear == false && (timing_type == PRESET || timing_type == CLEAR))
						{
							e->setBlocked( true );
							e->getOppositeEdge()->setBlocked( true );
						}
						else
						{
							e->setBlocked( false );
							e->getOppositeEdge()->setBlocked( false );
						}

						/* if edge e is not blocked (we talk about an asynch. edge )*/
						if (!e->isBlocked())
						{
							if (u_fall == NEGATIVE && t->getAtLateRise() != min_val)
							{
								at_late_f = t->getAtLateRise() + df;
								inverted_fall = true;
							}
							else if (u_fall == POSITIVE && t->getAtLateFall() != min_val)
							{
								at_late_f = t->getAtLateFall() + df;
								inverted_fall = false;
							}

							if (u_rise == NEGATIVE && t->getAtLateFall() != min_val)
							{
								at_late_r = t->getAtLateFall() + dr;
								inverted_rise = true;
							}
							else if (u_rise == POSITIVE && t->getAtLateRise() != min_val)
							{
								at_late_r = t->getAtLateRise() + dr;
								inverted_rise = false;
							}

							if (at_late_f > u->getAtLateFall())
							{
								u->setAtLateFall( at_late_f );
								u->setPreviousFall( t );
								u->setInvertedFall( inverted_fall );
							}

							if (at_late_r > u->getAtLateRise())
							{
								u->setAtLateRise( at_late_r );
								u->setPreviousRise( t );
								u->setInvertedRise( inverted_rise );
							}
						}

						if (tr_late_f > u->getTrLateFall())
						{
							u->setTrLateFall( tr_late_f );
						}

						if (tr_late_r > u->getTrLateRise())
						{
							u->setTrLateRise( tr_late_r );
						}
					}
				}
			}

			/* transfer the transiton and AT info form the outputs of the Cellinst of
			 * level i to the Cellinsts of level i+1 */

			//For every output of this cell
			for (unsigned int m = 0; m < current_cell->getOutputs().size(); m++)
			{
				Node * t = current_cell->getOutputs()[m];

				/* for all edges in G.adjacentEdges(t) loop */
				for (unsigned int l = 0; l < t->getForwardEdges().size(); ++l)
				{
					/* u <- G.adjancentVertex(t,e) */
					Edge * e = t->getForwardEdges()[l];

					Node *u = e->getConnectedTo();

					if (e->getNetType() == NET_EDGE)
					{
						if (t->getTrLateFall() != min_val)
						{
							u->setTrLateFall( (( NetEdge * ) e)->getOutputTransitionFall( t->getTrLateFall() ) );
						}
						if (t->getTrLateRise() != min_val)
						{
							u->setTrLateRise( (( NetEdge * ) e)->getOutputTransitionRise( t->getTrLateRise() ) );
						}

						if (t->getAtLateFall() != min_val)
						{
							u->setAtLateFall( t->getAtLateFall() + e->getDelayFall() );
							u->setPreviousFall( t );
							u->setInvertedFall( false );
						}
						if (t->getAtLateRise() != min_val)
						{
							u->setAtLateRise( t->getAtLateRise() + e->getDelayRise() );
							u->setPreviousRise( t );
							u->setInvertedRise( false );
						}

					}
				}
			}
		}
	}

	performed_stas++;

}

/* 1. Finds the minimum lelel from which the changes has to be applied
 * 2. Marks all the affected cell instances. */
int Graph::findMinimunLevel(vector<Cellinst *>& changed_cells, set<Node *>& changed_PIs)
{
	int min_level = numeric_limits<int>::max();

	for (unsigned r = 0; r < changed_cells.size(); r++)
	{
		Cellinst * changed_cell = changed_cells[r];
		for (unsigned int i = 0; i < changed_cell->getInputs().size(); i++)  // for every input of the cell, find the previous cell
		{
			Node * a = changed_cell->getInputs()[i];

			for (unsigned j = 0; j < a->getBackwardEdges().size(); j++)
			{
				Node * b = a->getBackwardEdges()[j]->getConnectedTo();

				Cellinst * cell = b->getCellInst();

				if (cell == NULL)
				{
					min_level = -1;
					changed_PIs.insert( b );
				}
				else
				{
					min_level = cell->getLevel() < min_level ? cell->getLevel() : min_level;
					cell->setIncrementalChange( true );
				}
			}
		}
	}

	return min_level;

}

void Graph::staIncremental(vector<Parasitic *>& parasitics, vector<Cellinst *>& changed_cells)
{

#if DEBUG
	cout << "\nNets delay calculation:\n";
#endif

	/* Loop over all nets and apply Elmore delay model */
#pragma omp parallel for schedule(dynamic)
	for (unsigned int i = 0; i < parasitics.size(); i++)
	{
#if 0
		cout << "Net " << parasitics[i]->getMappedName() << " (" << parasitics[i]->getOriginalName() << ")" << "\n";
#endif
		parasitics[i]->elmoreDelayCalculation();

		parasitics[i]->bFactorCalculation();
	}

#if DEBUG
	cout << "\n";
#endif

	/* reset the parasitics */
	parasitics.clear();

	set<Node *> changed_PIs;

	int min_level = findMinimunLevel( changed_cells, changed_PIs );

	int max_level = (*(terminal_nodes.begin())).level;

	double min_val = -numeric_limits<double>::max();

	if (min_level == -1)
	{
		set<Node *>::iterator it;
		for (it = changed_PIs.begin(); it != changed_PIs.end(); it++)
		{
			Node * t = *it;

			for (unsigned int i = 0; i < t->getForwardEdges().size(); i++)
			{
				/* u <- G.adjancentVertex(t,e) */
				Edge * e = t->getForwardEdges()[i];

				Node *u = e->getConnectedTo();

				if (e->getNetType() == NET_EDGE)
				{
					bool update = false;

					if (t->getTrLateFall() != min_val)
					{
						u->setTrLateFall( (( NetEdge * ) e)->getOutputTransitionFall( t->getTrLateFall() ) );
						update = true;
					}
					if (t->getTrLateRise() != min_val)
					{
						u->setTrLateRise( (( NetEdge * ) e)->getOutputTransitionRise( t->getTrLateRise() ) );
						update = true;
					}

					if (t->getAtLateFall() != min_val)
					{
						u->setAtLateFall( t->getAtLateFall() + e->getDelayFall() );
						u->setPreviousFall( t );
						u->setInvertedFall( false );
						update = true;
					}
					if (t->getAtLateRise() != min_val)
					{
						u->setAtLateRise( t->getAtLateRise() + e->getDelayRise() );
						u->setPreviousRise( t );
						u->setInvertedRise( false );
						update = true;
					}

					if (u->getCellInst() != NULL && update)
					{
						u->getCellInst()->setIncrementalChange( true );
					}
				}
			}
		}

		min_level = 0;
	}

	//Propage the transitions from level n to level n+1 CELLS
#pragma omp parallel
	for (unsigned int i = min_level; i < max_level; i++)
	{

		/* Loop over all cells @level i */
#pragma omp for
		for (unsigned int j = 0; j < cells_at_level[i]; j++)
		{
			Cellinst * current_cell = cells_hierarchy[i][j];

			if (current_cell->isIncrementalChanged())
			{
				current_cell->setIncrementalChange( false );

				//For every output of this cell
				for (unsigned int m = 0; m < current_cell->getOutputs().size(); m++)  // RESET THE CO, TRANSITIONS, AT
				{
					Node * t = current_cell->getOutputs()[m];

					t->setCoFall( -1 );
					t->setCoRise( -1 );

					t->setTrLateFall( min_val );
					t->setTrLateRise( min_val );

					t->setAtLateFall( min_val );
					t->setAtLateRise( min_val );
				}

				//For every input of this cell
				for (unsigned int m = 0; m < current_cell->getInputs().size(); m++)
				{
					Node * t = current_cell->getInputs()[m];

					/* for all edges in G.adjacentEdges(t) loop */
					for (unsigned int l = 0; l < t->getForwardEdges().size(); ++l)
					{
						/* u <- G.adjancentVertex(t,e) */
						Edge * e = t->getForwardEdges()[l];

						Node *u = e->getConnectedTo();

						if (e->getNetType() == CELL_EDGE)
						{
							if (u->getCoFall() == -1)
							{
								u->setCoRise( Util::calculateOutputCapacitance( u, RISE ) );
								u->setCoFall( Util::calculateOutputCapacitance( u, FALL ) );
							}

							double Co_r = u->getCoRise();
							double Co_f = u->getCoFall();

							Timing * temp_timing = (( CellEdge * ) e)->getTiming();

							double tr_late_f = min_val, tr_late_r = min_val;
							double df = min_val, dr = min_val;

							double at_late_f = min_val, at_late_r = min_val;

							bool inverted_rise = false;
							bool inverted_fall = false;

							UNATE u_fall = NON;
							UNATE u_rise = NON;

							ARC_TYPE timing_type = NO_TYPE;

							for (unsigned int k = 0; k < temp_timing->getWhen().size(); k++)  // for all whens
							{
								if (temp_timing->getWhen()[k]->getTimingSense() == POSITIVE)
								{
									double temp;
									/*****************TRANSITION RISE********************/
									if (temp_timing->getWhen()[k]->getRiseTransition() != NULL && t->getTrLateRise() != min_val)
									{
										temp = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getRiseTransition(), t->getTrLateRise(),
												Co_r );
										tr_late_r = temp > tr_late_r ? temp : tr_late_r;
									}

									/*****************TRANSITION FALL********************/

									if (temp_timing->getWhen()[k]->getFallTransition() != NULL && t->getTrLateFall() != min_val)
									{
										temp = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getFallTransition(), t->getTrLateFall(),
												Co_f );
										tr_late_f = temp > tr_late_f ? temp : tr_late_f;
									}

									/*******************DELAY RISE******************/
									if (temp_timing->getWhen()[k]->getCellRise() != NULL && t->getTrLateRise() != min_val)
									{
										temp = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getCellRise(), t->getTrLateRise(), Co_r );

										if (temp > dr)
										{
											u_rise = POSITIVE;
											dr = temp;
											timing_type = temp_timing->getWhen()[k]->getTimingType();
										}
									}
									else
									{
										if (dr == min_val)
										{
											u_rise = POSITIVE;
											dr = 0;
											timing_type = temp_timing->getWhen()[k]->getTimingType();
										}
									}

									/*******************DELAY FALL******************/
									if (temp_timing->getWhen()[k]->getCellFall() != NULL && t->getTrLateFall() != min_val)
									{
										temp = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getCellFall(), t->getTrLateFall(), Co_f );
										if (temp > df)
										{
											u_fall = POSITIVE;
											df = temp;
											timing_type = temp_timing->getWhen()[k]->getTimingType();
										}
									}
									else
									{
										if (df == min_val)
										{
											u_fall = POSITIVE;
											df = 0;
											timing_type = temp_timing->getWhen()[k]->getTimingType();
										}
									}

								}
								else if (temp_timing->getWhen()[k]->getTimingSense() == NEGATIVE)
								{
									double temp;
									/*****************TRANSITION RISE********************/
									if (temp_timing->getWhen()[k]->getRiseTransition() != NULL && t->getTrLateFall() != min_val)
									{
										temp = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getRiseTransition(), t->getTrLateFall(),
												Co_r );
										tr_late_r = temp > tr_late_r ? temp : tr_late_r;
									}

									/******************TRANSITION FALL*******************/
									if (temp_timing->getWhen()[k]->getFallTransition() != NULL && t->getTrLateRise() != min_val)
									{
										temp = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getFallTransition(), t->getTrLateRise(),
												Co_f );
										tr_late_f = temp > tr_late_f ? temp : tr_late_f;
									}

									/******************DELAY RISE*******************/
									if (temp_timing->getWhen()[k]->getCellRise() != NULL && t->getTrLateFall() != min_val)
									{
										temp = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getCellRise(), t->getTrLateFall(), Co_r );
										if (temp > dr)
										{
											u_rise = NEGATIVE;
											dr = temp;
											timing_type = temp_timing->getWhen()[k]->getTimingType();
										}
									}
									else
									{
										if (dr == min_val)
										{
											u_rise = NEGATIVE;
											dr = 0;
											timing_type = temp_timing->getWhen()[k]->getTimingType();
										}
									}

									/******************DELAY FALL*******************/
									if (temp_timing->getWhen()[k]->getCellFall() != NULL && t->getTrLateRise() != min_val)
									{
										temp = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getCellFall(), t->getTrLateRise(), Co_f );
										if (temp > df)
										{
											u_fall = NEGATIVE;
											df = temp;
											timing_type = temp_timing->getWhen()[k]->getTimingType();
										}
									}
									else
									{
										if (df == min_val)
										{
											u_fall = NEGATIVE;
											df = 0;
											timing_type = temp_timing->getWhen()[k]->getTimingType();
										}
									}

								}
								else if (temp_timing->getWhen()[k]->getTimingSense() == NON)
								{
									if (temp_timing->getWhen()[k]->getTimingType() == RISING_EDGE)
									{
										double temp;
										/****************TRANSITION RISE*********************/
										if (temp_timing->getWhen()[k]->getRiseTransition() != NULL && t->getTrLateRise() != min_val)
										{

											temp = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getRiseTransition(), t->getTrLateRise(),
													Co_r );

											tr_late_r = temp > tr_late_r ? temp : tr_late_r;
										}

										/******************TRANSITION FALL*******************/
										if (temp_timing->getWhen()[k]->getFallTransition() != NULL && t->getTrLateRise() != min_val)
										{
											temp = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getFallTransition(), t->getTrLateRise(),
													Co_f );

											tr_late_f = temp > tr_late_f ? temp : tr_late_f;
										}
										/******************DELAY RISE*******************/
										if (temp_timing->getWhen()[k]->getCellRise() != NULL && t->getTrLateRise() != min_val)
										{
											/* Assume that the edge is a POSITIVE UNATE edge */
											temp = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getCellRise(), t->getTrLateRise(), Co_r );

											if (temp > dr)
											{
												u_rise = POSITIVE;
												dr = temp;
												timing_type = temp_timing->getWhen()[k]->getTimingType();
											}
										}
										else
										{
											if (dr == min_val)
											{
												u_rise = POSITIVE;
												dr = 0;
												timing_type = temp_timing->getWhen()[k]->getTimingType();
											}
										}

										/********************DELAY FALL*****************/
										if (temp_timing->getWhen()[k]->getCellFall() != NULL && t->getTrLateRise() != min_val)
										{

											/* Assume that the edge is a  NEGATIVE UNATE edge*/
											temp = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getCellFall(), t->getTrLateRise(), Co_f );

											if (temp > df)
											{
												u_fall = NEGATIVE;
												df = temp;
												timing_type = temp_timing->getWhen()[k]->getTimingType();
											}
										}
										else
										{
											if (df == min_val)
											{
												u_fall = POSITIVE;
												df = 0;
												timing_type = temp_timing->getWhen()[k]->getTimingType();
											}
										}
										/*************************************/
									}
									else if (temp_timing->getWhen()[k]->getTimingType() == FALLING_EDGE)
									{
										double temp;
										/****************TRANSITION RISE*********************/
										if (temp_timing->getWhen()[k]->getRiseTransition() != NULL && t->getTrLateFall() != min_val)
										{

											temp = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getRiseTransition(), t->getTrLateFall(),
													Co_r );

											tr_late_r = temp > tr_late_r ? temp : tr_late_r;
										}

										/******************TRANSITION FALL*******************/
										if (temp_timing->getWhen()[k]->getFallTransition() != NULL && t->getTrLateFall() != min_val)
										{
											temp = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getFallTransition(), t->getTrLateFall(),
													Co_f );

											tr_late_f = temp > tr_late_f ? temp : tr_late_f;
										}
										/******************DELAY RISE*******************/
										if (temp_timing->getWhen()[k]->getCellRise() != NULL && t->getTrLateFall() != min_val)
										{
											/* Assume that the edge is a NEGATIVE UNATE edge */
											temp = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getCellRise(), t->getTrLateFall(), Co_r );

											if (temp > dr)
											{
												u_rise = NEGATIVE;
												dr = temp;
												timing_type = temp_timing->getWhen()[k]->getTimingType();
											}
										}
										else
										{
											if (dr == min_val)
											{
												u_rise = POSITIVE;
												dr = 0;
												timing_type = temp_timing->getWhen()[k]->getTimingType();
											}
										}

										/********************DELAY FALL*****************/
										if (temp_timing->getWhen()[k]->getCellFall() != NULL && t->getTrLateFall() != min_val)
										{

											/* Assume that the edge is a  POSITIVE UNATE edge*/
											temp = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getCellFall(), t->getTrLateFall(), Co_f );

											if (temp > df)
											{
												u_fall = POSITIVE;
												df = temp;
												timing_type = temp_timing->getWhen()[k]->getTimingType();
											}
										}
										else
										{
											if (df == min_val)
											{
												u_fall = POSITIVE;
												df = 0;
												timing_type = temp_timing->getWhen()[k]->getTimingType();
											}
										}
										/*************************************/
									}
									else if (temp_timing->getWhen()[k]->getTimingType() == COMBINATIONAL)
									{
										double temp1, temp2, temp;

										/****************TRANSITION RISE*********************/

										if (temp_timing->getWhen()[k]->getRiseTransition() != NULL && t->getTrLateRise() != min_val
												&& t->getTrLateFall() != min_val)
										{
											temp1 = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getRiseTransition(), t->getTrLateFall(),
													Co_r );
											temp2 = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getRiseTransition(), t->getTrLateRise(),
													Co_r );

											temp = temp1 > temp2 ? temp1 : temp2;
											tr_late_r = temp > tr_late_r ? temp : tr_late_r;
										}

										/******************TRANSITION FALL*******************/
										if (temp_timing->getWhen()[k]->getFallTransition() != NULL && t->getTrLateRise() != min_val
												&& t->getTrLateFall() != min_val)
										{
											temp1 = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getFallTransition(), t->getTrLateRise(),
													Co_f );
											temp2 = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getFallTransition(), t->getTrLateFall(),
													Co_f );

											temp = temp1 > temp2 ? temp1 : temp2;
											tr_late_f = temp > tr_late_f ? temp : tr_late_f;
										}

										/******************DELAY RISE*******************/
										if (temp_timing->getWhen()[k]->getCellRise() != NULL && t->getTrLateRise() != min_val
												&& t->getTrLateFall() != min_val)
										{

											/* Assume that the edge is a  NEGATIVE UNATE edge */
											temp1 = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getCellRise(), t->getTrLateFall(), Co_r );

											/* Assume that the edge is a POSITIVE UNATE edge */
											temp2 = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getCellRise(), t->getTrLateRise(), Co_r );

											UNATE temp_unate;

											if (temp1 > temp2)
											{
												temp = temp1;
												temp_unate = NEGATIVE;
											}
											else
											{
												temp = temp2;
												temp_unate = POSITIVE;
											}

											if (temp > dr)
											{
												u_rise = temp_unate;
												dr = temp;
												timing_type = temp_timing->getWhen()[k]->getTimingType();
											}
										}
										else
										{
											if (dr == min_val)
											{
												u_rise = POSITIVE;
												dr = 0;
												timing_type = temp_timing->getWhen()[k]->getTimingType();
											}
										}

										/********************DELAY FALL*****************/
										if (temp_timing->getWhen()[k]->getCellFall() != NULL && t->getTrLateRise() != min_val
												&& t->getTrLateFall() != min_val)
										{

											/* Assume that the edge is a  NEGATIVE UNATE edge*/
											temp1 = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getCellFall(), t->getTrLateRise(), Co_f );

											/* Assume that the edge is a  POSITIVE UNATE edge*/
											temp2 = Util::bilinearInterpolation( temp_timing->getWhen()[k]->getCellFall(), t->getTrLateFall(), Co_f );

											UNATE temp_unate;

											if (temp1 > temp2)
											{
												temp = temp1;
												temp_unate = NEGATIVE;
											}
											else
											{
												temp = temp2;
												temp_unate = POSITIVE;
											}

											if (temp > df)
											{
												u_fall = temp_unate;
												df = temp;
												timing_type = temp_timing->getWhen()[k]->getTimingType();
											}
										}
										else
										{
											if (df == min_val)
											{
												u_fall = POSITIVE;
												df = 0;
												timing_type = temp_timing->getWhen()[k]->getTimingType();
											}
										}
										/*************************************/
									}
								}
							}

							e->setDelayFall( df );
							(( CellEdge * ) e)->setFallTimingSense( u_fall );
							e->setDelayRise( dr );
							(( CellEdge * ) e)->setRiseTimingSense( u_rise );

							e->getOppositeEdge()->setDelayFall( df );
							(( CellEdge * ) e->getOppositeEdge())->setFallTimingSense( u_fall );
							e->getOppositeEdge()->setDelayRise( dr );
							(( CellEdge * ) e->getOppositeEdge())->setRiseTimingSense( u_rise );

							if (preset_clear == false && (timing_type == PRESET || timing_type == CLEAR))
							{
								e->setBlocked( true );
								e->getOppositeEdge()->setBlocked( true );
							}
							else
							{
								e->setBlocked( false );
								e->getOppositeEdge()->setBlocked( false );
							}

							if (!e->isBlocked())
							{
								if (u_fall == NEGATIVE && t->getAtLateRise() != min_val)
								{
									at_late_f = t->getAtLateRise() + df;
									inverted_fall = true;
								}
								else if (u_fall == POSITIVE && t->getAtLateFall() != min_val)
								{
									at_late_f = t->getAtLateFall() + df;
									inverted_fall = false;
								}

								if (u_rise == NEGATIVE && t->getAtLateFall() != min_val)
								{
									at_late_r = t->getAtLateFall() + dr;
									inverted_rise = true;
								}
								else if (u_rise == POSITIVE && t->getAtLateRise() != min_val)
								{
									at_late_r = t->getAtLateRise() + dr;
									inverted_rise = false;
								}

								if (at_late_f > u->getAtLateFall())
								{
									u->setAtLateFall( at_late_f );
									u->setPreviousFall( t );
									u->setInvertedFall( inverted_fall );
								}

								if (at_late_r > u->getAtLateRise())
								{
									u->setAtLateRise( at_late_r );
									u->setPreviousRise( t );
									u->setInvertedRise( inverted_rise );
								}
							}

							if (tr_late_f > u->getTrLateFall())
							{
								u->setTrLateFall( tr_late_f );
							}

							if (tr_late_r > u->getTrLateRise())
							{
								u->setTrLateRise( tr_late_r );
							}
						}
					}
				}

				/* For every output of this cell */
				for (unsigned int m = 0; m < current_cell->getOutputs().size(); m++)
				{
					Node * t = current_cell->getOutputs()[m];

					/* for all edges in G.adjacentEdges(t) loop */
					for (unsigned int l = 0; l < t->getForwardEdges().size(); ++l)
					{
						/* u <- G.adjancentVertex(t,e) */
						Edge * e = t->getForwardEdges()[l];

						/* get the end point(u) of Edge e */
						Node *u = e->getConnectedTo();

						if (e->getNetType() == NET_EDGE)
						{
							bool update = false;

							if (t->getTrLateFall() != min_val)
							{
								double temp_val = (( NetEdge * ) e)->getOutputTransitionFall( t->getTrLateFall() );

								if (u->getTrLateFall() != temp_val)
								{
									u->setTrLateFall( temp_val );
									update = true;
								}

							}

							if (t->getTrLateRise() != min_val)
							{
								double temp_val = (( NetEdge * ) e)->getOutputTransitionRise( t->getTrLateRise() );

								if (u->getTrLateRise() != temp_val)
								{
									u->setTrLateRise( temp_val );
									update = true;
								}
							}

							if (t->getAtLateFall() != min_val)
							{
								double temp_val = t->getAtLateFall() + e->getDelayFall();

								if (u->getAtLateFall() != temp_val)
								{
									u->setAtLateFall( temp_val );
									u->setPreviousFall( t );
									u->setInvertedFall( false );
									update = true;
								}
							}

							if (t->getAtLateRise() != min_val)
							{
								double temp_val = t->getAtLateRise() + e->getDelayRise();

								if (u->getAtLateRise() != temp_val)
								{
									u->setAtLateRise( temp_val );
									u->setPreviousRise( t );
									u->setInvertedRise( false );
									update = true;
								}
							}

							if (u->getCellInst() != NULL && update)
							{
								u->getCellInst()->setIncrementalChange( true );
							}

						}

					}
				}
			}
		}
	}

	performed_stas++;

}

void Graph::levelization(cellinst_map& instantiated_cells)
{

	cout << "Performing levelization...\n";

	queue<Cellinst *> Q;

	node_map::iterator p;

	/* enqueue all PIs */
	for (p = inputs.begin(); p != inputs.end(); ++p)
	{
		/* Get forward edges from node p(iterator) */
		for (unsigned int i = 0; i < p->second->getForwardEdges().size(); i++)
		{
			/* Get Cellinst connected with a PI */
			Cellinst * a = p->second->getForwardEdges()[i]->getConnectedTo()->getCellInst();

			/* if a is not directly connected to a primary output */
			if (a != NULL)
			{
				a->setLevel( 1 );

				total_levels = 1;

				a->setVisited( true );

				Q.push( a );
			}
		}
	}

	/* enqueue all Virtuals */
	for (p = virtual_nodes.begin(); p != virtual_nodes.end(); ++p)
	{
		for (unsigned int i = 0; i < p->second->getForwardEdges().size(); i++)
		{
			Cellinst * a = p->second->getForwardEdges()[i]->getConnectedTo()->getCellInst();
			if (a != NULL)
			{
				a->setLevel( 1 );

				total_levels = 1;

				a->setVisited( true );

				Q.push( a );
			}
		}

	}

	while (!Q.empty())
	{
		/* t <- Q.dequeue() */
		Cellinst *t = Q.front();

		total_levels = t->getLevel() > total_levels ? t->getLevel() : total_levels;

		for (unsigned int i = 0; i < t->getInputs().size(); i++)
		{
			Node * input = t->getInputs()[i];

			for (unsigned int j = 0; j < input->getForwardEdges().size(); j++)
			{
				Node * output = input->getForwardEdges()[j]->getConnectedTo();

				for (unsigned int k = 0; k < output->getForwardEdges().size(); k++)
				{

					Node * next_input = output->getForwardEdges()[k]->getConnectedTo();
					Cellinst * u = next_input->getCellInst();

					bool chain_reaction = false;

					/* if there is a Cellinst u and the "next input" isn't a D-pin of a FF  or the cell has an input that has no connection to the cell output */
					if (u != NULL && !(next_input->getForwardEdges().empty() && next_input->getPin()->getType() == INPUT))  // && u->isSequential()))
					{
						/* if the parent's level + 1 is greater than child's level */
						if (t->getLevel() + 1 > u->getLevel())
						{
							if (t->getLevel() + 1 == INVALID_LEVEL)
							{
								cout << "The circuit contains a cycle and cannot be levelized.\n";
								cout << "Terminating.\n";
								exit( -1 );
							}

							u->setLevel( t->getLevel() + 1 );
							chain_reaction = true;

						}

						if (!u->isVisited() || chain_reaction)
						{
							u->setVisited( true );
							Q.push( u );
						}

					}
				}
			}
		}

		Q.pop();

	}

	cout << "Maximum cell level: " << total_levels << "\n\n";

#if HISTOGRAM
	ofstream file;
	file.open( "cells_histogram" );
#endif

	total_levels++;

	cells_at_level.resize( total_levels );
	vector<int> used( total_levels );

	cellinst_map::iterator it;
	for (it = instantiated_cells.begin(); it != instantiated_cells.end(); it++)
	{
		cells_at_level[it->second->getLevel()]++;
	}

	cells_hierarchy = new Cellinst **[total_levels];
	for (unsigned int i = 0; i < total_levels; i++)
	{
#if HISTOGRAM
		file << i << " " << cells_at_level[i] << "\n";
#endif
		cells_hierarchy[i] = new Cellinst *[cells_at_level[i]];
	}

	for (it = instantiated_cells.begin(); it != instantiated_cells.end(); it++)
	{
		cells_hierarchy[it->second->getLevel()][used[it->second->getLevel()]] = it->second;
		used[it->second->getLevel()]++;
	}

#if HISTOGRAM
	file.close();

	FILE *pipe = popen( "gnuplot -persist", "w" );

	fprintf( pipe,
			"set terminal wxt size 800,600;set title \"Cells Level Histogram\";set xlabel \"Level\";set autoscale;set ylabel \"Cells at Level\";set grid;set boxwidth 0.80 relative;set style fill solid 0.8 noborder;plot \"cells_histogram\" u 1:2 w boxes lc rgb\"blue\" notitle;\n" );
	fclose( pipe );
#endif

}

void Graph::initializeTerminalNodes()
{
	node_map::iterator it;

	/* Consider critical paths begining from POs */
	if (pt == OUT)
	{
		for (it = outputs.begin(); it != outputs.end(); it++)
		{
			T_node temp;
			temp.terminal = it->second;

			for (unsigned int i = 0; i < it->second->getBackwardEdges().size(); i++)
			{
				Node * back = it->second->getBackwardEdges()[i]->getConnectedTo();

				Cellinst * back_cell = back->getCellInst();

				if (back_cell != NULL)
				{
					temp.level = back_cell->getLevel() + 1 > temp.level ? back_cell->getLevel() + 1 : temp.level;
				}
				else
				{
					temp.level = 1 > temp.level ? 1 : temp.level;
				}

			}

			terminal_nodes.insert( temp );

			it->second->setUsedAsTerminal( true );
		}
	}

	/* Consider critical paths begining from POs and D pins @FFs ( no forward edges ) */
	if (pt == ALL)
	{
		for (it = outputs.begin(); it != outputs.end(); it++)
		{
			T_node temp;
			temp.terminal = it->second;

			for (unsigned int i = 0; i < it->second->getBackwardEdges().size(); i++)
			{
				Node * back = it->second->getBackwardEdges()[i]->getConnectedTo();

				Cellinst * back_cell = back->getCellInst();

				if (back_cell != NULL)
				{
					temp.level = back_cell->getLevel() + 1 > temp.level ? back_cell->getLevel() + 1 : temp.level;
				}
				else
				{
					temp.level = 1 > temp.level ? 1 : temp.level;
				}

			}

			terminal_nodes.insert( temp );

			it->second->setUsedAsTerminal( true );
		}

		for (it = int_nodes.begin(); it != int_nodes.end(); it++)
		{
			if (it->second->getForwardEdges().empty())
			{
				T_node temp;
				temp.terminal = it->second;

				if (it->second->getPin()->getType() == INPUT)
				{
					for (unsigned int i = 0; i < it->second->getBackwardEdges().size(); i++)
					{
						Node * back = it->second->getBackwardEdges()[i]->getConnectedTo();

						Cellinst * back_cell = back->getCellInst();

						if (back_cell != NULL)
						{
							temp.level = back_cell->getLevel() + 1 > temp.level ? back_cell->getLevel() + 1 : temp.level;
						}
						else
						{
							temp.level = 1 > temp.level ? 1 : temp.level;
						}

					}
				}
				else
				{
					temp.level = it->second->getCellInst()->getLevel() + 1;
				}

				terminal_nodes.insert( temp );

				it->second->setUsedAsTerminal( true );
			}
		}
	}
	else if (pt == REG)
	{
		/* Loop over internal nodes */
		for (it = int_nodes.begin(); it != int_nodes.end(); it++)
		{
			/* Account for D-pins of FFs */
			if (it->second->getForwardEdges().empty() && it->second->getPin()->getType() == INPUT && it->second->getCellInst()->isSequential())
			{
				T_node temp;
				temp.terminal = it->second;

				for (unsigned int i = 0; i < it->second->getBackwardEdges().size(); i++)
				{
					Node * back = it->second->getBackwardEdges()[i]->getConnectedTo();

					Cellinst * back_cell = back->getCellInst();

					if (back_cell != NULL)
					{
						temp.level = back_cell->getLevel() + 1 > temp.level ? back_cell->getLevel() + 1 : temp.level;
					}
					else
					{
						temp.level = 1 > temp.level ? 1 : temp.level;
					}

				}

				terminal_nodes.insert( temp );

				it->second->setUsedAsTerminal( true );
			}
		}
	}

}

void Graph::reIninitializeIncremental()
{
	critical_path.reset();
}

void Graph::findCriticalPath()
{

	Node * max_fall;
	Node * max_rise;

	/* if there exist terminal nodes { POs and/or ff D-pins }*/
	if (terminal_nodes.size() > 0)
	{
		/* Assume that the first terminal node has the maximum fall/rise AT */

		max_fall = (*(terminal_nodes.begin())).terminal;
		max_rise = (*(terminal_nodes.begin())).terminal;

		multiset<T_node>::iterator max_f_it = terminal_nodes.begin();
		multiset<T_node>::iterator max_r_it = terminal_nodes.begin();

		multiset<T_node>::iterator it;

		/* For each terminal node do */
		for (it = terminal_nodes.begin(); it != terminal_nodes.end(); it++)
		{
			/* Find the maximum fall/rise AT between all ATs{fall/rise} @terminal_nodes */
			if ((*it).terminal->getAtLateFall() > max_fall->getAtLateFall())
			{
				max_fall = (*it).terminal;
				max_f_it = it;
			}
			if ((*it).terminal->getAtLateRise() > max_rise->getAtLateRise())
			{
				max_rise = (*it).terminal;
				max_r_it = it;
			}
		}

		Node * node;

		/* total delay of critical path */
		double delay;
		SLOPE state;

		/* Find the max between the fall and the rise ATs from all terminal nodes
		 * and follow the backwards direction from that node */
		if (max_fall->getAtLateFall() > max_rise->getAtLateRise())
		{
			node = max_fall;
			delay = max_fall->getAtLateFall();
			state = FALL;
			critical_path_node = max_f_it;
		}
		else
		{
			node = max_rise;
			delay = max_rise->getAtLateRise();
			state = RISE;
			critical_path_node = max_r_it;
		}

		if (delay == -numeric_limits<double>::max())
		{
			terminal_nodes.erase( critical_path_node );
			return;
		}

		/* At this point we know the maximum AT among all{rise/fall} ATs @POs */

#if TESTING
		if (!first_path_found)
		{
			cout << "\nCritical Path with delay = " << delay << "\n";
		}
#endif

#if DEBUG
		cout << "\nCritical Path with delay = " << delay << "\n";
#endif

		critical_path.setDelay( delay );
		critical_path.addNode( node );
		critical_path.addSlope( state );

#if TESTING
		if (!first_path_found)
		{
			if (state == RISE)
			{
				cout << node->getName() << "  [" << delay << ", RISE]\n";
			}
			else
			{
				cout << node->getName() << "  [" << delay << ", FALL]\n";
			}
		}
#endif

#if DEBUG
		if (state == RISE)
		{
			cout << node->getName() << "  [" << delay << ", RISE]\n";
		}
		else
		{
			cout << node->getName() << "  [" << delay << ", FALL]\n";
		}
#endif

		/* do as long as there is a previous node */
		do
		{
			if (state == FALL)
			{
				/* if there is a NEGATIVE unate edge */
				if (node->getInvertedFall())
				{
					state = RISE;
				}

				node = node->getPreviousFall();
			}
			else  // RISE
			{
				/* if there is a NEGATIVE unate edge */
				if (node->getInvertedRise())
				{
					state = FALL;
				}

				node = node->getPreviousRise();
			}

			if (node != NULL)
			{
				/* Collect nodes & slopes across path */
				critical_path.addNode( node );
				critical_path.addSlope( state );

#if TESTING
				if (!first_path_found)
				{
					if (state == RISE)
					{
						delay = node->getAtLateRise();
						cout << node->getName() << "  [" << delay << ", RISE]\n";
					}
					else
					{
						delay = node->getAtLateFall();
						cout << node->getName() << "  [" << delay << ", FALL]\n";
					}
				}
#endif

#if DEBUG
				if (state == RISE)
				{
					delay = node->getAtLateRise();
					cout << node->getName() << "  [" << delay << ", RISE]\n";
				}
				else
				{
					delay = node->getAtLateFall();
					cout << node->getName() << "  [" << delay << ", FALL]\n";
				}
#endif
			}

		} while (node != NULL);
	}

#if TESTING
	first_path_found = true;
#endif

#if RESIZE_FIRST_CELL_ON_PATH
	path_counter++;
#endif

}

typedef struct dfs
{
	unsigned int route;
	double at_late_rise;
	double at_late_fall;
} dfs;

/* for timing report */
/* Be careful ---> Exponential complexity [ALL PATHS] */
void Graph::findCriticalPaths(Node * start, multiset<Path>& critical_paths)
{

	vector<dfs> S;

	/* collect all Nodes accross the path */
	vector<Node *> path;

	vector<SLOPE> rise;
	vector<SLOPE> fall;

	dfs initial_path;

	/* Initialize dfs */
	initial_path.route = 0;
	initial_path.at_late_fall = start->getAtLateFall();  // AT_{late}{fall} @ start node (reminder: It can be a PI or a virtual node)
	initial_path.at_late_rise = start->getAtLateRise();  // AT_{late}{rise} @ start node (reminder: It can be a PI or a virtual node)

	path.push_back( start );  // begin the path from the start node
	S.push_back( initial_path );

	rise.push_back( RISE );
	fall.push_back( FALL );

	/* Run Depth First Search and Discover all paths */
	while (!S.empty())
	{
		dfs temp2 = S.back();

		Node * current = path.back();

		if (temp2.route < current->getForwardEdges().size())  // There are more routes from this node, not yet explored
		{
			Edge *ptr = current->getForwardEdges()[temp2.route];

			if (!ptr->isBlocked())  // this path is not blocked
			{

				dfs temp3 = temp2;

				Node * next = ptr->getConnectedTo();

				temp3.route = 0;  // always start exploring far left child

				if (ptr->getNetType() == CELL_EDGE)
				{
					if ((( CellEdge * ) ptr)->getFallTimingSense() == NEGATIVE)
					{
						temp3.at_late_fall = temp2.at_late_rise + ptr->getDelayFall();
						if (fall.back() == FALL)
						{
							fall.push_back( RISE );
						}
						else
						{
							fall.push_back( FALL );
						}
					}
					else
					{
						temp3.at_late_fall = temp2.at_late_fall + ptr->getDelayFall();
						fall.push_back( fall.back() );
					}

					if ((( CellEdge * ) ptr)->getRiseTimingSense() == NEGATIVE)
					{
						temp3.at_late_rise = temp2.at_late_fall + ptr->getDelayRise();
						if (rise.back() == FALL)
						{
							rise.push_back( RISE );
						}
						else
						{
							rise.push_back( FALL );
						}
					}
					else
					{
						temp3.at_late_rise = temp2.at_late_rise + ptr->getDelayRise();
						rise.push_back( rise.back() );
					}
				}
				else
				{
					temp3.at_late_fall = temp2.at_late_fall + ptr->getDelayFall();
					temp3.at_late_rise = temp2.at_late_rise + ptr->getDelayRise();
					fall.push_back( fall.back() );
					rise.push_back( rise.back() );
				}

				S.push_back( temp3 );
				path.push_back( next );
			}
			else
			{
				S.back().route++;
			}
		}
		else if (current->getForwardEdges().empty()
				&& (pt == ALL || (pt == OUT && current->getPin() == NULL && current != start)
						|| (pt == REG && current->getCellInst() != NULL && current->getCellInst()->isSequential()
								&& current->getPin()->getType() == INPUT)))  // End of path
		{

			Path temp4;

			if (temp2.at_late_fall > temp2.at_late_rise)
			{
				temp4.setDelay( temp2.at_late_fall );

				if (fall.back() == FALL)
				{
					temp4.setSlopes( fall );
				}
				else
				{
					temp4.setSlopes( rise );
				}

			}
			else
			{
				temp4.setDelay( temp2.at_late_rise );

				if (fall.back() == RISE)
				{
					temp4.setSlopes( fall );
				}
				else
				{
					temp4.setSlopes( rise );
				}
			}

			temp4.setNodes( path );

			/*Path temp4;
			 Path temp5;

			 temp4.setDelay( temp2.at_late_fall );
			 temp5.setDelay( temp2.at_late_rise );

			 vector<Node *> temp_nodes = path;
			 vector<SLOPE> temp_rise;
			 vector<SLOPE> temp_fall;

			 if (fall.back() == FALL)
			 {
			 temp_fall = fall;
			 temp_rise = rise;
			 reverse( temp_rise.begin(), temp_rise.end() );
			 reverse( temp_fall.begin(), temp_fall.end() );
			 temp4.setSlopes( temp_fall );
			 temp5.setSlopes( temp_rise );
			 }
			 else
			 {
			 temp_fall = rise;
			 temp_rise = fall;
			 temp4.setSlopes( temp_rise );
			 temp5.setSlopes( temp_fall );
			 reverse( temp_rise.begin(), temp_rise.end() );
			 reverse( temp_fall.begin(), temp_fall.end() );
			 temp4.setSlopes( temp_fall );
			 temp5.setSlopes( temp_rise );
			 }

			 reverse( temp_nodes.begin(), temp_nodes.end() );

			 temp4.setNodes( temp_nodes );
			 temp5.setNodes( temp_nodes );*/

			/*if (fall.back() == FALL)
			 {
			 temp4.setSlopes( fall );
			 temp5.setSlopes( rise );
			 }
			 else
			 {
			 temp4.setSlopes( rise );
			 temp5.setSlopes( fall );
			 }

			 temp4.setNodes( path );
			 temp5.setNodes( path );*/

			if (num_paths != ALL_PATHS && critical_paths.size() >= num_paths)
			{
				if (temp4.getDelay() > (*(critical_paths.rbegin())).getDelay())
				{
					critical_paths.insert( temp4 );  // add this path to the setup set
					critical_paths.erase( *(critical_paths.rbegin()) );  // remove the less critical
				}
			}
			else
			{
				critical_paths.insert( temp4 );  // add this path to the setup set
			}

			/*if (num_paths != ALL_PATHS && critical_paths.size() >= num_paths)
			 {
			 if (temp5.getDelay() > (*(critical_paths.rbegin())).getDelay())
			 {
			 critical_paths.insert( temp5 );  // add this path to the setup set
			 critical_paths.erase( *(critical_paths.rbegin()) );  // remove the less critical
			 }
			 }
			 else
			 {
			 critical_paths.insert( temp5 );  // add this path to the setup set
			 }*/

			path.pop_back();
			rise.pop_back();
			fall.pop_back();
			S.pop_back();

			if (!S.empty())
			{
				S.back().route++;
			}
		}
		else  //This node is completely explored
		{
			path.pop_back();
			rise.pop_back();
			fall.pop_back();
			S.pop_back();

			if (!S.empty())
			{
				S.back().route++;
			}
		}

	}

}

void Graph::pathTokenizer(Path path, vector<Path>& sub_paths)
{
	vector<Node *> temp_nodes;
	vector<SLOPE> temp_slopes;

	/* Loop over the nodes that constitute the path */
	for (unsigned int i = 0; i < path.getPath().size(); i++)
	{
		/* Consider every pin that belongs to a Cellinst that is NOT MARKED AS EXAMINED */
		if (path.getPath()[i]->getCellInst() != NULL && path.getPath()[i]->getCellInst()->isExamined() == false)
		{
			temp_nodes.push_back( path.getPath()[i] );
			temp_slopes.push_back( path.getSlopesConst()[i] );

			/* Reach the end of path */
			if (i == path.getPath().size() - 1)
			{
				/* If there exist a subpath */
				if (temp_nodes.size() > 0)
				{
					/* Add temp path to sub_paths */
					Path temp;

					temp.setNodes( temp_nodes );
					temp.setSlopes( temp_slopes );

					sub_paths.push_back( temp );

					temp_nodes.clear();
					temp_slopes.clear();
				}
			}
		}

		/* Consider a node that is a PI(NULL) or belongs to a Cellinst that IS MARKED AS EXAMINED */
		else if (path.getPath()[i]->getCellInst() == NULL
				|| (path.getPath()[i]->getCellInst() != NULL && path.getPath()[i]->getCellInst()->isExamined() == true))  // 
		{
			/* the subpath ends, thus we have to add it into the subpaths vector */

			/* If there exist a subpath */
			if (temp_nodes.size() > 0)
			{
				/* Consider the examined cell */
				if (path.getPath()[i]->getCellInst() != NULL && path.getPath()[i]->getCellInst()->isExamined() == true)
				{
					/* A checking done for debugging reasons */
					if (((i + 1) < path.getPath().size()) && path.getPath()[i]->getPin()->getType() == OUTPUT
							&& path.getPath()[i + 1]->getPin() != NULL && path.getPath()[i + 1]->getPin()->getType() == INPUT)
					{
						temp_nodes.push_back( path.getPath()[i] );
						temp_slopes.push_back( path.getSlopesConst()[i] );  //output
						temp_nodes.push_back( path.getPath()[i + 1] );
						temp_slopes.push_back( path.getSlopesConst()[i + 1] );  //input
					}
				}

				Path temp;

				temp.setNodes( temp_nodes );
				temp.setSlopes( temp_slopes );

				/* Add temp path to sub_paths */
				sub_paths.push_back( temp );

				/* reset temp path */
				temp_nodes.clear();
				temp_slopes.clear();

			}
		}
	}

}

void Graph::findSubPaths(cell_map & cells, vector<Parasitic *>& affected_nets, vector<Cellinst *>& changed_cells, double tau, int max_iterations,
		double tolerance)
{
	vector<Path> sub_paths;

	set<Parasitic *> changed_nets;

	/* Tokenize the critical path into subpaths */
	pathTokenizer( critical_path, sub_paths );

	/* Loop over the subpaths */
	for (unsigned int i = 0; i < sub_paths.size(); i++)
	{
#if DEBUG
		cout << "\n\nSubpath with output capacitance = "
		<< Util::calculateOutputCapacitance( sub_paths[i].getPath()[0], sub_paths[i].getSlopesConst()[0] ) << "\n";
		for (unsigned j = 0; j < sub_paths[i].getPath().size(); j++)
		{
			cout << sub_paths[i].getPath()[j]->getName() << "\n";
		}
#endif

#if RESIZE_FIRST_CELL_ON_PATH

		//path_counter++;
		if (sub_paths[i].getPath().size() > 2)// At least one cell
		{

#if DEBUG
			cout << "Subpath # " << path_counter << "\n";
#endif
			Util::resize_first_cell( path_counter, sub_paths[i], cells, cells_examined, cells_resized, cells_potentially_resized, changed_cells,
					changed_nets );
		}

#endif

		/* Apply unified logical effort on subpath i */
		bool res = sub_paths[i].unifiedLogicalEffort( cells, changed_nets, changed_cells, tau, upsizing, cells_examined, cells_resized,
				cells_potentially_resized, max_iterations, tolerance );

		/* if there are up-sized cell instances */
		if (res == false)  // the current sub path was not resized
		{
			pathTokenizer( sub_paths[i], sub_paths );  // tokenize the sub path even more
		}
	}

	set<Parasitic *>::iterator it;
	for (it = changed_nets.begin(); it != changed_nets.end(); it++)
	{
		affected_nets.push_back( *it );
	}

	Util::DrawProgressBar( 30, (( double ) cells_examined) / cells_no_design );

}

