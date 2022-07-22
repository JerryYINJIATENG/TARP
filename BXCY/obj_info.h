#pragma once
#ifndef OBJ_INFO_H
#define OBJ_INFO_H
#include <string>
#include <vector>
using namespace std;

/// <summary>
/// objective infomation
/// </summary>
class obj_info
{
public:
	obj_info() = default;
	string scen_name;
	vector<string> var_names;
	vector<double> var_coeffs;
};
#endif // !OBJ_INFO_H
