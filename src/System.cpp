/*
 * System.cpp
 *
 *  Created on: May 12, 2014
 *      Author: hrkalona
 */
#include "System.hpp"

#include <cstdlib>
#include <string.h>
#include <fstream>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/regex.hpp>
#include <iostream>
#include "CellEdge.hpp"
#include "NetEdge.hpp"
#include "Util.hpp"
#include <vector>

using namespace boost;

System::System()
{
	resize_sequential = NO_ACTION;
	cell_no_design = 0;
	cells_examined = 0;
	cells_resized = 0;
	parasitics_found = 0;
	combinational_cells = 0;
	sequential_cells = 0;
	cells_potentially_resized = 0;
	lib_cap_units = UNIT_INVALID_VAL;
	lib_time_units = UNIT_INVALID_VAL;
	max_iterations = 0;
	tolerance = 0;
}

System::~System()
{

	cell_map::iterator r;
	for (r = cells.begin(); r != cells.end(); ++r)
	{
		delete r->second;
	}

	cellinst_map::iterator o;
	for (o = instantiated_cells.begin(); o != instantiated_cells.end(); ++o)
	{
		delete o->second;
	}

	net_map::iterator m;
	for (m = wires.begin(); m != wires.end(); ++m)
	{
		delete m->second;
	}
}

void System::inputHandler(int argc, char *argv[])
{
	if (argc < 6)
	{
		cout << "Invalid argument count.\n";
		cout
				<< "./CCSopt <lib_file.lib> <main module> <output path> <config_file.cfg> [parasitics_file.spef] <verilog_file-1.v> [verilog_file-2.v ... verilog_file-n.v]"
				<< "\n";
		cout << "Terminating.\n";
		exit( -1 );
	}

	lib_file = argv[1];

	string lib = lib_file;

	unsigned found = lib.find_last_of( "." );

	if (found > lib.size() || lib.substr( found + 1 ).compare( "lib" ) != 0)
	{
		cout << "Invalid file format.\n";
		cout << "Requested .lib file.\n";
		cout << "Terminating.\n";
		exit( -1 );
	}

	main_module.setName( argv[2] );

	output_path = argv[3];

	cfg_file = argv[4];

	string cfg = cfg_file;

	found = cfg.find_last_of( "." );

	if (found > cfg.size() || cfg.substr( found + 1 ).compare( "cfg" ) != 0)
	{
		cout << "Invalid file format.\n";
		cout << "Requested .cfg file.\n";
		cout << "Terminating.\n";
		exit( -1 );
	}

	string spef = argv[5];

	found = spef.find_last_of( "." );

	if (found > spef.size() || (spef.substr( found + 1 ).compare( "spef" ) != 0 && spef.substr( found + 1 ).compare( "v" ) != 0))
	{
		cout << "Invalid file format.\n";
		cout << "Requested .spef or .v file.\n";
		cout << "Terminating.\n";
		exit( -1 );
	}

	unsigned int i;

	if (spef.substr( found + 1 ).compare( "v" ) == 0)
	{
		i = 5;
	}
	else if (spef.substr( found + 1 ).compare( "spef" ) == 0)
	{
		spef_file = spef;
		i = 6;
	}

	for (; i < argc; i++)
	{
		string last = argv[i];
		found = last.find_last_of( "." );

		if ((found > last.size() || last.substr( found + 1 ).compare( "v" ) != 0))
		{
			cout << "Invalid file format.\n";
			cout << "Requested .v file.\n";
			cout << "Terminating.\n";
			exit( -1 );
		}

		if (last.substr( found + 1 ).compare( "v" ) == 0)
		{
			verilog_files.push_back( argv[i] );
		}
	}

	parseFiles();
}

void System::parseFiles()
{
	dotLibParser();
	dotVParser();
	dotCfgParser();

	cell_map::iterator q;
	for (q = cells.begin(); q != cells.end(); q++)
	{
		q->second->errorCheck( cell_name_delims );
	}

	dotSpefParser();

	cout << "\nParsed " << cells.size() << " cells from the " << lib_file << " file.\n";
	cout << "Parsed " << instantiated_cells.size() << " instanced cells from the input verilog files.\n";
	cout << "Parsed " << wires.size() << " nets from the input verilog files.\n";
	if (!spef_file.empty())
	{
		cout << "Parsed " << parasitics_found << " parasitics from the " << spef_file << " file.\n";
	}
	cout << "\nPrimary Inputs: " << graph.getInputNodes().size() << "\n";
	cout << "Primary Outputs: " << graph.getOutputNodes().size() << "\n";

}

void System::engine()
{

	calcUnitInverterVals();

	/* !!!Extract Logical Effort paramerters!!! */
	cell_map::iterator q;
	for (q = cells.begin(); q != cells.end(); q++)
	{
		q->second->logicalEffortExtraction( un_in.C0 );  // un_in.C0 is the capacitance of the unit inverter ( min-sized inverter )
	}

#if LIB_VERIFICATION
	libVerification();
#endif

	if (!graph.getReportTiming())
	{
		sequentialCellsHandling();  // handle sequential cells according to "resize_sequential" option from the configuration file

		cout << "Combinational cells: " << combinational_cells << "\n";
		cout << "Sequential cells: " << sequential_cells << "\n";

		cout << "\n\n";
	}

	/* Core Code ...Performs all the tasks. */
	graph.resizer( cells, instantiated_cells, parasitics, un_in.tau, cell_no_design, cells_examined, cells_resized, cells_potentially_resized,
			max_iterations, tolerance );
}

void System::sequentialCellsHandling()
{
	node_map::iterator it;

	/* ( The resize_sequential option from the configuration file ) */
	if (resize_sequential == NO_ACTION)
	{
		cellinst_map::iterator it;

		/* Loop over all Cell instances */
		for (it = instantiated_cells.begin(); it != instantiated_cells.end(); it++)
		{
			/* Is the current Cell instance a Flip-Flop (FF)? */
			if (it->second->isSequential())
			{
#if DEBUG
				cout << "Sequential Cell " << it->second->getCell()->getName() << " will not be resized.\n";
#endif
				it->second->setExamined( true );

				cells_examined++;

				sequential_cells++;
			}
			else
			{
				combinational_cells++;
			}
		}
	}
	else if (resize_sequential == MIN_SIZE)
	{
		cellinst_map::iterator it;

		/* Loop over all cell instances */
		for (it = instantiated_cells.begin(); it != instantiated_cells.end(); it++)
		{

			/* if the current cell instance is a FF */
			if (it->second->isSequential())
			{

				Cellinst * cell_to_be_resized = it->second;

				/* Find the FF that has the lower drive strength in the .lib file */

				cell_map::iterator it2;

				string prefix = cell_to_be_resized->getCell()->getPrefix();

				double min = numeric_limits<double>::max();
				Cell* replacement = NULL;

				for (it2 = cells.lower_bound( prefix ); it2 != cells.end() && it2->second->getPrefix().compare( prefix ) == 0; it2++)
				{

					if (it2->second->getDriveStrenth() < min)
					{
						min = it2->second->getDriveStrenth();
						replacement = it2->second;
					}
				}

#if DEBUG
				cout << "Sequential Cell " << cell_to_be_resized->getCell()->getName() << " will be resized to minimum size.\n";
#endif

				/* Replace the drive strength of the current FF with the minimum drive strength
				 * that we found previously */

				if (replacement != NULL && replacement != cell_to_be_resized->getCell() && cell_to_be_resized->isExamined() == false)
				{
					cell_to_be_resized->setScaleFactor( replacement->getDriveStrenth() / cell_to_be_resized->getCell()->getDriveStrenth() );
					cell_to_be_resized->substitute( replacement );
					cells_resized++;
					cells_potentially_resized++;
					cell_to_be_resized->setResized( true );
				}

				cell_to_be_resized->setExamined( true );

				cells_examined++;

				sequential_cells++;
			}
			else
			{
				combinational_cells++;
			}
		}
	}
}

Graph& System::getGraph()
{
	return graph;
}

net_map& System::getNets()
{
	return wires;
}

cellinst_map& System::getInstantiatedCells()
{
	return instantiated_cells;
}

cell_map& System::getCells()
{
	return cells;
}

Module& System::getMainModule()
{
	return main_module;
}

/* Lib File */

/* .lib file parser */
/* dotLibParser, getPinAttrs, getTimingAttrs, getCellAttrs, getGroups, libVerification
 * methods should be transfered into a new class */

void System::dotLibParser()
{

	cout << "\nParsing the liberty format file...\n\n";

	si2drErrorT err;
	si2drGroupsIdT groups;
	si2drGroupIdT group;

	InputPin *new_in_pin = NULL;
	OutputPin *new_out_pin = NULL;
	Cell *new_cell = NULL;
	Timing *new_timing = NULL;
	When *new_timing_when = NULL;

	si2drPIInit( &err );
	si2drReadLibertyFile( lib_file, &err );

	if (err != 0)
	{
		exit( -1 );
	}

	groups = si2drPIGetGroups( &err );
	group = si2drIterNextGroup( groups, &err );
	si2drIterQuit( groups, &err );

	getGroups( group, new_cell, new_in_pin, new_out_pin, new_timing, new_timing_when );

	cell_map::iterator q;
	for (q = cells.begin(); q != cells.end(); q++)
	{
		q->second->inputCapacitanceCheck();
	}

	if (Util::variable_1 == UNDEFINED || Util::variable_2 == UNDEFINED)
	{
		cout << "Look up table for timing was not defined, in the file " << lib_file << ".\n";
		cout << "Terminating.\n";
		exit( -1 );
	}

}

void System::getPinAttrs(si2drGroupIdT _gp, Cell * &new_cell, si2drStringT _name, InputPin *&new_in_pin, OutputPin *&new_out_pin)
{
	si2drErrorT err;
	si2drAttrsIdT ats = si2drGroupGetAttrs( _gp, &err );
	si2drAttrIdT at;
	si2drStringT g1;

	NODE_TYPE ntype = ERROR_CODE;
	double c_fall = 0;
	double c_rise = 0;
	double c = 0;
	bool clock = false;

	while (!si2drObjectIsNull( (at = si2drIterNextAttr( ats, &err )), &err ))
	{
		g1 = si2drAttrGetName( at, &err );
		if (strcmp( g1, "direction" ) == 0)
		{
			if (strcmp( si2drSimpleAttrGetStringValue( at, &err ), "input" ) == 0)
			{
				ntype = INPUT;
			}
			else if (strcmp( si2drSimpleAttrGetStringValue( at, &err ), "output" ) == 0)
			{
				ntype = OUTPUT;
			}
			else if (strcmp( si2drSimpleAttrGetStringValue( at, &err ), "internal" ) == 0)
			{
				ntype = UNUSED;
			}
		}
		else if (strcmp( g1, "fall_capacitance" ) == 0)
		{
			c_fall = si2drSimpleAttrGetFloat64Value( at, &err );
		}
		else if (strcmp( g1, "rise_capacitance" ) == 0)
		{
			c_rise = si2drSimpleAttrGetFloat64Value( at, &err );
		}
		else if (strcmp( g1, "capacitance" ) == 0)
		{
			c = si2drSimpleAttrGetFloat64Value( at, &err );
		}
		else if (strcmp( g1, "clock" ) == 0)
		{
			clock = si2drSimpleAttrGetBooleanValue( at, &err );
		}
	}

	if (ntype == ERROR_CODE)
	{
		cout << "Pin " << _name << " of the cell " << new_cell->getName() << " has no direction defined.\n";
		cout << "Terminating.\n";
		exit( -1 );
	}
	else if (ntype == INPUT)
	{
		new_in_pin = ( InputPin * ) new_cell->getInput( _name );
		new_in_pin->setCfall( c_fall );
		new_in_pin->setCrise( c_rise );
		new_in_pin->setC( c );
		new_in_pin->setClockPin( clock );
	}
	else if (ntype == OUTPUT)
	{
		new_out_pin = ( OutputPin * ) new_cell->getOutput( _name );
	}

	si2drIterQuit( ats, &err );
}

void System::getTimingAttrs(si2drGroupIdT _gp, OutputPin *&new_out_pin, Timing *&new_timing, When *&new_timing_when, Cell *&new_cell)
{
	si2drErrorT err;
	si2drAttrsIdT ats = si2drGroupGetAttrs( _gp, &err );
	si2drAttrIdT at;
	si2drStringT g1;

	UNATE temp_unate = NON;
	ARC_TYPE temp_arc = NO_TYPE;
	string when_string;
	string related_pin_string;

	while (!si2drObjectIsNull( (at = si2drIterNextAttr( ats, &err )), &err ))
	{
		g1 = si2drAttrGetName( at, &err );
		if (strcmp( g1, "related_pin" ) == 0)
		{
			related_pin_string = si2drSimpleAttrGetStringValue( at, &err );
		}
		else if (strcmp( g1, "when" ) == 0)
		{
			when_string = si2drSimpleAttrGetStringValue( at, &err );
		}
		else if (strcmp( g1, "timing_sense" ) == 0)
		{
			if (strcmp( si2drSimpleAttrGetStringValue( at, &err ), "positive_unate" ) == 0)
			{
				temp_unate = POSITIVE;
			}
			else if (strcmp( si2drSimpleAttrGetStringValue( at, &err ), "negative_unate" ) == 0)
			{
				temp_unate = NEGATIVE;
			}
			else if (strcmp( si2drSimpleAttrGetStringValue( at, &err ), "non_unate" ) == 0)
			{
				temp_unate = NON;
			}
		}
		else if (strcmp( g1, "timing_type" ) == 0)
		{
			if (strcmp( si2drSimpleAttrGetStringValue( at, &err ), "clear" ) == 0)
			{
				temp_arc = CLEAR;
			}
			else if (strcmp( si2drSimpleAttrGetStringValue( at, &err ), "preset" ) == 0)
			{
				temp_arc = PRESET;
			}
			else if (strcmp( si2drSimpleAttrGetStringValue( at, &err ), "rising_edge" ) == 0)
			{
				temp_arc = RISING_EDGE;
			}
			else if (strcmp( si2drSimpleAttrGetStringValue( at, &err ), "falling_edge" ) == 0)
			{
				temp_arc = FALLING_EDGE;
			}
			else if (strcmp( si2drSimpleAttrGetStringValue( at, &err ), "combinational" ) == 0)
			{
				temp_arc = COMBINATIONAL;
			}

		}
	}

	if (related_pin_string.empty())
	{
		cout << "Pin " << new_out_pin->getName() << " of the cell " << new_cell->getName() << " has no related pin in its timing.\n";
		cout << "Terminating.\n";
		exit( -1 );
	}
	else
	{
		new_timing = new_out_pin->addTiming( new_cell->getInput( related_pin_string ) );

		new_timing->addTimingWhen( new_timing_when );
		new_timing_when->setWhen( when_string );
		new_timing_when->setTimingSense( temp_unate );
		new_timing_when->setTimingType( temp_arc );
	}

	si2drIterQuit( ats, &err );

}

void System::getCellAttrs(si2drGroupIdT _gp, Cell *&new_cell)
{
	si2drErrorT err;
	si2drAttrsIdT ats = si2drGroupGetAttrs( _gp, &err );
	si2drAttrIdT at;
	si2drStringT g1;

	while (!si2drObjectIsNull( (at = si2drIterNextAttr( ats, &err )), &err ))
	{
		g1 = si2drAttrGetName( at, &err );
		if (strcmp( g1, "drive_strength" ) == 0 && new_cell != NULL)
		{
			if (si2drSimpleAttrGetValueType( at, &err ) == SI2DR_INT32)
			{
				new_cell->setDriveStrength( si2drSimpleAttrGetInt32Value( at, &err ) );
			}
			else if (si2drSimpleAttrGetValueType( at, &err ) == SI2DR_FLOAT64)
			{
				new_cell->setDriveStrength( si2drSimpleAttrGetFloat64Value( at, &err ) );
			}

		}
	}
	si2drIterQuit( ats, &err );

}

void System::getLutAttrs(si2drGroupIdT _gp, si2drStringT _name)
{
	si2drErrorT err;
	si2drAttrsIdT ats = si2drGroupGetAttrs( _gp, &err );
	si2drAttrIdT at;
	si2drStringT g1;

	INTERP_VARIABLE variable_1 = UNDEFINED, variable_2 = UNDEFINED;

	while (!si2drObjectIsNull( (at = si2drIterNextAttr( ats, &err )), &err ))
	{
		g1 = si2drAttrGetName( at, &err );
		if (strcmp( g1, "variable_1" ) == 0)
		{
			if (strcmp( si2drSimpleAttrGetStringValue( at, &err ), "input_net_transition" ) == 0)
			{
				variable_1 = INPUT_NET_TRANSITION;
			}
			else if (strcmp( si2drSimpleAttrGetStringValue( at, &err ), "total_output_net_capacitance" ) == 0)
			{
				variable_1 = TOTAL_OUTPUT_NET_CAPACITANCE;
			}
		}
		else if (strcmp( g1, "variable_2" ) == 0)
		{
			if (strcmp( si2drSimpleAttrGetStringValue( at, &err ), "input_net_transition" ) == 0)
			{
				variable_2 = INPUT_NET_TRANSITION;
			}
			else if (strcmp( si2drSimpleAttrGetStringValue( at, &err ), "total_output_net_capacitance" ) == 0)
			{
				variable_2 = TOTAL_OUTPUT_NET_CAPACITANCE;
			}
		}
	}

	if (variable_1 != UNDEFINED && variable_2 != UNDEFINED)
	{
		if (!timing_table_name.empty())
		{
			cout << "Found more than one look up tables for timing, in the file " << lib_file << ".\n";
			cout << "Terminating.\n";
			exit( -1 );
		}
		timing_table_name = _name;
		Util::variable_1 = variable_1;
		Util::variable_2 = variable_2;

	}

	si2drIterQuit( ats, &err );
}

void System::getGroups(si2drGroupIdT _gp, Cell * &new_cell, InputPin *&new_in_pin, OutputPin *&new_out_pin, Timing *&new_timing,
		When *&new_timing_when)
{
	si2drErrorT err;
	si2drGroupsIdT gps = si2drGroupGetGroups( _gp, &err );
	si2drGroupIdT gp;

	si2drNamesIdT ns;
	si2drStringT s1;
	si2drStringT g1;
	si2drStringT g2;

	while (!si2drObjectIsNull( (gp = si2drIterNextGroup( gps, &err )), &err ))
	{

		g1 = si2drGroupGetGroupType( gp, &err );    //cell type
		ns = si2drGroupGetNames( gp, &err );    //cell parameters

		if (strcmp( g1, "lu_table_template" ) == 0)
		{
			getLutAttrs( gp, si2drIterNextName( ns, &err ) );
			getGroups( gp, new_cell, new_in_pin, new_out_pin, new_timing, new_timing_when );
		}
		else if (strcmp( g1, "cell" ) == 0)
		{
			new_cell = new Cell( si2drIterNextName( ns, &err ) );
			cells.insert( cell_entry( new_cell->getName(), new_cell ) );
			getCellAttrs( gp, new_cell );
			getGroups( gp, new_cell, new_in_pin, new_out_pin, new_timing, new_timing_when );
			new_cell = NULL;
		}
		else if (strcmp( g1, "pin" ) == 0)
		{
			s1 = si2drIterNextName( ns, &err );
			getPinAttrs( gp, new_cell, s1, new_in_pin, new_out_pin );
			getGroups( gp, new_cell, new_in_pin, new_out_pin, new_timing, new_timing_when );
			new_out_pin = NULL;
			new_in_pin = NULL;

		}
		else if (strcmp( g1, "timing" ) == 0 && new_out_pin != NULL)
		{
			new_timing_when = new When();
			getTimingAttrs( gp, new_out_pin, new_timing, new_timing_when, new_cell );
			getGroups( gp, new_cell, new_in_pin, new_out_pin, new_timing, new_timing_when );
			new_timing = NULL;
			new_timing_when = NULL;
		}
		else if (strcmp( g1, "cell_fall" ) == 0 && new_timing_when != NULL)
		{
			if (timing_table_name.empty())
			{
				cout << "Look up table for timing was not defined, in the file " << lib_file << ".\n";
				cout << "Terminating.\n";
				exit( -1 );
			}

			tables * temp = liberty_get_values_data( gp );

			g2 = si2drIterNextName( ns, &err );

			if ((timing_table_name.compare( g2 ) == 0 && temp->dimensions == 2) || (strcmp( g2, "scalar" ) == 0 && temp->dimensions == 1))
			{
				new_timing_when->setCellFall( temp );
			}

			getGroups( gp, new_cell, new_in_pin, new_out_pin, new_timing, new_timing_when );
		}
		else if (strcmp( g1, "cell_rise" ) == 0 && new_timing_when != NULL)
		{
			if (timing_table_name.empty())
			{
				cout << "Look up table for timing was not defined, in the file " << lib_file << ".\n";
				cout << "Terminating.\n";
				exit( -1 );
			}

			tables * temp = liberty_get_values_data( gp );

			g2 = si2drIterNextName( ns, &err );

			if ((timing_table_name.compare( g2 ) == 0 && temp->dimensions == 2) || (strcmp( g2, "scalar" ) == 0 && temp->dimensions == 1))
			{
				new_timing_when->setCellRise( temp );
			}

			getGroups( gp, new_cell, new_in_pin, new_out_pin, new_timing, new_timing_when );
		}
		else if (strcmp( g1, "fall_transition" ) == 0 && new_timing_when != NULL)
		{
			if (timing_table_name.empty())
			{
				cout << "Look up table for timing was not defined, in the file " << lib_file << ".\n";
				cout << "Terminating.\n";
				exit( -1 );
			}

			tables * temp = liberty_get_values_data( gp );

			g2 = si2drIterNextName( ns, &err );

			if ((timing_table_name.compare( g2 ) == 0 && temp->dimensions == 2) || (strcmp( g2, "scalar" ) == 0 && temp->dimensions == 1))
			{
				new_timing_when->setFallTransition( temp );
			}

			getGroups( gp, new_cell, new_in_pin, new_out_pin, new_timing, new_timing_when );
		}
		else if (strcmp( g1, "rise_transition" ) == 0 && new_timing_when != NULL)
		{
			if (timing_table_name.empty())
			{
				cout << "Look up table for timing was not defined, in the file " << lib_file << ".\n";
				cout << "Terminating.\n";
				exit( -1 );
			}

			tables * temp = liberty_get_values_data( gp );

			g2 = si2drIterNextName( ns, &err );

			if ((timing_table_name.compare( g2 ) == 0 && temp->dimensions == 2) || (strcmp( g2, "scalar" ) == 0 && temp->dimensions == 1))
			{
				new_timing_when->setRiseTransition( temp );
			}

			getGroups( gp, new_cell, new_in_pin, new_out_pin, new_timing, new_timing_when );
		}
		else
		{
			getGroups( gp, new_cell, new_in_pin, new_out_pin, new_timing, new_timing_when );
		}
		si2drIterQuit( ns, &err );

	}
	si2drIterQuit( gps, &err );

}

void System::libVerification()
{
	ofstream output_file;

	/* Open output file */
	output_file.open( "out.my_lib" );

	cell_map::iterator cell_iter;

	/* for each cell do */
	for (cell_iter = cells.begin(); cell_iter != cells.end(); ++cell_iter)
	{
		output_file << "cell (" << cell_iter->first << ") {" << endl << endl;

		/* for each output pin do */
		for (unsigned int i = 0; i < cell_iter->second->getInputs().size(); ++i)
		{
			InputPin *in_pin = ( InputPin * ) cell_iter->second->getInputs()[i];
			output_file << "\tpin (" << in_pin->getName() << ") {" << endl << endl;

			/* for each input pin */
			output_file << "\t\tg fall: " << in_pin->getGfall() << endl;
			output_file << "\t\tg rise: " << in_pin->getGrise() << endl << endl;

			output_file << "\t}" << endl << endl;
		}

		output_file << "}" << endl << endl;
	}
}

/* End of Lib File */

/* Verilog File Parser */
/* V File */

/* Again Verilog file parser should be moved to a new class ( maybe in the same with .lib parser ) */

void System::dotVParser()
{
	cout << "\nParsing the verilog format file...\n\n";

	ifstream file;
	string buf;

	string working_file;

	unsigned int line_cnt = 1;

	module_map modules;

	set<string> instances;

	string main_module_file;

	bool end_module_found = true;

	/* Collect modules from all the verilog files */
	for (unsigned int i = 0; i < verilog_files.size(); i++)
	{
		working_file = verilog_files[i];

		line_cnt = 1;

		file.open( working_file.c_str() );

		if (!file.good())
		{
			cout << "The file " << working_file << " was not found.\n";
			cout << "Terminating.\n";
			exit( -1 );
		}

		end_module_found = true;

		/* Parse the whole file to gather all the modules, except the main module */
		while (true)
		{
			getline( file, buf );

			if (file.eof())
			{
				if (end_module_found)
				{
					break;
				}
				else
				{
					cout << "Missing endmodule declaration at line " << line_cnt << " of the file " << working_file << "\n";
					cout << "Terminating.\n";
					exit( -1 );
				}
			}

			typedef tokenizer<char_separator<char> > tokenizer;

			char_separator<char> sep( " \t;" );
			tokenizer tokens( buf, sep );

			tokenizer::iterator tok_iter = tokens.begin();

			if (tok_iter != tokens.end())
			{
				if ((*tok_iter).compare( "module" ) == 0)  // we found a module
				{
					vector<string> parsed_module;

					vector<string> temp;

					string name;

					parse_module( buf, file, line_cnt, working_file, name, parsed_module, false, temp );  // collect the module arguments

					if (name.empty())
					{
						cout << "Required module name at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					if (parsed_module.size() < 1)
					{
						cout << "Required module arguments at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					int temp_line;

					if (name.compare( main_module.getName() ) != 0)  // its not the main module
					{

						end_module_found = false;

						temp_line = line_cnt;

						Module * temp_module = new Module();

						temp_module->setName( name );

						temp_module->setFileName( working_file );

						for (unsigned int p = 0; p < parsed_module.size(); p++)
						{
							temp_module->addArgument( parsed_module[p] );
						}

						temp_module->getPortType().resize( temp_module->getArguments().size() );
						temp_module->getWireType().resize( temp_module->getArguments().size() );

						line_cnt++;

						while (true)  //collect the module's code
						{
							getline( file, buf );

							if (file.eof())
								break;

							tokenizer tokens2( buf, sep );

							tokenizer::iterator tok_iter2 = tokens2.begin();

							if (tok_iter2 != tokens2.end())
							{
								if (starts_with( *tok_iter2, "//" ))
								{
									// comments
								}
								else if (starts_with( *tok_iter2, "/*" ))
								{
									parse_multiline_comment( buf, file, line_cnt, working_file );
								}
								else if ((*tok_iter2).compare( "module" ) == 0)  // found a module inside a module error check
								{
									cout << "Module declaration found at line " << line_cnt << " of the file " << working_file
											<< " but the previous endmodule declaration is missing.\n";
									cout << "Terminating.\n";
									exit( -1 );
								}
								else if ((*tok_iter2).compare( "endmodule" ) == 0)  // endmodule check
								{
									tok_iter2++;

									if (tok_iter2 != tokens2.end())
									{
										if (!starts_with( *tok_iter2, "//" ))
										{
											cout
													<< "The verilog parser cannot continue parsing the same line, after reaching endmodule declaration at line "
													<< line_cnt << " of the file " << working_file << ".\n";
											cout << "Terminating.\n";
											exit( -1 );
										}
									}

									end_module_found = true;
									break;
								}
								else if (starts_with( *tok_iter2, "endmodule//" ))  // endmodule check
								{
									end_module_found = true;
									break;
								}
								else  // code, we dont check for errors
								{
									temp_module->addCode( buf );
									temp_module->addLine( line_cnt );
								}
							}

							line_cnt++;

						}

						pair<module_map::iterator, bool> ret;
						ret = modules.insert( module_entry( temp_module->getName(), temp_module ) );

						if (ret.second == false)  // this module was parsed before
						{
							cout << "Multiple declaration of the module " << temp_module->getName() << " found at line " << temp_line
									<< " of the file " << working_file << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
					}
					else  //we found the main module
					{
						if (main_module_file.compare( "" ) != 0)
						{
							cout << "Multiple declaration of main module " << main_module.getName() << " found at line " << line_cnt
									<< " of the file " << working_file << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
						main_module_file = working_file;  //keep track of the main module file name
					}
				}
			}

			line_cnt++;

		}

		file.close();
	}

	if (main_module_file.empty())  //we didnt find the main module at all
	{
		cout << "Main module " << main_module.getName() << " not found while parsing the files.\n";
		cout << "Terminating.\n";
		exit( -1 );
	}

	/* Open the main module file */

	working_file = main_module_file;
	file.open( main_module_file.c_str() );

	line_cnt = 1;

	end_module_found = true;

	while (true)
	{
		getline( file, buf );

		if (file.eof())
		{
			if (end_module_found)
			{
				break;
			}
			else
			{
				cout << "Missing endmodule declaration at line " << line_cnt << " of the file " << working_file << "\n";
				cout << "Terminating.\n";
				exit( -1 );
			}
		}

		typedef tokenizer<char_separator<char> > tokenizer;

		char_separator<char> sep( " \t;" );
		tokenizer tokens( buf, sep );

		tokenizer::iterator tok_iter = tokens.begin();

		if (tok_iter != tokens.end())
		{
			if ((*tok_iter).compare( "module" ) == 0)
			{
				vector<string> parsed_module;

				vector<string> temp;

				string name;

				parse_module( buf, file, line_cnt, working_file, name, parsed_module, false, temp );  // collect the main module arguments

				if (name.empty())
				{
					cout << "Required module name at line " << line_cnt << " of the file " << working_file << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}

				if (parsed_module.size() < 1)
				{
					cout << "Required module arguments at line " << line_cnt << " of the file " << working_file << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}

				if (name.compare( main_module.getName() ) == 0)  //parse the main module file again untill you find the main module
				{
					end_module_found = false;

					for (unsigned int p = 0; p < parsed_module.size(); p++)
					{
						main_module.addArgument( parsed_module[p] );
					}

					main_module.getPortType().resize( main_module.getArguments().size() );
					main_module.getWireType().resize( main_module.getArguments().size() );

					line_cnt++;

					while (true)
					{
						getline( file, buf );

						if (file.eof())
							break;

						tokenizer tokens2( buf, sep );

						tokenizer::iterator tok_iter2 = tokens2.begin();

						if (tok_iter2 != tokens.end())
						{
							if (starts_with( *tok_iter2, "//" ))
							{
								//comments
							}
							else if (starts_with( *tok_iter2, "/*" ))
							{
								parse_multiline_comment( buf, file, line_cnt, working_file );
							}
							else if ((*tok_iter2).compare( "module" ) == 0)  // find module inside a module error check
							{
								cout << "Module declaration found at line " << line_cnt << " of the file " << working_file
										<< " but the previous endmodule declaration is missing.\n";
								cout << "Terminating.\n";
								exit( -1 );
							}
							else if ((*tok_iter2).compare( "endmodule" ) == 0)  //endmodule check
							{
								tok_iter2++;

								if (tok_iter2 != tokens2.end())
								{
									if (!starts_with( *tok_iter2, "//" ))
									{
										cout
												<< "The verilog parser cannot continue parsing the same line, after reaching endmodule declaration at line "
												<< line_cnt << " of the file " << working_file << ".\n";
										cout << "Terminating.\n";
										exit( -1 );
									}
								}

								end_module_found = true;
								break;
							}
							else if (starts_with( *tok_iter2, "endmodule//" ))  //endmodule check
							{
								end_module_found = true;
								break;
							}
							else if ((*tok_iter2).compare( "input" ) == 0)  // parse the inputs
							{
								vector<string> parsed_inputs;

								parse_input( buf, file, line_cnt, working_file, parsed_inputs );

								if (parsed_inputs.empty())
								{
									cout << "Required input name at line " << line_cnt << " of the file " << working_file << ".\n";
									cout << "Terminating.\n";
									exit( -1 );
								}

								for (unsigned int p = 0; p < parsed_inputs.size(); p++)
								{

									unsigned int j;
									for (j = 0; j < main_module.getArguments().size(); j++)  // check if this input is in the arguments list
									{
										if (parsed_inputs[p].compare( main_module.getArguments()[j] ) == 0)
										{
											main_module.getPortType()[j] = INPUT;
											main_module.getWireType()[j] = 0;
											break;
										}
										else if (parsed_inputs[p].substr( 0, parsed_inputs[p].find_first_of( "[" ) ).compare(
												main_module.getArguments()[j] ) == 0)
										{
											main_module.getPortType()[j] = INPUT;
											main_module.getWireType()[j]++;
											break;
										}
									}
									if (j != main_module.getArguments().size())
									{
										Node * input_node = input_mapping( parsed_inputs[p] );
										Net * input_net = wire_mapping( parsed_inputs[p], false );  // inputs are by default wires

										connect_wiring_and_internalconnect( input_node, input_net, NULL, OUTPUT, "", working_file, line_cnt );
									}
									else
									{
										cout << "Input " << parsed_inputs[p] << " not found as a module argument at line " << line_cnt
												<< " of the file " << working_file << ".\n";
										cout << "Terminating.\n";
										exit( -1 );
									}
								}

							}
							else if ((*tok_iter2).compare( "output" ) == 0)  //parse the outputs
							{
								vector<string> parsed_outputs;

								parse_output( buf, file, line_cnt, working_file, parsed_outputs );

								if (parsed_outputs.empty())
								{
									cout << "Required output name at line " << line_cnt << " of the file " << working_file << ".\n";
									cout << "Terminating.\n";
									exit( -1 );
								}

								for (unsigned int p = 0; p < parsed_outputs.size(); p++)
								{

									unsigned int j;
									for (j = 0; j < main_module.getArguments().size(); j++)  // check if this input is in the arguments list
									{
										if (parsed_outputs[p].compare( main_module.getArguments()[j] ) == 0)
										{
											main_module.getPortType()[j] = OUTPUT;
											main_module.getWireType()[j] = 0;
											break;
										}
										else if (parsed_outputs[p].substr( 0, parsed_outputs[p].find_first_of( "[" ) ).compare(
												main_module.getArguments()[j] ) == 0)
										{
											main_module.getPortType()[j] = OUTPUT;
											main_module.getWireType()[j]++;
											break;
										}
									}
									if (j != main_module.getArguments().size())
									{
										Node * output_node = output_mapping( parsed_outputs[p] );
										Net * output_net = wire_mapping( parsed_outputs[p], false );  // outputs are by default wires

										connect_wiring_and_internalconnect( output_node, output_net, NULL, INPUT, "", working_file, line_cnt );
									}
									else
									{
										cout << "Output " << parsed_outputs[p] << " not found as a module argument at line " << line_cnt
												<< " of the file " << working_file << ".\n";
										cout << "Terminating.\n";
										exit( -1 );
									}
								}
							}
							else if ((*tok_iter2).compare( "wire" ) == 0)  // parse the wires
							{
								vector<string> parsed_wires;

								parse_wire( buf, file, line_cnt, working_file, parsed_wires );

								if (parsed_wires.empty())
								{
									cout << "Required wire name at line " << line_cnt << " of the file " << working_file << ".\n";
									cout << "Terminating.\n";
									exit( -1 );
								}

								for (unsigned int p = 0; p < parsed_wires.size(); p++)
								{
									wire_mapping( parsed_wires[p], false );
								}
							}
							else if ((*tok_iter2).compare( "assign" ) == 0)  // parse the assigns
							{
								vector<string> assigns;

								parse_assign( buf, file, line_cnt, working_file, assigns );

								string left_wire = assigns[0];
								string right_wire = assigns[1];

								if (regex_match( left_wire, regex( "(1'b)(0|1)" ) ))
								{
									cout << "Wire " << left_wire << " cannot be used as a right operand in assign operations at line " << line_cnt
											<< " of the file " << working_file << ".\n";
									cout << "Terminating.\n";
									exit( -1 );
								}

								net_map::iterator find_it = wires.find( left_wire );

								if (find_it == wires.end())
								{
									cout << "Wire " << left_wire << " not found at line " << line_cnt << " of the file " << working_file << ".\n";
									cout << "Terminating.\n";
									exit( -1 );
								}

								Net * temp_wire2 = find_it->second;  // left wire

								if (regex_match( right_wire, regex( "(1'b)(0|1)" ) ))
								{
									Net * temp_wire3 = wire_mapping( right_wire, true );  // right wire

									Node * virtual_node = virtual_mapping( right_wire );

									connect_wiring_and_internalconnect( virtual_node, temp_wire3, NULL, OUTPUT, "", working_file, line_cnt );

									if (temp_wire2->getParasitic()->hasStartPoint())
									{
										cout << "Assigned wire " << left_wire << " has already a starting point defined at line " << line_cnt
												<< " of the file " << working_file << ".\n";
										cout << "Terminating.\n";
										exit( -1 );
									}

									Parasitic * old_parasitic = temp_wire2->getParasitic();

									temp_wire2->setAssignedValue( right_wire );
									temp_wire2->setParasitic( temp_wire3->getParasitic() );

									for (unsigned int k = 0; k < old_parasitic->getEndPoints().size(); k++)
									{
										temp_wire2->getParasitic()->addEndPoint( old_parasitic->getEndPoints()[k]->graph_node );
									}
									temp_wire3->addAssignedNet( temp_wire2 );  // right wire was assigned to left

									delete old_parasitic;

								}
								else
								{
									find_it = wires.find( right_wire );  // right wire

									if (find_it == wires.end())
									{
										cout << "Wire " << right_wire << " not found at line " << line_cnt << " of the file " << working_file
												<< ".\n";
										cout << "Terminating.\n";
										exit( -1 );
									}

									Net * temp_wire3 = find_it->second;

									if (temp_wire2->getParasitic()->hasStartPoint())
									{
										cout << "Assigned wire " << left_wire << " has already a starting point defined at line " << line_cnt
												<< " of the file " << working_file << ".\n";
										cout << "Terminating.\n";
										exit( -1 );
									}

									if (!temp_wire2->getAssignedValue().empty())
									{
										cout << left_wire << " has already an assigned value " << temp_wire2->getAssignedValue() << " at line "
												<< line_cnt << " of the file " << working_file << ".\n";
										cout << "Terminating.\n";
										exit( -1 );

									}

									if (temp_wire3 != temp_wire2)  //different wires
									{
										change_affected_nets_parasitic( temp_wire2, temp_wire3 );

										Parasitic * old_parasitic = temp_wire2->getParasitic();

										temp_wire2->setAssignedValue( right_wire );
										temp_wire2->setParasitic( temp_wire3->getParasitic() );

										for (unsigned int k = 0; k < old_parasitic->getEndPoints().size(); k++)
										{
											temp_wire2->getParasitic()->addEndPoint( old_parasitic->getEndPoints()[k]->graph_node );
										}
										temp_wire3->addAssignedNet( temp_wire2 );  // right wire was assigned to left

										delete old_parasitic;
									}
								}
							}
							else  //parse cells or modules
							{
								string name;

								string cell_name;

								if (regex_match( *tok_iter2, regex( "([A-Za-z_]+)([A-Za-z.0-9_]*)" ) ))
								{
									cell_name = name = *tok_iter2;
								}
								else
								{
									cout << "Invalid name " << *tok_iter2 << " at line " << line_cnt << " of the file " << working_file << ".\n";
									cout << "Terminating.\n";
									exit( -1 );
								}

								module_map::iterator find_it = modules.find( name );

								if (find_it == modules.end())  // cell handling
								{

									if (cell_name.compare( main_module.getName() ) == 0)
									{
										cout << cell_name << " is the main module and cannot be reinstantiated at " << line_cnt << " of the file "
												<< working_file << ".\n";
										cout << "Terminating.\n";
										exit( -1 );
									}

									cell_map::iterator find_it = cells.find( cell_name );

									if (find_it == cells.end())
									{
										cout << cell_name << " not found in lib file or in modules at line " << line_cnt << " of the file "
												<< working_file << ".\n";
										cout << "Terminating.\n";
										exit( -1 );
									}

									Cell * temp_cell = find_it->second;

									string instance_name;

									vector<string> parsed_pins;
									vector<string> parsed_wires;

									parse_cell( buf, file, line_cnt, working_file, instance_name, parsed_pins, parsed_wires );  // collect the pin/wire connections for this cell

									if (instance_name.empty())
									{
										cout << "Required instance name at line " << line_cnt << " of the file " << working_file << ".\n";
										cout << "Terminating.\n";
										exit( -1 );
									}

									if (parsed_pins.size() < 1 || parsed_pins.size() != parsed_wires.size())
									{
										cout << "Required cell connection info at line " << line_cnt << " of the file " << working_file << ".\n";
										cout << "Terminating.\n";
										exit( -1 );
									}

									pair<set<string>::iterator, bool> ret;

									ret = instances.insert( instance_name );

									if (ret.second == false)
									{
										cout << "The instance name " << instance_name << " is already in use at line " << line_cnt << " of the file "
												<< working_file << ".\n";
										cout << "Terminating.\n";
										exit( -1 );
									}

									name += "<" + instance_name + ">";

									Node * temp_node = NULL;
									Net * temp_wire = NULL;

									NODE_TYPE pin_type;

									set<string> examined_pins;

									for (unsigned int p = 0; p < parsed_wires.size(); p++)
									{
										/* Pins */
										pin_type = temp_cell->getPinType( parsed_pins[p] );

										if (pin_type == ERROR_CODE)
										{
											cout << "Pin " << parsed_pins[p] << " not found in lib file at line " << line_cnt << " of the file "
													<< working_file << ".\n";
											cout << "Terminating.\n";
											exit( -1 );
										}

										pair<set<string>::iterator, bool> ret;

										ret = examined_pins.insert( parsed_pins[p] );

										if (ret.second == false)
										{
											cout << "Pin " << parsed_pins[p] << " found more than once at line " << line_cnt << " of the file "
													<< working_file << ".\n";
											cout << "Terminating.\n";
											exit( -1 );
										}

										temp_node = node_mapping( name + ":" + parsed_pins[p], parsed_pins[p], temp_cell, pin_type );

										/*****/

										/* Wires */
										if (parsed_wires[p].empty())
										{
											connect_wiring_and_internalconnect( temp_node, NULL, temp_cell, pin_type,
													cell_name + "<" + instance_name + ">", working_file, line_cnt );
										}
										else if (regex_match( parsed_wires[p], regex( "(1'b)(0|1)" ) ))
										{
											temp_wire = wire_mapping( parsed_wires[p], true );

											Node * virtual_node = virtual_mapping( parsed_wires[p] );

											connect_wiring_and_internalconnect( virtual_node, temp_wire, NULL, OUTPUT, "", working_file, line_cnt );

											connect_wiring_and_internalconnect( temp_node, temp_wire, temp_cell, pin_type,
													cell_name + "<" + instance_name + ">", working_file, line_cnt );
										}
										else
										{
											net_map::iterator find_it = wires.find( parsed_wires[p] );

											if (find_it == wires.end())
											{
												cout << "Wire " << parsed_wires[p] << " not found at line " << line_cnt << " of the file "
														<< working_file << ".\n";
												cout << "Terminating.\n";
												exit( -1 );
											}

											temp_wire = find_it->second;

											connect_wiring_and_internalconnect( temp_node, temp_wire, temp_cell, pin_type,
													cell_name + "<" + instance_name + ">", working_file, line_cnt );
										}

										/*****/

									}

									/* check if the user has defined the connection for every pin of the cell */
									io_map::iterator chk;

									for (chk = temp_cell->getIOPins().begin(); chk != temp_cell->getIOPins().end(); chk++)
									{
										if (examined_pins.find( chk->first ) == examined_pins.end())
										{
											//cout << "Missing pin(s) wire connection info at line " << line_cnt << " of the file " << working_file
											//<< ".\n";
											//cout << "Terminating.\n";
											//exit( -1 );

											cout << "Warning pin " << chk->first << " connection is not specified at line " << line_cnt
													<< " of the file " << working_file << ".\n";
											cout << "Assuming to be not connected.\n";

											pin_type = temp_cell->getPinType( chk->first );

											temp_node = node_mapping( name + ":" + chk->first, chk->first, temp_cell, pin_type );

											connect_wiring_and_internalconnect( temp_node, NULL, temp_cell, pin_type,
													cell_name + "<" + instance_name + ">", working_file, line_cnt );
										}
									}
								}
								else
								{  // user created module handling
									Module * temp_module = find_it->second;

									string instance_name;

									vector<string> instance_args;

									vector<string> port_map;

									parse_module( buf, file, line_cnt, working_file, instance_name, instance_args, true, port_map );

									if (instance_name.empty())
									{
										cout << "Required instance name at line " << line_cnt << " of the file " << working_file << ".\n";
										cout << "Terminating.\n";
										exit( -1 );
									}

									if (instance_args.size() < 1)
									{
										cout << "Required module call arguments at line " << line_cnt << " of the file " << working_file << ".\n";
										cout << "Terminating.\n";
										exit( -1 );
									}

									pair<set<string>::iterator, bool> ret;

									ret = instances.insert( instance_name );

									if (ret.second == false)
									{
										cout << "The instance name " << instance_name << " is already in use at line at line " << line_cnt
												<< " of the file " << working_file << ".\n";
										cout << "Terminating.\n";
										exit( -1 );
									}

									if (port_map.size() > 0)
									{
										vector<string> temp_vector( temp_module->getArguments().size() );

										for (unsigned int p = 0; p < port_map.size(); p++)
										{
											unsigned int loc;
											for (loc = 0; loc < temp_module->getArguments().size(); loc++)
											{
												if (port_map[p].compare( temp_module->getArguments()[loc] ) == 0)
												{
													break;
												}
											}

											if (loc == temp_module->getArguments().size())
											{
												cout << "Pin " << port_map[p] << " not found in module " << temp_module->getName() << " at line "
														<< line_cnt << " of the file " << working_file << ".\n";
												cout << "Terminating.\n";
												exit( -1 );
											}

											temp_vector[loc] = instance_args[p];

										}

										instance_args = temp_vector;
									}

									for (unsigned int p = 0; p < instance_args.size(); p++)
									{
										if (regex_match( instance_args[p], regex( "(1'b)(0|1)" ) ))
										{
											Net * temp_wire = wire_mapping( instance_args[p], true );

											Node * virtual_node = virtual_mapping( instance_args[p] );

											connect_wiring_and_internalconnect( virtual_node, temp_wire, NULL, OUTPUT, "", working_file, line_cnt );
										}
										else if (!instance_args[p].empty())
										{

											net_map::iterator find_it = wires.find( instance_args[p] );

											if (find_it == wires.end())
											{
												cout << "Wire " << instance_args[p] << " not found at line " << line_cnt << " of the file "
														<< working_file << ".\n";
												cout << "Terminating.\n";
												exit( -1 );
											}
										}
									}

									/* Instantiate this module with the given arguments */
									instantiate_module( temp_module, instance_name, instance_args, modules, working_file, line_cnt, instances );

								}
							}

						}
						line_cnt++;
					}
				}
			}

		}
		line_cnt++;
	}

	file.close();

//Check for inputs and outputs
	for (unsigned int i = 0; i < main_module.getArguments().size(); i++)
	{
		if (graph.isInput( "<input>:" + main_module.getArguments()[i] ) == false
				&& graph.isOutput( "<output>:" + main_module.getArguments()[i] ) == false)
		{
			cout << "The argument " << main_module.getArguments()[i] << " was not declared either as input or output.\n";
			cout << "While parsing main module " << main_module.getName() << ".\n";
			cout << "Terminating.\n";
			exit( -1 );
		}
	}

	graph.setName( main_module.getName() );

	add_edges();

	creteCellInstances();

//clear mem

	module_map::iterator it;

	for (it = modules.begin(); it != modules.end(); it++)
	{
		delete it->second;
	}

}

/*
 * This function works like the parsing of the main module, with the only difference that everything parsed
 * in this module, that are not included in the main module, are considered to be instanced and need to be fully defined
 * by the callee instance name, like local wires or cells or modules called inside this module.
 */
void System::instantiate_module(Module * temp_module, string instance_name, vector<string> instance_args, module_map& modules, string file_name,
		int line, set<string>& instances)
{

	if (temp_module->getArguments().size() != instance_args.size())
	{
		cout << "Argument count doesnt match at line " << line << " of the file " << file_name << ".\n";
		cout << "Terminating.\n";
		exit( -1 );
	}

	string buf;

	vector<string> inputs;
	vector<string> outputs;

	for (unsigned int i = 0; i < temp_module->getCode().size(); i++)
	{
		buf = temp_module->getCode()[i];

		typedef tokenizer<char_separator<char> > tokenizer;

		char_separator<char> sep( " \t;" );
		tokenizer tokens( buf, sep );

		tokenizer::iterator tok_iter = tokens.begin();

		if (tok_iter != tokens.end())
		{
			if ((*tok_iter).compare( "input" ) == 0)
			{
				vector<string> parsed_inputs;

				parse_input_instance_module( temp_module, i, parsed_inputs );

				if (parsed_inputs.empty())
				{
					cout << "Required input name at line " << temp_module->getLines()[i] << " of the file " << temp_module->getFileName() << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}

				for (unsigned int p = 0; p < parsed_inputs.size(); p++)
				{

					unsigned int j;
					for (j = 0; j < temp_module->getArguments().size(); j++)
					{
						if (parsed_inputs[p].compare( temp_module->getArguments()[j] ) == 0)
						{
							temp_module->getPortType()[j] = INPUT;
							temp_module->getWireType()[j] = 0;
							break;
						}
					}

					if (j != temp_module->getArguments().size())
					{
						inputs.push_back( parsed_inputs[p] );
					}
					else
					{
						cout << "Input " << parsed_inputs[p] << " not found as a module argument at line " << temp_module->getLines()[i]
								<< " of the file " << temp_module->getFileName() << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}
				}

			}
			else if ((*tok_iter).compare( "output" ) == 0)
			{
				vector<string> parsed_outputs;

				parse_output_instance_module( temp_module, i, parsed_outputs );

				if (parsed_outputs.empty())
				{
					cout << "Required output name at line " << temp_module->getLines()[i] << " of the file " << temp_module->getFileName() << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}

				for (unsigned int p = 0; p < parsed_outputs.size(); p++)
				{

					unsigned int j;
					for (j = 0; j < temp_module->getArguments().size(); j++)
					{
						if (parsed_outputs[p].compare( temp_module->getArguments()[j] ) == 0)
						{
							temp_module->getPortType()[j] = OUTPUT;
							temp_module->getWireType()[j] = 0;
							break;
						}
					}

					if (j != temp_module->getArguments().size())
					{
						outputs.push_back( parsed_outputs[p] );
					}
					else
					{
						cout << "Output " << parsed_outputs[p] << " not found as a module argument at line " << temp_module->getLines()[i]
								<< " of the file " << temp_module->getFileName() << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}
				}
			}
			else if ((*tok_iter).compare( "wire" ) == 0)
			{
				vector<string> parsed_wires;

				parse_wire_instance_module( temp_module, i, parsed_wires );

				if (parsed_wires.empty())
				{
					cout << "Required wire name at line " << temp_module->getLines()[i] << " of the file " << temp_module->getFileName() << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}

				for (unsigned int p = 0; p < parsed_wires.size(); p++)
				{

					unsigned int j;
					for (j = 0; j < temp_module->getArguments().size(); j++)
					{
						if (parsed_wires[p].compare( temp_module->getArguments()[j] ) == 0)
						{
							break;
						}
					}

					if (j == temp_module->getArguments().size())
					{
						wire_mapping( instance_name + "." + parsed_wires[p], false );
					}
				}

			}
			else if ((*tok_iter).compare( "assign" ) == 0)  // parse the assigns
			{
				string left_wire, left_wire_new;
				string right_wire, right_wire_new;

				vector<string> assigns;

				parse_assign_instance_module( temp_module, i, assigns );

				left_wire = assigns[0];
				right_wire = assigns[1];

				if (regex_match( left_wire, regex( "(1'b)(0|1)" ) ))  // left wire cannot be a constant
				{
					cout << "Wire " << left_wire << " cannot be used as a left operand in assign operations at line " << temp_module->getLines()[i]
							<< " of the file " << temp_module->getFileName() << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}

				unsigned int j;
				for (j = 0; j < temp_module->getArguments().size(); j++)
				{
					if (left_wire.compare( temp_module->getArguments()[j] ) == 0)
					{
						break;
					}
				}

				if (j == temp_module->getArguments().size())  //check if the left wire is a passed argument
				{
					left_wire_new = instance_name + "." + left_wire;
				}
				else
				{
					left_wire_new = instance_args[j];
				}

				if (!left_wire_new.empty())  // it was an empty argument, skip it
				{
					if (regex_match( left_wire_new, regex( "(1'b)(0|1)" ) ))  // the argument was a constant, the left wire cannot be a constant
					{
						cout << "Wire " << left_wire << " (" << left_wire_new << ") cannot be used as a left operand in assign operations at line "
								<< temp_module->getLines()[i] << " of the file " << temp_module->getFileName() << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					net_map::iterator find_it = wires.find( left_wire_new );

					if (find_it == wires.end())
					{
						cout << "Wire " << left_wire << " not found at line " << temp_module->getLines()[i] << " of the file "
								<< temp_module->getFileName() << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					Net * temp_wire2 = find_it->second;  // left wire

					if (regex_match( right_wire, regex( "(1'b)(0|1)" ) ))  // check if the right wire is a constant
					{
						Net * temp_wire3 = wire_mapping( right_wire, true );  // right wire

						Node * virtual_node = virtual_mapping( right_wire );

						connect_wiring_and_internalconnect( virtual_node, temp_wire3, NULL, OUTPUT, "", temp_module->getFileName(),
								temp_module->getLines()[i] );

						if (temp_wire2->getParasitic()->hasStartPoint())
						{
							cout << "Assigned wire " << left_wire << " has already a starting point defined at line " << temp_module->getLines()[i]
									<< " of the file " << temp_module->getFileName() << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}

						Parasitic * old_parasitic = temp_wire2->getParasitic();

						temp_wire2->setAssignedValue( right_wire );
						temp_wire2->setParasitic( temp_wire3->getParasitic() );

						for (unsigned int k = 0; k < old_parasitic->getEndPoints().size(); k++)
						{
							temp_wire2->getParasitic()->addEndPoint( old_parasitic->getEndPoints()[k]->graph_node );
						}
						temp_wire3->addAssignedNet( temp_wire2 );  // right wire was assigned to left

						delete old_parasitic;
					}
					else  // it is not a constant
					{
						for (j = 0; j < temp_module->getArguments().size(); j++)
						{
							if (right_wire.compare( temp_module->getArguments()[j] ) == 0)
							{
								break;
							}
						}

						if (j == temp_module->getArguments().size())  //check if the right wire is a passed argument
						{
							right_wire_new = instance_name + "." + right_wire;
						}
						else
						{
							right_wire_new = instance_args[j];
						}

						if (!right_wire_new.empty())  // it was an empty argument, skip it
						{
							if (regex_match( right_wire_new, regex( "(1'b)(0|1)" ) ))  // the passed argument was a constant
							{
								Net * temp_wire3 = wire_mapping( right_wire_new, true );  // right wire

								Node * virtual_node = virtual_mapping( right_wire_new );

								connect_wiring_and_internalconnect( virtual_node, temp_wire3, NULL, OUTPUT, "", temp_module->getFileName(),
										temp_module->getLines()[i] );

								if (temp_wire2->getParasitic()->hasStartPoint())
								{
									cout << "Assigned wire " << left_wire << " has already a starting point defined at line "
											<< temp_module->getLines()[i] << " of the file " << temp_module->getFileName() << ".\n";
									cout << "Terminating.\n";
									exit( -1 );
								}

								Parasitic * old_parasitic = temp_wire2->getParasitic();

								temp_wire2->setAssignedValue( right_wire_new );
								temp_wire2->setParasitic( temp_wire3->getParasitic() );

								for (unsigned int k = 0; k < old_parasitic->getEndPoints().size(); k++)
								{
									temp_wire2->getParasitic()->addEndPoint( old_parasitic->getEndPoints()[k]->graph_node );
								}
								temp_wire3->addAssignedNet( temp_wire2 );  // right wire was assigned to left

								delete old_parasitic;
							}
							else  // the right wire is not a constant
							{
								net_map::iterator find_it = wires.find( right_wire_new );

								if (find_it == wires.end())
								{
									cout << "Wire " << right_wire << " not found at line " << temp_module->getLines()[i] << " of the file "
											<< temp_module->getFileName() << ".\n";
									cout << "Terminating.\n";
									exit( -1 );
								}

								Net * temp_wire3 = find_it->second;  // right wire

								if (temp_wire2->getParasitic()->hasStartPoint())
								{
									cout << "Assigned wire " << left_wire << " has already a starting point defined at line "
											<< temp_module->getLines()[i] << " of the file " << temp_module->getFileName() << ".\n";
									cout << "Terminating.\n";
									exit( -1 );
								}

								if (!temp_wire2->getAssignedValue().empty())
								{
									cout << left_wire << " has already an assigned value " << temp_wire2->getAssignedValue() << " at line "
											<< temp_module->getLines()[i] << " of the file " << temp_module->getFileName() << ".\n";
									cout << "Terminating.\n";
									exit( -1 );

								}

								if (temp_wire3 != temp_wire2)  // different wires
								{
									change_affected_nets_parasitic( temp_wire2, temp_wire3 );

									Parasitic * old_parasitic = temp_wire2->getParasitic();

									temp_wire2->setAssignedValue( right_wire_new );
									temp_wire2->setParasitic( temp_wire3->getParasitic() );

									for (unsigned int k = 0; k < old_parasitic->getEndPoints().size(); k++)
									{
										temp_wire2->getParasitic()->addEndPoint( old_parasitic->getEndPoints()[k]->graph_node );
									}
									temp_wire3->addAssignedNet( temp_wire2 );  // right wire was assigned to left

									delete old_parasitic;
								}
							}
						}
					}
				}
			}
			else
			{
				string name;

				string cell_name;

				if (regex_match( *tok_iter, regex( "([A-Za-z_]+)([A-Za-z0-9._]*)" ) ))
				{
					cell_name = name = *tok_iter;
				}
				else
				{
					cout << "Invalid name " << *tok_iter << " at line " << temp_module->getLines()[i] << " of the file " << temp_module->getFileName()
							<< ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}

				module_map::iterator find_it = modules.find( name );

				if (find_it == modules.end())  // cell handling
				{
					if (cell_name.compare( main_module.getName() ) == 0)
					{
						cout << cell_name << " is the main module and cannot be reinstantiated at " << temp_module->getLines()[i] << " of the file "
								<< temp_module->getFileName() << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					cell_map::iterator find_it = cells.find( cell_name );

					if (find_it == cells.end())
					{
						cout << cell_name << " not found in lib file or in modules at line " << temp_module->getLines()[i] << " of the file "
								<< temp_module->getFileName() << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					Cell * temp_cell = find_it->second;

					string temp_name;

					vector<string> parsed_pins;
					vector<string> parsed_wires;

					parse_cell_instance_module( temp_module, i, temp_name, parsed_pins, parsed_wires );  // collect the pin/wire connections for this cell

					if (temp_name.empty())
					{
						cout << "Required instance name at line " << temp_module->getLines()[i] << " of the file " << temp_module->getFileName()
								<< ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					if (parsed_pins.size() < 1 || parsed_pins.size() != parsed_wires.size())
					{
						cout << "Required cell connection info at line " << temp_module->getLines()[i] << " of the file "
								<< temp_module->getFileName() << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					string new_instance_name = instance_name + "." + temp_name;

					pair<set<string>::iterator, bool> ret;

					ret = instances.insert( new_instance_name );

					if (ret.second == false)
					{
						cout << "The instance name " << temp_name << " is already in use at line " << temp_module->getLines()[i] << " of the file "
								<< temp_module->getFileName() << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					name += "<" + instance_name + "." + temp_name + ">";

					Node * temp_node = NULL;
					Net * temp_wire = NULL;

					set<string> examined_pins;

					NODE_TYPE pin_type;

					for (unsigned int p = 0; p < parsed_wires.size(); p++)
					{
						/* Pins */
						pin_type = temp_cell->getPinType( parsed_pins[p] );

						if (pin_type == ERROR_CODE)
						{
							cout << "Pin " << parsed_pins[p] << " not found in lib file at line " << temp_module->getLines()[i] << " of the file "
									<< temp_module->getFileName() << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}

						pair<set<string>::iterator, bool> ret;

						ret = examined_pins.insert( parsed_pins[p] );

						if (ret.second == false)
						{
							cout << "Pin " << parsed_pins[p] << " found more than once at line " << temp_module->getLines()[i] << " of the file "
									<< temp_module->getFileName() << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}

						temp_node = node_mapping( name + ":" + parsed_pins[p], parsed_pins[p], temp_cell, pin_type );
						/******/

						/* Wires */

						if (parsed_wires[p].empty())
						{
							connect_wiring_and_internalconnect( temp_node, NULL, temp_cell, pin_type, cell_name + "<" + new_instance_name + ">",
									temp_module->getFileName(), temp_module->getLines()[i] );
						}
						else if (regex_match( parsed_wires[p], regex( "(1'b)(0|1)" ) ))
						{
							temp_wire = wire_mapping( parsed_wires[p], true );

							Node * virtual_node = virtual_mapping( parsed_wires[p] );

							connect_wiring_and_internalconnect( virtual_node, temp_wire, NULL, OUTPUT, "", temp_module->getFileName(),
									temp_module->getLines()[i] );

							connect_wiring_and_internalconnect( temp_node, temp_wire, temp_cell, pin_type, cell_name + "<" + new_instance_name + ">",
									temp_module->getFileName(), temp_module->getLines()[i] );
						}
						else
						{
							string wire_name = parsed_wires[p];

							unsigned int j;
							for (j = 0; j < temp_module->getArguments().size(); j++)
							{
								if ((wire_name).compare( temp_module->getArguments()[j] ) == 0)
								{
									break;
								}
							}

							if (j != temp_module->getArguments().size())
							{
								wire_name = instance_args[j];
							}
							else
							{
								wire_name = instance_name + "." + wire_name;
							}

							if (regex_match( wire_name, regex( "(1'b)(0|1)" ) ))
							{
								temp_wire = wire_mapping( wire_name, true );

								Node * virtual_node = virtual_mapping( wire_name );

								connect_wiring_and_internalconnect( virtual_node, temp_wire, NULL, OUTPUT, "", temp_module->getFileName(),
										temp_module->getLines()[i] );

								connect_wiring_and_internalconnect( temp_node, temp_wire, temp_cell, pin_type, cell_name + "<" + new_instance_name + ">",
										temp_module->getFileName(), temp_module->getLines()[i] );
							}
							else if (!wire_name.empty())
							{
								net_map::iterator find_it = wires.find( wire_name );

								if (find_it == wires.end())
								{
									cout << "Wire " << parsed_wires[p] << " not found at line " << temp_module->getLines()[i] << " of the file "
											<< temp_module->getFileName() << ".\n";
									cout << "Terminating.\n";
									exit( -1 );
								}

								temp_wire = find_it->second;

								connect_wiring_and_internalconnect( temp_node, temp_wire, temp_cell, pin_type, cell_name + "<" + new_instance_name + ">",
										temp_module->getFileName(), temp_module->getLines()[i] );
							}
							else
							{
								connect_wiring_and_internalconnect( temp_node, NULL, temp_cell, pin_type, cell_name + "<" + new_instance_name + ">",
										temp_module->getFileName(), temp_module->getLines()[i] );
							}
						}

						/******/
					}

					/* check if the user has defined the connection for every pin of the cell */
					io_map::iterator chk;

					for (chk = temp_cell->getIOPins().begin(); chk != temp_cell->getIOPins().end(); chk++)
					{
						if (examined_pins.find( chk->first ) == examined_pins.end())
						{
							//cout << "Missing pin(s) wire connection info at line " << temp_module->getLines()[i] << " of the file "
							//<< temp_module->getFileName() << ".\n";
							//cout << "Terminating.\n";
							//exit( -1 );

							cout << "Warning pin " << chk->first << " connection is not specified at line " << temp_module->getLines()[i]
									<< " of the file " << temp_module->getFileName() << ".\n";
							cout << "Assuming to be not connected.\n";

							pin_type = temp_cell->getPinType( chk->first );

							temp_node = node_mapping( name + ":" + chk->first, chk->first, temp_cell, pin_type );

							connect_wiring_and_internalconnect( temp_node, NULL, temp_cell, pin_type, cell_name + "<" + new_instance_name + ">",
									temp_module->getFileName(), temp_module->getLines()[i] );
						}
					}
				}
				else
				{  // user created module handling
					Module * temp_module2 = find_it->second;

					if (temp_module2->getName().compare( temp_module->getName() ) == 0)
					{
						cout << "Recursive module call found at line " << temp_module->getLines()[i] << " of the file " << temp_module->getFileName()
								<< ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					vector<string> temp_args;

					vector<string> temp_port_map;

					string temp_name;

					parse_module_instance_module( temp_module, i, temp_name, temp_args, temp_port_map );

					if (temp_name.empty())
					{
						cout << "Required instance name at line " << temp_module->getLines()[i] << " of the file " << temp_module->getFileName()
								<< ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					if (temp_args.size() < 1)
					{
						cout << "Required module call arguments at line " << temp_module->getLines()[i] << " of the file "
								<< temp_module->getFileName() << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					string new_instance_name = instance_name + "." + temp_name;

					pair<set<string>::iterator, bool> ret;

					ret = instances.insert( new_instance_name );

					if (ret.second == false)
					{
						cout << "The instance name " << temp_name << " is already in use at line " << temp_module->getLines()[i] << " of the file "
								<< temp_module->getFileName() << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					if (temp_port_map.size() > 0)
					{
						vector<string> temp_vector( temp_module2->getArguments().size() );

						for (unsigned int p = 0; p < temp_port_map.size(); p++)
						{
							unsigned int loc;
							for (loc = 0; loc < temp_module2->getArguments().size(); loc++)
							{
								if (temp_port_map[p].compare( temp_module2->getArguments()[loc] ) == 0)
								{
									break;
								}
							}

							if (loc == temp_module2->getArguments().size())
							{
								cout << "Pin " << temp_port_map[p] << " not found in module " << temp_module2->getName() << " at line "
										<< temp_module->getLines()[i] << " of the file " << temp_module->getFileName() << ".\n";
								cout << "Terminating.\n";
								exit( -1 );
							}

							temp_vector[loc] = temp_args[p];

						}

						temp_args = temp_vector;
					}

					vector<string> new_instance_args;

					for (unsigned int p = 0; p < temp_args.size(); p++)
					{

						string name = temp_args[p];

						if (regex_match( name, regex( "(1'b)(0|1)" ) ))
						{
							Net * temp_wire = wire_mapping( name, true );

							Node * virtual_node = virtual_mapping( name );

							connect_wiring_and_internalconnect( virtual_node, temp_wire, NULL, OUTPUT, "", temp_module->getFileName(),
									temp_module->getLines()[i] );
						}
						else if (!name.empty())
						{

							unsigned int j;
							for (j = 0; j < temp_module->getArguments().size(); j++)
							{
								if ((name).compare( temp_module->getArguments()[j] ) == 0)
								{
									break;
								}
							}

							if (j != temp_module->getArguments().size())
							{
								name = instance_args[j];
							}
							else
							{
								name = instance_name + "." + name;
							}

							if (!name.empty())
							{
								net_map::iterator find_it = wires.find( name );

								if (find_it == wires.end())
								{
									cout << "Wire " << temp_args[p] << " not found at line " << temp_module->getLines()[i] << " of the file "
											<< temp_module->getFileName() << ".\n";
									cout << "Terminating.\n";
									exit( -1 );
								}
							}
						}

						new_instance_args.push_back( name );
					}

					/* Recursively instantiate this module with its arguments */
					instantiate_module( temp_module2, new_instance_name, new_instance_args, modules, temp_module->getFileName(),
							temp_module->getLines()[i], instances );

				}
			}
		}
	}

	for (unsigned int i = 0; i < temp_module->getArguments().size(); i++)
	{
		bool inputs_found = false;
		bool outputs_found = false;

		for (unsigned j = 0; j < inputs.size(); j++)
		{
			if (inputs[j].compare( temp_module->getArguments()[i] ) == 0
					|| inputs[j].substr( 0, inputs[j].find_first_of( "[" ) ).compare( temp_module->getArguments()[i] ) == 0)
			{
				inputs_found = true;
				break;
			}
		}

		if (inputs_found)
		{
			continue;
		}

		for (unsigned j = 0; j < outputs.size(); j++)
		{
			if (outputs[j].compare( temp_module->getArguments()[i] ) == 0
					|| outputs[j].substr( 0, outputs[j].find_first_of( "[" ) ).compare( temp_module->getArguments()[i] ) == 0)
			{
				outputs_found = true;
				break;
			}
		}

		if (!inputs_found && !outputs_found)
		{
			cout << "The argument " << temp_module->getArguments()[i] << " was not declared either as input or output,\n";
			cout << "while parsing module " << temp_module->getName() << " of the file " << temp_module->getFileName() << ".\n";
			cout << "Terminating.\n";
			exit( -1 );
		}
	}

}

/* End of Verilog Parser */

Node * System::node_mapping(string node_name, string pin_name, Cell * cell_type, NODE_TYPE type)
{

	Pin * pin;

	if (type == INPUT)
	{
		pin = cell_type->getInput( pin_name );
	}
	else
	{
		pin = cell_type->getOutput( pin_name );
	}

	return graph.addInternalNode( node_name, pin );

}

Node * System::input_mapping(string node_name)
{

	return graph.addInputNode( node_name, NULL );

}

Node * System::output_mapping(string node_name)
{

	return graph.addOutputNode( node_name, NULL );

}

Node * System::virtual_mapping(string node_name)
{

	return graph.addVirtualNode( node_name, NULL );

}

Net * System::wire_mapping(string wire_name, bool virtual_wire)
{

	Net * temp = NULL;

	net_map::iterator it = wires.lower_bound( wire_name );

	if (it != wires.end() && (*it).first.compare( wire_name ) == 0)
	{
		return (*it).second;
	}
	else
	{
		temp = new Net( wire_name, virtual_wire );

		wires.insert( it, net_entry( wire_name, temp ) );

		return temp;
	}
}

void System::connect_wiring_and_internalconnect(Node * temp_node, Net * temp_wire, Cell * temp_cell, NODE_TYPE type, string c_i_name, string file_name,
		int line)
{

	if (type == OUTPUT)
	{
		if (temp_wire != NULL)
		{
			if (temp_wire->getParasitic()->hasStartPoint() && temp_wire->getParasitic()->getStartPoint()->graph_node != temp_node)
			{
				cout << "The wire " << temp_wire->getName() << " has multiple starting pins at line " << line << " of the file " << file_name
						<< ".\n";
				cout << "Terminating.\n";
				exit( -1 );
			}
			temp_wire->getParasitic()->setStartPoint( temp_node );

			if (temp_node->getNet() == NULL)
			{
				temp_node->setNet( temp_wire );
			}
			else if (temp_node->getNet() != temp_wire)
			{
				cout << "Node " << temp_node->getName() << " is connected to more than one nets.\n";
				cout << "Terminating.\n";
				exit( -1 );
			}
		}

		if (temp_cell != NULL)
		{
			timing_map::iterator it;

			OutputPin * output = ( OutputPin* ) temp_node->getPin();

			for (it = output->getTimings().begin(); it != output->getTimings().end(); it++)
			{

				Node * new_node = node_mapping( c_i_name + ":" + it->first, it->first, temp_cell, it->second->getRelatedPin()->getType() );

				Edge * temp = new CellEdge( "" );
				temp->setConnectedTo( temp_node );
				(( CellEdge * ) temp)->setTiming( it->second );

				new_node->addForwardEdge( temp );

				Edge * temp2 = new CellEdge( "" );
				temp2->setConnectedTo( new_node );
				(( CellEdge * ) temp2)->setTiming( it->second );

				temp_node->addBackwardEdge( temp2 );

				temp->setOppositeEdge( temp2 );
				temp2->setOppositeEdge( temp );

			}
		}

	}
	else
	{
		if (temp_wire != NULL)
		{
			temp_wire->getParasitic()->addEndPoint( temp_node );

			if (temp_node->getNet() == NULL)
			{
				temp_node->setNet( temp_wire );
			}
			else if (temp_node->getNet() != temp_wire)
			{
				cout << "Node " << temp_node->getName() << " is connected to more than one nets.\n";
				cout << "Terminating.\n";
				exit( -1 );
			}
		}
	}
}

void System::add_edges()
{

	net_map::iterator q;

	for (q = wires.begin(); q != wires.end(); q++)
	{
		Parasitic * w = q->second->getParasitic();

		if (q->second->getAssignedValue().empty())
		{
			if (w->hasStartPoint() == false && w->hasEndPoints() == false)
			{
				cout << "Warning the wire " << q->first << " is not used.\n";
			}
			else if (w->hasStartPoint() == false && w->hasEndPoints() == true)
			{
				cout << "Warning the wire " << q->first << " is not connected in any output of a cell.\n";
			}
			else if (w->hasStartPoint() == true && w->hasEndPoints() == false)
			{
				cout << "Warning the wire " << q->first << " is not connected in any input of a cell.\n";
			}

			if (w->hasStartPoint())
			{
				for (unsigned int j = 0; j < w->getEndPoints().size(); j++)
				{

					Edge * temp = new NetEdge( w->getOriginalName() );
					temp->setConnectedTo( w->getEndPoints()[j]->graph_node );

					w->getStartPoint()->graph_node->addForwardEdge( temp );

					Edge * temp2 = new NetEdge( w->getOriginalName() );
					temp2->setConnectedTo( w->getStartPoint()->graph_node );

					w->getEndPoints()[j]->graph_node->addBackwardEdge( temp2 );

					temp->setOppositeEdge( temp2 );
					temp2->setOppositeEdge( temp );
				}
			}
		}
	}
}

void System::parse_module(string buf, ifstream& file, unsigned int& line_cnt, string file_name, string& module_name, vector<string>& parsed_module,
		bool state, vector<string>& port_map)
{

	typedef tokenizer<char_separator<char> > tokenizer;

	char_separator<char> sep_module( " ,\t()" );
	tokenizer tokens_module( buf, sep_module );

	tokenizer::iterator tok_module_iter = tokens_module.begin();

	bool port_map_instantiation = false;
	bool first_module_arg_found = false;
	bool pin_found = false;

	tok_module_iter++;

	do
	{
		while (tok_module_iter != tokens_module.end())
		{
			if (starts_with( *tok_module_iter, "//" ))
			{
				getline( file, buf );

				if (file.eof())
				{
					cout << "Semicolon not found for module declaration at line " << line_cnt << " of the file " << file_name << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}

				line_cnt++;

				tokenizer tokens_module( buf, sep_module );

				tok_module_iter = tokens_module.begin();

				continue;
			}
			else if (starts_with( *tok_module_iter, "/*" ))
			{
				cout << "Comment found inside module declaration at line " << line_cnt << " of the file " << file_name << ".\n";
				cout << "Terminating.\n";
				exit( -1 );
			}

			if (module_name.empty())
			{
				unsigned found = (*tok_module_iter).find( ";" );

				if (found > (*tok_module_iter).size())
				{
					string name = *tok_module_iter;

					if (regex_match( name, regex( "([A-Za-z_]+)([A-Za-z0-9_]*)" ) ))
					{
						module_name = name;
					}
					else
					{
						if (state)
						{
							cout << "Invalid module instance name " << name << " at line " << line_cnt << " of the file " << file_name << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
						else
						{
							cout << "Invalid module name " << name << " at line " << line_cnt << " of the file " << file_name << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
					}
				}
				else
				{
					if ((*tok_module_iter).at( 0 ) != ';')
					{
						string name = (*tok_module_iter).substr( 0, found );

						if (regex_match( name, regex( "([A-Za-z_]+)([A-Za-z0-9_]*)" ) ))
						{
							module_name = name;
						}
						else
						{
							if (state)
							{
								cout << "Invalid module instance name " << name << " at line " << line_cnt << " of the file " << file_name << ".\n";
								cout << "Terminating.\n";
								exit( -1 );
							}
							else
							{
								cout << "Invalid module name " << name << " at line " << line_cnt << " of the file " << file_name << ".\n";
								cout << "Terminating.\n";
								exit( -1 );
							}
						}
					}

					/* After Semicolon check */
					string temp_buf = buf.substr( buf.find_first_of( ";" ) + 1, buf.size() );

					char_separator<char> sep_buf( " \t;" );
					tokenizer tokens_buf( temp_buf, sep_buf );

					tokenizer::iterator tok_buf_iter = tokens_buf.begin();

					if (tok_buf_iter != tokens_buf.end())
					{
						if (!starts_with( *tok_buf_iter, "//" ))
						{
							cout << "The verilog parser cannot continue parsing the same line, after reaching semicolon at line " << line_cnt
									<< " of the file " << file_name << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
					}
					/*************************/

					return;
				}
			}
			else
			{
				if (first_module_arg_found == false && starts_with( *tok_module_iter, "." ) && state)
				{
					port_map_instantiation = true;
				}

				first_module_arg_found = true;

				if (port_map_instantiation == false)
				{
					unsigned found = (*tok_module_iter).find( ";" );

					if (found > (*tok_module_iter).size())
					{
						string name = *tok_module_iter;

						if (regex_match( name, regex( "(([A-Za-z_]+)([A-Za-z0-9_]*)|((1'b)(0|1)))" ) ))
						{
							parsed_module.push_back( name );
						}
						else if (state && regex_match( name, regex( "([A-Za-z_]+)([A-Za-z0-9_]*)(\\[)([0-9]+)(\\])" ) ))
						{
							char_separator<char> bus( "[]" );
							tokenizer bus_token( name, bus );

							tokenizer::iterator bus_iter = bus_token.begin();

							string temp_name = *bus_iter;
							bus_iter++;
							string id = to_string( stoi( *bus_iter ) );

							parsed_module.push_back( temp_name + "[" + id + "]" );
						}
						else
						{
							if (state)
							{
								cout << "Invalid wire name " << name << " at line " << line_cnt << " of the file " << file_name << ".\n";
								cout << "Terminating.\n";
								exit( -1 );
							}
							else
							{
								cout << "Invalid argument name " << name << " at line " << line_cnt << " of the file " << file_name << ".\n";
								cout << "Terminating.\n";
								exit( -1 );
							}
						}
					}
					else
					{
						if ((*tok_module_iter).at( 0 ) != ';')
						{
							string name = (*tok_module_iter).substr( 0, found );

							if (regex_match( name, regex( "(([A-Za-z_]+)([A-Za-z0-9_]*)|((1'b)(0|1)))" ) ))
							{
								parsed_module.push_back( name );
							}
							else if (state && regex_match( name, regex( "([A-Za-z_]+)([A-Za-z0-9_]*)(\\[)([0-9]+)(\\])" ) ))
							{
								char_separator<char> bus( "[]" );
								tokenizer bus_token( name, bus );

								tokenizer::iterator bus_iter = bus_token.begin();

								string temp_name = *bus_iter;
								bus_iter++;
								string id = to_string( stoi( *bus_iter ) );

								parsed_module.push_back( temp_name + "[" + id + "]" );
							}
							else
							{
								if (state)
								{
									cout << "Invalid wire name " << name << " at line " << line_cnt << " of the file " << file_name << ".\n";
									cout << "Terminating.\n";
									exit( -1 );
								}
								else
								{
									cout << "Invalid argument name " << name << " at line " << line_cnt << " of the file " << file_name << ".\n";
									cout << "Terminating.\n";
									exit( -1 );
								}
							}
						}

						/* After Semicolon check */
						string temp_buf = buf.substr( buf.find_first_of( ";" ) + 1, buf.size() );

						char_separator<char> sep_buf( " \t;" );
						tokenizer tokens_buf( temp_buf, sep_buf );

						tokenizer::iterator tok_buf_iter = tokens_buf.begin();

						if (tok_buf_iter != tokens_buf.end())
						{
							if (!starts_with( *tok_buf_iter, "//" ))
							{
								cout << "The verilog parser cannot continue parsing the same line, after reaching semicolon at line " << line_cnt
										<< " of the file " << file_name << ".\n";
								cout << "Terminating.\n";
								exit( -1 );
							}
						}
						/*************************/

						return;
					}
				}
				else
				{
					char_separator<char> sep2( "." );
					tokenizer tokens3( *tok_module_iter, sep2 );

					tokenizer::iterator tok_iter3 = tokens3.begin();

					if (starts_with( *tok_module_iter, "." ) && !pin_found)
					{
						if (tok_iter3 == tokens3.end())
						{
							cout << "Required pin name at line " << line_cnt << " of the file " << file_name << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}

						pin_found = true;

						unsigned found = (*tok_iter3).find( ";" );

						if (found > (*tok_iter3).size())
						{
							string name = *tok_iter3;

							if (regex_match( name, regex( "([A-Za-z_]+)([A-Za-z0-9_]*)" ) ))
							{
								port_map.push_back( name );
							}
							else
							{
								cout << "Invalid pin name " << name << " at line " << line_cnt << " of the file " << file_name << ".\n";
								cout << "Terminating.\n";
								exit( -1 );
							}
						}
						else
						{
							cout << "Required wire name at line " << line_cnt << " of the file " << file_name << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
					}
					else if (starts_with( *tok_module_iter, "." ) && pin_found)
					{
						parsed_module.push_back( "" );
						pin_found = false;
						continue;
					}
					else if (!starts_with( *tok_module_iter, "." ) && !pin_found)
					{
						if ((*tok_module_iter).at( 0 ) == ';')
						{
							/* After Semicolon check */
							string temp_buf = buf.substr( buf.find_first_of( ";" ) + 1, buf.size() );

							char_separator<char> sep_buf( " \t;" );
							tokenizer tokens_buf( temp_buf, sep_buf );

							tokenizer::iterator tok_buf_iter = tokens_buf.begin();

							if (tok_buf_iter != tokens_buf.end())
							{
								if (!starts_with( *tok_buf_iter, "//" ))
								{
									cout << "The verilog parser cannot continue parsing the same line, after reaching semicolon at line " << line_cnt
											<< " of the file " << file_name << ".\n";
									cout << "Terminating.\n";
									exit( -1 );
								}
							}
							/*************************/

							return;
						}
						else
						{
							cout << "Required pin name at line " << line_cnt << " of the file " << file_name << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
					}
					else if (!starts_with( *tok_module_iter, "." ) && pin_found)
					{
						unsigned found = (*tok_iter3).find( ";" );

						if (found > (*tok_iter3).size())
						{
							string name = *tok_iter3;

							if (regex_match( name, regex( "(([A-Za-z_]+)([A-Za-z0-9_]*)|((1'b)(0|1)))" ) ))
							{
								parsed_module.push_back( name );
							}
							else if (regex_match( name, regex( "([A-Za-z_]+)([A-Za-z0-9_]*)(\\[)([0-9]+)(\\])" ) ))
							{
								char_separator<char> bus( "[]" );
								tokenizer bus_token( name, bus );

								tokenizer::iterator bus_iter = bus_token.begin();

								string temp_name = *bus_iter;
								bus_iter++;
								string id = to_string( stoi( *bus_iter ) );

								parsed_module.push_back( temp_name + "[" + id + "]" );
							}
							else
							{
								cout << "Invalid wire name " << name << " at line " << line_cnt << " of the file " << file_name << ".\n";
								cout << "Terminating.\n";
								exit( -1 );
							}
						}
						else
						{
							if ((*tok_iter3).at( 0 ) != ';')
							{
								string name = (*tok_iter3).substr( 0, found );

								if (regex_match( name, regex( "(([A-Za-z_]+)([A-Za-z0-9_]*)|((1'b)(0|1)))" ) ))
								{
									parsed_module.push_back( name );
								}
								else if (regex_match( name, regex( "([A-Za-z_]+)([A-Za-z0-9_]*)(\\[)([0-9]+)(\\])" ) ))
								{
									char_separator<char> bus( "[]" );
									tokenizer bus_token( name, bus );

									tokenizer::iterator bus_iter = bus_token.begin();

									string temp_name = *bus_iter;
									bus_iter++;

									string id = to_string( stoi( *bus_iter ) );

									parsed_module.push_back( temp_name + "[" + id + "]" );
								}
								else
								{
									cout << "Invalid wire name " << name << " at line " << line_cnt << " of the file " << file_name << ".\n";
									cout << "Terminating.\n";
									exit( -1 );
								}
							}
							else
							{
								parsed_module.push_back( "" );
							}

							/* After Semicolon check */
							string temp_buf = buf.substr( buf.find_first_of( ";" ) + 1, buf.size() );

							char_separator<char> sep_buf( " \t;" );
							tokenizer tokens_buf( temp_buf, sep_buf );

							tokenizer::iterator tok_buf_iter = tokens_buf.begin();

							if (tok_buf_iter != tokens_buf.end())
							{
								if (!starts_with( *tok_buf_iter, "//" ))
								{
									cout << "The verilog parser cannot continue parsing the same line, after reaching semicolon at line " << line_cnt
											<< " of the file " << file_name << ".\n";
									cout << "Terminating.\n";
									exit( -1 );
								}
							}
							/*************************/

							return;
						}
						pin_found = false;
					}
				}
			}

			tok_module_iter++;
		}

		getline( file, buf );

		if (file.eof())
		{
			cout << "Semicolon not found for module declaration at line " << line_cnt << " of the file " << file_name << ".\n";
			cout << "Terminating.\n";
			exit( -1 );
		}

		line_cnt++;

		tokenizer tokens_module( buf, sep_module );

		tok_module_iter = tokens_module.begin();
	} while (true);

}

void System::parse_wire(string buf, ifstream& file, unsigned int& line_cnt, string file_name, vector<string>& parsed_wires)
{

	typedef tokenizer<char_separator<char> > tokenizer;

	char_separator<char> sep_wire( " ,\t" );
	tokenizer tokens_wire( buf, sep_wire );

	tokenizer::iterator tok_wire_iter = tokens_wire.begin();

	tok_wire_iter++;

	bool found_bus = false;
	int number;

	do
	{
		while (tok_wire_iter != tokens_wire.end())
		{

			if (starts_with( *tok_wire_iter, "//" ))
			{
				getline( file, buf );

				if (file.eof())
				{
					cout << "Semicolon not found for wire declaration at line " << line_cnt << " of the file " << file_name << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}

				line_cnt++;

				tokenizer tokens_wire( buf, sep_wire );

				tok_wire_iter = tokens_wire.begin();

				continue;
			}
			else if (starts_with( *tok_wire_iter, "/*" ))
			{
				cout << "Comment found inside wire declaration at line " << line_cnt << " of the file " << file_name << ".\n";
				cout << "Terminating.\n";
				exit( -1 );
			}

			unsigned found = (*tok_wire_iter).find( ";" );

			if (found > (*tok_wire_iter).size())
			{
				string name = *tok_wire_iter;

				if (regex_match( name, regex( "([A-Za-z_]+)([A-Za-z0-9_]*)" ) ))
				{
					if (found_bus)
					{
						for (unsigned int i = 0; i <= number; i++)
						{
							string temp = name + "[" + to_string( i ) + "]";
							parsed_wires.push_back( temp );
						}

						found_bus = false;
					}
					else
					{
						parsed_wires.push_back( name );
					}
				}
				else if (regex_match( name, regex( "(\\[)([0-9]+)(:0\\])" ) ))
				{
					if (found_bus == false)
					{
						char_separator<char> sep_bus( "[:" );
						tokenizer tokens_bus( name, sep_bus );

						tokenizer::iterator tokens_bus_iter = tokens_bus.begin();

						number = stoi( (*tokens_bus_iter) );

						found_bus = true;
					}
					else
					{
						cout << "Bus declaration " << name << " found before a bus name at line " << line_cnt << " of the file " << file_name
								<< ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}
				}
				else
				{
					cout << "Invalid wire name " << name << " at line " << line_cnt << " of the file " << file_name << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}
			}
			else
			{
				if ((*tok_wire_iter).at( 0 ) != ';')
				{
					string name = (*tok_wire_iter).substr( 0, found );

					if (regex_match( name, regex( "([A-Za-z_]+)([A-Za-z0-9_]*)" ) ))
					{
						if (found_bus)
						{
							for (unsigned int i = 0; i <= number; i++)
							{
								string temp = name + "[" + to_string( i ) + "]";
								parsed_wires.push_back( temp );
							}

							found_bus = false;
						}
						else
						{
							parsed_wires.push_back( name );
						}
					}
					else if (regex_match( name, regex( "(\\[)([0-9]+)(:0\\])" ) ))
					{
						if (found_bus == false)
						{
							char_separator<char> sep_bus( "[:" );
							tokenizer tokens_bus( name, sep_bus );

							tokenizer::iterator tokens_bus_iter = tokens_bus.begin();

							number = stoi( (*tokens_bus_iter) );

							found_bus = true;
						}
						else
						{
							cout << "Bus declaration " << name << " found before a bus name at line " << line_cnt << " of the file " << file_name
									<< ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
					}
					else
					{
						cout << "Invalid wire name " << name << " at line " << line_cnt << " of the file " << file_name << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}
				}

				if (found_bus)
				{
					cout << "Required bus name at line " << line_cnt << " of the file " << file_name << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}

				/* After Semicolon check */
				string temp_buf = buf.substr( buf.find_first_of( ";" ) + 1, buf.size() );

				char_separator<char> sep_buf( " \t;" );
				tokenizer tokens_buf( temp_buf, sep_buf );

				tokenizer::iterator tok_buf_iter = tokens_buf.begin();

				if (tok_buf_iter != tokens_buf.end())
				{
					if (!starts_with( *tok_buf_iter, "//" ))
					{
						cout << "The verilog parser cannot continue parsing the same line, after reaching semicolon at line " << line_cnt
								<< " of the file " << file_name << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}
				}
				/*************************/

				return;
			}
			tok_wire_iter++;
		}

		getline( file, buf );

		if (file.eof())
		{
			cout << "Semicolon not found for wire declaration at line " << line_cnt << " of the file " << file_name << ".\n";
			cout << "Terminating.\n";
			exit( -1 );
		}

		line_cnt++;

		tokenizer tokens_wire( buf, sep_wire );

		tok_wire_iter = tokens_wire.begin();
	} while (true);

}

void System::parse_input(string buf, ifstream& file, unsigned int& line_cnt, string file_name, vector<string>& parsed_inputs)
{

	typedef tokenizer<char_separator<char> > tokenizer;

	char_separator<char> sep_inputs( " ,\t" );
	tokenizer tokens_inputs( buf, sep_inputs );

	tokenizer::iterator tok_inputs_iter = tokens_inputs.begin();

	tok_inputs_iter++;

	bool found_bus = false;
	int number;

	do
	{
		while (tok_inputs_iter != tokens_inputs.end())
		{
			if (starts_with( *tok_inputs_iter, "//" ))
			{
				getline( file, buf );

				if (file.eof())
				{
					cout << "Semicolon not found for input declaration at line " << line_cnt << " of the file " << file_name << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}

				line_cnt++;

				tokenizer tokens_inputs( buf, sep_inputs );

				tok_inputs_iter = tokens_inputs.begin();

				continue;
			}
			else if (starts_with( *tok_inputs_iter, "/*" ))
			{
				cout << "Comment found inside input declaration at line " << line_cnt << " of the file " << file_name << ".\n";
				cout << "Terminating.\n";
				exit( -1 );
			}

			unsigned found = (*tok_inputs_iter).find( ";" );

			if (found > (*tok_inputs_iter).size())
			{
				string name = *tok_inputs_iter;

				if (regex_match( name, regex( "([A-Za-z_]+)([A-Za-z0-9_]*)" ) ))
				{
					if (found_bus)
					{
						for (unsigned int i = 0; i <= number; i++)
						{
							string temp = name + "[" + to_string( i ) + "]";
							parsed_inputs.push_back( temp );
						}

						found_bus = false;
					}
					else
					{
						parsed_inputs.push_back( name );
					}
				}
				else if (regex_match( name, regex( "(\\[)([0-9]+)(:0\\])" ) ))
				{
					if (found_bus == false)
					{
						char_separator<char> sep_bus( "[:" );
						tokenizer tokens_bus( name, sep_bus );

						tokenizer::iterator tokens_bus_iter = tokens_bus.begin();

						number = stoi( (*tokens_bus_iter) );

						found_bus = true;
					}
					else
					{
						cout << "Bus declaration " << name << " found before a bus name at line " << line_cnt << " of the file " << file_name
								<< ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}
				}
				else
				{
					cout << "Invalid input name " << name << " at line " << line_cnt << " of the file " << file_name << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}
			}
			else
			{
				if ((*tok_inputs_iter).at( 0 ) != ';')
				{
					string name = (*tok_inputs_iter).substr( 0, found );

					if (regex_match( name, regex( "([A-Za-z_]+)([A-Za-z0-9_]*)" ) ))
					{
						if (found_bus)
						{
							for (unsigned int i = 0; i <= number; i++)
							{
								string temp = name + "[" + to_string( i ) + "]";
								parsed_inputs.push_back( temp );
							}

							found_bus = false;
						}
						else
						{
							parsed_inputs.push_back( name );
						}
					}
					else if (regex_match( name, regex( "(\\[)([0-9]+)(:0\\])" ) ))
					{
						if (found_bus == false)
						{
							char_separator<char> sep_bus( "[:" );
							tokenizer tokens_bus( name, sep_bus );

							tokenizer::iterator tokens_bus_iter = tokens_bus.begin();

							number = stoi( (*tokens_bus_iter) );

							found_bus = true;
						}
						else
						{
							cout << "Bus declaration " << name << " found before a bus name at line " << line_cnt << " of the file " << file_name
									<< ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
					}
					else
					{
						cout << "Invalid input name " << name << " at line " << line_cnt << " of the file " << file_name << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}
				}

				if (found_bus)
				{
					cout << "Required bus name at line " << line_cnt << " of the file " << file_name << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}

				/* After Semicolon check */
				string temp_buf = buf.substr( buf.find_first_of( ";" ) + 1, buf.size() );

				char_separator<char> sep_buf( " \t;" );
				tokenizer tokens_buf( temp_buf, sep_buf );

				tokenizer::iterator tok_buf_iter = tokens_buf.begin();

				if (tok_buf_iter != tokens_buf.end())
				{
					if (!starts_with( *tok_buf_iter, "//" ))
					{
						cout << "The verilog parser cannot continue parsing the same line, after reaching semicolon at line " << line_cnt
								<< " of the file " << file_name << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}
				}
				/*************************/

				return;
			}

			tok_inputs_iter++;
		}

		getline( file, buf );

		if (file.eof())
		{
			cout << "Semicolon not found for input declaration at line " << line_cnt << " of the file " << file_name << ".\n";
			cout << "Terminating.\n";
			exit( -1 );
		}

		line_cnt++;

		tokenizer tokens_inputs( buf, sep_inputs );

		tok_inputs_iter = tokens_inputs.begin();
	} while (true);

}

void System::parse_output(string buf, ifstream& file, unsigned int& line_cnt, string file_name, vector<string>& parsed_outputs)
{

	typedef tokenizer<char_separator<char> > tokenizer;

	char_separator<char> sep_outputs( " ,\t" );
	tokenizer tokens_outputs( buf, sep_outputs );

	tokenizer::iterator tok_outputs_iter = tokens_outputs.begin();

	tok_outputs_iter++;

	bool found_bus = false;
	int number;

	do
	{
		while (tok_outputs_iter != tokens_outputs.end())
		{
			if (starts_with( *tok_outputs_iter, "//" ))
			{
				getline( file, buf );

				if (file.eof())
				{
					cout << "Semicolon not found for output declaration at line " << line_cnt << " of the file " << file_name << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}

				line_cnt++;

				tokenizer tokens_outputs( buf, sep_outputs );

				tok_outputs_iter = tokens_outputs.begin();

				continue;
			}
			else if (starts_with( *tok_outputs_iter, "/*" ))
			{
				cout << "Comment found inside output declaration at line " << line_cnt << " of the file " << file_name << ".\n";
				cout << "Terminating.\n";
				exit( -1 );
			}

			unsigned found = (*tok_outputs_iter).find( ";" );

			if (found > (*tok_outputs_iter).size())
			{
				string name = *tok_outputs_iter;

				if (regex_match( name, regex( "([A-Za-z_]+)([A-Za-z0-9_]*)" ) ))
				{
					if (found_bus)
					{
						for (unsigned int i = 0; i <= number; i++)
						{
							string temp = name + "[" + to_string( i ) + "]";
							parsed_outputs.push_back( temp );
						}

						found_bus = false;
					}
					else
					{
						parsed_outputs.push_back( name );
					}
				}
				else if (regex_match( name, regex( "(\\[)([0-9]+)(:0\\])" ) ))
				{
					if (found_bus == false)
					{
						char_separator<char> sep_bus( "[:" );
						tokenizer tokens_bus( name, sep_bus );

						tokenizer::iterator tokens_bus_iter = tokens_bus.begin();

						number = stoi( (*tokens_bus_iter) );

						found_bus = true;
					}
					else
					{
						cout << "Bus declaration " << name << " found before a bus name at line " << line_cnt << " of the file " << file_name
								<< ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}
				}
				else
				{
					cout << "Invalid output name " << name << " at line " << line_cnt << " of the file " << file_name << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}
			}
			else
			{
				if ((*tok_outputs_iter).at( 0 ) != ';')
				{
					string name = (*tok_outputs_iter).substr( 0, found );

					if (regex_match( name, regex( "([A-Za-z_]+)([A-Za-z0-9_]*)" ) ))
					{
						if (found_bus)
						{
							for (unsigned int i = 0; i <= number; i++)
							{
								string temp = name + "[" + to_string( i ) + "]";
								parsed_outputs.push_back( temp );
							}

							found_bus = false;
						}
						else
						{
							parsed_outputs.push_back( name );
						}
					}
					else if (regex_match( name, regex( "(\\[)([0-9]+)(:0\\])" ) ))
					{
						if (found_bus == false)
						{
							char_separator<char> sep_bus( "[:" );
							tokenizer tokens_bus( name, sep_bus );

							tokenizer::iterator tokens_bus_iter = tokens_bus.begin();

							number = stoi( (*tokens_bus_iter) );

							found_bus = true;
						}
						else
						{
							cout << "Bus declaration " << name << " found before a bus name at line " << line_cnt << " of the file " << file_name
									<< ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
					}
					else
					{
						cout << "Invalid output name " << name << " at line " << line_cnt << " of the file " << file_name << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}
				}

				if (found_bus)
				{
					cout << "Required bus name at line " << line_cnt << " of the file " << file_name << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}

				/* After Semicolon check */
				string temp_buf = buf.substr( buf.find_first_of( ";" ) + 1, buf.size() );

				char_separator<char> sep_buf( " \t;" );
				tokenizer tokens_buf( temp_buf, sep_buf );

				tokenizer::iterator tok_buf_iter = tokens_buf.begin();

				if (tok_buf_iter != tokens_buf.end())
				{
					if (!starts_with( *tok_buf_iter, "//" ))
					{
						cout << "The verilog parser cannot continue parsing the same line, after reaching semicolon at line " << line_cnt
								<< " of the file " << file_name << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}
				}
				/*************************/

				return;
			}

			tok_outputs_iter++;
		}

		getline( file, buf );

		if (file.eof())
		{
			cout << "Semicolon not found for output declaration at line " << line_cnt << " of the file " << file_name << ".\n";
			cout << "Terminating.\n";
			exit( -1 );
		}

		line_cnt++;

		tokenizer tokens_outputs( buf, sep_outputs );

		tok_outputs_iter = tokens_outputs.begin();
	} while (true);

}

void System::parse_cell(string buf, ifstream& file, unsigned int& line_cnt, string file_name, string& instance_name, vector<string>& parsed_pins,
		vector<string>& parsed_wires)
{

	typedef tokenizer<char_separator<char> > tokenizer;

	char_separator<char> sep_cell( " ,\t()" );
	tokenizer tokens_cell( buf, sep_cell );

	tokenizer::iterator tok_cell_iter = tokens_cell.begin();

	tok_cell_iter++;

	bool pin_found = false;

	do
	{
		while (tok_cell_iter != tokens_cell.end())
		{
			if (starts_with( *tok_cell_iter, "//" ))
			{
				getline( file, buf );

				if (file.eof())
				{
					cout << "Semicolon not found for cell declaration at line " << line_cnt << " of the file " << file_name << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}

				line_cnt++;

				tokenizer tokens_cell( buf, sep_cell );

				tok_cell_iter = tokens_cell.begin();

				continue;
			}
			else if (starts_with( *tok_cell_iter, "/*" ))
			{
				cout << "Comment found inside cell declaration at line " << line_cnt << " of the file " << file_name << ".\n";
				cout << "Terminating.\n";
				exit( -1 );
			}

			if (instance_name.empty())
			{
				unsigned found = (*tok_cell_iter).find( ";" );

				if (found > (*tok_cell_iter).size())
				{
					string name = *tok_cell_iter;

					if (regex_match( name, regex( "([A-Za-z_]+)([A-Za-z0-9_]*)" ) ))
					{
						instance_name = name;
					}
					else
					{
						cout << "Invalid cell instance name " << name << " at line " << line_cnt << " of the file " << file_name << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}
				}
				else
				{
					if ((*tok_cell_iter).at( 0 ) != ';')
					{
						string name = (*tok_cell_iter).substr( 0, found );

						if (regex_match( name, regex( "([A-Za-z_]+)([A-Za-z0-9_]*)" ) ))
						{
							instance_name = name;
						}
						else
						{
							cout << "Invalid cell instance name " << name << " at line " << line_cnt << " of the file " << file_name << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
					}

					/* After Semicolon check */
					string temp_buf = buf.substr( buf.find_first_of( ";" ) + 1, buf.size() );

					char_separator<char> sep_buf( " \t;" );
					tokenizer tokens_buf( temp_buf, sep_buf );

					tokenizer::iterator tok_buf_iter = tokens_buf.begin();

					if (tok_buf_iter != tokens_buf.end())
					{
						if (!starts_with( *tok_buf_iter, "//" ))
						{
							cout << "The verilog parser cannot continue parsing the same line, after reaching semicolon at line " << line_cnt
									<< " of the file " << file_name << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
					}
					/*************************/

					return;
				}
			}
			else
			{
				char_separator<char> sep2( "." );
				tokenizer tokens3( *tok_cell_iter, sep2 );

				tokenizer::iterator tok_iter3 = tokens3.begin();

				if (starts_with( *tok_cell_iter, "." ) && !pin_found)
				{
					if (tok_iter3 == tokens3.end())
					{
						cout << "Required pin name at line " << line_cnt << " of the file " << file_name << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					pin_found = true;

					unsigned found = (*tok_iter3).find( ";" );

					if (found > (*tok_iter3).size())
					{
						string name = *tok_iter3;

						if (regex_match( name, regex( "([A-Za-z_]+)([A-Za-z0-9_]*)" ) ))
						{
							parsed_pins.push_back( name );
						}
						else
						{
							cout << "Invalid pin name " << name << " at line " << line_cnt << " of the file " << file_name << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
					}
					else
					{
						cout << "Required wire name at line " << line_cnt << " of the file " << file_name << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}
				}
				else if (starts_with( *tok_cell_iter, "." ) && pin_found)
				{
					parsed_wires.push_back( "" );
					pin_found = false;
					continue;
				}
				else if (!starts_with( *tok_cell_iter, "." ) && !pin_found)
				{
					if ((*tok_cell_iter).at( 0 ) == ';')
					{
						/* After Semicolon check */
						string temp_buf = buf.substr( buf.find_first_of( ";" ) + 1, buf.size() );

						char_separator<char> sep_buf( " \t;" );
						tokenizer tokens_buf( temp_buf, sep_buf );

						tokenizer::iterator tok_buf_iter = tokens_buf.begin();

						if (tok_buf_iter != tokens_buf.end())
						{
							if (!starts_with( *tok_buf_iter, "//" ))
							{
								cout << "The verilog parser cannot continue parsing the same line, after reaching semicolon at line " << line_cnt
										<< " of the file " << file_name << ".\n";
								cout << "Terminating.\n";
								exit( -1 );
							}
						}
						/*************************/

						return;
					}
					else
					{
						cout << "Required pin name at line " << line_cnt << " of the file " << file_name << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}
				}
				else if (!starts_with( *tok_cell_iter, "." ) && pin_found)
				{
					unsigned found = (*tok_iter3).find( ";" );

					if (found > (*tok_iter3).size())
					{
						string name = *tok_iter3;

						if (regex_match( name, regex( "(([A-Za-z_]+)([A-Za-z0-9_]*)|((1'b)(0|1)))" ) ))
						{
							parsed_wires.push_back( name );
						}
						else if (regex_match( name, regex( "([A-Za-z_]+)([A-Za-z0-9_]*)(\\[)([0-9]+)(\\])" ) ))
						{
							char_separator<char> bus( "[]" );
							tokenizer bus_token( name, bus );

							tokenizer::iterator bus_iter = bus_token.begin();

							string temp_name = *bus_iter;
							bus_iter++;
							string id = to_string( stoi( *bus_iter ) );

							parsed_wires.push_back( temp_name + "[" + id + "]" );
						}
						else
						{
							cout << "Invalid wire name " << name << " at line " << line_cnt << " of the file " << file_name << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
					}
					else
					{
						if ((*tok_iter3).at( 0 ) != ';')
						{
							string name = (*tok_iter3).substr( 0, found );

							if (regex_match( name, regex( "(([A-Za-z_]+)([A-Za-z0-9_]*)|((1'b)(0|1)))" ) ))
							{
								parsed_wires.push_back( name );
							}
							else if (regex_match( name, regex( "([A-Za-z_]+)([A-Za-z0-9_]*)(\\[)([0-9]+)(\\])" ) ))
							{
								char_separator<char> bus( "[]" );
								tokenizer bus_token( name, bus );

								tokenizer::iterator bus_iter = bus_token.begin();

								string temp_name = *bus_iter;
								bus_iter++;
								string id = to_string( stoi( *bus_iter ) );

								parsed_wires.push_back( temp_name + "[" + id + "]" );
							}
							else
							{
								cout << "Invalid wire name " << name << " at line " << line_cnt << " of the file " << file_name << ".\n";
								cout << "Terminating.\n";
								exit( -1 );
							}
						}
						else
						{
							parsed_wires.push_back( "" );
						}

						/* After Semicolon check */
						string temp_buf = buf.substr( buf.find_first_of( ";" ) + 1, buf.size() );

						char_separator<char> sep_buf( " \t;" );
						tokenizer tokens_buf( temp_buf, sep_buf );

						tokenizer::iterator tok_buf_iter = tokens_buf.begin();

						if (tok_buf_iter != tokens_buf.end())
						{
							if (!starts_with( *tok_buf_iter, "//" ))
							{
								cout << "The verilog parser cannot continue parsing the same line, after reaching semicolon at line " << line_cnt
										<< " of the file " << file_name << ".\n";
								cout << "Terminating.\n";
								exit( -1 );
							}
						}
						/*************************/

						return;
					}
					pin_found = false;
				}
			}

			tok_cell_iter++;

		}

		getline( file, buf );

		if (file.eof())
		{
			cout << "Semicolon not found for cell declaration at line " << line_cnt << " of the file " << file_name << ".\n";
			cout << "Terminating.\n";
			exit( -1 );
		}

		line_cnt++;

		tokenizer tokens_cell( buf, sep_cell );

		tok_cell_iter = tokens_cell.begin();
	} while (true);

}

void System::parse_multiline_comment(string buf, ifstream& file, unsigned int& line_cnt, string file_name)
{

	typedef tokenizer<char_separator<char> > tokenizer;

	do
	{
		unsigned found = buf.find( "*/" );

		if (found <= buf.size())
		{
			/* After end multiline comment check */
			string temp_buf = buf.substr( buf.find_first_of( "*/" ) + 2, buf.size() );

			char_separator<char> sep_buf( " \t;" );
			tokenizer tokens_buf( temp_buf, sep_buf );

			tokenizer::iterator tok_buf_iter = tokens_buf.begin();

			if (tok_buf_iter != tokens_buf.end())
			{
				if (!starts_with( *tok_buf_iter, "//" ))
				{
					cout << "The verilog parser cannot continue parsing the same line, after reaching the end of a multiline comment at line "
							<< line_cnt << " of the file " << file_name << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}
			}
			/*************************/

			return;
		}

		getline( file, buf );

		if (file.eof())
		{
			cout << "Non ending comment found at line " << line_cnt << " of the file " << file_name << ".\n";
			cout << "Terminating.\n";
			exit( -1 );
		}

		line_cnt++;
	} while (true);

}

void System::parse_assign(string buf, ifstream& file, unsigned int& line_cnt, string file_name, vector<string>& assigns)
{
	typedef tokenizer<char_separator<char> > tokenizer;

	char_separator<char> sep_assign( " =\t" );
	tokenizer tokens_assign( buf, sep_assign );

	tokenizer::iterator tok_assign_iter = tokens_assign.begin();

	tok_assign_iter++;

	bool found_left = false;
	bool found_right = false;

	do
	{
		while (tok_assign_iter != tokens_assign.end())
		{
			if (starts_with( *tok_assign_iter, "//" ))
			{
				getline( file, buf );

				if (file.eof())
				{
					cout << "Semicolon not found for assign declaration at line " << line_cnt << " of the file " << file_name << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}

				line_cnt++;

				tokenizer tokens_assign( buf, sep_assign );

				tok_assign_iter = tokens_assign.begin();

				continue;
			}
			else if (starts_with( *tok_assign_iter, "/*" ))
			{
				cout << "Comment found inside assign declaration at line " << line_cnt << " of the file " << file_name << ".\n";
				cout << "Terminating.\n";
				exit( -1 );
			}

			unsigned found = (*tok_assign_iter).find( ";" );

			if (!found_left)
			{
				if (found > (*tok_assign_iter).size())
				{
					string name = *tok_assign_iter;

					if (regex_match( name, regex( "(([A-Za-z_]+)([A-Za-z0-9_]*)|((1'b)(0|1)))" ) ))
					{
						assigns.push_back( name );
						found_left = true;
					}
					else if (regex_match( name, regex( "([A-Za-z_]+)([A-Za-z0-9_]*)(\\[)([0-9]+)(\\])" ) ))
					{
						char_separator<char> bus( "[]" );
						tokenizer bus_token( name, bus );

						tokenizer::iterator bus_iter = bus_token.begin();

						string temp_name = *bus_iter;
						bus_iter++;
						string id = to_string( stoi( *bus_iter ) );

						assigns.push_back( temp_name + "[" + id + "]" );
						found_left = true;
					}
					else
					{
						cout << "Invalid assigned wire name " << name << " at line " << line_cnt << " of the file " << file_name << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}
				}
				else
				{
					cout << "Semicolon found for assign declaration at line " << line_cnt << " of the file " << file_name << ".\n";
					cout << "before left side assignment declaration.\n";
					cout << "Terminating.\n";
					exit( -1 );
				}
			}
			else
			{
				if (found > (*tok_assign_iter).size())
				{
					string name = *tok_assign_iter;

					if (regex_match( name, regex( "(([A-Za-z_]+)([A-Za-z0-9_]*)|((1'b)(0|1)))" ) ))
					{
						assigns.push_back( name );
						found_right = true;
					}
					else if (regex_match( name, regex( "([A-Za-z_]+)([A-Za-z0-9_]*)(\\[)([0-9]+)(\\])" ) ))
					{
						char_separator<char> bus( "[]" );
						tokenizer bus_token( name, bus );

						tokenizer::iterator bus_iter = bus_token.begin();

						string temp_name = *bus_iter;
						bus_iter++;
						string id = to_string( stoi( *bus_iter ) );

						assigns.push_back( temp_name + "[" + id + "]" );
						found_right = true;
					}
					else
					{
						cout << "Invalid assigned wire name " << name << " at line " << line_cnt << " of the file " << file_name << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}
				}
				else
				{
					if ((*tok_assign_iter).at( 0 ) != ';')
					{
						string name = (*tok_assign_iter).substr( 0, found );

						if (regex_match( name, regex( "(([A-Za-z_]+)([A-Za-z0-9_]*)|((1'b)(0|1)))" ) ))
						{
							assigns.push_back( name );
							found_right = true;
						}
						else if (regex_match( name, regex( "([A-Za-z_]+)([A-Za-z0-9_]*)(\\[)([0-9]+)(\\])" ) ))
						{
							char_separator<char> bus( "[]" );
							tokenizer bus_token( name, bus );

							tokenizer::iterator bus_iter = bus_token.begin();

							string temp_name = *bus_iter;
							bus_iter++;
							string id = to_string( stoi( *bus_iter ) );

							assigns.push_back( temp_name + "[" + id + "]" );
							found_right = true;
						}
						else
						{
							cout << "Invalid assigned wire name " << name << " at line " << line_cnt << " of the file " << file_name << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
					}
					else if (!found_right)
					{
						cout << "Semicolon found for assign declaration at line " << line_cnt << " of the file " << file_name << ".\n";
						cout << "before right side assignment declaration.\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					/* After Semicolon check */
					string temp_buf = buf.substr( buf.find_first_of( ";" ) + 1, buf.size() );

					char_separator<char> sep_buf( " \t;" );
					tokenizer tokens_buf( temp_buf, sep_buf );

					tokenizer::iterator tok_buf_iter = tokens_buf.begin();

					if (tok_buf_iter != tokens_buf.end())
					{
						if (!starts_with( *tok_buf_iter, "//" ))
						{
							cout << "The verilog parser cannot continue parsing the same line, after reaching semicolon at line " << line_cnt
									<< " of the file " << file_name << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
					}
					/*************************/

					return;
				}
			}

			tok_assign_iter++;
		}

		getline( file, buf );

		if (file.eof())
		{
			cout << "Semicolon not found for assign declaration at line " << line_cnt << " of the file " << file_name << ".\n";
			cout << "Terminating.\n";
			exit( -1 );
		}

		line_cnt++;

		tokenizer tokens_outputs( buf, sep_assign );

		tok_assign_iter = tokens_outputs.begin();
	} while (true);

}

void System::parse_module_instance_module(Module* temp_module, unsigned int& index, string& module_name, vector<string>& parsed_module,
		vector<string>& port_map)
{

	string buf = temp_module->getCode()[index];

	typedef tokenizer<char_separator<char> > tokenizer;

	char_separator<char> sep_module( " ,\t()" );
	tokenizer tokens_module( buf, sep_module );

	tokenizer::iterator tok_module_iter = tokens_module.begin();

	bool port_map_instantiation = false;
	bool first_module_arg_found = false;
	bool pin_found = false;

	tok_module_iter++;

	do
	{
		while (tok_module_iter != tokens_module.end())
		{

			if (starts_with( *tok_module_iter, "//" ))
			{
				index++;

				if (index >= temp_module->getCode().size())
				{
					cout << "Semicolon not found for module declaration at line " << temp_module->getLines()[index - 1] << " of the file "
							<< temp_module->getFileName() << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}

				buf = temp_module->getCode()[index];

				tokenizer tokens_module( buf, sep_module );

				tok_module_iter = tokens_module.begin();

				continue;
			}
			else if (starts_with( *tok_module_iter, "/*" ))
			{
				cout << "Comment found inside module declaration at line " << temp_module->getLines()[index] << " of the file "
						<< temp_module->getFileName() << ".\n";
				cout << "Terminating.\n";
				exit( -1 );
			}

			if (module_name.empty())
			{
				unsigned found = (*tok_module_iter).find( ";" );

				if (found > (*tok_module_iter).size())
				{
					string name = *tok_module_iter;

					if (regex_match( name, regex( "([A-Za-z_]+)([A-Za-z0-9_]*)" ) ))
					{
						module_name = name;
					}
					else
					{

						cout << "Invalid module instance name " << name << " at line " << temp_module->getLines()[index] << " of the file "
								<< temp_module->getFileName() << ".\n";
						cout << "Terminating.\n";
						exit( -1 );

					}
				}
				else
				{
					if ((*tok_module_iter).at( 0 ) != ';')
					{
						string name = (*tok_module_iter).substr( 0, found );

						if (regex_match( name, regex( "([A-Za-z_]+)([A-Za-z0-9_]*)" ) ))
						{
							module_name = name;
						}
						else
						{

							cout << "Invalid module instance name " << name << " at line " << temp_module->getLines()[index] << " of the file "
									<< temp_module->getFileName() << ".\n";
							cout << "Terminating.\n";
							exit( -1 );

						}
					}

					/* After Semicolon check */
					string temp_buf = buf.substr( buf.find_first_of( ";" ) + 1, buf.size() );

					char_separator<char> sep_buf( " \t;" );
					tokenizer tokens_buf( temp_buf, sep_buf );

					tokenizer::iterator tok_buf_iter = tokens_buf.begin();

					if (tok_buf_iter != tokens_buf.end())
					{
						if (!starts_with( *tok_buf_iter, "//" ))
						{
							cout << "The verilog parser cannot continue parsing the same line, after reaching semicolon at line "
									<< temp_module->getLines()[index] << " of the file " << temp_module->getFileName() << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
					}
					/*************************/

					return;
				}
			}
			else
			{
				if (first_module_arg_found == false && starts_with( *tok_module_iter, "." ))
				{
					port_map_instantiation = true;
				}

				first_module_arg_found = true;

				if (port_map_instantiation == false)
				{
					unsigned found = (*tok_module_iter).find( ";" );

					if (found > (*tok_module_iter).size())
					{
						string name = *tok_module_iter;

						if (regex_match( name, regex( "(([A-Za-z_]+)([A-Za-z0-9_]*)|((1'b)(0|1)))" ) ))
						{
							parsed_module.push_back( name );
						}
						else if (regex_match( name, regex( "([A-Za-z_]+)([A-Za-z0-9_]*)(\\[)([0-9]+)(\\])" ) ))
						{
							char_separator<char> bus( "[]" );
							tokenizer bus_token( name, bus );

							tokenizer::iterator bus_iter = bus_token.begin();

							string temp_name = *bus_iter;
							bus_iter++;

							string id = to_string( stoi( *bus_iter ) );

							parsed_module.push_back( temp_name + "[" + id + "]" );
						}
						else
						{

							cout << "Invalid wire name " << name << " at line " << temp_module->getLines()[index] << " of the file "
									<< temp_module->getFileName() << ".\n";
							cout << "Terminating.\n";
							exit( -1 );

						}
					}
					else
					{
						if ((*tok_module_iter).at( 0 ) != ';')
						{
							string name = (*tok_module_iter).substr( 0, found );

							if (regex_match( name, regex( "(([A-Za-z_]+)([A-Za-z0-9_]*)|((1'b)(0|1)))" ) ))
							{
								parsed_module.push_back( name );
							}
							else if (regex_match( name, regex( "([A-Za-z_]+)([A-Za-z0-9_]*)(\\[)([0-9]+)(\\])" ) ))
							{
								char_separator<char> bus( "[]" );
								tokenizer bus_token( name, bus );

								tokenizer::iterator bus_iter = bus_token.begin();

								string temp_name = *bus_iter;
								bus_iter++;

								string id = to_string( stoi( *bus_iter ) );

								parsed_module.push_back( temp_name + "[" + id + "]" );
							}
							else
							{

								cout << "Invalid wire name " << name << " at line " << temp_module->getLines()[index] << " of the file "
										<< temp_module->getFileName() << ".\n";
								cout << "Terminating.\n";
								exit( -1 );

							}
						}

						/* After Semicolon check */
						string temp_buf = buf.substr( buf.find_first_of( ";" ) + 1, buf.size() );

						char_separator<char> sep_buf( " \t;" );
						tokenizer tokens_buf( temp_buf, sep_buf );

						tokenizer::iterator tok_buf_iter = tokens_buf.begin();

						if (tok_buf_iter != tokens_buf.end())
						{
							if (!starts_with( *tok_buf_iter, "//" ))
							{
								cout << "The verilog parser cannot continue parsing the same line, after reaching semicolon at line "
										<< temp_module->getLines()[index] << " of the file " << temp_module->getFileName() << ".\n";
								cout << "Terminating.\n";
								exit( -1 );
							}
						}
						/*************************/

						return;
					}
				}
				else
				{
					char_separator<char> sep2( "." );
					tokenizer tokens3( *tok_module_iter, sep2 );

					tokenizer::iterator tok_iter3 = tokens3.begin();

					if (starts_with( *tok_module_iter, "." ) && !pin_found)
					{
						if (tok_iter3 == tokens3.end())
						{
							cout << "Required pin name at line " << temp_module->getLines()[index] << " of the file " << temp_module->getFileName()
									<< ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}

						pin_found = true;

						unsigned found = (*tok_iter3).find( ";" );

						if (found > (*tok_iter3).size())
						{
							string name = *tok_iter3;

							if (regex_match( name, regex( "([A-Za-z_]+)([A-Za-z0-9_]*)" ) ))
							{
								port_map.push_back( name );
							}
							else
							{
								cout << "Invalid pin name " << name << " at line " << temp_module->getLines()[index] << " of the file "
										<< temp_module->getFileName() << ".\n";
								cout << "Terminating.\n";
								exit( -1 );
							}
						}
						else
						{
							cout << "Required wire name at line " << temp_module->getLines()[index] << " of the file " << temp_module->getFileName()
									<< ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
					}
					else if (starts_with( *tok_module_iter, "." ) && pin_found)
					{
						parsed_module.push_back( "" );
						pin_found = false;
						continue;
					}
					else if (!starts_with( *tok_module_iter, "." ) && !pin_found)
					{
						if ((*tok_module_iter).at( 0 ) == ';')
						{
							/* After Semicolon check */
							string temp_buf = buf.substr( buf.find_first_of( ";" ) + 1, buf.size() );

							char_separator<char> sep_buf( " \t;" );
							tokenizer tokens_buf( temp_buf, sep_buf );

							tokenizer::iterator tok_buf_iter = tokens_buf.begin();

							if (tok_buf_iter != tokens_buf.end())
							{
								if (!starts_with( *tok_buf_iter, "//" ))
								{
									cout << "The verilog parser cannot continue parsing the same line, after reaching semicolon at line "
											<< temp_module->getLines()[index] << " of the file " << temp_module->getFileName() << ".\n";
									cout << "Terminating.\n";
									exit( -1 );
								}
							}
							/*************************/

							return;
						}
						else
						{
							cout << "Required pin name at line " << temp_module->getLines()[index] << " of the file " << temp_module->getFileName()
									<< ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
					}
					else if (!starts_with( *tok_module_iter, "." ) && pin_found)
					{
						unsigned found = (*tok_iter3).find( ";" );

						if (found > (*tok_iter3).size())
						{
							string name = *tok_iter3;

							if (regex_match( name, regex( "(([A-Za-z_]+)([A-Za-z0-9_]*)|((1'b)(0|1)))" ) ))
							{
								parsed_module.push_back( name );
							}
							else if (regex_match( name, regex( "([A-Za-z_]+)([A-Za-z0-9_]*)(\\[)([0-9]+)(\\])" ) ))
							{
								char_separator<char> bus( "[]" );
								tokenizer bus_token( name, bus );

								tokenizer::iterator bus_iter = bus_token.begin();

								string temp_name = *bus_iter;
								bus_iter++;

								string id = to_string( stoi( *bus_iter ) );

								parsed_module.push_back( temp_name + "[" + id + "]" );
							}
							else
							{
								cout << "Invalid wire name " << name << " at line " << temp_module->getLines()[index] << " of the file "
										<< temp_module->getFileName() << ".\n";
								cout << "Terminating.\n";
								exit( -1 );
							}
						}
						else
						{
							if ((*tok_iter3).at( 0 ) != ';')
							{
								string name = (*tok_iter3).substr( 0, found );

								if (regex_match( name, regex( "(([A-Za-z_]+)([A-Za-z0-9_]*)|((1'b)(0|1)))" ) ))
								{
									parsed_module.push_back( name );
								}
								else if (regex_match( name, regex( "([A-Za-z_]+)([A-Za-z0-9_]*)(\\[)([0-9]+)(\\])" ) ))
								{
									char_separator<char> bus( "[]" );
									tokenizer bus_token( name, bus );

									tokenizer::iterator bus_iter = bus_token.begin();

									string temp_name = *bus_iter;
									bus_iter++;

									string id = to_string( stoi( *bus_iter ) );

									parsed_module.push_back( temp_name + "[" + id + "]" );
								}
								else
								{
									cout << "Invalid wire name " << name << " at line " << temp_module->getLines()[index] << " of the file "
											<< temp_module->getFileName() << ".\n";
									cout << "Terminating.\n";
									exit( -1 );
								}
							}
							else
							{
								parsed_module.push_back( "" );
							}

							/* After Semicolon check */
							string temp_buf = buf.substr( buf.find_first_of( ";" ) + 1, buf.size() );

							char_separator<char> sep_buf( " \t;" );
							tokenizer tokens_buf( temp_buf, sep_buf );

							tokenizer::iterator tok_buf_iter = tokens_buf.begin();

							if (tok_buf_iter != tokens_buf.end())
							{
								if (!starts_with( *tok_buf_iter, "//" ))
								{
									cout << "The verilog parser cannot continue parsing the same line, after reaching semicolon at line "
											<< temp_module->getLines()[index] << " of the file " << temp_module->getFileName() << ".\n";
									cout << "Terminating.\n";
									exit( -1 );
								}
							}
							/*************************/

							return;
						}
						pin_found = false;
					}
				}
			}

			tok_module_iter++;
		}

		index++;

		if (index >= temp_module->getCode().size())
		{
			cout << "Semicolon not found for module declaration at line " << temp_module->getLines()[index - 1] << " of the file "
					<< temp_module->getFileName() << ".\n";
			cout << "Terminating.\n";
			exit( -1 );
		}

		buf = temp_module->getCode()[index];

		tokenizer tokens_module( buf, sep_module );

		tok_module_iter = tokens_module.begin();
	} while (true);

}

void System::parse_wire_instance_module(Module* temp_module, unsigned int& index, vector<string>& parsed_wires)
{

	string buf = temp_module->getCode()[index];

	typedef tokenizer<char_separator<char> > tokenizer;

	char_separator<char> sep_wire( " ,\t" );
	tokenizer tokens_wire( buf, sep_wire );

	tokenizer::iterator tok_wire_iter = tokens_wire.begin();

	tok_wire_iter++;

	bool found_bus = false;
	int number;

	do
	{
		while (tok_wire_iter != tokens_wire.end())
		{

			if (starts_with( *tok_wire_iter, "//" ))
			{
				index++;

				if (index >= temp_module->getCode().size())
				{
					cout << "Semicolon not found for wire declaration at line " << temp_module->getLines()[index - 1] << " of the file "
							<< temp_module->getFileName() << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}

				buf = temp_module->getCode()[index];

				tokenizer tokens_wire( buf, sep_wire );

				tok_wire_iter = tokens_wire.begin();

				continue;
			}
			else if (starts_with( *tok_wire_iter, "/*" ))
			{
				cout << "Comment found inside wire declaration at line " << temp_module->getLines()[index] << " of the file "
						<< temp_module->getFileName() << ".\n";
				cout << "Terminating.\n";
				exit( -1 );
			}

			unsigned found = (*tok_wire_iter).find( ";" );

			if (found > (*tok_wire_iter).size())
			{
				string name = *tok_wire_iter;

				if (regex_match( name, regex( "([A-Za-z_]+)([A-Za-z0-9_]*)" ) ))
				{
					if (found_bus)
					{
						for (unsigned int i = 0; i <= number; i++)
						{
							string temp = name + "[" + to_string( i ) + "]";
							parsed_wires.push_back( temp );
						}

						found_bus = false;
					}
					else
					{
						parsed_wires.push_back( name );
					}
				}
				else if (regex_match( name, regex( "(\\[)([0-9]+)(:0\\])" ) ))
				{
					if (found_bus == false)
					{
						char_separator<char> sep_bus( "[:" );
						tokenizer tokens_bus( name, sep_bus );

						tokenizer::iterator tokens_bus_iter = tokens_bus.begin();

						number = stoi( (*tokens_bus_iter) );

						found_bus = true;
					}
					else
					{
						cout << "Bus declaration " << name << " found before a bus name at line " << temp_module->getLines()[index] << " of the file "
								<< temp_module->getFileName() << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}
				}
				else
				{
					cout << "Invalid wire name " << name << " at line " << temp_module->getLines()[index] << " of the file "
							<< temp_module->getFileName() << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}
			}
			else
			{
				if ((*tok_wire_iter).at( 0 ) != ';')
				{
					string name = (*tok_wire_iter).substr( 0, found );

					if (regex_match( name, regex( "([A-Za-z_]+)([A-Za-z0-9_]*)" ) ))
					{
						if (found_bus)
						{
							for (unsigned int i = 0; i <= number; i++)
							{
								string temp = name + "[" + to_string( i ) + "]";
								parsed_wires.push_back( temp );
							}

							found_bus = false;
						}
						else
						{
							parsed_wires.push_back( name );
						}
					}
					else if (regex_match( name, regex( "(\\[)([0-9]+)(:0\\])" ) ))
					{
						if (found_bus == false)
						{
							char_separator<char> sep_bus( "[:" );
							tokenizer tokens_bus( name, sep_bus );

							tokenizer::iterator tokens_bus_iter = tokens_bus.begin();

							number = stoi( (*tokens_bus_iter) );

							found_bus = true;
						}
						else
						{
							cout << "Bus declaration " << name << " found before a bus name at line " << temp_module->getLines()[index]
									<< " of the file " << temp_module->getFileName() << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
					}
					else
					{
						cout << "Invalid wire name " << name << " at line " << temp_module->getLines()[index] << " of the file "
								<< temp_module->getFileName() << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}
				}

				if (found_bus)
				{
					cout << "Required bus name at line " << temp_module->getLines()[index] << " of the file " << temp_module->getFileName() << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}

				/* After Semicolon check */
				string temp_buf = buf.substr( buf.find_first_of( ";" ) + 1, buf.size() );

				char_separator<char> sep_buf( " \t;" );
				tokenizer tokens_buf( temp_buf, sep_buf );

				tokenizer::iterator tok_buf_iter = tokens_buf.begin();

				if (tok_buf_iter != tokens_buf.end())
				{
					if (!starts_with( *tok_buf_iter, "//" ))
					{
						cout << "The verilog parser cannot continue parsing the same line, after reaching semicolon at line "
								<< temp_module->getLines()[index] << " of the file " << temp_module->getFileName() << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}
				}
				/*************************/

				return;
			}

			tok_wire_iter++;
		}

		index++;

		if (index >= temp_module->getCode().size())
		{
			cout << "Semicolon not found for wire declaration at line " << temp_module->getLines()[index - 1] << " of the file "
					<< temp_module->getFileName() << ".\n";
			cout << "Terminating.\n";
			exit( -1 );
		}

		buf = temp_module->getCode()[index];

		tokenizer tokens_wire( buf, sep_wire );

		tok_wire_iter = tokens_wire.begin();
	} while (true);

}

void System::parse_input_instance_module(Module* temp_module, unsigned int& index, vector<string>& parsed_inputs)
{

	string buf = temp_module->getCode()[index];

	typedef tokenizer<char_separator<char> > tokenizer;

	char_separator<char> sep_inputs( " ,\t" );
	tokenizer tokens_inputs( buf, sep_inputs );

	tokenizer::iterator tok_inputs_iter = tokens_inputs.begin();

	tok_inputs_iter++;

	bool found_bus = false;
	int number;

	do
	{
		while (tok_inputs_iter != tokens_inputs.end())
		{

			if (starts_with( *tok_inputs_iter, "//" ))
			{
				index++;

				if (index >= temp_module->getCode().size())
				{
					cout << "Semicolon not found for input declaration at line " << temp_module->getLines()[index - 1] << " of the file "
							<< temp_module->getFileName() << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}

				buf = temp_module->getCode()[index];

				tokenizer tokens_inputs( buf, sep_inputs );

				tok_inputs_iter = tokens_inputs.begin();

				continue;
			}
			else if (starts_with( *tok_inputs_iter, "/*" ))
			{
				cout << "Comment found inside input declaration at line " << temp_module->getLines()[index] << " of the file "
						<< temp_module->getFileName() << ".\n";
				cout << "Terminating.\n";
				exit( -1 );
			}

			unsigned found = (*tok_inputs_iter).find( ";" );

			if (found > (*tok_inputs_iter).size())
			{
				string name = *tok_inputs_iter;

				if (regex_match( name, regex( "([A-Za-z_]+)([A-Za-z0-9_]*)" ) ))
				{
					if (found_bus)
					{
						for (unsigned int i = 0; i <= number; i++)
						{
							string temp = name + "[" + to_string( i ) + "]";
							parsed_inputs.push_back( temp );
						}

						found_bus = false;
					}
					else
					{
						parsed_inputs.push_back( name );
					}
				}
				/*else if (regex_match( name, regex( "(\\[)([0-9]+)(:0\\])" ) ))
				 {
				 if (found_bus == false)
				 {
				 char_separator<char> sep_bus( "[:" );
				 tokenizer tokens_bus( name, sep_bus );

				 tokenizer::iterator tokens_bus_iter = tokens_bus.begin();

				 number = stoi( (*tokens_bus_iter) );

				 found_bus = true;
				 }
				 else
				 {
				 cout << "Bus declaration " << name << " found before a bus name at line " << temp_module->getLines()[index] << " of the file "
				 << temp_module->getFileName() << ".\n";
				 cout << "Terminating.\n";
				 exit( -1 );
				 }
				 }*/
				else
				{
					cout << "Invalid input name " << name << " at line " << temp_module->getLines()[index] << " of the file "
							<< temp_module->getFileName() << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}
			}
			else
			{
				if ((*tok_inputs_iter).at( 0 ) != ';')
				{
					string name = (*tok_inputs_iter).substr( 0, found );

					if (regex_match( name, regex( "([A-Za-z_]+)([A-Za-z0-9_]*)" ) ))
					{
						if (found_bus)
						{
							for (unsigned int i = 0; i <= number; i++)
							{
								string temp = name + "[" + to_string( i ) + "]";
								parsed_inputs.push_back( temp );
							}

							found_bus = false;
						}
						else
						{
							parsed_inputs.push_back( name );
						}
					}
					/*else if (regex_match( name, regex( "(\\[)([0-9]+)(:0\\])" ) ))
					 {
					 if (found_bus == false)
					 {
					 char_separator<char> sep_bus( "[:" );
					 tokenizer tokens_bus( name, sep_bus );

					 tokenizer::iterator tokens_bus_iter = tokens_bus.begin();

					 number = stoi( (*tokens_bus_iter) );

					 found_bus = true;
					 }
					 else
					 {
					 cout << "Bus declaration " << name << " found before a bus name at line " << temp_module->getLines()[index]
					 << " of the file " << temp_module->getFileName() << ".\n";
					 cout << "Terminating.\n";
					 exit( -1 );
					 }
					 }*/
					else
					{
						cout << "Invalid input name " << name << " at line " << temp_module->getLines()[index] << " of the file "
								<< temp_module->getFileName() << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}
				}

				if (found_bus)
				{
					cout << "Required bus name at line " << temp_module->getLines()[index] << " of the file " << temp_module->getFileName() << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}

				/* After Semicolon check */
				string temp_buf = buf.substr( buf.find_first_of( ";" ) + 1, buf.size() );

				char_separator<char> sep_buf( " \t;" );
				tokenizer tokens_buf( temp_buf, sep_buf );

				tokenizer::iterator tok_buf_iter = tokens_buf.begin();

				if (tok_buf_iter != tokens_buf.end())
				{
					if (!starts_with( *tok_buf_iter, "//" ))
					{
						cout << "The verilog parser cannot continue parsing the same line, after reaching semicolon at line "
								<< temp_module->getLines()[index] << " of the file " << temp_module->getFileName() << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}
				}
				/*************************/

				return;
			}

			tok_inputs_iter++;
		}

		index++;

		if (index >= temp_module->getCode().size())
		{
			cout << "Semicolon not found for input declaration at line " << temp_module->getLines()[index - 1] << " of the file "
					<< temp_module->getFileName() << ".\n";
			cout << "Terminating.\n";
			exit( -1 );
		}

		buf = temp_module->getCode()[index];

		tokenizer tokens_inputs( buf, sep_inputs );

		tok_inputs_iter = tokens_inputs.begin();
	} while (true);

}

void System::parse_output_instance_module(Module* temp_module, unsigned int& index, vector<string>& parsed_outputs)
{

	string buf = temp_module->getCode()[index];

	typedef tokenizer<char_separator<char> > tokenizer;

	char_separator<char> sep_outputs( " ,\t" );
	tokenizer tokens_outputs( buf, sep_outputs );

	tokenizer::iterator tok_outputs_iter = tokens_outputs.begin();

	tok_outputs_iter++;

	bool found_bus = false;
	int number;

	do
	{
		while (tok_outputs_iter != tokens_outputs.end())
		{

			if (starts_with( *tok_outputs_iter, "//" ))
			{
				index++;

				if (index >= temp_module->getCode().size())
				{
					cout << "Semicolon not found for output declaration at line " << temp_module->getLines()[index - 1] << " of the file "
							<< temp_module->getFileName() << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}

				buf = temp_module->getCode()[index];

				tokenizer tokens_outputs( buf, sep_outputs );

				tok_outputs_iter = tokens_outputs.begin();

				continue;
			}
			else if (starts_with( *tok_outputs_iter, "/*" ))
			{
				cout << "Comment found inside output declaration at line " << temp_module->getLines()[index] << " of the file "
						<< temp_module->getFileName() << ".\n";
				cout << "Terminating.\n";
				exit( -1 );
			}

			unsigned found = (*tok_outputs_iter).find( ";" );

			if (found > (*tok_outputs_iter).size())
			{
				string name = *tok_outputs_iter;

				if (regex_match( name, regex( "([A-Za-z_]+)([A-Za-z0-9_]*)" ) ))
				{
					if (found_bus)
					{
						for (unsigned int i = 0; i <= number; i++)
						{
							string temp = name + "[" + to_string( i ) + "]";
							parsed_outputs.push_back( temp );
						}

						found_bus = false;
					}
					else
					{
						parsed_outputs.push_back( name );
					}
				}
				/*else if (regex_match( name, regex( "(\\[)([0-9]+)(:0\\])" ) ))
				 {
				 if (found_bus == false)
				 {
				 char_separator<char> sep_bus( "[:" );
				 tokenizer tokens_bus( name, sep_bus );

				 tokenizer::iterator tokens_bus_iter = tokens_bus.begin();

				 number = stoi( (*tokens_bus_iter) );

				 found_bus = true;
				 }
				 else
				 {
				 cout << "Bus declaration " << name << " found before a bus name at line " << temp_module->getLines()[index] << " of the file "
				 << temp_module->getFileName() << ".\n";
				 cout << "Terminating.\n";
				 exit( -1 );
				 }
				 }*/
				else
				{
					cout << "Invalid output name " << name << " at line " << temp_module->getLines()[index] << " of the file "
							<< temp_module->getFileName() << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}
			}
			else
			{
				if ((*tok_outputs_iter).at( 0 ) != ';')
				{
					string name = (*tok_outputs_iter).substr( 0, found );

					if (regex_match( name, regex( "([A-Za-z_]+)([A-Za-z0-9_]*)" ) ))
					{
						if (found_bus)
						{
							for (unsigned int i = 0; i <= number; i++)
							{
								string temp = name + "[" + to_string( i ) + "]";
								parsed_outputs.push_back( temp );
							}

							found_bus = false;
						}
						else
						{
							parsed_outputs.push_back( name );
						}
					}
					/*else if (regex_match( name, regex( "(\\[)([0-9]+)(:0\\])" ) ))
					 {
					 if (found_bus == false)
					 {
					 char_separator<char> sep_bus( "[:" );
					 tokenizer tokens_bus( name, sep_bus );

					 tokenizer::iterator tokens_bus_iter = tokens_bus.begin();

					 number = stoi( (*tokens_bus_iter) );

					 found_bus = true;
					 }
					 else
					 {
					 cout << "Bus declaration " << name << " found before a bus name at line " << temp_module->getLines()[index]
					 << " of the file " << temp_module->getFileName() << ".\n";
					 cout << "Terminating.\n";
					 exit( -1 );
					 }
					 }*/
					else
					{
						cout << "Invalid output name " << name << " at line " << temp_module->getLines()[index] << " of the file "
								<< temp_module->getFileName() << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

				}

				if (found_bus)
				{
					cout << "Required bus name at line " << temp_module->getLines()[index] << " of the file " << temp_module->getFileName() << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}

				/* After Semicolon check */
				string temp_buf = buf.substr( buf.find_first_of( ";" ) + 1, buf.size() );

				char_separator<char> sep_buf( " \t;" );
				tokenizer tokens_buf( temp_buf, sep_buf );

				tokenizer::iterator tok_buf_iter = tokens_buf.begin();

				if (tok_buf_iter != tokens_buf.end())
				{
					if (!starts_with( *tok_buf_iter, "//" ))
					{
						cout << "The verilog parser cannot continue parsing the same line, after reaching semicolon at line "
								<< temp_module->getLines()[index] << " of the file " << temp_module->getFileName() << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}
				}
				/*************************/

				return;
			}

			tok_outputs_iter++;
		}

		index++;

		if (index >= temp_module->getCode().size())
		{
			cout << "Semicolon not found for output declaration at line " << temp_module->getLines()[index - 1] << " of the file "
					<< temp_module->getFileName() << ".\n";
			cout << "Terminating.\n";
			exit( -1 );
		}

		buf = temp_module->getCode()[index];

		tokenizer tokens_outputs( buf, sep_outputs );

		tok_outputs_iter = tokens_outputs.begin();
	} while (true);

}

void System::parse_assign_instance_module(Module* temp_module, unsigned int& index, vector<string>& assigns)
{
	string buf = temp_module->getCode()[index];

	typedef tokenizer<char_separator<char> > tokenizer;

	char_separator<char> sep_assign( " =\t" );
	tokenizer tokens_assign( buf, sep_assign );

	tokenizer::iterator tok_assign_iter = tokens_assign.begin();

	tok_assign_iter++;

	bool found_left = false;
	bool found_right = false;

	do
	{
		while (tok_assign_iter != tokens_assign.end())
		{
			if (starts_with( *tok_assign_iter, "//" ))
			{
				index++;

				if (index >= temp_module->getCode().size())
				{
					cout << "Semicolon not found for assign declaration at line " << temp_module->getLines()[index - 1] << " of the file "
							<< temp_module->getFileName() << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}

				buf = temp_module->getCode()[index];

				tokenizer tokens_assign( buf, sep_assign );

				tok_assign_iter = tokens_assign.begin();

				continue;
			}
			else if (starts_with( *tok_assign_iter, "/*" ))
			{
				cout << "Comment found inside assign declaration at line " << temp_module->getLines()[index] << " of the file "
						<< temp_module->getFileName() << ".\n";
				cout << "Terminating.\n";
				exit( -1 );
			}

			unsigned found = (*tok_assign_iter).find( ";" );

			if (!found_left)
			{
				if (found > (*tok_assign_iter).size())
				{
					string name = *tok_assign_iter;

					if (regex_match( name, regex( "(([A-Za-z_]+)([A-Za-z0-9_]*)|((1'b)(0|1)))" ) ))
					{
						assigns.push_back( name );
						found_left = true;
					}
					else if (regex_match( name, regex( "([A-Za-z_]+)([A-Za-z0-9_]*)(\\[)([0-9]+)(\\])" ) ))
					{
						char_separator<char> bus( "[]" );
						tokenizer bus_token( name, bus );

						tokenizer::iterator bus_iter = bus_token.begin();

						string temp_name = *bus_iter;
						bus_iter++;
						string id = to_string( stoi( *bus_iter ) );

						assigns.push_back( temp_name + "[" + id + "]" );
						found_left = true;
					}
					else
					{
						cout << "Invalid assigned wire name " << name << " at line " << temp_module->getLines()[index] << " of the file "
								<< temp_module->getFileName() << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}
				}
				else
				{
					cout << "Semicolon found for assign declaration at line " << temp_module->getLines()[index] << " of the file "
							<< temp_module->getFileName() << ".\n";
					cout << "before left side assignment declaration.\n";
					cout << "Terminating.\n";
					exit( -1 );
				}
			}
			else
			{
				if (found > (*tok_assign_iter).size())
				{
					string name = *tok_assign_iter;

					if (regex_match( name, regex( "(([A-Za-z_]+)([A-Za-z0-9_]*)|((1'b)(0|1)))" ) ))
					{
						assigns.push_back( name );
						found_right = true;
					}
					else if (regex_match( name, regex( "([A-Za-z_]+)([A-Za-z0-9_]*)(\\[)([0-9]+)(\\])" ) ))
					{
						char_separator<char> bus( "[]" );
						tokenizer bus_token( name, bus );

						tokenizer::iterator bus_iter = bus_token.begin();

						string temp_name = *bus_iter;
						bus_iter++;
						string id = to_string( stoi( *bus_iter ) );

						assigns.push_back( temp_name + "[" + id + "]" );
						found_right = true;
					}
					else
					{
						cout << "Invalid assigned wire name " << name << " at line " << temp_module->getLines()[index] << " of the file "
								<< temp_module->getFileName() << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}
				}
				else
				{
					if ((*tok_assign_iter).at( 0 ) != ';')
					{
						string name = (*tok_assign_iter).substr( 0, found );

						if (regex_match( name, regex( "(([A-Za-z_]+)([A-Za-z0-9_]*)|((1'b)(0|1)))" ) ))
						{
							assigns.push_back( name );
							found_right = true;
						}
						else if (regex_match( name, regex( "([A-Za-z_]+)([A-Za-z0-9_]*)(\\[)([0-9]+)(\\])" ) ))
						{
							char_separator<char> bus( "[]" );
							tokenizer bus_token( name, bus );

							tokenizer::iterator bus_iter = bus_token.begin();

							string temp_name = *bus_iter;
							bus_iter++;
							string id = to_string( stoi( *bus_iter ) );

							assigns.push_back( temp_name + "[" + id + "]" );
							found_right = true;
						}
						else
						{
							cout << "Invalid assigned wire name " << name << " at line " << temp_module->getLines()[index] << " of the file "
									<< temp_module->getFileName() << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
					}
					else if (!found_right)
					{
						cout << "Semicolon found for assign declaration at line " << temp_module->getLines()[index] << " of the file "
								<< temp_module->getFileName() << ".\n";
						cout << "before right side assignment declaration.\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					/* After Semicolon check */
					string temp_buf = buf.substr( buf.find_first_of( ";" ) + 1, buf.size() );

					char_separator<char> sep_buf( " \t;" );
					tokenizer tokens_buf( temp_buf, sep_buf );

					tokenizer::iterator tok_buf_iter = tokens_buf.begin();

					if (tok_buf_iter != tokens_buf.end())
					{
						if (!starts_with( *tok_buf_iter, "//" ))
						{
							cout << "The verilog parser cannot continue parsing the same line, after reaching semicolon at line "
									<< temp_module->getLines()[index] << " of the file " << temp_module->getFileName() << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
					}
					/*************************/

					return;
				}
			}

			tok_assign_iter++;
		}

		index++;

		if (index >= temp_module->getCode().size())
		{
			cout << "Semicolon not found for assign declaration at line " << temp_module->getLines()[index - 1] << " of the file "
					<< temp_module->getFileName() << ".\n";
			cout << "Terminating.\n";
			exit( -1 );
		}

		buf = temp_module->getCode()[index];

		tokenizer tokens_assign( buf, sep_assign );

		tok_assign_iter = tokens_assign.begin();

	} while (true);

}

void System::parse_cell_instance_module(Module* temp_module, unsigned int& index, string& instance_name, vector<string>& parsed_pins,
		vector<string>& parsed_wires)
{
	string buf = temp_module->getCode()[index];

	typedef tokenizer<char_separator<char> > tokenizer;

	char_separator<char> sep_cell( " ,\t()" );
	tokenizer tokens_cell( buf, sep_cell );

	tokenizer::iterator tok_cell_iter = tokens_cell.begin();

	tok_cell_iter++;

	bool pin_found = false;

	do
	{
		while (tok_cell_iter != tokens_cell.end())
		{
			if (starts_with( *tok_cell_iter, "//" ))
			{
				index++;

				if (index >= temp_module->getCode().size())
				{
					cout << "Semicolon not found for cell declaration at line " << temp_module->getLines()[index - 1] << " of the file "
							<< temp_module->getFileName() << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}

				buf = temp_module->getCode()[index];

				tokenizer tokens_cell( buf, sep_cell );

				tok_cell_iter = tokens_cell.begin();

				continue;
			}
			else if (starts_with( *tok_cell_iter, "/*" ))
			{
				cout << "Comment found inside cell declaration at line " << temp_module->getLines()[index] << " of the file "
						<< temp_module->getFileName() << ".\n";
				cout << "Terminating.\n";
				exit( -1 );
			}

			if (instance_name.empty())
			{
				unsigned found = (*tok_cell_iter).find( ";" );

				if (found > (*tok_cell_iter).size())
				{
					string name = *tok_cell_iter;

					if (regex_match( name, regex( "([A-Za-z_]+)([A-Za-z0-9_]*)" ) ))
					{
						instance_name = name;
					}
					else
					{
						cout << "Invalid cell instance name " << name << " at line " << temp_module->getLines()[index] << " of the file "
								<< temp_module->getFileName() << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}
				}
				else
				{
					if ((*tok_cell_iter).at( 0 ) != ';')
					{
						string name = (*tok_cell_iter).substr( 0, found );

						if (regex_match( name, regex( "([A-Za-z_]+)([A-Za-z0-9_]*)" ) ))
						{
							instance_name = name;
						}
						else
						{
							cout << "Invalid cell instance name " << name << " at line " << temp_module->getLines()[index] << " of the file "
									<< temp_module->getFileName() << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
					}

					/* After Semicolon check */
					string temp_buf = buf.substr( buf.find_first_of( ";" ) + 1, buf.size() );

					char_separator<char> sep_buf( " \t;" );
					tokenizer tokens_buf( temp_buf, sep_buf );

					tokenizer::iterator tok_buf_iter = tokens_buf.begin();

					if (tok_buf_iter != tokens_buf.end())
					{
						if (!starts_with( *tok_buf_iter, "//" ))
						{
							cout << "The verilog parser cannot continue parsing the same line, after reaching semicolon at line "
									<< temp_module->getLines()[index] << " of the file " << temp_module->getFileName() << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
					}
					/*************************/

					return;
				}
			}
			else
			{
				char_separator<char> sep2( "." );
				tokenizer tokens3( *tok_cell_iter, sep2 );

				tokenizer::iterator tok_iter3 = tokens3.begin();

				if (starts_with( *tok_cell_iter, "." ) && !pin_found)
				{
					if (tok_iter3 == tokens3.end())
					{
						cout << "Required pin name at line " << temp_module->getLines()[index] << " of the file " << temp_module->getFileName()
								<< ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					pin_found = true;

					unsigned found = (*tok_iter3).find( ";" );

					if (found > (*tok_iter3).size())
					{
						string name = *tok_iter3;

						if (regex_match( name, regex( "([A-Za-z_]+)([A-Za-z0-9_]*)" ) ))
						{
							parsed_pins.push_back( name );
						}
						else
						{
							cout << "Invalid pin name " << name << " at line " << temp_module->getLines()[index] << " of the file "
									<< temp_module->getFileName() << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
					}
					else
					{
						cout << "Required wire name at line " << temp_module->getLines()[index] << " of the file " << temp_module->getFileName()
								<< ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}
				}
				else if (starts_with( *tok_cell_iter, "." ) && pin_found)
				{
					parsed_wires.push_back( "" );
					pin_found = false;
					continue;
				}
				else if (!starts_with( *tok_cell_iter, "." ) && !pin_found)
				{
					if ((*tok_cell_iter).at( 0 ) == ';')
					{
						/* After Semicolon check */
						string temp_buf = buf.substr( buf.find_first_of( ";" ) + 1, buf.size() );

						char_separator<char> sep_buf( " \t;" );
						tokenizer tokens_buf( temp_buf, sep_buf );

						tokenizer::iterator tok_buf_iter = tokens_buf.begin();

						if (tok_buf_iter != tokens_buf.end())
						{
							if (!starts_with( *tok_buf_iter, "//" ))
							{
								cout << "The verilog parser cannot continue parsing the same line, after reaching semicolon at line "
										<< temp_module->getLines()[index] << " of the file " << temp_module->getFileName() << ".\n";
								cout << "Terminating.\n";
								exit( -1 );
							}
						}
						/*************************/

						return;
					}
					else
					{
						cout << "Required pin name at line " << temp_module->getLines()[index] << " of the file " << temp_module->getFileName()
								<< ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}
				}
				else if (!starts_with( *tok_cell_iter, "." ) && pin_found)
				{
					unsigned found = (*tok_iter3).find( ";" );

					if (found > (*tok_iter3).size())
					{
						string name = *tok_iter3;

						if (regex_match( name, regex( "(([A-Za-z_]+)([A-Za-z0-9_]*)|((1'b)(0|1)))" ) ))
						{
							parsed_wires.push_back( name );
						}
						else if (regex_match( name, regex( "([A-Za-z_]+)([A-Za-z0-9_]*)(\\[)([0-9]+)(\\])" ) ))
						{
							char_separator<char> bus( "[]" );
							tokenizer bus_token( name, bus );

							tokenizer::iterator bus_iter = bus_token.begin();

							string temp_name = *bus_iter;
							bus_iter++;
							string id = to_string( stoi( *bus_iter ) );

							parsed_wires.push_back( temp_name + "[" + id + "]" );
						}
						else
						{
							cout << "Invalid wire name " << name << " at line " << temp_module->getLines()[index] << " of the file "
									<< temp_module->getFileName() << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
					}
					else
					{
						if ((*tok_iter3).at( 0 ) != ';')
						{
							string name = (*tok_iter3).substr( 0, found );

							if (regex_match( name, regex( "(([A-Za-z_]+)([A-Za-z0-9_]*)|((1'b)(0|1)))" ) ))
							{
								parsed_wires.push_back( name );
							}
							else if (regex_match( name, regex( "([A-Za-z_]+)([A-Za-z0-9_]*)(\\[)([0-9]+)(\\])" ) ))
							{
								char_separator<char> bus( "[]" );
								tokenizer bus_token( name, bus );

								tokenizer::iterator bus_iter = bus_token.begin();

								string temp_name = *bus_iter;
								bus_iter++;
								string id = to_string( stoi( *bus_iter ) );

								parsed_wires.push_back( temp_name + "[" + id + "]" );
							}
							else
							{
								cout << "Invalid wire name " << name << " at line " << temp_module->getLines()[index] << " of the file "
										<< temp_module->getFileName() << ".\n";
								cout << "Terminating.\n";
								exit( -1 );
							}
						}
						else
						{
							parsed_wires.push_back( "" );
						}

						/* After Semicolon check */
						string temp_buf = buf.substr( buf.find_first_of( ";" ) + 1, buf.size() );

						char_separator<char> sep_buf( " \t;" );
						tokenizer tokens_buf( temp_buf, sep_buf );

						tokenizer::iterator tok_buf_iter = tokens_buf.begin();

						if (tok_buf_iter != tokens_buf.end())
						{
							if (!starts_with( *tok_buf_iter, "//" ))
							{
								cout << "The verilog parser cannot continue parsing the same line, after reaching semicolon at line "
										<< temp_module->getLines()[index] << " of the file " << temp_module->getFileName() << ".\n";
								cout << "Terminating.\n";
								exit( -1 );
							}
						}
						/*************************/

						return;
					}
					pin_found = false;
				}
			}

			tok_cell_iter++;

		}

		index++;

		if (index >= temp_module->getCode().size())
		{
			cout << "Semicolon not found for cell declaration at line " << temp_module->getLines()[index - 1] << " of the file "
					<< temp_module->getFileName() << ".\n";
			cout << "Terminating.\n";
			exit( -1 );
		}

		buf = temp_module->getCode()[index];

		tokenizer tokens_cell( buf, sep_cell );

		tok_cell_iter = tokens_cell.begin();
	} while (true);

}

/* End of V File */

void System::creteCellInstances()
{
	node_map::iterator p;

	typedef tokenizer<char_separator<char> > tokenizer;

	char_separator<char> sep( ":" );

//gather the cell information
	for (p = graph.getInternalNodes().begin(); p != graph.getInternalNodes().end(); ++p)
	{
		tokenizer tokens( p->second->getName(), sep );

		tokenizer::iterator tok_iter = tokens.begin();
		Cellinst * temp = instance_cell_mapping( *tok_iter );

		p->second->setCellInst( temp );

		if (p->second->getPin()->getType() == INPUT)
		{
			temp->addInputPin( p->second );

			if ((( InputPin * ) p->second->getPin())->isClockPin())
			{
				temp->setSequential( true );
			}
		}
		else if (p->second->getPin()->getType() == OUTPUT)
		{
			temp->addOutputPin( p->second );
		}
	}

	cell_no_design = instantiated_cells.size();
}

void System::calcUnitInverterVals()
{

	double c_fall = (( InputPin * ) un_in.cell->getInputs()[0])->getCfall();
	double c_rise = (( InputPin * ) un_in.cell->getInputs()[0])->getCrise();

	un_in.C0 = (c_fall + c_rise) / 2;

	Timing * timing = (( OutputPin * ) un_in.cell->getOutputs()[0])->getTimings().begin()->second;

	double dr = -numeric_limits<double>::max(), df = -numeric_limits<double>::max();

	for (unsigned int i = 0; i < timing->getWhen().size(); i++)
	{
		if (timing->getWhen()[i]->getCellRise() != NULL)
		{
			double temp = Util::bilinearInterpolation( timing->getWhen()[i]->getCellRise(), 0, un_in.C0 );
			dr = temp > dr ? temp : dr;
		}

		if (timing->getWhen()[i]->getCellFall() != NULL)
		{
			double temp = Util::bilinearInterpolation( timing->getWhen()[i]->getCellFall(), 0, un_in.C0 );
			df = temp > df ? temp : df;
		}
	}

	if (df != -numeric_limits<double>::max() && dr != -numeric_limits<double>::max())
	{
		un_in.tau = (df + dr) / 2;
	}
	else if (df == -numeric_limits<double>::max() && dr != -numeric_limits<double>::max())
	{
		un_in.tau = dr;
	}
	else if (dr == -numeric_limits<double>::max() && df != -numeric_limits<double>::max())
	{
		un_in.tau = df;
	}
	else
	{
		un_in.tau = 0;
	}

}

Cellinst * System::instance_cell_mapping(string node_name)
{

	Cellinst * temp = NULL;

	cellinst_map::iterator it = instantiated_cells.lower_bound( node_name );

	if (it != instantiated_cells.end() && (*it).first.compare( node_name ) == 0)
	{
		return (*it).second;
	}
	else
	{
		typedef tokenizer<char_separator<char> > tokenizer;

		char_separator<char> sep2( "<>" );

		tokenizer tokens( node_name, sep2 );

		tokenizer::iterator tok_iter = tokens.begin();

		string cell_name = *tok_iter;

		tok_iter++;

		string instance_name = *tok_iter;

		Cell * cell = cells.find( cell_name )->second;

		temp = new Cellinst( cell, instance_name );

		instantiated_cells.insert( it, cellinst_entry( node_name, temp ) );

		return temp;
	}

}

void System::outputHandler()
{

	if (graph.getReportTiming())
	{
		return;
	}

	ofstream file;
	file.open( (output_path + graph.getName() + "_resized.v").c_str() );

	ofstream file2;
	file2.open( (output_path + graph.getName() + ".scf").c_str() );
	file2.precision( 12 );
	file2 << fixed;

	file << "//Created by CCSopt Resizing tool.\n\n";

	file << "module " << graph.getName() << "_resized" << "(";

//print the module arguments
	for (unsigned int k = 0; k < main_module.getArguments().size(); k++)
	{
		if (k != 0)
		{
			file << ", ";
		}

		file << main_module.getArguments()[k];
	}
	file << ");";

	int last_printed = ERROR_CODE;

	for (unsigned int k = 0; k < main_module.getArguments().size(); k++)
	{
		if (last_printed != main_module.getPortType()[k])
		{
			if (main_module.getPortType()[k] == INPUT)
			{
				if (last_printed == ERROR_CODE)
				{
					file << "\n\n";
				}
				else
				{
					file << ";\n\n";
				}

				file << "  input ";

				if (main_module.getWireType()[k] == 0)
				{
					file << main_module.getArguments()[k];
				}
				else
				{
					file << "[" << main_module.getWireType()[k] - 1 << ":0] " << main_module.getArguments()[k];
				}
			}
			else
			{
				if (last_printed == ERROR_CODE)
				{
					file << "\n\n";
				}
				else
				{
					file << ";\n\n";
				}

				file << "  output ";

				if (main_module.getWireType()[k] == 0)
				{
					file << main_module.getArguments()[k];
				}
				else
				{
					file << "[" << main_module.getWireType()[k] - 1 << ":0] " << main_module.getArguments()[k];
				}
			}

			last_printed = main_module.getPortType()[k];
		}
		else
		{
			if (main_module.getWireType()[k] == 0)
			{
				file << ", " << main_module.getArguments()[k];
			}
			else
			{
				file << ", " << "[" << main_module.getWireType()[k] - 1 << ":0] " << main_module.getArguments()[k];
			}
		}
	}

	if (last_printed == ERROR_CODE)
	{
		file << "\n\n";
	}
	else
	{
		file << ";\n\n";
	}

//print the wire declarations
	file << "  wire ";

	net_map::iterator q;

	bool printed_first = false;

	for (q = wires.begin(); q != wires.end();)
	{
		if (!q->second->isVirtual())
		{
			string temp = q->second->getName();

			if (q != wires.begin() && printed_first)
			{
				file << ", ";
			}

			if (temp.find_first_of( "[" ) < temp.size())
			{
				string prefix = temp.substr( 0, temp.find_first_of( "[" ) + 1 );

				int count = 0;
				for (; q != wires.end() && starts_with( q->second->getName(), prefix ); ++q)
				{
					count++;
				}

				temp = temp.substr( 0, temp.find_first_of( "[" ) );

				temp = "[" + to_string( count - 1 ) + ":0] " + temp;

				replace( temp.begin(), temp.end(), '.', '_' );

				file << temp;

				printed_first = true;
			}
			else
			{
				replace( temp.begin(), temp.end(), '.', '_' );

				file << temp;

				printed_first = true;

				++q;
			}
		}
		else
		{
			++q;
		}
	}
	file << ";\n\n";

//assigns

	for (q = wires.begin(); q != wires.end(); ++q)
	{
		if (!q->second->getAssignedValue().empty())
		{
			string temp = q->second->getName();
			replace( temp.begin(), temp.end(), '.', '_' );

			string temp2 = q->second->getAssignedValue();
			replace( temp2.begin(), temp2.end(), '.', '_' );

			file << "  assign " << temp << " = " << temp2 << ";\n";
		}
	}

	file << "\n";

	int commas;
	int i;

//print the cells
	cellinst_map::iterator r;
	for (r = instantiated_cells.begin(); r != instantiated_cells.end(); ++r)
	{

		file << "  " << r->second->getCell()->getName() << " ";
		file2 << r->second->getInstanceName() << " " << r->second->getScaleFactor() << "\n";

		i = 0;

		commas = r->second->getInputs().size() + r->second->getOutputs().size() - 1;

		string temp = r->second->getInstanceName();
		replace( temp.begin(), temp.end(), '.', '_' );

		file << temp << "(";

		for (unsigned int k = 0; k < r->second->getInputs().size(); k++)
		{
			file << "." << r->second->getInputs()[k]->getPin()->getName() << " (";

			if (r->second->getInputs()[k]->getNet() != NULL)
			{
				string temp = r->second->getInputs()[k]->getNet()->getName();
				replace( temp.begin(), temp.end(), '.', '_' );
				file << temp;
			}
			file << ")";
			if (i < commas)
			{
				file << ", ";
			}
			i++;
		}

		for (unsigned int k = 0; k < r->second->getOutputs().size(); k++)
		{
			file << "." << r->second->getOutputs()[k]->getPin()->getName() << " (";

			if (r->second->getOutputs()[k]->getNet() != NULL)
			{
				string temp = r->second->getOutputs()[k]->getNet()->getName();
				replace( temp.begin(), temp.end(), '.', '_' );
				file << temp;
			}
			file << ")";
			if (i < commas)
			{
				file << ", ";
			}
			i++;
		}

		file << ");\n";

	}

	file << "\nendmodule" << endl;

	file.close();
	file2.close();

}

void System::dotCfgParser()
{

	cout << "\nParsing the config format file...\n\n";

	string unit_inv_name;

	bool rep_timing = false;

	if (!cfg_file.empty())
	{
		ifstream file;
		string buf;

		string working_file;

		unsigned int line_cnt = 1;

		working_file = cfg_file;

		file.open( working_file.c_str() );

		if (!file.good())
		{
			cout << "The file " << working_file << " was not found.\n";
			cout << "Terminating.\n";
			exit( -1 );
		}

		while (true)
		{
			getline( file, buf );

			if (file.eof())
			{

				break;

			}

			typedef tokenizer<char_separator<char> > tokenizer;

			char_separator<char> sep( " \t" );
			tokenizer tokens( buf, sep );

			tokenizer::iterator tok_iter = tokens.begin();

			if (tok_iter != tokens.end())
			{
				if ((*tok_iter).compare( "output_capacitance" ) == 0)
				{
					tok_iter++;

					string output_name;
					string mode;
					double cap;

					if (tok_iter != tokens.end())
					{
						mode = *tok_iter;

						if (!(mode.compare( "-fall" ) == 0 || mode.compare( "-rise" ) == 0))
						{
							cout << "Invalid mode specifier " << mode << " at line " << line_cnt << " of the file " << working_file << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}

					}
					else
					{
						cout << "Required output capacitance mode (-fall / -rise) at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					tok_iter++;
					if (tok_iter != tokens.end())
					{
						output_name = *tok_iter;
					}
					else
					{
						cout << "Required output pin name at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					tok_iter++;

					if (tok_iter != tokens.end())
					{
						if (regex_match( *tok_iter, regex( "([-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?)" ) ))
						{
							cap = stod( (*tok_iter) );
						}
						else
						{
							cout << "Invalid capacitance value at line " << line_cnt << " of the file " << working_file << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}

						if (cap < 0)
						{
							cout << "Capacitance must be greater than -1 at line " << line_cnt << " of the file " << working_file << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
					}
					else
					{
						cout << "Required capacitance at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					tok_iter++;

					if (tok_iter != tokens.end())
					{
						cout << "Expected less inputs at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					if (output_name.compare( "-all" ) == 0)
					{
						node_map::iterator it;

						for (it = getGraph().getOutputNodes().begin(); it != getGraph().getOutputNodes().end(); it++)
						{
							if (mode.compare( "-fall" ) == 0)
							{
								it->second->setUserCapacitanceFall( cap );
								cout << "Setting " << cap << " as fall capacitance to the output node " << it->second->getName() << ".\n";
							}
							else
							{
								it->second->setUserCapacitanceRise( cap );
								cout << "Setting " << cap << " as rise capacitance to the output node " << it->second->getName() << ".\n";
							}
						}
					}
					else
					{
						node_map::iterator it = getGraph().getOutputNodes().find( "<output>:" + output_name );

						if (it != getGraph().getOutputNodes().end())
						{
							if (mode.compare( "-fall" ) == 0)
							{
								it->second->setUserCapacitanceFall( cap );
								cout << "Setting " << cap << " as fall capacitance to the output node " << it->second->getName() << ".\n";
							}
							else
							{
								it->second->setUserCapacitanceRise( cap );
								cout << "Setting " << cap << " as rise capacitance to the output node " << it->second->getName() << ".\n";
							}
						}
						else
						{
							cout << "Output " << output_name << " not found at line " << line_cnt << " of the file " << working_file << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
					}

				}
				else if ((*tok_iter).compare( "input_slew" ) == 0)
				{
					tok_iter++;

					string input_name;
					string mode;
					string mode2;
					double slew;

					if (tok_iter != tokens.end())
					{
						mode = *tok_iter;

						if (!(mode.compare( "-fall" ) == 0 || mode.compare( "-rise" ) == 0))
						{
							cout << "Invalid mode specifier " << mode << " at line " << line_cnt << " of the file " << working_file << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}

					}
					else
					{
						cout << "Required input slew mode (-fall / -rise) at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					tok_iter++;

					if (tok_iter != tokens.end())
					{
						mode2 = *tok_iter;

						if (!(mode2.compare( "-late" ) == 0))
						{
							cout << "Invalid mode specifier " << mode2 << " at line " << line_cnt << " of the file " << working_file << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}

					}
					else
					{
						cout << "Required input slew mode (-late) at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					tok_iter++;

					if (tok_iter != tokens.end())
					{
						input_name = *tok_iter;
					}
					else
					{
						cout << "Required input pin name at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					tok_iter++;

					if (tok_iter != tokens.end())
					{
						if (regex_match( *tok_iter, regex( "([-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?)" ) ))
						{
							slew = stod( (*tok_iter) );
						}
						else
						{
							cout << "Invalid input slew value at line " << line_cnt << " of the file " << working_file << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}

						if (slew < 0)
						{
							cout << "Slew must be greater than -1 at line " << line_cnt << " of the file " << working_file << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
					}
					else
					{
						cout << "Required slew at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					tok_iter++;

					if (tok_iter != tokens.end())
					{
						cout << "Expected less inputs at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					if (input_name.compare( "-all" ) == 0)
					{
						node_map::iterator it;

						for (it = getGraph().getInputNodes().begin(); it != getGraph().getInputNodes().end(); it++)
						{
							if (mode.compare( "-fall" ) == 0)
							{
								it->second->setTrLateFall( slew );
								cout << "Setting " << slew << " as fall late slew to the input node " << it->second->getName() << ".\n";
							}
							else
							{
								it->second->setTrLateRise( slew );
								cout << "Setting " << slew << " as rise late slew to the input node " << it->second->getName() << ".\n";
							}
						}
					}
					else
					{
						node_map::iterator it = getGraph().getInputNodes().find( "<input>:" + input_name );

						if (it != getGraph().getInputNodes().end())
						{
							if (mode.compare( "-fall" ) == 0)
							{
								it->second->setTrLateFall( slew );
								cout << "Setting " << slew << " as fall late slew to the input node " << it->second->getName() << ".\n";
							}
							else
							{
								it->second->setTrLateRise( slew );
								cout << "Setting " << slew << " as rise late slew to the input node " << it->second->getName() << ".\n";
							}
						}
						else
						{
							cout << "Input " << input_name << " not found at line " << line_cnt << " of the file " << working_file << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
					}

				}
				else if ((*tok_iter).compare( "input_at" ) == 0)
				{
					tok_iter++;

					string input_name;
					string mode;
					string mode2;
					double at;

					if (tok_iter != tokens.end())
					{
						mode = *tok_iter;

						if (!(mode.compare( "-fall" ) == 0 || mode.compare( "-rise" ) == 0))
						{
							cout << "Invalid mode specifier " << mode << " at line " << line_cnt << " of the file " << working_file << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}

					}
					else
					{
						cout << "Required input arrival time mode (-fall / -rise) at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					tok_iter++;

					if (tok_iter != tokens.end())
					{
						mode2 = *tok_iter;

						if (!(mode2.compare( "-late" ) == 0))
						{
							cout << "Invalid mode specifier " << mode2 << " at line " << line_cnt << " of the file " << working_file << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}

					}
					else
					{
						cout << "Required input arrival time mode (-late) at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					tok_iter++;

					if (tok_iter != tokens.end())
					{
						input_name = *tok_iter;
					}
					else
					{
						cout << "Required input pin name at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					tok_iter++;

					if (tok_iter != tokens.end())
					{

						if (regex_match( *tok_iter, regex( "([-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?)" ) ))
						{
							at = stod( (*tok_iter) );
						}
						else
						{
							cout << "Invalid arrival time value at line " << line_cnt << " of the file " << working_file << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}

						if (at < 0)
						{
							cout << "Arrival time must be greater than -1 at line " << line_cnt << " of the file " << working_file << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
					}
					else
					{
						cout << "Required arrival time at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					tok_iter++;

					if (tok_iter != tokens.end())
					{
						cout << "Expected less inputs at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					if (input_name.compare( "-all" ) == 0)
					{
						node_map::iterator it;

						for (it = getGraph().getInputNodes().begin(); it != getGraph().getInputNodes().end(); it++)
						{
							if (mode.compare( "-fall" ) == 0)
							{
								it->second->setAtLateFall( at );
								cout << "Setting " << at << " as fall late arrival time to the input node " << it->second->getName() << ".\n";
							}
							else
							{
								it->second->setAtLateRise( at );
								cout << "Setting " << at << " as rise late arrival time to the input node " << it->second->getName() << ".\n";
							}
						}
					}
					else
					{
						node_map::iterator it = getGraph().getInputNodes().find( "<input>:" + input_name );

						if (it != getGraph().getInputNodes().end())
						{
							if (mode.compare( "-fall" ) == 0)
							{
								it->second->setAtLateFall( at );
								cout << "Setting " << at << " as fall late arrival time to the input node " << it->second->getName() << ".\n";
							}
							else
							{
								it->second->setAtLateRise( at );
								cout << "Setting " << at << " as rise late arrival time to the input node " << it->second->getName() << ".\n";
							}
						}
						else
						{
							cout << "Input " << input_name << " not found at line " << line_cnt << " of the file " << working_file << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
					}

				}
				else if ((*tok_iter).compare( "report_timing" ) == 0)
				{
					tok_iter++;

					string token;

					if (tok_iter != tokens.end())
					{
						token = *tok_iter;
					}
					else
					{
						cout << "Required number of paths at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					tok_iter++;

					if (tok_iter != tokens.end())
					{
						cout << "Expected less inputs at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					graph.setReportTiming( true );
					rep_timing = true;

					if (token.compare( "-all" ) == 0)
					{
						graph.setNumPaths( ALL_PATHS );
						cout << "Enabling timing report (worst case) for all paths (Resizing will not be performed).\n";
					}
					else
					{
						int num_paths;

						if (regex_match( token, regex( "([-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?)" ) ))
						{
							num_paths = stoi( token );
						}
						else
						{
							cout << "Invalid path number value at line " << line_cnt << " of the file " << working_file << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}

						if (num_paths < 1)
						{
							cout << "Paths number must be greater that 0 at line " << line_cnt << " of the file " << working_file << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}

						cout << "Enabling timing report (worst case) for " << num_paths << " paths (Resizing will not be performed).\n";

						graph.setNumPaths( num_paths );
					}
				}
				else if ((*tok_iter).compare( "resize_sequential" ) == 0)
				{
					tok_iter++;

					string token;

					if (tok_iter != tokens.end())
					{
						token = *tok_iter;
					}
					else
					{
						cout << "Required resize action at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					tok_iter++;

					if (tok_iter != tokens.end())
					{
						cout << "Expected less inputs at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					if (token.compare( "-no" ) == 0)
					{
						resize_sequential = NO_ACTION;
						cout << "Setting resizing rule for all sequential cells to no action.\n";
					}
					else if (token.compare( "-min" ) == 0)
					{
						resize_sequential = MIN_SIZE;
						cout << "Setting resizing rule for all sequential cells to minimun size.\n";
					}
					else
					{
						cout << "Invalid resize action found at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}
				}
				else if ((*tok_iter).compare( "path_type" ) == 0)
				{
					tok_iter++;

					string token;

					if (tok_iter != tokens.end())
					{
						token = *tok_iter;
					}
					else
					{
						cout << "Required path type at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					tok_iter++;

					if (tok_iter != tokens.end())
					{
						cout << "Expected less inputs at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					if (token.compare( "-out" ) == 0)
					{
						graph.setPathType( OUT );
						cout << "Setting path classification, Input to Output.\n";
					}
					else if (token.compare( "-all" ) == 0)
					{
						graph.setPathType( ALL );
						cout << "Setting path classification, Input to All.\n";
					}
					else if (token.compare( "-reg" ) == 0)
					{
						graph.setPathType( REG );
						cout << "Setting path classification, Input to Register.\n";
					}
					else
					{
						cout << "Invalid path type found at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}
				}
				else if ((*tok_iter).compare( "preset_clear_arcs" ) == 0)
				{
					tok_iter++;

					string token;

					if (tok_iter != tokens.end())
					{
						token = *tok_iter;
					}
					else
					{
						cout << "Required enable/disable option at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					tok_iter++;

					if (tok_iter != tokens.end())
					{
						cout << "Expected less inputs at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					if (token.compare( "-on" ) == 0)
					{
						graph.setPresetClear( true );
						cout << "Enabling preset/clear timing arcs.\n";
					}
					else if (token.compare( "-off" ) == 0)
					{
						graph.setPresetClear( false );
						cout << "Disabling preset/clear timing arcs.\n";
					}
					else
					{
						cout << "Invalid option found at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}
				}
				else if ((*tok_iter).compare( "upsizing" ) == 0)
				{
					tok_iter++;

					string token;

					if (tok_iter != tokens.end())
					{
						token = *tok_iter;
					}
					else
					{
						cout << "Required enable/disable option at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					tok_iter++;

					if (tok_iter != tokens.end())
					{
						cout << "Expected less inputs at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					if (token.compare( "-on" ) == 0)
					{
						graph.setUpsizingAction( true );
						cout << "Allowing upsizing on cells.\n";
					}
					else if (token.compare( "-off" ) == 0)
					{
						graph.setUpsizingAction( false );
						cout << "Disabling upsizing on cells.\n";
					}
					else
					{
						cout << "Invalid option found at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}
				}
				else if ((*tok_iter).compare( "unit_inverter" ) == 0)
				{
					tok_iter++;

					string token;

					if (tok_iter != tokens.end())
					{
						token = *tok_iter;
					}
					else
					{
						cout << "Required unit inverter name at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					tok_iter++;

					if (tok_iter != tokens.end())
					{
						cout << "Expected less inputs at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					unit_inv_name = token;

				}
				else if ((*tok_iter).compare( "c_unit_lib" ) == 0)
				{
					tok_iter++;

					double value;

					if (tok_iter != tokens.end())
					{
						if (regex_match( *tok_iter, regex( "([-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?)" ) ))
						{
							value = stod( *tok_iter );
						}
						else
						{
							cout << "Invalid unit value at line " << line_cnt << " of the file " << working_file << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}

						if (value != 1 && value != 10 && value != 100)
						{
							cout << "Unit value must be (1 or 10 or 100) at line " << line_cnt << " of the file " << working_file << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
					}
					else
					{
						cout << "Required unit value (1 or 10 or 100) at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					tok_iter++;

					if (tok_iter != tokens.end())
					{
						if ((*tok_iter).compare( "PF" ) == 0)
						{
							lib_cap_units = -12;
						}
						else if ((*tok_iter).compare( "FF" ) == 0)
						{
							lib_cap_units = -15;
						}
						else
						{
							cout << "Invalid unit name at line " << line_cnt << " of the file " << working_file << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
					}
					else
					{
						cout << "Required unit name (PF or FF) at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					tok_iter++;

					if (tok_iter != tokens.end())
					{
						cout << "Expected less inputs at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					if (value == 10)
					{
						lib_cap_units++;
					}
					else if (value == 100)
					{
						lib_cap_units += 2;
					}

					cout << "Setting liberty file capacitance unit to 10^" << lib_cap_units << " farad.\n";
				}
				else if ((*tok_iter).compare( "t_unit_lib" ) == 0)
				{
					tok_iter++;

					double value;

					if (tok_iter != tokens.end())
					{
						if (regex_match( *tok_iter, regex( "([-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?)" ) ))
						{
							value = stod( *tok_iter );
						}
						else
						{
							cout << "Invalid unit value at line " << line_cnt << " of the file " << working_file << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}

						if (value != 1 && value != 10 && value != 100)
						{
							cout << "Unit value must be (1 or 10 or 100) at line " << line_cnt << " of the file " << working_file << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
					}
					else
					{
						cout << "Required unit value (1 or 10 or 100) at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					tok_iter++;

					if (tok_iter != tokens.end())
					{
						if ((*tok_iter).compare( "NS" ) == 0)
						{
							lib_time_units = -9;
						}
						else if ((*tok_iter).compare( "PS" ) == 0)
						{
							lib_time_units = -12;
						}
						else
						{
							cout << "Invalid unit name at line " << line_cnt << " of the file " << working_file << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
					}
					else
					{
						cout << "Required unit name (NS or PS) at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					tok_iter++;

					if (tok_iter != tokens.end())
					{
						cout << "Expected less inputs at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					if (value == 10)
					{
						lib_time_units++;
					}
					else if (value == 100)
					{
						lib_time_units += 2;
					}

					cout << "Setting liberty file time unit to 10^" << lib_time_units << " second.\n";
				}
				else if ((*tok_iter).compare( "max_iterations" ) == 0)
				{

					tok_iter++;

					string token;

					if (tok_iter != tokens.end())
					{
						token = *tok_iter;
					}
					else
					{
						cout << "Required maximum iterations number at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					if (regex_match( token, regex( "([-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?)" ) ))
					{
						max_iterations = stoi( token );
					}
					else
					{
						cout << "Invalid maximum iterations value at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					if (max_iterations < 1)
					{
						cout << "Maximum iterations number must be greater that 0 at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					tok_iter++;

					if (tok_iter != tokens.end())
					{
						cout << "Expected less inputs at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					cout << "Setting maximum iterations number to " << max_iterations << ".\n";

					if (max_iterations % 2 == 0)
					{
						max_iterations++;  // make it an odd number
					}
				}
				else if ((*tok_iter).compare( "cell_delimiters" ) == 0)
				{
					tok_iter++;

					string token;

					if (tok_iter != tokens.end())
					{
						token = *tok_iter;
					}
					else
					{
						cout << "Required cell name delimiters at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					cell_name_delims = token;

					tok_iter++;

					if (tok_iter != tokens.end())
					{
						cout << "Expected less inputs at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					cout << "Setting cell name delimiters {" << cell_name_delims << "}.\n";
				}

				else if ((*tok_iter).compare( "tolerance" ) == 0)
				{
					tok_iter++;

					string token;

					if (tok_iter != tokens.end())
					{
						token = *tok_iter;
					}
					else
					{
						cout << "Required tolerance number at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					if (regex_match( token, regex( "([-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?)" ) ))
					{
						tolerance = stod( token );
					}
					else
					{
						cout << "Invalid tolerance value at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					if (tolerance <= 0)
					{
						cout << "Tolerance number must be greater that 0 at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					tok_iter++;

					if (tok_iter != tokens.end())
					{
						cout << "Expected less inputs at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					cout << "Setting tolerance number to " << tolerance << ".\n";
				}
				else if (starts_with( *tok_iter, "//" ))
				{

				}
				else
				{
					cout << "Invalid command " << *tok_iter << " found at line " << line_cnt << " of the file " << working_file << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}
			}

			line_cnt++;
		}

		file.close();
	}

	node_map::iterator it;

	for (it = getGraph().getOutputNodes().begin(); it != getGraph().getOutputNodes().end(); it++)
	{
		if (it->second->getUserCapacitanceFall() == -1)
		{
			cout << "Error output fall capacitance not assigned for " << it->second->getName() << ".\n";
			cout << "Terminating.\n";
			exit( -1 );
		}

		if (it->second->getUserCapacitanceRise() == -1)
		{
			cout << "Error output rise capacitance not assigned for " << it->second->getName() << ".\n";
			cout << "Terminating.\n";
			exit( -1 );
		}
	}

	for (it = getGraph().getInputNodes().begin(); it != getGraph().getInputNodes().end(); it++)
	{
		if (it->second->getTrLateFall() < 0)
		{
			cout << "Error input fall late slew not assigned for " << it->second->getName() << ".\n";
			cout << "Terminating.\n";
			exit( -1 );
		}

		if (it->second->getTrLateRise() < 0)
		{
			cout << "Error input rise late slew not assigned for " << it->second->getName() << ".\n";
			cout << "Terminating.\n";
			exit( -1 );
		}

		if (it->second->getAtLateFall() < 0)
		{
			cout << "Error input fall late arrival time not assigned for " << it->second->getName() << ".\n";
			cout << "Terminating.\n";
			exit( -1 );
		}

		if (it->second->getAtLateRise() < 0)
		{
			cout << "Error input rise late arrival time not assigned for " << it->second->getName() << ".\n";
			cout << "Terminating.\n";
			exit( -1 );
		}
	}

	if (unit_inv_name.empty())
	{
		cout << "Unit inverter was not specified.\n";
		cout << "Terminating.\n";
		exit( -1 );
	}

	if (lib_cap_units == UNIT_INVALID_VAL || lib_time_units == UNIT_INVALID_VAL)
	{
		cout << "The units for time, and capacitance for the lib file, must be defined.\n";
		cout << "Terminating.\n";
		exit( -1 );
	}

	if (max_iterations == 0 && rep_timing == false)
	{
		cout << "Maximum iterations must be defined.\n";
		cout << "Terminating.\n";
		exit( -1 );
	}

	if (tolerance == 0 && rep_timing == false)
	{
		cout << "Tolerance must be defined.\n";
		cout << "Terminating.\n";
		exit( -1 );
	}

	if (cell_name_delims.empty())
	{
		cout << "Cell name delimiters must be defined.\n";
		cout << "Terminating.\n";
		exit( -1 );
	}

	cell_map::iterator it2 = cells.find( unit_inv_name );

	if (it2 == cells.end())
	{
		cout << "Unit inverter " << unit_inv_name << " was not found in the lib file.\n";
		cout << "Terminating.\n";
		exit( -1 );
	}

	if (it2->second->getInputs().size() > 1 || it2->second->getOutputs().size() > 1)
	{
		cout << "The unit inverter must have one input and one output.\n";
		cout << "Terminating.\n";
		exit( -1 );
	}

	un_in.cell = it2->second;

}

void System::dotSpefParser()
{
	net_map::iterator w;

	for (w = wires.begin(); w != wires.end(); w++)
	{
		if (w->second->getAssignedValue().empty())
		{
			parasitics.push_back( w->second->getParasitic() );
		}
	}

	if (spef_file.empty())
	{
		return;
	}

	cout << "\nParsing the standard parasitic exchange format file...\n\n";

	ifstream file;
	string buf;

	string working_file;

	unsigned int line_cnt = 1;

	working_file = spef_file;

	file.open( working_file.c_str() );

	spef_map mapping;
	string delimiter;
	string divider;
	int cap_units = UNIT_INVALID_VAL;
	int res_units = UNIT_INVALID_VAL;

	double r_scale = -1;
	double c_scale = -1;

	if (!file.good())
	{
		cout << "The file " << working_file << " was not found.\n";
		cout << "Terminating.\n";
		exit( -1 );
	}

	cout << "Ignoring all coupling capacitances in the spef file.\n";

	while (true)
	{
		getline( file, buf );

		if (file.eof())
		{

			break;

		}

		typedef tokenizer<char_separator<char> > tokenizer;

		char_separator<char> sep( " \t" );
		tokenizer tokens( buf, sep );

		tokenizer::iterator tok_iter = tokens.begin();

		if (tok_iter != tokens.end())
		{
			if ((*tok_iter).compare( "*NAME_MAP" ) == 0)
			{
				parse_spef_mapping( file, line_cnt, working_file, mapping );
				continue;
			}
			else if ((*tok_iter).compare( "*DELIMITER" ) == 0)
			{
				tok_iter++;

				if (tok_iter != tokens.end())
				{
					delimiter = *tok_iter;
				}
				else
				{
					cout << "Required delimiter character at line " << line_cnt << " of the file " << working_file << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}

				tok_iter++;

				if (tok_iter != tokens.end())
				{
					cout << "Expected less inputs at line " << line_cnt << " of the file " << working_file << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}
			}
			else if ((*tok_iter).compare( "*DIVIDER" ) == 0)
			{
				tok_iter++;

				if (tok_iter != tokens.end())
				{
					divider = *tok_iter;
				}
				else
				{
					cout << "Required divider character at line " << line_cnt << " of the file " << working_file << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}

				tok_iter++;

				if (tok_iter != tokens.end())
				{
					cout << "Expected less inputs at line " << line_cnt << " of the file " << working_file << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}
			}
			else if ((*tok_iter).compare( "*C_UNIT" ) == 0)
			{
				tok_iter++;

				double value;

				if (tok_iter != tokens.end())
				{
					value = stod( (*tok_iter) );

					if (value != 1 && value != 10 && value != 100)
					{
						cout << "Capacitance unit number must be (1 or 10 or 100) at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}
				}
				else
				{
					cout << "Required capacitance unit at line " << line_cnt << " of the file " << working_file << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}

				tok_iter++;

				if (tok_iter != tokens.end())
				{
					if ((*tok_iter).compare( "PF" ) == 0)
					{
						cap_units = -12;
					}
					else if ((*tok_iter).compare( "FF" ) == 0)
					{
						cap_units = -15;
					}
					else
					{
						cout << "The capacitance unit must be PF or FF at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}
				}
				else
				{
					cout << "Required capacitance unit at line " << line_cnt << " of the file " << working_file << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}

				tok_iter++;

				if (tok_iter != tokens.end())
				{
					cout << "Expected less inputs at line " << line_cnt << " of the file " << working_file << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}

				if (value == 10)
				{
					cap_units++;
				}
				else if (value == 100)
				{
					cap_units += 2;
				}

				c_scale = pow( 10, cap_units - lib_cap_units );
			}
			else if ((*tok_iter).compare( "*R_UNIT" ) == 0)
			{
				tok_iter++;

				double value;

				if (tok_iter != tokens.end())
				{
					value = stod( (*tok_iter) );

					if (value != 1 && value != 10 && value != 100)
					{
						cout << "Resistance unit number must be (1 or 10 or 100) at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}
				}
				else
				{
					cout << "Required resistance unit at line " << line_cnt << " of the file " << working_file << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}

				tok_iter++;

				if (tok_iter != tokens.end())
				{
					if ((*tok_iter).compare( "OHM" ) == 0)
					{
						res_units = 0;
					}
					else if ((*tok_iter).compare( "KOHM" ) == 0)
					{
						res_units = 3;
					}
					else
					{
						cout << "The resistance unit must be OHM or KOHM at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}
				}
				else
				{
					cout << "Required resistance unit at line " << line_cnt << " of the file " << working_file << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}

				tok_iter++;

				if (tok_iter != tokens.end())
				{
					cout << "Expected less inputs at line " << line_cnt << " of the file " << working_file << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}

				if (value == 10)
				{
					res_units++;
				}
				else if (value == 100)
				{
					res_units += 2;
				}

			}
			else if ((*tok_iter).compare( "*D_NET" ) == 0)
			{
				if (delimiter.empty() || divider.empty() || res_units == UNIT_INVALID_VAL || cap_units == UNIT_INVALID_VAL)
				{
					cout << "Missing information in spef file.\n";
					cout << "Terminating.\n";
					exit( -1 );
				}

				string node_name;
				string mapped_name;

				if (r_scale == -1)
				{
					int lib_res_units = lib_time_units - lib_cap_units;
					r_scale = pow( 10, res_units - lib_res_units );
				}

				if (mapping.empty())
				{
					tok_iter++;

					if (tok_iter != tokens.end())
					{
						node_name = *tok_iter;
						mapped_name = *tok_iter;
					}
					else
					{
						cout << "Required node name at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}
				}
				else
				{
					tok_iter++;

					if (tok_iter != tokens.end())
					{
						spef_map::iterator find_it = mapping.find( *tok_iter );

						if (find_it == mapping.end())
						{
							cout << *tok_iter << " was not found in the mapping at line " << line_cnt << " of the file " << working_file << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}

						node_name = find_it->first;
						mapped_name = find_it->second;
					}
					else
					{
						cout << "Required node name at line " << line_cnt << " of the file " << working_file << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}
				}

				tok_iter++;

				if (tok_iter != tokens.end())
				{
					//IGNORE TOTAL CAPACITANCE
				}
				else
				{
					cout << "Required total capacitance at line " << line_cnt << " of the file " << working_file << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}

				tok_iter++;

				if (tok_iter != tokens.end())
				{
					cout << "Expected less inputs at line " << line_cnt << " of the file " << working_file << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}

				net_map::iterator find_it2 = wires.find( mapped_name );

				if (find_it2 == wires.end())
				{
					cout << mapped_name << " was not found in the nets at line " << line_cnt << " of the file " << working_file << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}

				Parasitic* parasitic_net = find_it2->second->getParasitic();

				parasitic_net->setMappedName( node_name );

				//STARTPOINT
				p_node * start = parasitic_net->getStartPoint();
				if (start != NULL && start->graph_node->getPin() == NULL)
				{
					if (mapping.empty())
					{
						parasitic_net->addPoint( start->graph_node->getName(), start );
					}
					else
					{
						spef_map::iterator q = mapping.find( start->graph_node->getName() );

						if (q != mapping.end())
						{
							parasitic_net->addPoint( q->second, start );
						}
						else
						{
							cout << "Missing information in the spef file for the net " << node_name << " (" << mapped_name << "), ";
							cout << "no connection to the port " << start->graph_node->getName() << " found.\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
					}

				}
				else if (start != NULL)
				{
					string instance_name = start->graph_node->getCellInst()->getInstanceName();
					string pin_name = start->graph_node->getPin()->getName();

					if (mapping.empty())
					{
						parasitic_net->addPoint( instance_name + delimiter + pin_name, start );
					}
					else
					{
						spef_map::iterator q = mapping.find( instance_name );

						if (q != mapping.end())
						{
							parasitic_net->addPoint( q->second + delimiter + pin_name, start );
						}
						else
						{
							cout << "Missing information in the spef file for the net " << node_name << " (" << mapped_name << "), ";
							cout << "no connection to the cell instance " << instance_name << " found.\n";
							cout << "Terminating.\n";
							exit( -1 );
						}

					}
				}

				//ENDPOINTS
				for (unsigned int i = 0; i < parasitic_net->getEndPoints().size(); i++)
				{
					p_node * end = parasitic_net->getEndPoints()[i];

					if (end->graph_node->getPin() == NULL)
					{
						if (mapping.empty())
						{
							parasitic_net->addPoint( end->graph_node->getName(), end );
						}
						else
						{
							spef_map::iterator q = mapping.find( end->graph_node->getName() );

							if (q != mapping.end())
							{
								parasitic_net->addPoint( q->second, end );
							}
							else
							{
								cout << "Missing information in the spef file for the net " << node_name << " (" << mapped_name << "), ";
								cout << "no connection to the port " << end->graph_node->getName() << " found.\n";
								cout << "Terminating.\n";
								exit( -1 );
							}
						}
					}
					else
					{
						string instance_name = end->graph_node->getCellInst()->getInstanceName();
						string pin_name = end->graph_node->getPin()->getName();

						if (mapping.empty())
						{
							parasitic_net->addPoint( instance_name + delimiter + pin_name, end );
						}
						else
						{
							spef_map::iterator q = mapping.find( instance_name );

							if (q != mapping.end())
							{
								parasitic_net->addPoint( q->second + delimiter + pin_name, end );
							}
							else
							{
								cout << "Missing information in the spef file for the net " << node_name << " (" << mapped_name << "), ";
								cout << "no connection to the cell instance " << instance_name << " found.\n";
								cout << "Terminating.\n";
								exit( -1 );
							}

						}
					}
				}

				parasitics_found++;

				parse_spef_d_net( file, line_cnt, working_file, node_name, parasitic_net, r_scale, c_scale, delimiter, mapping.empty() );
			}
			else if (starts_with( *tok_iter, "//" ))
			{

			}
			/*else
			 {
			 cout << "Invalid command " << *tok_iter << " found at line " << line_cnt << " of the file " << working_file << ".\n";
			 cout << "Terminating.\n";
			 exit( -1 );
			 }*/
		}

		line_cnt++;
	}

}

void System::parse_spef_mapping(ifstream& file, unsigned int& line_cnt, string file_name, spef_map& mapping)
{
	string buf;

	line_cnt++;

	while (true)
	{
		getline( file, buf );

		if (file.eof())
		{

			break;

		}

		typedef tokenizer<char_separator<char> > tokenizer;

		char_separator<char> sep( " \t" );
		tokenizer tokens( buf, sep );

		tokenizer::iterator tok_iter = tokens.begin();

		if (tok_iter != tokens.end())
		{
			if (regex_match( *tok_iter, regex( "(\\*[0-9]+)" ) ))
			{
				string name = *tok_iter;

				tok_iter++;

				if (tok_iter != tokens.end())
				{
					string name2 = *tok_iter;

					mapping.insert( spef_map_entry( name, name2 ) );
					mapping.insert( spef_map_entry( name2, name ) );
				}
				else
				{
					cout << "Required mapped node's name at line " << line_cnt << " of the file " << file_name << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}

				tok_iter++;

				if (tok_iter != tokens.end())
				{
					cout << "Expected less inputs at line " << line_cnt << " of the file " << file_name << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}
			}
			else if (starts_with( *tok_iter, "//" ))
			{

			}
			else
			{
				file.seekg( -(buf.size() + 1), ios::cur );
				break;
			}
		}

		line_cnt++;

	}
}

void System::parse_spef_d_net(ifstream& file, unsigned int& line_cnt, string file_name, string node_name, Parasitic * parasitic_net, double r_scale,
		double c_scale, string delimiter, bool state)
{
	string buf;

	line_cnt++;

	while (true)
	{

		getline( file, buf );

		if (file.eof())
		{

			break;

		}

		typedef tokenizer<char_separator<char> > tokenizer;

		char_separator<char> sep( " \t" );
		tokenizer tokens( buf, sep );

		tokenizer::iterator tok_iter = tokens.begin();

		if (tok_iter != tokens.end())
		{
			if ((*tok_iter).compare( "*END" ) == 0)
			{
				break;
			}
			else if ((*tok_iter).compare( "*CAP" ) == 0)
			{
				parse_spef_cap( file, line_cnt, file_name, node_name, parasitic_net, c_scale, delimiter, state );
				continue;
			}
			else if ((*tok_iter).compare( "*RES" ) == 0)
			{
				parse_spef_res( file, line_cnt, file_name, node_name, parasitic_net, r_scale, delimiter, state );
				continue;
			}
		}

		line_cnt++;
	}

}

void System::parse_spef_cap(ifstream& file, unsigned int& line_cnt, string file_name, string node_name, Parasitic * parasitic_net, double c_scale,
		string delimiter, bool state)
{
	string buf;

	line_cnt++;

	while (true)
	{

		getline( file, buf );

		if (file.eof())
		{

			break;

		}

		typedef tokenizer<char_separator<char> > tokenizer;

		char_separator<char> sep( " \t" );
		tokenizer tokens( buf, sep );

		tokenizer::iterator tok_iter = tokens.begin();

		if (tok_iter != tokens.end())
		{
			if (regex_match( *tok_iter, regex( "([0-9]+)" ) ))
			{
				tok_iter++;

				string name;

				if (tok_iter != tokens.end())
				{
					if (state)  // mapping is empty
					{
						if (regex_match( *tok_iter, regex( "(([a-zA-Z0-9_]+)((" + delimiter + ")([a-zA-Z0-9]+))?)" ) ))
						{
							name = *tok_iter;
						}
						else
						{
							cout << "Invalid node name found at line " << line_cnt << " of the file " << file_name << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
					}
					else
					{
						if (regex_match( *tok_iter, regex( "(((\\*)[0-9]+)((" + delimiter + ")([a-zA-Z0-9]+))?)" ) ))
						{
							name = *tok_iter;
						}
						else
						{
							cout << "Invalid node name found at line " << line_cnt << " of the file " << file_name << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
					}
				}
				else
				{
					cout << "Required node name at line " << line_cnt << " of the file " << file_name << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}

				tok_iter++;

				bool coupling = false;

				if (tok_iter != tokens.end())
				{

					if (state)  // mapping is empty
					{
						if (regex_match( *tok_iter, regex( "([-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?)" ) ))
						{
							p_node * temp = parasitic_net->addPoint( name, NULL );

							temp->capacitance = stod( (*tok_iter) ) * c_scale;
						}
						else if (regex_match( *tok_iter, regex( "(([a-zA-Z0-9_]+)((" + delimiter + ")([a-zA-Z0-9]+))?)" ) ))  // IGNORE COUPLING
						{
							coupling = true;
						}
						else
						{
							cout << "Invalid node name found at line " << line_cnt << " of the file " << file_name << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
					}
					else
					{

						if (regex_match( *tok_iter, regex( "([-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?)" ) ))
						{
							p_node * temp = parasitic_net->addPoint( name, NULL );

							temp->capacitance = stod( (*tok_iter) ) * c_scale;
						}
						else if (regex_match( *tok_iter, regex( "(((\\*)[0-9]+)((" + delimiter + ")([a-zA-Z0-9]+))?)" ) ))  // IGNORE COUPLING
						{
							coupling = true;
						}
						else
						{
							cout << "Invalid node name found at line " << line_cnt << " of the file " << file_name << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
					}

				}
				else
				{
					cout << "Required node name or capacitance value at line " << line_cnt << " of the file " << file_name << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}

				if (!coupling)
				{
					tok_iter++;

					if (tok_iter != tokens.end())
					{
						cout << "Too many inputs found at line " << line_cnt << " of the file " << file_name << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}
				}
				else
				{
					tok_iter++;

					if (tok_iter != tokens.end())
					{

					}
					else
					{
						cout << "Required capacitance value at line " << line_cnt << " of the file " << file_name << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}

					tok_iter++;

					if (tok_iter != tokens.end())
					{
						cout << "Expected less inputs at line " << line_cnt << " of the file " << file_name << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}
				}

			}
			else if (starts_with( *tok_iter, "//" ))
			{

			}
			else
			{
				file.seekg( -(buf.size() + 1), ios::cur );
				break;
			}
		}

		line_cnt++;
	}

}

void System::parse_spef_res(ifstream& file, unsigned int& line_cnt, string file_name, string node_name, Parasitic * parasitic_net, double r_scale,
		string delimiter, bool state)
{
	string buf;

	line_cnt++;

	while (true)
	{

		getline( file, buf );

		if (file.eof())
		{

			break;

		}

		typedef tokenizer<char_separator<char> > tokenizer;

		char_separator<char> sep( " \t" );
		tokenizer tokens( buf, sep );

		tokenizer::iterator tok_iter = tokens.begin();

		if (tok_iter != tokens.end())
		{
			if (regex_match( *tok_iter, regex( "([0-9]+)" ) ))
			{
				tok_iter++;

				string name1;

				string name2;

				if (tok_iter != tokens.end())
				{
					if (state)  // mapping is empty
					{
						if (regex_match( *tok_iter, regex( "(([a-zA-Z0-9_]+)((" + delimiter + ")([a-zA-Z0-9]+))?)" ) ))
						{
							name1 = *tok_iter;
						}
						else
						{
							cout << "Invalid node name found at line " << line_cnt << " of the file " << file_name << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
					}
					else
					{
						if (regex_match( *tok_iter, regex( "(((\\*)[0-9]+)((" + delimiter + ")([a-zA-Z0-9]+))?)" ) ))
						{
							name1 = *tok_iter;
						}
						else
						{
							cout << "Invalid node name found at line " << line_cnt << " of the file " << file_name << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
					}

				}
				else
				{
					cout << "Required node name at line " << line_cnt << " of the file " << file_name << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}

				tok_iter++;

				if (tok_iter != tokens.end())
				{
					if (state)  // mapping is empty
					{
						if (regex_match( *tok_iter, regex( "(([a-zA-Z0-9_]+)((" + delimiter + ")([a-zA-Z0-9]+))?)" ) ))
						{
							name2 = *tok_iter;
						}
						else
						{
							cout << "Invalid node name found at line " << line_cnt << " of the file " << file_name << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
					}
					else
					{
						if (regex_match( *tok_iter, regex( "(((\\*)[0-9]+)((" + delimiter + ")([a-zA-Z0-9]+))?)" ) ))
						{
							name2 = *tok_iter;
						}
						else
						{
							cout << "Invalid node name found at line " << line_cnt << " of the file " << file_name << ".\n";
							cout << "Terminating.\n";
							exit( -1 );
						}
					}

				}
				else
				{
					cout << "Required node name at line " << line_cnt << " of the file " << file_name << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}

				tok_iter++;

				if (tok_iter != tokens.end())
				{
					if (regex_match( *tok_iter, regex( "([-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?)" ) ))
					{
						double val = stod( (*tok_iter) ) * r_scale;

						p_node * temp1 = parasitic_net->addPoint( name1, NULL );
						p_node * temp2 = parasitic_net->addPoint( name2, NULL );

						p_edge* temp_edge = new p_edge();

						temp_edge->resistance = val;
						temp_edge->connected_to_a = temp1;
						temp_edge->connected_to_b = temp2;

						temp1->adjacent.push_back( temp_edge );
						temp2->adjacent.push_back( temp_edge );

						parasitic_net->addEdge( temp_edge );
					}
					else
					{
						cout << "Invalid resistance value at line " << line_cnt << " of the file " << file_name << ".\n";
						cout << "Terminating.\n";
						exit( -1 );
					}
				}
				else
				{
					cout << "Required resistance value at line " << line_cnt << " of the file " << file_name << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}

				tok_iter++;

				if (tok_iter != tokens.end())
				{
					cout << "Expected less inputs at line " << line_cnt << " of the file " << file_name << ".\n";
					cout << "Terminating.\n";
					exit( -1 );
				}
			}
			else if (starts_with( *tok_iter, "//" ))
			{

			}
			else
			{
				file.seekg( -(buf.size() + 1), ios::cur );
				break;
			}
		}

		line_cnt++;
	}

}

void System::change_affected_nets_parasitic(Net* left_wire, Net* right_wire)
{

	for (unsigned int i = 0; i < left_wire->getAssignedTo().size(); i++)  // the left wire was reassigned before, change all dependacies also
	{
		change_affected_nets_parasitic( left_wire->getAssignedTo()[i], right_wire );
		left_wire->getAssignedTo()[i]->setParasitic( right_wire->getParasitic() );
	}
}

