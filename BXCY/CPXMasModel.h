#pragma once
#ifndef CPXMASMODEL_H
#define CPXMASMODEL_H
#include <ilcplex/cplex.h>
#include <map>
#include <string>
#include <vector>
#include "inequality.h"
using namespace std;

/// <summary>
/// the model of the master problem
/// </summary>
class CPXMasModel
{
public:
	CPXMasModel();
	~CPXMasModel();
	CPXMasModel* deepcopy();

	CPXENVptr env;
	CPXLPptr model = nullptr;
	string name;
	map<int, inequality*> constraints;//key: constraints indices
	vector<int> opt_cuts;//optimality cuts
	//map<int, inequality*> feas_cuts;//feasibility cuts
	//map<int, inequality*> valid_cuts;//valid inequalities
	map<string, int> var_map; //map<name,index>, excludes slacks and estimators
	map<string, char> var_types;// C-continues, B-binary, I-integer
	map<int, string> var_names;
	map<string, pair<double, double>> var_bounds;// map<name, <lb,ub>>
	map<string, int> estimator_map;
	//map<string, int> slack_map;//slack variables
	map<int, double> objective;//map<var_index,coefficient>
	double obj_value = 0;
	map<string, double> var_values;
	vector<double> duals;
	string ins_name;//instance name
	string model_type;//LP or IP
	int var_num = 0;
	int cons_num = 0;
};
#endif // !CPXMASMODEL_H
