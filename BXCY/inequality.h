#pragma once
#ifndef INEQUALITY_H
#define INEQUALITY_H
#include <string>
#include <vector>
using namespace std;

/// <summary>
/// the type which defines an inequality
/// </summary>
class inequality
{
public:
	inequality() = default;
	inequality* deepcopy();

	double rhs = 0;
	char sense = ' ';//'G' for ">=", 'E' for '=', 'L' for "<="
	vector<int> var_indices;
	vector<double> lhs;
	string cons_name;
};
#endif // !INEQUALITY_H