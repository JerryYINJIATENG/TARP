#pragma once
#ifndef CPXHELPER_H
#define CPXHELPER_H
#include <map>
#include <string>
#include "inequality.h"
#include <ilcplex/cplex.h>
using namespace std;

class CPXHelper
{
public:
	CPXHelper() = default;

	/// <summary>
	/// add new rows into the cplex model
	/// </summary>
	/// <param name="rows"></param>
	/// <param name="env"></param>
	/// <param name="model"></param>
	static void AddRows(map<int, inequality*>* rows, CPXENVptr env, CPXLPptr model);
};
#endif // !CPXHELPER_H
