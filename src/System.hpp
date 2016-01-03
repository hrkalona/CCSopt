/*
 * System.hpp
 *
 *  Created on: May 12, 2014
 *      Author: hrkalona
 */

#ifndef SYSTEM_HPP_
#define SYSTEM_HPP_

#include <vector>
#include <string>
#include "types.hpp"
#include "si2dr_liberty.h"
#include "Cell.hpp"
#include "OutputPin.hpp"
#include "InputPin.hpp"
#include "Timing.hpp"
#include "When.hpp"
#include "Cellinst.hpp"
#include "Net.hpp"
#include "Graph.hpp"
#include "Module.hpp"
#include "Parasitic.hpp"

using namespace std;

typedef struct _unit_inverter
{
	Cell * cell;
	double C0;
	double tau;
} unit_inverter;


typedef pair<string, string> spef_map_entry;
typedef map<string, string> spef_map;

class System
{
private:
	Module main_module;
	vector<string> verilog_files;
	char * lib_file; // Liberty File (the input library)
	string cfg_file; // A file that configures the resizer
	string spef_file; // A file containing the RC-parasitic of the wires
	cell_map cells; // Keeps all cells that exist in the Liberty File (the library)
	cellinst_map instantiated_cells; // Keeps all the cell instances extracted from the Verilog (.v) description of the circuit
	net_map wires; // Keeps all the nets extracted from the Verilog (.v) description of the circuit
	Graph graph; // The (pin-based)-graph abstraction of the circuit
	RESIZE_SEQUENTIAL_ACTION resize_sequential; // An option retrieved from the configuration file that determines if the sequential elements are going to be resized-to-min or not
	int cell_no_design; // Total number of cells in the design
	int cells_examined; // How many cells have been examined??
	int cells_resized; // How many cells have actually been resized??
	int cells_potentially_resized; // How many cells are going to be downscaled?
	unit_inverter un_in; // The unit (min-size) inverter
	int lib_cap_units;
	int lib_time_units;
	int max_iterations;
	double tolerance;
	string timing_table_name; // The look up table name for timing
	vector<Parasitic *> parasitics;
	int parasitics_found;
	int combinational_cells;
	int sequential_cells;
	string cell_name_delims;
	string output_path;

public:
	System();
	~System();
	void inputHandler(int argc, char *argv[]);
	void engine();
	void outputHandler();
	Graph& getGraph();
	net_map& getNets();
	cellinst_map& getInstantiatedCells();
	cell_map& getCells();
	Module& getMainModule();

private:
	void parseFiles();

	void dotLibParser();
	void getCellAttrs(si2drGroupIdT, Cell *&);
	void getPinAttrs(si2drGroupIdT, Cell * &, si2drStringT, InputPin *&, OutputPin *&);
	void getLutAttrs(si2drGroupIdT, si2drStringT);
	void getTimingAttrs(si2drGroupIdT, OutputPin *&, Timing *&, When *&, Cell *&);
	void getGroups(si2drGroupIdT, Cell * &, InputPin *&, OutputPin *&, Timing *&, When *&);
	void libVerification();

    /* V Parser */
	void dotVParser();
	void parse_module(string, ifstream&, unsigned int&, string, string&, vector<string>&, bool, vector<string>&);
	void parse_input(string, ifstream&, unsigned int&, string, vector<string>&);
	void parse_output(string, ifstream&, unsigned int&, string, vector<string>&);
	void parse_wire(string, ifstream&, unsigned int&, string, vector<string>&);
	void parse_cell(string, ifstream&, unsigned int&, string, string&, vector<string>&, vector<string>&);
	void parse_assign(string, ifstream&, unsigned int&, string, vector<string>&);
	void parse_multiline_comment(string, ifstream&, unsigned int&, string);

	/* for instance module call */
	void parse_input_instance_module(Module*, unsigned int&, vector<string>&);
	void parse_module_instance_module(Module*, unsigned int&, string&, vector<string>&, vector<string>& );
	void parse_output_instance_module(Module*, unsigned int&, vector<string>&);
	void parse_assign_instance_module(Module*, unsigned int&, vector<string> &);
	void parse_wire_instance_module(Module*, unsigned int&, vector<string>&);
	void parse_cell_instance_module(Module*, unsigned int&, string&, vector<string>&, vector<string>&);
	/**************************/

	Net * wire_mapping(string, bool);
	Node * node_mapping(string, string, Cell *, NODE_TYPE);
	Node * input_mapping(string);
	Node * output_mapping(string);
	Node * virtual_mapping(string);

	void change_affected_nets_parasitic(Net*, Net*);

	void instantiate_module(Module *, string, vector<string>, module_map&, string, int, set<string>&);

	void connect_wiring_and_internalconnect(Node *, Net *, Cell *, NODE_TYPE, string, string, int);

	void add_edges();

	Cellinst * instance_cell_mapping(string);
	void creteCellInstances();

	void calcUnitInverterVals();

	void sequentialCellsHandling();

	void dotCfgParser();

	void dotSpefParser();
	void parse_spef_mapping(ifstream&, unsigned int&, string, spef_map&);
	void parse_spef_d_net(ifstream&, unsigned int&, string, string, Parasitic *, double, double, string, bool);
	void parse_spef_cap(ifstream&, unsigned int&, string, string, Parasitic *, double, string, bool);
	void parse_spef_res(ifstream&, unsigned int&, string, string, Parasitic *, double, string, bool);

};

#endif /* SYSTEM_HPP_ */
