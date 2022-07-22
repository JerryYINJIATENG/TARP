#pragma once
#ifndef SCEN_INFO_H
#define SCEN_INFO_H
#include <string>
#include <vector>
#include "obj_info.h"
using namespace std;

/// <summary>
/// scenario information
/// </summary>
class scen_info
{
public:
	scen_info() = default;
	string scen_name;
	string pre_scen_name;//previous scenario name
	string stage_name;
	vector<string> var_names;
	vector<string> cons_names;
	obj_info* objective_info;
	double prob;//possibility
	double lower_bound;
};
#endif // !SCEN_INFO_H
