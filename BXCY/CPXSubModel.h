#pragma once
#ifndef CPXSUBMODEL_H
#define CPXSUBMODEL_H
#include <ilcplex/cplex.h>
#include <map>
#include <string>
#include <vector>
#include "inequality.h"
using namespace std;

/// <summary>
/// the model of the subproblem
/// </summary>
class CPXSubModel
{
public:
	CPXSubModel();
	~CPXSubModel();
	CPXSubModel* deepcopy();
	void UpdateModel();

	CPXENVptr env;
	CPXLPptr model;
	string name;
	map<int, inequality*> constraints;//constraints indices
	map<int, inequality*> binding_cons;//binding constraints
	map<string, int> var_map; //map<name,index>, excludes slacks
	map<string, char> var_types;// C-continues, B-binary, I-integer
	map<int, string> var_names;
	map<string, pair<double, double>> var_bounds;// map<name, <lb,ub>>
	map<string, int> slack_map;//slack variables
	map<string, int> binding_var_map;//binding variables (the involved vars from the master problem)
	map<int, double> objective;//map<var_index,coefficient>
	double obj_value = 0;
	bool is_integer;
	map<string, double> var_values;
	/*map<string, double> best_var_values;*/
	vector<double> duals;
	string scen_name;//scenario name
	string ins_name;//instance name
	string model_type;//LP or IP
	int var_num = 0;
	int cons_num = 0;
};
#endif // !CPXSUBMODEL_H
