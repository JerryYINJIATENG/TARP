#include "CoverInequalitySeparator.h"

void CoverInequalitySeparator::separateMinimalCover(map<string, double>* variableValues) {
	this->variableValues = variableValues;
	//compute minimal cover
	computeMinimalCover();
}

void CoverInequalitySeparator::normalizeCoverConstraint(CPXENVptr env, CPXSubModel* submodel, inequality* cons) {
	this->env = env;
	this->submodel = submodel;
	this->cons = cons;

	/* check any constraints for the model, and return is_cover_cons to indicate whether the constraint is formulated to a cover inequality.
		i.e., Since the constraints in the sub_model is tranform to the "Equility" sense (adding a slack varible), this function aims to
		check if the given constraint can be reformulated as the cover inequality "\sum_i c_i * y_i \le b" where c_i and b are non-negative integers
		*NOTE: the given constraint contains the first stage variables which are fixed in the subproblem, the rhs of this constraint need to be re-calculated
		via moving the values of first stage variables to the right hand side to get true_rhs.
		When the given constraint looks like "\sum_i c_i * y_i \ge b", we will replace y_i with z_i (i.e., y_i=1-z_i) to obtain the standard form:
		"\sum_i c_i * z_i \le b_bar", where "b_bar = -b + \sum_i c_i", and is_var_replaced returns the flag whether the varibles are replaced
	*/

	int slack_num = 0;//number of slack variables in this constraint
	int slack_coeff = 0;//the coefficient of the slack variable
	vector<int> bi_signs;//the signs of binary variables (1 for positive, -1 for negative)
	vector<int> bi_var_idx_list;//the indices of binary variables
	vector<double> bi_var_lhs_list;//the coefficients of binary variables	
	true_rhs = cons->rhs;//move values of binding variables to the rhs to get the true rhs
	for (int i = 0; i < cons->var_indices.size(); i++) {
		auto var_idx = cons->var_indices.at(i);
		auto var_name = submodel->var_names.at(var_idx);
		auto var_type = submodel->var_types.at(var_name);
		auto coeff = cons->lhs.at(i);
		//is slack variable
		if (submodel->slack_map.find(var_name) != submodel->slack_map.end()) {
			slack_num++;
			slack_coeff = coeff;
			//the constraint may be some other cuts
			if (slack_num > 1) {
				is_cover_cons = false;
				return;
			}
		}
		//is binding variable, update true_rhs
		else if (submodel->binding_var_map.find(var_name) != submodel->binding_var_map.end())
		{
			binding_var_lhs.insert(make_pair(var_name, coeff));
			true_rhs -= coeff * submodel->var_values.at(var_name);
		}
		//is binary variable
		else if (var_type == 'B')
		{
			bi_var_idx_list.push_back(var_idx);
			bi_var_lhs_list.push_back(coeff);
			if (coeff < 0) {
				bi_signs.push_back(-1);
			}
			else if (coeff > 0)
			{
				bi_signs.push_back(1);
			}
		}
		//is pure integers, or continuous variables not belong to slack varibles
		else
		{
			is_cover_cons = false;
			return;
		}
	}

	//if sum_signs==-1*binary_var_num, means the signs are all negative; ==1*binary_var_num means positive
	auto binary_var_num = nrVars = bi_var_idx_list.size();
	double sum_signs = accumulate(bi_signs.begin(), bi_signs.end(), 0);
	//it means some signs of binary variables are opposit
	if (sum_signs != -1 * binary_var_num && sum_signs != binary_var_num) {
		is_cover_cons = false;
		return;
	}
	//check the sense of the constraint
	if (slack_coeff < 0) {
		/* it means the primal sense is >= , needs to replace variables with "y_i=1-z_i" to get the form with sense <=
		   "\sum_i c_i * y_i \ge b" => "\sum_i c_i * z_i \le -b+\sum_i c_i"
		*/
		is_var_replaced = true;
		double b_bar = -true_rhs;
		for (int i = 0; i < binary_var_num; i++) {
			auto coeffi = bi_var_lhs_list.at(i);
			b_bar += coeffi;
		}
		//if b_bar is non-positive
		if (b_bar <= 0) {
			is_cover_cons = false;
			return;
		}
		//write the cover constraint
		is_cover_cons = true;
		b = b_bar;
		for (int i = 0; i < binary_var_num; i++) {
			auto var_idx = bi_var_idx_list.at(i);
			auto coeffi = bi_var_lhs_list.at(i);
			auto var_name = submodel->var_names.at(var_idx);
			knapsackCoefficients.insert(make_pair(var_name, coeffi));
		}

		return;
	}
	else if (slack_coeff > 0)
	{
		//it means the primal sense is <=
		is_var_replaced = false;
		//if b is non-positive
		if (true_rhs <= 0) {
			is_cover_cons = false;
			return;
		}
		//write the cover constraint
		is_cover_cons = true;
		b = true_rhs;
		for (int i = 0; i < binary_var_num; i++) {
			auto var_idx = bi_var_idx_list.at(i);
			auto coeffi = bi_var_lhs_list.at(i);
			auto var_name = submodel->var_names.at(var_idx);
			knapsackCoefficients.insert(make_pair(var_name, coeffi));
		}
		return;
	}
}

void CoverInequalitySeparator::computeMinimalCover() {
	//Compute a minimal cover by solving :
	//{S = min \sum_{i = 0}^n (1 - variableValues[i]) * z_i}
	//{s.t. \sum_{i = 0}^n a_i * z_i \geq b + 1}
	//{z_i binary}
	//if {S < 1}, the cover is violated.
	//Note: Instead of solving the above problem, we transform it into a knapsack problem by substituting{ z_i = 1 - y_i }, i.e. we solve:
	//{max  \sum_{i = 0}^n (1 - variableValues[i])*y_i - \sum_{i = 0}^n (1 - variableValues[i]) }
	//{s.t. \sum_{i = 0}^n a_i * y_i \leq \sum_{i = 0}^n a_i - b - 1}
	//{y_i binary}
	//The desired z_i values can be obtained : {z_i = 1 - y_i}
	
	map<string, double> itemValues;
	int maxKnapsackWeight = 0;//select all items, obtaining the max weight of this knapsack problem
	for (auto iter : *variableValues) {
		auto var_name = iter.first;
		auto var_val = iter.second;
		if (is_var_replaced) {
			//since the variables y are replaced by 1-y, therefore the var_val is 1-y*
			itemValues.insert(make_pair(var_name, var_val));
		}
		else
		{
			itemValues.insert(make_pair(var_name, 1 - var_val));
		}
		maxKnapsackWeight += knapsackCoefficients.at(var_name);
	}
	//the capacity of the knapsack is sufficient enough, select all items even won't exceed its capacity
	if (maxKnapsackWeight <= b) {
		coverInequalityExists = minimalCoverIsViolated = false;
		return;
	}
	else
	{
		coverInequalityExists = true;
	}

	//using cplex to solve the knapsack problem
	{
		int status = 0;
		auto model = CPXcreateprob(env, &status, "knapsack_problem");

		//generate the model
		const int var_num = nrVars;
		vector<string> var_name_list;
		auto obj = new double[var_num];
		auto lb = new double[var_num];
		auto ub = new double[var_num];
		auto xtype = new char[var_num];
		auto var_names = new char* [var_num];
		int idx = 0;
		//populate columns
		for (auto iter : itemValues) {
			auto var_name = new string(iter.first);
			var_name_list.push_back(*var_name);
			obj[idx] = itemValues.at(*var_name);
			lb[idx] = 0;
			ub[idx] = 1;
			xtype[idx] = 'B';
			var_names[idx] = (char*)var_name->data();
			idx++;
		}
		CPXnewcols(env, model, var_num, obj, lb, ub, xtype, var_names);
		//populate constraints
		auto cons = new inequality;
		cons->rhs = 0;
		for (int i = 0; i < var_num; i++) {
			auto var_name = var_name_list.at(i);
			auto ai = knapsackCoefficients.at(var_name);
			if (ai != 0) {
				cons->var_indices.push_back(i);
				cons->lhs.push_back(ai);
				cons->rhs += ai;
			}
		}
		cons->rhs += -b - 1;
		cons->sense = 'L';
		auto constraints = map<int, inequality*>{ make_pair(0,cons) };
		CPXHelper::AddRows(&constraints, env, model);
		CommonHelper::Free(constraints);
		//solve model
		CPXchgobjsen(env, model, CPX_MAX);
		/*CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON);*/
		int error = CPXmipopt(env, model);
		string file_name = "cover.lp";
		CPXwriteprob(env, model, file_name.data(), NULL);
		if (error) {
			throw "error code " + error;
		}
		//get optimal solution
		int solstat = 0;
		double objval = 0;
		auto varvals = new double[var_num];
		CPXsolution(env, model, &solstat, &objval, varvals, NULL, NULL, NULL);
		//Convert back into cover by substituting y_i=1-z_i
		double minimalCoverValue = 0;//the value of S
		minimalCoverRHS = 0;
		for (int i = 0; i < var_num; i++) {
			auto var_name = var_name_list.at(i);
			//y_i
			int var_val = (int)round(varvals[i]);
			//z_i=1-y_i
			bool is_in_cover = 1 - var_val == 0 ? false : true;
			minimalCover.insert(make_pair(var_name, is_in_cover));
			if (is_in_cover) {
				//itemValue = 1 - variableValues[i]
				minimalCoverValue += itemValues.at(var_name);
				minimalCoverRHS++;
				minimalCoverSet.push_back(var_name);
			}
		}
		minimalCoverRHS--;
		minimalCoverIsViolated = minimalCoverValue <= (1 - PRECISION);
	}
}

inequality* CoverInequalitySeparator::generateCut() {
	double lambda = 0;
	double b_bar = 0;//b_bar=max{j\in C b_j}
	vector<string> vars_in_N0;
	vector<string> vars_in_N1;
	

	/* calculate lambda.i.e.:
		lambda=\sum_{j\in C} b_j - d
	*/
	for (auto var_name : minimalCoverSet) {
		auto b_j = knapsackCoefficients.at(var_name);
		lambda += b_j;
		if (b_j > b_bar) {
			b_bar = b_j;
		}
	}
	lambda -= b;

	//calculate the sets N0 and N1
	for (auto iter : submodel->binding_var_map) {
		auto var_name = iter.first;
		auto var_val = submodel->var_values.at(var_name);
		if (var_val == 0) {
			vars_in_N0.push_back(var_name);
		}
		else
		{
			vars_in_N1.push_back(var_name);
		}
	}

	//calculate the cut
	for (auto var_name : minimalCoverSet) {
		//for j in C
		cut_lhs.insert(make_pair(var_name, 1));
	}
	for (auto iter : knapsackCoefficients) {
		auto var_name = iter.first;
		auto coeff = iter.second;
		//for j not in C
		if (find(minimalCoverSet.begin(), minimalCoverSet.end(), var_name) == minimalCoverSet.end()) {
			cut_lhs.insert(make_pair(var_name, CommonHelper::Floor(coeff / b_bar)));
			//cut_lhs.insert(make_pair(var_name, coeff / b_bar));
		}
	}
	//rhs
	cut_rhs = minimalCoverSet.size() - 1;
	//cut_rhs = minimalCoverSet.size() - lambda / b_bar;
	//for j in N0
	for (auto var_name : vars_in_N0) {
		auto coeff = binding_var_lhs.at(var_name);
		cut_lhs.insert(make_pair(var_name, CommonHelper::Floor(coeff / b_bar)));
		//cut_lhs.insert(make_pair(var_name, coeff / b_bar));
	}
	//for j in N1
	for (auto var_name : vars_in_N1) {
		auto coeff = binding_var_lhs.at(var_name);
		cut_lhs.insert(make_pair(var_name, -CommonHelper::Floor(-coeff / b_bar)));
		cut_rhs -= CommonHelper::Floor(-coeff / b_bar);
		//cut_lhs.insert(make_pair(var_name, coeff / b_bar));
		//cut_rhs += coeff / b_bar;
	}

	//check
	double cover_lhs = 0;
	for (auto var_name : minimalCoverSet) {
		cover_lhs += knapsackCoefficients.at(var_name);
	}
	if (cover_lhs < b) {
		int kksd = 0;
	}

	//covert back 
	if (is_var_replaced) {
		for (auto iter : cut_lhs) {
			auto var_name = iter.first;
			auto lhs = iter.second;
			//not a binding variable
			if (submodel->binding_var_map.find(var_name) == submodel->binding_var_map.end()) {
				cut_lhs.at(var_name) = -lhs;
				cut_rhs -= lhs;
			}
		}
	}

	double temp_lhs = 0;
	for (auto iter : cut_lhs) {
		auto var_name = iter.first;
		auto coeff = iter.second;
		auto var_val = submodel->var_values.at(var_name);
		temp_lhs += coeff * var_val;
	}
	if (temp_lhs <= cut_rhs) {
		int kksd = 0;
	}

	auto cut = new inequality();
	cut->cons_name = "kc_" + to_string(submodel->cons_num++);
	for (auto iter : cut_lhs) {
		auto var_name = iter.first;
		auto lhs = iter.second;
		auto var_idx = submodel->var_map.at(var_name);
		cut->var_indices.push_back(var_idx);
		cut->lhs.push_back(lhs);
	}
	cut->rhs = cut_rhs;

	//adding a slack variable to the cut
	{
		auto slack_var_idx = submodel->var_num++;
		auto slack_var_name = "sk" + to_string(slack_var_idx);
		submodel->slack_map.insert(make_pair(slack_var_name, slack_var_idx));
		submodel->var_names.insert(make_pair(slack_var_idx, slack_var_name));
		//continues variable
		submodel->var_types.insert(make_pair(slack_var_name, 'C'));
		//set bounds
		submodel->var_bounds.insert(make_pair(slack_var_name, make_pair(0, INFINITY)));
		//add to the model
		const int ccnt = 1;
		int nzcnt = 0;
		auto lb = new double[ccnt] {0};
		auto ub = new double[ccnt] {INFINITY};
		auto obj = new double[ccnt] {0};
		auto var_type = new char[ccnt] {'C'};
		auto colname = new char* [ccnt] {(char*)submodel->var_names.at(slack_var_idx).data()};
		CPXnewcols(env, submodel->model, ccnt, obj, lb, ub, var_type, colname);

		//the new slack variable changes the sense to the "Equality"
		cut->sense = 'E';
		//adding the slack variable to the cut
		cut->var_indices.push_back(slack_var_idx);
		cut->lhs.push_back(1);
	}              

	return cut;
}