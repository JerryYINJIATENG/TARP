#pragma once
#ifndef CIS_H
#define CIS_H
#include <map>
#include <vector>
#include <string>
#include <ilcplex/cplex.h>
#include "inequality.h"
#include "CPXHelper.h"
#include "CommonHelper.h"
#include "CPXSubModel.h"
#include <numeric>
using namespace std;

class CoverInequalitySeparator
{
public:
	CoverInequalitySeparator() = default;
	const double PRECISION = 0.000001;

	//input elements
	CPXENVptr env;
	CPXSubModel* submodel;
	inequality* cons;

	//auxiliary elements
	bool is_cover_cons = true;
	bool is_var_replaced = false;
	map<string, double> binding_var_lhs;

	//Knapsack constraint {\sum_{i=0}^n a_i * x_i \leq b}
	int nrVars;//Number of variables
	map<string, double> knapsackCoefficients;//a_i. key is var_name
	double b;//the rhs of the knapsack constraint
	map<string, double>* variableValues;//x_i. key is var_name
	double true_rhs = 0;

	/*Indicates whether a cover inequality exists.
	If {\sum_i a_i \leq b}, then no cover exists and hence no inequality can be generated.*/
	bool coverInequalityExists;

	//minimal cover
	int minimalCoverRHS;//Right hand side of the minimal cover inequality
	map<string, bool> minimalCover;//Boolean map indicating whether a variable is in the minimal cover
	vector<string> minimalCoverSet;//Set containing the variables which are part of the minimal cover
	bool minimalCoverIsViolated;//Returns true if LHS > minimalCoverSize-1 for the given variableValues

	//cut
	map<string, double> cut_lhs;
	double cut_rhs;

	//check a constraint whther can be reformulated to a cover constraint
	void normalizeCoverConstraint(CPXENVptr env, CPXSubModel* submodel, inequality* cons);

	/// <summary>
	/// Given a knapsack constraint: {\sum_{i=0}^n a_ix_i \leq b}. This method separates minimal Cover Inequalities, i.e it will search for a valid cover
	/// {\sum_{ i\in C } x_i \leq | C | -1} which is violated by the current variable values
	/// </summary>
	/// <param name="variableValues">values of the x_i variables</param>
	void separateMinimalCover(map<string, double>* variableValues);

	/// <summary>
	/// After computing the minimal cut C, we implement the 0-1 extended cover inequality on page 10 of paper
	/// "Weninger, Dieter, and Laurence A. Wolsey. Benders¡¯ algorithm with (mixed)-integer subproblems." 
	/// </summary>
	/// <returns>the generated cut/constraint </returns>
	inequality* generateCut();

private:
	/// <summary>
	/// Compute a minimal cover by solving a knapsack model
	/// </summary>
	void computeMinimalCover();
};
#endif // !LCIS_H
