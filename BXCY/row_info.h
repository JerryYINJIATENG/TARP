#pragma once
#include <string>
#include <vector>
#include <map>
#ifndef ROW_INFO_H
#define ROW_INFO_H
using namespace std;

/// <summary>
/// row infomation
/// </summary>
class row_info
{
public:
	row_info() = default;
	string row_name;
	vector<string> var_names;
	map<string, double> var_coeffs;
	string control_symbol;//"E" for "equal", "G" for "grater than and equal to", "L" for "less than and equal to"
	double rhs;
};
#endif // !ROW_INFO_H
