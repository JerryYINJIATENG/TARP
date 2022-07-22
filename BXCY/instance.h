#pragma once
#ifndef INSTANCE_H
#define INSTANCE_H
#include <map>
#include <string>
#include <vector>
#include "row_info.h"
#include "stage_info.h"
#include "scen_info.h"
using namespace std;

/// <summary>
/// instance including a fleet of scenarios
/// </summary>
class instance
{
public:
	instance() = default;
	string ins_name;
	map<string, string> var_map; //var_map[var_name,var_type]: var_type-"INT" for int, "DOUBLE" for double
	vector<string> sorted_var_list;
	map<string, row_info*> row_map;
	vector<string> sorted_row_list;
	vector<stage_info*> stages_map;
	map<string, scen_info*> scens_map;
	map<string, double> upper_bounds;
	map<string, double> lower_bounds;
	vector<pair<string, double>> master_obj;
};
#endif // !INSTANCE_H
