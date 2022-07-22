#include "SMPS_Reader.h"
#include "node.h"
using namespace std;
#define eps 0.00001
#define eta 0.0001

SMPS_Reader::SMPS_Reader() {

}

void SMPS_Reader::ReadFiles(string folder_path) {
	cout << "Reading files..." << endl;
	folder = folder_path;
	vector<string> files;
	vector<string> own_names;
	FileHelper::GetFiles(folder_path, files, own_names);
	vector<string> instance_names;
	for (auto name : own_names) {
		vector<string> temp;
		StringHelper::SplitString(name, temp, ".");
		if (temp[1] == "cor" || temp[1] == "tim" || temp[1] == "sto") {
			if (find(instance_names.begin(), instance_names.end(), temp[0]) == instance_names.end()) {
				instance_names.push_back(temp[0]);
			}
		}
	}

	for (auto ins_name : instance_names) {
		auto ins = new instance();
		ins->ins_name = ins_name;
		//read core file
		{
			string file_path = folder_path + ins_name + ".cor";
			ifstream infile;
			infile.open(file_path);
			if (infile.is_open()) {
				string line;
				while (getline(infile, line))
				{
					vector<string> temp;
					StringHelper::SplitString(line, temp, " ");
					//skip the annotation
					if (temp[0] == "*") {
						continue;
					}
					//read rows
					if (line == "ROWS") {
						while (true)
						{
							getline(infile, line);
							if (line == "COLUMNS") {
								break;
							}
							temp.clear();
							StringHelper::SplitString(line, temp, " ");
							StringHelper::EraseSpace(temp);
							string control = temp[0];
							string row_name = temp[1];
							auto row = new row_info();
							row->row_name = row_name;
							row->control_symbol = control;
							ins->row_map.insert(make_pair(row->row_name, row));
							ins->sorted_row_list.push_back(row->row_name);
						}
					}
					//read columns
					if (line == "COLUMNS") {
						string var_type = "DOUBLE";
						while (true)
						{
							getline(infile, line);
							if (line == "RHS") {
								break;
							}
							temp.clear();
							StringHelper::SplitString(line, temp, " ");
							StringHelper::EraseSpace(temp);
							//a marker row
							if (temp[1] == "'MARKER'") {
								//integer begins
								if (temp[2] == "'INTORG'") {
									var_type = "INT";
								}
								else if (temp[2] == "'INTEND'") {
									var_type = "DOUBLE";
								}
								continue;
							}
							string var_name = temp[0];
							for (int i = 1; i < temp.size(); i += 2) {
								string row_name = temp[i];
								string coeff = temp[i + 1];
								//add vars
								if (ins->var_map.find(var_name) == ins->var_map.end()) {
									ins->var_map.insert(make_pair(var_name, var_type));
									ins->sorted_var_list.push_back(var_name);
								}
								if (var_name == "q_00") {
									int sdsds = 0;
								}
								//update rows
								ins->row_map.at(row_name)->var_names.push_back(var_name);
								ins->row_map.at(row_name)->var_coeffs.insert(make_pair(var_name, atof(coeff.data())));
							}
						}
					}
					//read RHS
					if (line == "RHS") {
						while (true)
						{
							getline(infile, line);
							if (line == "BOUNDS") {
								break;
							}
							temp.clear();
							StringHelper::SplitString(line, temp, " ");
							StringHelper::EraseSpace(temp);
							for (int i = 1; i < temp.size(); i += 2) {
								string row_name = temp[i];
								string rhs = temp[i + 1];
								//update rhs
								auto row = ins->row_map.at(row_name);
								row->rhs = atof(rhs.data());
							}
						}
					}
					//read BOUNDS
					if (line == "BOUNDS") {
						while (true)
						{
							getline(infile, line);
							if (line == "ENDATA") {
								break;
							}
							temp.clear();
							StringHelper::SplitString(line, temp, " ");
							StringHelper::EraseSpace(temp);
							string bound_type = temp[0];
							string var_name = temp[2];
							string bound_value = temp[3];
							if (bound_type == "UP") {
								ins->upper_bounds.insert(make_pair(var_name, atof(bound_value.data())));
							}
							else if (bound_type == "LO")
							{
								ins->lower_bounds.insert(make_pair(var_name, atof(bound_value.data())));
							}
						}
					}
				}
			}
		}
		//read time file
		{
			string file_path = folder_path + ins_name + ".tim";
			ifstream infile;
			infile.open(file_path);
			if (infile.is_open()) {
				string line;
				while (getline(infile, line))
				{
					vector<string> temp;
					StringHelper::SplitString(line, temp, " ");
					StringHelper::EraseSpace(temp);
					//skip the annotation
					if (temp[0] == "*") {
						continue;
					}
					//read PERIODS (LP)
					vector<string> stage_names;
					vector<string> begin_columns;
					vector<string> begin_rows;
					if (temp[0] == "PERIODS") {
						while (true)
						{
							getline(infile, line);
							if (line == "ENDATA") {
								break;
							}
							temp.clear();
							StringHelper::SplitString(line, temp, " ");
							StringHelper::EraseSpace(temp);
							begin_columns.push_back(temp[0]);
							begin_rows.push_back(temp[1]);
							stage_names.push_back(temp[2]);
						}

						//set datas
						for (int i = 0; i < stage_names.size() - 1; i++) {
							auto stage_name = stage_names.at(i);
							auto begin_column = begin_columns.at(i);
							auto begin_row = begin_rows.at(i);
							auto end_column = begin_columns.at(i + 1);
							auto end_row = begin_rows.at(i + 1);
							auto s_info = new stage_info();
							s_info->stage_name = stage_name;
							auto begin_colum_iter = find(ins->sorted_var_list.begin(),
								ins->sorted_var_list.end(), begin_column);
							auto end_colum_iter = find(ins->sorted_var_list.begin(),
								ins->sorted_var_list.end(), end_column);
							auto begin_row_iter = find(ins->sorted_row_list.begin(),
								ins->sorted_row_list.end(), begin_row);
							auto end_row_iter = find(ins->sorted_row_list.begin(),
								ins->sorted_row_list.end(), end_row);
							//set columns
							while (begin_colum_iter != end_colum_iter)
							{
								s_info->var_names.push_back(*begin_colum_iter);
								++begin_colum_iter;
							}
							//set rows
							while (begin_row_iter != end_row_iter)
							{
								s_info->cons_names.push_back(*begin_row_iter);
								++begin_row_iter;
							}
							ins->stages_map.push_back(s_info);
						}
						//the last stage
						{
							auto stage_name = stage_names.back();
							auto begin_column = begin_columns.back();
							auto begin_row = begin_rows.back();
							auto s_info = new stage_info();
							s_info->stage_name = stage_name;
							auto begin_colum_iter = find(ins->sorted_var_list.begin(),
								ins->sorted_var_list.end(), begin_column);
							auto begin_row_iter = find(ins->sorted_row_list.begin(),
								ins->sorted_row_list.end(), begin_row);
							//set columns
							while (begin_colum_iter != ins->sorted_var_list.end())
							{
								s_info->var_names.push_back(*begin_colum_iter);
								++begin_colum_iter;
							}
							//set rows
							while (begin_row_iter != ins->sorted_row_list.end())
							{
								s_info->cons_names.push_back(*begin_row_iter);
								++begin_row_iter;
							}
							ins->stages_map.push_back(s_info);
						}
					}
				}
			}
		}
		//read the stoch file
		{
			string file_path = folder_path + ins_name + ".sto";
			ifstream infile;
			infile.open(file_path);
			if (infile.is_open()) {
				string line;
				while (getline(infile, line))
				{
					vector<string> temp;
					StringHelper::SplitString(line, temp, " ");
					StringHelper::EraseSpace(temp);
					//skip the annotation
					if (temp[0] == "*") {
						continue;
					}
					//read PERIODS (LP)
					vector<string> stage_names;
					vector<string> begin_columns;
					vector<string> begin_rows;
					if (temp[0] == "SCENARIOS") {
						scen_info* info = nullptr;
						while (true)
						{
							getline(infile, line);
							if (line == "ENDATA") {
								break;
							}
							temp.clear();
							StringHelper::SplitString(line, temp, " ");
							StringHelper::EraseSpace(temp);
							// a new scenario
							if (temp[0] == "SC")
							{
								auto scen_name = temp[1];
								auto pre_scen_name = temp[2];
								auto prob = temp[3];
								auto stage_name = temp[4];
								info = new scen_info();
								info->scen_name = scen_name;
								info->pre_scen_name = pre_scen_name;
								info->prob = atof(prob.data());
								info->stage_name = stage_name;
								info->objective_info = new obj_info();
								info->objective_info->scen_name = info->scen_name;
								ins->scens_map.insert(make_pair(info->scen_name, info));
							}
							else
							{
								auto var_name = temp[0];
								auto row_name = temp[1];
								auto coeff = temp[2];
								if (info != nullptr) {
									if (row_name == "obj") {
										info->objective_info->var_names.push_back(var_name);
										info->objective_info->var_coeffs.push_back(atof(coeff.data()));
									}
									//set vars in this scenario
									if (find(info->var_names.begin(), info->var_names.end(), var_name) == info->var_names.end()) {
										info->var_names.push_back(var_name);
									}
								}
							}
						}
					}
				}
				//set variables and constraints in this scenario
				for (auto iter : ins->scens_map) {
					auto scen = iter.second;
					auto stage_name = scen->stage_name;
					auto stage = new stage_info();
					{
						/*stage = from(ins->stages_map).where([&stage_name](stage_info* p) {
							return p->stage_name == stage_name;
							}).toStdList().front();*/
							//the second stage
						stage = ins->stages_map.at(1);
					}
					for (auto cons_name : stage->cons_names) {
						auto row = ins->row_map.at(cons_name);
						for (auto var_name : row->var_names) {
							//if this row contrains a variable of this scenario
							if (find(scen->var_names.begin(), scen->var_names.end(), var_name) != scen->var_names.end()) {
								scen->cons_names.push_back(cons_name);
								break;
							}
						}
					}
				}
			}
		}
		instances_map.insert(make_pair(ins->ins_name, ins));
		//set master obj
		{
			auto stage = ins->stages_map.front();
			for (auto row_iter : ins->row_map) {
				auto row_info = row_iter.second;
				auto symbol = row_info->control_symbol;
				//objective
				if (symbol == "N") {
					for (auto var_name : row_info->var_names) {
						//if this var belongs to the first stage
						if (find(stage->var_names.begin(), stage->var_names.end(), var_name) != stage->var_names.end()) {
							auto coeff = row_info->var_coeffs.at(var_name);
							ins->master_obj.push_back(make_pair(var_name, coeff));
						}
					}
				}
			}
		}
		//set cuts_waitted_map
		/*cuts_waitted_map.insert(make_pair(ins->ins_name, new vector<IloRange>()));*/
	}
}

// solve the whole model
void SMPS_Reader::SolveMeanModels() {
	cout << "Creating mean models..." << endl;
	string root = folder + "MeanModels\\";
	FileHelper::CreatFolderIfNotExist(root);
	FileHelper::ClearFolder(root);
	//creat a mean model for each instance
	for (auto ins_iter : instances_map) {
		int status;
		auto ins = ins_iter.second;
		auto mean_model = new CPXMeanModel();
		mean_model->env = CPXopenCPLEX(&status);
		mean_model->name = ins->ins_name + "_mean";
		mean_model->model = CPXcreateprob(mean_model->env, &status, mean_model->name.data());
		//initialize vars
		{
			//thetas: using a varible theta to capture the objective of each scenario			
			for (auto scen_iter : ins->scens_map) {
				auto sceninfo = scen_iter.second;
				string var_name = "theta{" + sceninfo->scen_name + "}";
				int var_idx = mean_model->var_num++;
				mean_model->estimator_map.insert(make_pair(var_name, var_idx));
				mean_model->var_names.insert(make_pair(var_idx, var_name));
				//continues variable
				mean_model->var_types.insert(make_pair(var_name, 'C'));
				//set bounds
				mean_model->var_bounds.insert(make_pair(var_name, make_pair(-INFINITY, INFINITY)));
			}
			//first stage variables
			{
				auto& first_stage = ins->stages_map.front();
				for (auto var_name : first_stage->var_names) {
					int var_idx = mean_model->var_num++;
					double lb = 0, ub = INFINITY;
					auto var_type = ins->var_map.at(var_name);
					if (ins->lower_bounds.find(var_name) != ins->lower_bounds.end()) {
						lb = ins->lower_bounds.at(var_name);
					}
					if (ins->upper_bounds.find(var_name) != ins->upper_bounds.end()) {
						ub = ins->upper_bounds.at(var_name);
					}
					if (var_type == "INT") {
						mean_model->var_types.insert(make_pair(var_name, 'I'));
					}
					else if (var_type == "DOUBLE")
					{
						mean_model->var_types.insert(make_pair(var_name, 'C'));
					}
					mean_model->var_map.insert(make_pair(var_name, var_idx));
					mean_model->var_names.insert(make_pair(var_idx, var_name));
					mean_model->var_bounds.insert(make_pair(var_name, make_pair(lb, ub)));
				}
			}
			//second stage vars
			{
				auto& second_stage = ins->stages_map.at(1);
				for (auto scen_iter : ins->scens_map) {
					auto scen_name = scen_iter.first;
					auto& sceninfo = scen_iter.second;
					for (auto var_name : sceninfo->var_names) {
						int var_idx = mean_model->var_num++;
						auto var_type = ins->var_map.at(var_name);
						string name = var_name + "{" + scen_name + "}";
						double lb = 0, ub = INFINITY;
						if (ins->lower_bounds.find(var_name) != ins->lower_bounds.end()) {
							lb = ins->lower_bounds.at(var_name);
						}
						if (ins->upper_bounds.find(var_name) != ins->upper_bounds.end()) {
							ub = ins->upper_bounds.at(var_name);
						}
						if (var_type == "INT") {
							mean_model->var_types.insert(make_pair(name, 'I'));
						}
						else if (var_type == "DOUBLE")
						{
							mean_model->var_types.insert(make_pair(name, 'C'));
						}
						mean_model->var_map.insert(make_pair(name, var_idx));
						mean_model->var_names.insert(make_pair(var_idx, name));
						mean_model->var_bounds.insert(make_pair(name, make_pair(lb, ub)));
					}
				}
			}
		}

		//set the objective
		{
			auto& first_stage = ins->stages_map.front();
			//from the first stage
			for (auto obj_iter : ins->master_obj) {
				auto var_name = obj_iter.first;
				auto coeff = obj_iter.second;
				auto var_idx = mean_model->var_map.at(var_name);
				mean_model->objective.insert(make_pair(var_idx, coeff));
			}
			//from the second stage
			for (auto scen_iter : ins->scens_map) {
				auto scen_name = scen_iter.first;
				auto& scen = scen_iter.second;

				string var_name = "theta{" + scen->scen_name + "}";
				auto& estimator = mean_model->estimator_map.at(var_name);
				auto estima_idx = mean_model->estimator_map.at(var_name);
				mean_model->objective.insert(make_pair(estima_idx, scen->prob));

				//binding estimator with a fleet of variables
				auto cons_idx = mean_model->cons_num++;
				auto cons = new inequality{ 0,'E' };
				cons->cons_name = "esti_cons" + to_string(cons_idx);
				auto& objinfo = scen->objective_info;
				for (int i = 0; i < objinfo->var_names.size(); i++) {
					auto var_name = objinfo->var_names[i];
					auto name = var_name + "{" + scen_name + "}";
					auto coeff = objinfo->var_coeffs[i];
					auto var_idx = mean_model->var_map.at(name);
					cons->var_indices.push_back(var_idx);
					cons->lhs.push_back(-coeff);
				}
				cons->var_indices.push_back(estima_idx);
				cons->lhs.push_back(1);
				mean_model->constraints.insert(make_pair(cons_idx, cons));
			}
		}

		//set constraints
		{
			//constraint (first satge)
			auto& first_stage = ins->stages_map.front();
			for (auto row_name : first_stage->cons_names) {
				auto row_info = ins->row_map.at(row_name);
				auto symbol = row_info->control_symbol;
				if (symbol != "N")
				{
					if (find(first_stage->cons_names.begin(), first_stage->cons_names.end(), row_name) !=
						first_stage->cons_names.end()) {
						auto cons = new inequality;
						auto cons_idx = mean_model->cons_num++;
						for (auto var_name : row_info->var_names) {
							auto coeff = row_info->var_coeffs.at(var_name);
							auto var_idx = mean_model->var_map.at(var_name);
							cons->lhs.push_back(coeff);
							cons->var_indices.push_back(var_idx);
						}
						auto rhs = row_info->rhs;
						cons->rhs = rhs;
						cons->sense = symbol.front();
						cons->cons_name = row_name;
						mean_model->constraints.insert(make_pair(cons_idx, cons));
					}
				}
			}

			//constraints (second stage)
			auto& second_stage = ins->stages_map.at(1);
			for (auto scen_iter : ins->scens_map) {
				auto scen_name = scen_iter.first;
				auto& sceninfo = scen_iter.second;
				for (auto cons_name : sceninfo->cons_names) {
					auto row_info = ins->row_map.at(cons_name);
					auto symbol = row_info->control_symbol;
					auto cons = new inequality;
					auto cons_idx = mean_model->cons_num++;
					for (auto var_name : row_info->var_names) {
						//second stage var
						if (find(sceninfo->var_names.begin(), sceninfo->var_names.end(), var_name) != sceninfo->var_names.end()) {
							auto name = var_name + "{" + scen_name + "}";
							auto var_idx = mean_model->var_map.at(name);
							auto coeff = row_info->var_coeffs.at(var_name);
							cons->lhs.push_back(coeff);
							cons->var_indices.push_back(var_idx);
						}
						//first stage var
						else
						{
							auto var_idx = mean_model->var_map.at(var_name);
							auto coeff = row_info->var_coeffs.at(var_name);
							cons->lhs.push_back(coeff);
							cons->var_indices.push_back(var_idx);
						}
					}
					auto rhs = row_info->rhs;
					auto row_name = row_info->row_name + "_" + scen_name;
					cons->rhs = rhs;
					cons->sense = symbol.front();
					cons->cons_name = row_name;
					mean_model->constraints.insert(make_pair(cons_idx, cons));
				}
			}

			//initialize slack variables
			{
				int cons_num = mean_model->cons_num;
				for (int i = 0; i < cons_num; i++) {
					auto cons = mean_model->constraints.at(i);
					//need a slack variable
					if (cons->sense != 'E') {
						int var_idx = mean_model->var_num++;
						string var_name = "slack{" + to_string(var_idx) + "}";
						mean_model->slack_map.insert(make_pair(var_name, var_idx));
						mean_model->var_names.insert(make_pair(var_idx, var_name));
						//continues variable
						mean_model->var_types.insert(make_pair(var_name, 'C'));
						//set bounds
						mean_model->var_bounds.insert(make_pair(var_name, make_pair(0, INFINITY)));
					}
				}
			}

			CCSVParser parser;
			parser.OpenCSVFile("var_vals.txt", true);
			while (parser.ReadRecord()) {
				string var_name = "";
				parser.GetValueByFieldName("var", var_name);
				int var_idx = mean_model->var_map.at(var_name);

				string var_val_str = "";
				parser.GetValueByFieldName("val", var_val_str);
				double var_val = atof(var_val_str.c_str());

				auto cons = new inequality;
				auto cons_idx = mean_model->cons_num++;
				cons->lhs.push_back(1);
				cons->var_indices.push_back(var_idx);
				cons->rhs = var_val;
				cons->sense = 'E';
				cons->cons_name = "bd_" + to_string(cons_idx);
				mean_model->constraints.insert(make_pair(cons_idx, cons));
			}
		}

		//generate model
		{
			const int var_num = mean_model->var_num;
			double* obj = new double[var_num];
			double* lb = new double[var_num];
			double* ub = new double[var_num];
			char* xtype = new char[var_num];
			char** var_names = new char* [var_num];
			for (auto iter : mean_model->var_names) {
				auto var_idx = iter.first;
				auto& var_name = iter.second;

				//if the objective includes this variable 
				if (mean_model->objective.find(var_idx) != mean_model->objective.end()) {
					obj[var_idx] = mean_model->objective.at(var_idx);
				}
				else
				{
					obj[var_idx] = 0;
				}

				//set bounds
				auto bound = mean_model->var_bounds.at(var_name);
				lb[var_idx] = bound.first;
				ub[var_idx] = bound.second;

				//set type
				xtype[var_idx] = mean_model->var_types.at(var_name);

				//set name
				var_names[var_idx] = (char*)mean_model->var_names.at(var_idx).data();
			}
			CPXnewcols(mean_model->env, mean_model->model, var_num, obj, lb, ub, xtype, var_names);

			int cons_num = mean_model->cons_num;
			double* rhs = new double[cons_num];
			char* sense = new char[cons_num];
			char** row_names = new char* [cons_num];
			for (auto iter : mean_model->constraints) {
				auto cons_idx = iter.first;
				auto cons = iter.second;
				rhs[cons_idx] = cons->rhs;
				sense[cons_idx] = cons->sense;
				row_names[cons_idx] = const_cast<char*>(cons->cons_name.data());
			}
			CPXnewrows(mean_model->env, mean_model->model, cons_num, rhs, sense, NULL, row_names);

			//populate the A matrix
			int num_coef = 0;
			for (auto iter : mean_model->constraints) {
				auto cons = iter.second;
				num_coef += cons->var_indices.size();
			}
			auto row_list = new int[num_coef];
			auto col_list = new int[num_coef];
			auto val_list = new double[num_coef];
			int idx = 0;
			for (auto iter : mean_model->constraints) {
				auto cons_idx = iter.first;
				auto cons = iter.second;
				for (int i = 0; i < cons->var_indices.size(); i++) {
					auto var_idx = cons->var_indices.at(i);
					auto coef = cons->lhs.at(i);
					row_list[idx] = cons_idx;
					col_list[idx] = var_idx;
					val_list[idx] = coef;
					idx++;
				}
			}
			CPXchgcoeflist(mean_model->env, mean_model->model, num_coef, row_list, col_list, val_list);
		}

		//export model
		{
			string ins_folder = root + ins->ins_name + "\\";
			FileHelper::CreatFolderIfNotExist(ins_folder);
			string model_path = ins_folder + "mean_model.lp";
			CPXwriteprob(mean_model->env, mean_model->model, model_path.data(), NULL);
		}

		mean_models.insert(make_pair(ins->ins_name, mean_model));
		mean_model->ins_name = ins->ins_name;

		//enable screen output
		CPXsetintparam(mean_model->env, CPX_PARAM_SCRIND, CPX_ON);

		cout << "Solve the mean model..." << endl;
		cout << endl;

		//optimize the problem
		int error = CPXmipopt(mean_model->env, mean_model->model);

		//get the optimal solution
		if (!error)
		{
			int solstat = -1;
			double objval = 0;
			double* varvals = new double[mean_model->var_num];
			CPXsolution(mean_model->env, mean_model->model, &solstat, &objval, varvals, NULL, NULL, NULL);
			mean_model->obj_value = objval;
			for (auto iter : mean_model->var_names) {
				auto var_idx = iter.first;
				auto var_name = iter.second;
				mean_model->var_values.insert(make_pair(var_name, varvals[var_idx]));
			}
			cout << setprecision(4) << fixed << "Best objective: " << objval << endl;
		}
		else
		{
			throw "error code " + error;
		}
	}
}

void SMPS_Reader::RunBxCy() {
	int status = 0;
	//the global unique enviroment
	auto env = CPXopenCPLEX(&status);
	cout << "Creating two-stage models..." << endl;
	string root = folder + "TwoStageModels\\";
	FileHelper::CreatFolderIfNotExist(root);
	FileHelper::ClearFolder(root);
	//for each instance
	for (auto ins_iter : instances_map) {
		int status;
		auto ins = ins_iter.second;
		auto first_stage = ins->stages_map.front();

		//save the initial subproblems. key: scen_name
		map<string, CPXSubModel*> subprob_map;

		//save LBs for each subproblem. key: scen_name
		map<string, double> subprob_lb_map;

		//create subproblems
		for (auto scen_iter : ins->scens_map) {
			auto sceninfo = scen_iter.second;
			auto sub_model = new CPXSubModel();
			sub_model->env = env;
			sub_model->scen_name = sceninfo->scen_name;
			sub_model->ins_name = ins->ins_name;
			sub_model->name = ins->ins_name + "_" + sceninfo->scen_name;
			sub_model->model = CPXcreateprob(sub_model->env, &status, sub_model->name.data());

			//initialize vars
			{
				for (auto var_name : sceninfo->var_names) {
					int var_idx = sub_model->var_num++;
					double lb = 0, ub = INFINITY;
					auto var_type = ins->var_map.at(var_name);
					if (ins->lower_bounds.find(var_name) != ins->lower_bounds.end()) {
						lb = ins->lower_bounds.at(var_name);
					}
					if (ins->upper_bounds.find(var_name) != ins->upper_bounds.end()) {
						ub = ins->upper_bounds.at(var_name);
					}
					if (var_type == "INT") {
						//binary
						if (lb == 0 && ub == 1) {
							sub_model->var_types.insert(make_pair(var_name, 'B'));
						}
						//pure integer
						else
						{
							sub_model->var_types.insert(make_pair(var_name, 'I'));
						}
					}
					else if (var_type == "DOUBLE")
					{
						sub_model->var_types.insert(make_pair(var_name, 'C'));
					}
					sub_model->var_map.insert(make_pair(var_name, var_idx));
					sub_model->var_names.insert(make_pair(var_idx, var_name));
					sub_model->var_bounds.insert(make_pair(var_name, make_pair(lb, ub)));
					//implement upper bounds as constraints (not binding vars)
					if (ub != INFINITY &&						
						sub_model->binding_var_map.find(var_name) == sub_model->binding_var_map.end()) {
						//relax the bound
						sub_model->var_bounds.at(var_name) = make_pair(lb, INFINITY);
						//generate a constraint for the upper bound
						auto cons = new inequality;
						auto cons_idx = sub_model->cons_num++;
						cons->var_indices.push_back(var_idx);
						cons->lhs.push_back(1);
						cons->rhs = ub;
						cons->sense = 'L';
						cons->cons_name = "ub_" + to_string(cons_idx);
						sub_model->constraints.insert(make_pair(cons_idx, cons));
					}
				}
			}

			//set the objective
			{
				auto objinfo = sceninfo->objective_info;
				objinfo->var_names;
				for (int i = 0; i < objinfo->var_names.size(); i++) {
					auto var_name = objinfo->var_names[i];
					auto coef = objinfo->var_coeffs[i];
					auto var_idx = sub_model->var_map.at(var_name);
					sub_model->objective.insert(make_pair(var_idx, coef));
				}
			}

			//set constraints
			{
				for (auto cons_name : sceninfo->cons_names) {
					auto row_info = ins->row_map.at(cons_name);
					auto symbol = row_info->control_symbol;
					//"N" represents the objective row
					if (symbol != "N")
					{
						auto cons = new inequality;
						auto cons_idx = sub_model->cons_num++;
						for (auto var_name : row_info->var_names) {
							//the variable is a binding variable derived from the master problem
							if (sub_model->var_map.find(var_name) == sub_model->var_map.end()) {
								if (sub_model->binding_var_map.find(var_name) == sub_model->binding_var_map.end()) {
									//declare this variable in this subproblem
									int var_idx = sub_model->var_num++;
									double lb = 0, ub = INFINITY;
									if (ins->lower_bounds.find(var_name) != ins->lower_bounds.end()) {
										lb = ins->lower_bounds.at(var_name);
									}
									if (ins->upper_bounds.find(var_name) != ins->upper_bounds.end()) {
										ub = ins->upper_bounds.at(var_name);
									}
									sub_model->var_types.insert(make_pair(var_name, 'C'));
									sub_model->var_map.insert(make_pair(var_name, var_idx));
									sub_model->var_names.insert(make_pair(var_idx, var_name));
									sub_model->var_bounds.insert(make_pair(var_name, make_pair(lb, ub)));
									sub_model->binding_var_map.insert(make_pair(var_name, var_idx));
								}
							}

							auto coeff = row_info->var_coeffs.at(var_name);
							auto var_idx = sub_model->var_map.at(var_name);
							cons->lhs.push_back(coeff);
							cons->var_indices.push_back(var_idx);
						}
						auto rhs = row_info->rhs;
						cons->rhs = rhs;
						cons->sense = symbol.front();
						cons->cons_name = cons_name;
						sub_model->constraints.insert(make_pair(cons_idx, cons));
					}
				}

				//initialize slack variables
				{
					int cons_num = sub_model->cons_num;
					for (int i = 0; i < cons_num; i++) {
						auto cons = sub_model->constraints.at(i);
						//need a slack variable
						if (cons->sense != 'E') {
							int var_idx = sub_model->var_num++;
							string var_name = "sk" + to_string(var_idx);
							sub_model->slack_map.insert(make_pair(var_name, var_idx));
							sub_model->var_names.insert(make_pair(var_idx, var_name));
							//continues variable
							sub_model->var_types.insert(make_pair(var_name, 'C'));
							//set bounds
							sub_model->var_bounds.insert(make_pair(var_name, make_pair(0, INFINITY)));
							//alter this constraint
							if (cons->sense == 'G') {
								cons->lhs.push_back(-1);
								cons->var_indices.push_back(var_idx);
							}
							else if (cons->sense == 'L') {
								cons->lhs.push_back(1);
								cons->var_indices.push_back(var_idx);
							}
							cons->sense = 'E';
						}
					}
				}
			}

			//generate model
			{
				const int var_num = sub_model->var_num;
				auto obj = new double[var_num];
				auto lb = new double[var_num];
				auto ub = new double[var_num];
				auto xtype = new char[var_num];
				auto var_names = new char* [var_num];
				for (auto iter : sub_model->var_names) {
					auto var_idx = iter.first;
					auto var_name = iter.second;

					//if the objective includes this variable 
					if (sub_model->objective.find(var_idx) != sub_model->objective.end()) {
						obj[var_idx] = sub_model->objective.at(var_idx);
					}
					else
					{
						obj[var_idx] = 0;
					}

					//set bounds
					auto bound = sub_model->var_bounds.at(var_name);
					lb[var_idx] = bound.first;
					ub[var_idx] = bound.second;

					//set type
					xtype[var_idx] = sub_model->var_types.at(var_name);

					//set name
					var_names[var_idx] = (char*)sub_model->var_names.at(var_idx).data();
				}
				CPXnewcols(sub_model->env, sub_model->model, var_num, obj, lb, ub, xtype, var_names);
				CPXHelper::AddRows(&sub_model->constraints, sub_model->env, sub_model->model);
			}

			//solve LP relaxation to obtain the lower bound
			{
				TurnLP(sub_model);
				//optimize the problem
				int error = CPXlpopt(sub_model->env, sub_model->model);
				//get the optimal solution
				if (!error)
				{
					CPXgetobjval(sub_model->env, sub_model->model, &sub_model->obj_value);
					subprob_lb_map.insert(make_pair(sub_model->scen_name, sub_model->obj_value));
				}
				else
				{
					throw "error code " + error;
				}
			}

			subprob_map.insert(make_pair(sceninfo->scen_name, sub_model));
		}

#pragma region create the master problem
		auto master_model = new CPXMasModel();
		master_model->env = env;
		master_model->name = ins->ins_name + "_master";
		master_model->model = CPXcreateprob(master_model->env, &status, master_model->name.data());

		//initialize vars
		{
			//thetas: using a varible theta to capture the objective of each scenario			
			for (auto scen_iter : ins->scens_map) {
				auto sceninfo = scen_iter.second;
				string var_name = "theta{" + sceninfo->scen_name + "}";
				int var_idx = master_model->var_num++;
				master_model->estimator_map.insert(make_pair(var_name, var_idx));
				master_model->var_names.insert(make_pair(var_idx, var_name));
				//continues variable
				master_model->var_types.insert(make_pair(var_name, 'C'));
				//set bounds
				auto model_key = make_pair(ins->ins_name, sceninfo->scen_name);
				auto lb = subprob_lb_map.at(sceninfo->scen_name);
				master_model->var_bounds.insert(make_pair(var_name, make_pair(lb, INFINITY)));
			}
			//first stage variables
			{
				auto& first_stage = ins->stages_map.front();
				for (auto var_name : first_stage->var_names) {
					int var_idx = master_model->var_num++;
					double lb = 0, ub = INFINITY;
					auto var_type = ins->var_map.at(var_name);
					if (ins->lower_bounds.find(var_name) != ins->lower_bounds.end()) {
						lb = ins->lower_bounds.at(var_name);
					}
					if (ins->upper_bounds.find(var_name) != ins->upper_bounds.end()) {
						ub = ins->upper_bounds.at(var_name);
					}
					if (var_type == "INT") {
						master_model->var_types.insert(make_pair(var_name, 'I'));
					}
					else if (var_type == "DOUBLE")
					{
						master_model->var_types.insert(make_pair(var_name, 'C'));
					}
					master_model->var_map.insert(make_pair(var_name, var_idx));
					master_model->var_names.insert(make_pair(var_idx, var_name));
					master_model->var_bounds.insert(make_pair(var_name, make_pair(lb, ub)));
				}
			}
		}

		//set the objective
		{
			for (auto iter : ins->master_obj) {
				auto var_name = iter.first;
				auto coef = iter.second;
				auto var_idx = master_model->var_map.at(var_name);
				master_model->objective.insert(make_pair(var_idx, coef));
			}
			//from the second stage
			for (auto scen_iter : ins->scens_map) {
				auto scen_name = scen_iter.first;
				auto& scen = scen_iter.second;
				string var_name = "theta{" + scen->scen_name + "}";
				auto& estimator = master_model->estimator_map.at(var_name);
				auto estima_idx = master_model->estimator_map.at(var_name);
				master_model->objective.insert(make_pair(estima_idx, scen->prob));
			}
		}

		//set constraints
		{
			for (auto cons_name : first_stage->cons_names) {
				auto row_info = ins->row_map.at(cons_name);
				auto symbol = row_info->control_symbol;
				//"N" represents the objective row
				if (symbol != "N")
				{
					auto cons = new inequality;
					auto cons_idx = master_model->cons_num++;
					for (auto var_name : row_info->var_names) {
						auto coeff = row_info->var_coeffs.at(var_name);
						auto var_idx = master_model->var_map.at(var_name);
						cons->lhs.push_back(coeff);
						cons->var_indices.push_back(var_idx);
					}
					auto rhs = row_info->rhs;
					cons->rhs = rhs;
					cons->sense = symbol.front();
					cons->cons_name = cons_name;
					master_model->constraints.insert(make_pair(cons_idx, cons));
				}
			}
		}

		//generate model
		{
			const int var_num = master_model->var_num;
			double* obj = new double[var_num];
			double* lb = new double[var_num];
			double* ub = new double[var_num];
			char* xtype = new char[var_num];
			char** var_names = new char* [var_num];
			for (auto iter : master_model->var_names) {
				auto var_idx = iter.first;
				auto& var_name = iter.second;

				//if the objective includes this variable 
				if (master_model->objective.find(var_idx) != master_model->objective.end()) {
					obj[var_idx] = master_model->objective.at(var_idx);
				}
				else
				{
					obj[var_idx] = 0;
				}

				//set bounds
				auto bound = master_model->var_bounds.at(var_name);
				lb[var_idx] = bound.first;
				ub[var_idx] = bound.second;

				//set type
				xtype[var_idx] = master_model->var_types.at(var_name);

				//set name
				var_names[var_idx] = (char*)master_model->var_names.at(var_idx).data();
			}
			CPXnewcols(master_model->env, master_model->model, var_num, obj, lb, ub, xtype, var_names);
			CPXHelper::AddRows(&master_model->constraints, master_model->env, master_model->model);
		}
#pragma endregion

		//run BxCy
		vector<node*> queue;//the nodes waited to be explored
		vector<double> trend_UB;//the trend of the global upper bound
		vector<double> trend_LB;
		double global_LB = -INFINITY;
		double global_UB = INFINITY;
		map<string, map<string, double>> best_sol;
		node* incum_node = nullptr;
		double gap = INFINITY;
		//creat the root node
		int node_idx = 0;
		auto root_node = new node(master_model, node_idx++);
		root_node->node_idx = node_idx;
		queue.push_back(root_node);
		int count = 0;
		bool goto_case_5 = false;
		//loop
		while (queue.size() > 0 && global_UB - global_LB > eps)
		{
			count++;
			//deep-first search, last in first out
			auto cur_node = queue.back();
			auto cur_mas_model = cur_node->model;
			queue.pop_back();
			//disable screen output
			CPXsetintparam(cur_mas_model->env, CPX_PARAM_SCRIND, CPX_OFF);
			//trun-off the presolve
			CPXsetintparam(cur_mas_model->env, CPX_PARAM_PREIND, CPX_OFF);
			//solve this node (relaxed master problem)
			TurnLP(cur_mas_model);

		solve_master:
			/*cout << "nodes in queue: " << queue.size() << endl;*/
			int error = CPXlpopt(cur_mas_model->env, cur_mas_model->model);
			//is feasible
			if (!error)
			{
				//export model
				{
					string ins_folder = root + ins->ins_name + "\\";
					FileHelper::CreatFolderIfNotExist(ins_folder);
					string model_path = ins_folder + "master_model.lp";
					CPXwriteprob(cur_mas_model->env, cur_mas_model->model, model_path.data(), NULL);
				}
				//get the optimal solution of the master problem
				int solstat = -1;
				cur_node->is_integer = true;
				double objval = 0;
				auto varvals = new double[cur_mas_model->var_num];
				CPXsolution(cur_mas_model->env, cur_mas_model->model, &solstat,
					&objval, varvals, NULL, NULL, NULL);
				//not optimal
				if (solstat != CPX_STAT_OPTIMAL) {
					throw "error!The solution is not optimal";
				}
				cur_mas_model->obj_value = objval;
				cur_mas_model->var_values.clear();
				cur_node->branch_var_list.clear();
				for (auto iter : cur_mas_model->var_names) {
					auto var_idx = iter.first;
					auto var_name = iter.second;

					////not meet the precision of integral
					//if (abs(varvals[var_idx] - round(varvals[var_idx])) > eps) {
					//	//not estimators, not integers
					//	if (cur_mas_model->estimator_map.find(var_name) == cur_mas_model->estimator_map.end()) {
					//		cur_node->is_integer = false;
					//		cur_node->branch_var_list.push_back(var_name);
					//	}
					//}

					//check integral
					if (abs(varvals[var_idx] - round(varvals[var_idx])) <= eps) {
						varvals[var_idx]= round(varvals[var_idx]);
					}
					else
					{
						//not estimators, not integers
						if (cur_mas_model->estimator_map.find(var_name) == cur_mas_model->estimator_map.end()) {
							cur_node->is_integer = false;
							cur_node->branch_var_list.push_back(var_name);
						}
					}
					cur_mas_model->var_values.insert(make_pair(var_name, varvals[var_idx]));
				}

				//check whether all subproblems are well estimated by thetas
				bool all_estimated = true;
				//check whether all subproblems obtain integer solutions
				bool all_integer = true;
				map<string, double> theta_vals;

				//set relaxed subproblems
				for (auto scen_iter : ins->scens_map) {
					auto sceninfo = scen_iter.second;
					CPXSubModel* submodel = nullptr;
					submodel = subprob_map.at(sceninfo->scen_name);
					//set estimators information
					auto theta_var_name = "theta{" + sceninfo->scen_name + "}";
					auto theta_val = cur_mas_model->var_values.at(theta_var_name);
					theta_vals.insert(make_pair(sceninfo->scen_name, theta_val));

					//set binding constraints
					if (submodel->binding_cons.size() == 0) {
						for (auto var_iter : submodel->binding_var_map) {
							auto var_name = var_iter.first;
							auto var_idx = var_iter.second;
							auto cons_idx = submodel->cons_num++;
							auto cons = new inequality();
							cons->cons_name = "bd_" + to_string(cons_idx);
							cons->var_indices.push_back(var_idx);
							cons->lhs.push_back(1);
							cons->sense = 'E';
							cons->rhs = cur_mas_model->var_values.at(var_name);
							submodel->constraints.insert(make_pair(cons_idx, cons));
							submodel->binding_cons.insert(make_pair(cons_idx, cons));
						}
						CPXHelper::AddRows(&submodel->binding_cons, submodel->env, submodel->model);
					}
					else
					{
						int cnt = submodel->binding_cons.size();
						auto indices = new int[cnt]();
						auto values = new double[cnt]();
						int idx = 0;
						for (auto cons_iter : submodel->binding_cons) {
							auto cons_idx = cons_iter.first;
							auto var_idx = cons_iter.second->var_indices.front();
							auto var_name = submodel->var_names.at(var_idx);
							//change the rhs of each binding constraint to update the problem
							indices[idx] = cons_idx;
							values[idx] = cur_mas_model->var_values.at(var_name);
							idx++;
						}
						CPXchgrhs(submodel->env, submodel->model, cnt, indices, values);
					}
				}

			solve_subs:
				for (auto scen_iter : ins->scens_map) {
					auto sceninfo = scen_iter.second;
					CPXSubModel* submodel = nullptr;
					submodel = subprob_map.at(sceninfo->scen_name);
					//optimize the problem	
					TurnLP(submodel);
					//trun-off the presolve
					CPXsetintparam(submodel->env, CPX_PARAM_PREIND, CPX_OFF);
					CPXsetintparam(submodel->env, CPXPARAM_LPMethod, CPX_ALG_PRIMAL);
					int error = CPXlpopt(submodel->env, submodel->model);
					// CPXMIP_OPTIMAL();
					//get the optimal solution
					if (error) {
						throw "error code " + error;
					}

					//save the solution
					submodel->is_integer = true;
					submodel->var_values.clear();
					submodel->duals.clear();
					auto var_vals = new double[submodel->var_num];
					auto duals = new double[submodel->cons_num];
					CPXsolution(submodel->env, submodel->model, &solstat,
						&submodel->obj_value, var_vals, duals, NULL, NULL);
					//not optimal
					if (solstat != CPX_STAT_OPTIMAL) {
						string file_name = "error_model.lp";
						CPXwriteprob(env, submodel->model, file_name.data(), NULL);
						throw "error!The solution is not optimal";
					}
					all_integer = true;
					for (auto iter : submodel->var_names) {
						auto var_idx = iter.first;
						auto var_name = iter.second;
						auto var_val = var_vals[var_idx];

						////not integer
						//if (abs(var_val - round(var_val)) > eta) {
						//	auto var_type = submodel->var_types.at(var_name);
						//	//the variable is required to be an integer
						//	if (all_integer && var_type != 'C') {
						//		//violated
						//		all_integer = false;
						//		submodel->is_integer = false;
						//	}
						//}

						//meet the precision of integral
						if (abs(var_val - round(var_val)) <= eps) {
							var_val = round(var_val);
						}
						else
						{
							auto var_type = submodel->var_types.at(var_name);
							//the variable is required to be an integer
							if (all_integer && (var_type == 'B' || var_type == 'I')) {
								//violated
								all_integer = false;
								submodel->is_integer = false;
							}
						}
						submodel->var_values.insert(make_pair(var_name, var_val));
					}
					for (int i = 0; i < submodel->cons_num; i++) {
						submodel->duals.push_back(duals[i]);
					}

					//check estimators	
					auto theta_var_name = "theta{" + sceninfo->scen_name + "}";
					auto theta_val = cur_mas_model->var_values.at(theta_var_name);
					if (abs(theta_val - submodel->obj_value) > eps) {
						all_estimated = false;
					}
				}

				cout << setprecision(8) << fixed << "node: " << node_idx << endl;

				if (goto_case_5) {
					goto case5;
				}

				//case 1
				if (!(cur_node->is_integer) && all_estimated) {
					//the variable needed to be branched
					auto brach_var_name = cur_node->branch_var_list.front();
					auto bracn_var_val = cur_mas_model->var_values.at(brach_var_name);
					auto brach_var_idx = cur_mas_model->var_map.at(brach_var_name);
					//the left node
					{
						auto mas_mode_copy = cur_mas_model->deepcopy();
						auto left_node = new node(mas_mode_copy, node_idx++);
						//add the branching constraint
						auto cut_idx = mas_mode_copy->cons_num++;
						auto cut = new inequality();
						cut->cons_name = "branch_" + to_string(cut_idx);
						cut->rhs = floor(bracn_var_val);
						cut->sense = 'L';
						cut->var_indices.push_back(brach_var_idx);
						cut->lhs.push_back(1);
						mas_mode_copy->constraints.insert(make_pair(cut_idx, cut));
						//add a new row
						map<int, inequality*> temp_map;
						temp_map.insert(make_pair(cut_idx, cut));
						CPXHelper::AddRows(&temp_map, mas_mode_copy->env, mas_mode_copy->model);
						//append to the stack
						queue.push_back(left_node);
					}
					//the right node
					{
						auto mas_mode_copy = cur_mas_model->deepcopy();
						auto right_node = new node(mas_mode_copy, node_idx++);
						//add the branching constraint
						auto cut_idx = mas_mode_copy->cons_num++;
						auto cut = new inequality();
						cut->cons_name = "branch_" + to_string(cut_idx);
						cut->rhs = ceil(bracn_var_val);
						cut->sense = 'G';
						cut->var_indices.push_back(brach_var_idx);
						cut->lhs.push_back(1);
						mas_mode_copy->constraints.insert(make_pair(cut_idx, cut));
						//add a new row
						map<int, inequality*> temp_map;
						temp_map.insert(make_pair(cut_idx, cut));
						CPXHelper::AddRows(&temp_map, mas_mode_copy->env, mas_mode_copy->model);
						//append to the stack
						queue.push_back(right_node);
					}
					//free the current node
					delete cur_node;
				}
				//case 2
				else if (!(cur_node->is_integer) && !all_estimated)
				{
					//add a Banders optimality cut for each underestimated subproblem
					for (auto iter : subprob_map) {
						auto scen_name = iter.first;
						auto submodel = iter.second;
						if (abs(theta_vals.at(scen_name) - submodel->obj_value) > eps) {
							AddOptCut(cur_mas_model, submodel);
						}
					}
					//return to the master problem
					goto solve_master;
				}
				//case 3
				else if (cur_node->is_integer && all_integer && !all_estimated) {
					//feasible solution
					double temp_UB = cur_mas_model->obj_value;
					for (auto iter : cur_mas_model->estimator_map) {
						auto var_name = iter.first;
						auto idx = iter.second;
						temp_UB -= cur_mas_model->objective.at(idx) *
							cur_mas_model->var_values.at(var_name);
					}
					//add a Banders optimality cut for each underestimated subproblem
					for (auto iter : subprob_map) {
						auto scen_name = iter.first;
						auto submodel = iter.second;
						if (abs(theta_vals.at(scen_name) - submodel->obj_value) > eps) {
							AddOptCut(cur_mas_model, submodel);
						}
						string esti_name = "theta{" + scen_name + "}";
						auto esti_idx = cur_mas_model->estimator_map.at(esti_name);
						temp_UB += cur_mas_model->objective.at(esti_idx) * submodel->obj_value;

						string file_name = "test_sub.lp";
						int error_code = CPXwriteprob(env, submodel->model, file_name.data(), NULL);
					}
					//update the global UB and incumbent
					if (temp_UB < global_UB) {
						incum_node = cur_node;
						best_sol.clear();
						for (auto iter : subprob_map) {
							auto scen_name = iter.first;
							auto submodel = iter.second;
							map<string, double> temp_map;
							for (auto iter : submodel->var_map) {
								auto var_name = iter.first;
								//don not save slack variables and binding variables 
								if (submodel->slack_map.find(var_name) == submodel->slack_map.end() &&
									submodel->binding_var_map.find(var_name) == submodel->binding_var_map.end()) {
									temp_map.insert(make_pair(var_name, submodel->var_values.at(var_name)));
								}
							}
							best_sol.insert(make_pair(scen_name, temp_map));
						}
						global_UB = temp_UB;
						cout << "---------------------------------------------------------------------------" << endl;
						cout << "Update incumbent: " << global_UB << endl;
					}
					//return to the master problem
					goto solve_master;
				}
				//case 4
				else if (cur_node->is_integer && all_integer && all_estimated) {

					//double temp_UB = cur_mas_model->obj_value;
					//for (auto iter : subprob_map) {
					//	auto scen_name = iter.first;
					//	auto submodel = iter.second;
					//	/*vector<string> var_names = { "x0","x1","y0","y1","y2","y3" };
					//	vector<double> var_vals = { 1,0,0,0,1,0 };*/
					//	vector<string> var_names = { "y0","y1","y2","y3" };
					//	vector<double> var_vals = { 0,0,1,0 };
					//	map<int, inequality*> temp_map;
					//	for (int i = 0; i < var_names.size(); i++) {
					//		auto var_name = var_names.at(i);
					//		auto var_idx = submodel->var_map.at(var_name);
					//		auto val = var_vals.at(i);
					//		auto cons = new inequality();
					//		cons->lhs.push_back(1);
					//		cons->sense = 'E';
					//		cons->rhs = val;
					//		cons->var_indices.push_back(var_idx);
					//		auto cons_idx = submodel->cons_num++;
					//		cons->cons_name = "bd_" + to_string(cons_idx);
					//		temp_map.insert(make_pair(cons_idx, cons));
					//	}
					//	AddRows(temp_map, env, submodel->model);
					//	TurnLP(submodel);
					//	string file_name = "test_sub.lp";
					//	int error_code = CPXwriteprob(env, submodel->model, file_name.data(), NULL);
					//	auto error = CPXlpopt(env, submodel->model);
					//	{
					//		submodel->is_integer = true;
					//		submodel->var_values.clear();
					//		auto var_vals = new double[submodel->var_num];
					//		CPXsolution(env, submodel->model, &solstat,
					//			&submodel->obj_value, var_vals, NULL, NULL, NULL);
					//		if (solstat == CPX_STAT_INFEASIBLE) {
					//			int test = CPXclpwrite(env, submodel->model, "conflict.lp");
					//			int kkksd = 0;
					//		}

					//		for (auto iter : submodel->var_names) {
					//			auto var_idx = iter.first;
					//			auto var_name = iter.second;
					//			auto var_val = var_vals[var_idx];
					//			submodel->var_values.insert(make_pair(var_name, var_val));
					//			auto var_type = submodel->var_types.at(var_name);
					//			//the variable is required to be an integer
					//			if (all_integer &&
					//				(var_type == 'B' || var_type == 'I')) {
					//				//violated
					//				if (abs(var_val - (int)var_val) > eps) {
					//					all_integer = false;
					//					submodel->is_integer = false;
					//				}
					//			}
					//		}
					//	}
					//	int kkskd = 0;
					//}
					
					//prune this node
					if (cur_node != incum_node) {
						delete cur_node;
					}
				}
				//case 5
				else if (cur_node->is_integer && !all_integer && all_estimated) {
				case5:
					int ncut = 0;
					//generate valid inequalities for subproblems
					vector<double> obj_vals;
					for (auto iter : subprob_map) {
						auto scen_name = iter.first;
						auto submodel = iter.second;
						obj_vals.push_back(submodel->obj_value);
						//cout << endl;
						//cout << "subproblem " << scen_name << "-------------------------------------------" << endl;
						if (!submodel->is_integer) {
							//0-1 extended cover inequalities
							ncut += AddCoverCut(cur_mas_model, submodel);
							//gomory cuts
							/*ncut += AddGomoryCut(cur_mas_model, submodel);*/
							if (ncut == 0) {
								ncut += AddGomoryCut(cur_mas_model, submodel);
							}

							/*string file_name = scen_name + ".lp";
							CPXwriteprob(env, submodel->model, file_name.data(), NULL);
							int kksd = 0;*/
						}
					}
					cout << endl;
					for (auto iter : subprob_map) {
						auto scen_name = iter.first;
						auto submodel = iter.second;
						cout << setprecision(8) << fixed << submodel->scen_name << ": " <<
							submodel->obj_value << "	";
					}
					cout << endl;
					//return to subproblems
					goto solve_subs;
				}
				//case 6
				else if (cur_node->is_integer && !all_integer && !all_estimated)
				{
					//add a Banders optimality cut for each underestimated subproblem
					for (auto iter : subprob_map) {
						auto scen_name = iter.first;
						auto submodel = iter.second;
						auto theta_val = theta_vals.at(scen_name);
						if (abs(theta_val - submodel->obj_value) > eps) {
							AddOptCut(cur_mas_model, submodel);
						}
					}
					//return to the master problem
					goto solve_master;
				}
			}
			//is not feasible
			else
			{
				throw "error code " + error;
			}
		}

		cout << endl;
		cout << setprecision(2) << fixed << "Incumbent: " << global_UB << endl;
	}
}

template<typename T> void SMPS_Reader::TurnLP(T model) {
	static_assert(is_same<T, CPXMasModel*>::value || is_same<T, CPXSubModel*>::value, "Only support CPXMasModel*, CPXSubModel*");
	CPXchgprobtype(model->env, model->model, CPXPROB_LP);
	model->model_type = "LP";
}

template<typename T> void SMPS_Reader::TurnIP(T model) {
	static_assert(is_same<T, CPXMasModel*>::value || is_same<T, CPXSubModel*>::value, "Only support CPXMasModel*, CPXSubModel*");
	CPXchgprobtype(model->env, model->model, CPXPROB_MILP);
	vector<int> idx_list;
	vector<char> type_list;
	int cnt = 0;
	for (auto iter : model->var_names) {
		auto idx = iter.first;
		auto var_name = iter.second;
		auto var_type = model->var_types.at(var_name);
		if (var_type != 'C') {
			idx_list.push_back(idx);
			type_list.push_back(var_type);
			cnt++;
		}
	}
	auto indices = new int[cnt];
	auto xctype = new char[cnt];
	for (int i = 0; i < cnt; i++) {
		indices[i] = idx_list[i];
		xctype[i] = type_list[i];
	}
	CPXchgctype(model->env, model->model, cnt, indices, xctype);
	model->model_type = "IP";
}

void SMPS_Reader::AddOptCut(CPXMasModel* mas_model, CPXSubModel* sub_model) {
	auto opt_cut = new inequality();
	auto cut_idx = mas_model->cons_num++;
	mas_model->constraints.insert(make_pair(cut_idx, opt_cut));
	mas_model->opt_cuts.push_back(cut_idx);
	opt_cut->sense = 'G';
	//the index of the corresponding estimator in the master problem
	auto est_idx = mas_model->estimator_map.at("theta{" + sub_model->scen_name + "}");
	opt_cut->var_indices.push_back(est_idx);
	int non_zeros = 1;
	opt_cut->lhs.push_back(1);
	opt_cut->rhs += sub_model->obj_value;
	opt_cut->cons_name = "optCut_" + to_string(cut_idx);
	for (auto iter : sub_model->binding_cons) {
		auto cons_idx = iter.first;
		auto cons = iter.second;
		auto dual_value = sub_model->duals.at(cons_idx);
		auto master_var_name = sub_model->var_names.at(cons->var_indices.front());
		auto master_var_val = sub_model->var_values.at(master_var_name);
		//the index of the corresponding master variable
		auto master_var_idx = mas_model->var_map.at(master_var_name);
		opt_cut->var_indices.push_back(master_var_idx);
		opt_cut->lhs.push_back(-1 * dual_value);
		non_zeros++;
		//alter rhs
		opt_cut->rhs += -dual_value * master_var_val;
	}

	//populate this cut into the model
	map<int, inequality*> temp_map;
	temp_map.insert(make_pair(cut_idx, opt_cut));
	CPXHelper::AddRows(&temp_map, mas_model->env, mas_model->model);
}

int SMPS_Reader::AddGomoryCut(CPXMasModel* mas_model, CPXSubModel* sub_model) {
	int ncut = 0;
	auto env = sub_model->env;
	auto model = sub_model->model;
	int col_num = sub_model->var_num;
	int row_num = sub_model->cons_num;
	auto true_col_num = CPXgetnumcols(env, sub_model->model);

	auto cstat = new int[col_num]();
	CPXgetbase(env, sub_model->model, cstat, NULL);
	vector<int> temp_cstas;
	vector<string> var_names;
	for (int i = 0; i < col_num; i++) {
		temp_cstas.push_back(cstat[i]);
		var_names.push_back(sub_model->var_names.at(i));
	}

	auto solstat = CPXgetstat(env, sub_model->model);

	//get b_head
	auto bc_heads = new int[row_num]();//the index of the corresponding basic variable on each row of the optimal tableau
	auto bc_vals = new double[row_num]();//the value of the corresponding basic variable for each row
	int error = CPXgetbhead(env, model, bc_heads, bc_vals);

	map<string, int> b_head_map;
	for (int i = 0; i < row_num; i++) {
		auto idx = bc_heads[i];
		auto var_name = sub_model->var_names.at(idx);
		b_head_map.insert(make_pair(var_name, i));
	}

	//cout << "mas sol: ";
	//for (auto iter : sub_model->binding_var_map) {
	//	auto bd_name = iter.first;
	//	cout << setprecision(2) << fixed << bd_name << "=" << sub_model->var_values.at(bd_name) << "	";
	//}
	//cout << endl;

	//cout << "basic:	";
	//for (int i = 0; i < row_num; i++) {
	//	auto var_name = sub_model->var_names.at(bc_heads[i]);
	//	/*cout << setprecision(4) << fixed << setw(4) << var_name << "	";*/
	//	if (sub_model->slack_map.find(var_name) == sub_model->slack_map.end() &&
	//		sub_model->binding_var_map.find(var_name) == sub_model->binding_var_map.end()) {
	//		cout << setprecision(4) << fixed << setw(4) << var_name << "	";
	//	}
	//}
	//cout << endl;

	//cout << "value:	";
	//for (int i = 0; i < row_num; i++) {
	//	auto var_name = sub_model->var_names.at(bc_heads[i]);
	//	if (sub_model->slack_map.find(var_name) == sub_model->slack_map.end() &&
	//		sub_model->binding_var_map.find(var_name) == sub_model->binding_var_map.end()) {
	//		cout << setprecision(4) << fixed << setw(4) << bc_vals[i] << "	";
	//	}
	//}
	//cout << endl;

	for (int i = 0; i < col_num; i++) {
		auto temp_name = sub_model->var_names.at(i);
		auto temp_val = sub_model->var_values.at(temp_name);
		if (cstat[i] == CPX_AT_UPPER) {
			int kkkds = 0;
		}
		else if (cstat[i] == CPX_FREE_SUPER) {
			int kkkds = 0;
		}
		/*temp_cstas.push_back(cstat[i]);
		var_names.push_back(sub_model->var_names.at(i));*/
	}

	//string file_path = "var_vals.txt";
	//FileHelper::CreatFileIfNotExist(file_path);
	//ofstream ofs;
	//ofs.open(file_path);
	//for (int i = 0; i < col_num; i++) {
	//	auto var_name = sub_model->var_names.at(i);
	//	auto var_val = sub_model->var_values.at(var_name);
	//	string str = "";
	//	if (cstat[i] == CPX_AT_LOWER) {
	//		/*str = "lb";*/
	//	}
	//	else if (cstat[i] == CPX_BASIC) {
	//		str = "bs";
	//	}
	//	else
	//	{
	//		int kkksd = 0;
	//	}
	//	if (var_name[0] == 'x') {
	//		ofs << var_name << "," << var_val << endl;
	//	}
	//	/*ofs << setprecision(4) << fixed << var_name << setw(6) << "	" << var_val << "	" << str << endl;*/
	//}
	//ofs.close();

	/*printf("\nOptimal solution (non basic variables are equal to zero):\n");
	for (int i = 0; i < row_num; i++) {
		auto z = new double[col_num];
		int error_code = CPXbinvarow(env, model, i, z);
		for (int j = 0; j < col_num; j++) {
			auto col_name = sub_model->var_names.at(j);
			cout << setprecision(2) << fixed << setw(8) << z[j] << "*" << col_name << "  ";
		}
		cout << bc_vals[i] << endl;
	}
	cout << endl;*/

	//generate cuts
	map<int, string> sk_map;//save the information of new slack variables to be added
	map<int, inequality*> temp_map;//save the information of new cuts
	for (int i = 0; i < row_num; i++) {
		//the basic information of row i
		auto bc_idx = bc_heads[i];
		auto bc_val = bc_vals[i];
		auto var_name = sub_model->var_names.at(bc_idx);
		//ignore the slack variable
		if (sub_model->slack_map.find(var_name) != sub_model->slack_map.end()) {
			continue;
		}

		//if the basic is not integer, a gomory cut is needed
		if (abs(bc_val - round(bc_val)) > 0) {
			//adding a slack variable for the cut
			auto slack_var_idx = sub_model->var_num++;
			auto slack_var_name = "sk" + to_string(slack_var_idx);
			sub_model->slack_map.insert(make_pair(slack_var_name, slack_var_idx));
			sub_model->var_names.insert(make_pair(slack_var_idx, slack_var_name));
			//continues variable
			sub_model->var_types.insert(make_pair(slack_var_name, 'C'));
			//set bounds
			sub_model->var_bounds.insert(make_pair(slack_var_name, make_pair(0, INFINITY)));
			//add to the map
			sk_map.insert(make_pair(slack_var_idx, slack_var_name));

			auto cut_idx = sub_model->cons_num++;
			auto z = new double[col_num];
			//get the i-th row of the optimal tableau
			CPXbinvarow(env, model, i, z);
			auto cut = new inequality();
			//the slack varible changes the sensitivity to "equal"
			cut->sense = 'E';
			cut->rhs = floor(bc_val);
			cut->cons_name = "gc_" + to_string(cut_idx);

			//string file_path = "cut.txt";
			//FileHelper::CreatFileIfNotExist(file_path);
			//ofstream ofs;
			//ofs.open(file_path);

			//string orin_file_path = "orin_cut.txt";
			//FileHelper::CreatFileIfNotExist(orin_file_path);
			//ofstream orin_ofs;
			//orin_ofs.open(orin_file_path);
			
			for (int j = 0; j < col_num; j++) {
				//only non-zeros
				auto z_val = CommonHelper::Floor(z[j]);
				auto temp_name = sub_model->var_names[j];
				if (z_val != 0) {					
					cut->var_indices.push_back(j);
					cut->lhs.push_back(z_val);

					/*ofs << setprecision(4) << fixed << z_val << " * " << temp_name << endl;		*/			
				}

				/*orin_ofs << setprecision(4) << fixed << z[j] << " * " << temp_name << endl;*/
			}

			/*ofs.close();

			orin_ofs.close();*/

			//adding the slack variable to the cut
			cut->var_indices.push_back(slack_var_idx);
			cut->lhs.push_back(1);
			//lazy update
			temp_map.insert(make_pair(cut_idx, cut));
			ncut++;
			//output to the screen
			/*cout << "cut for " << var_name << ": ";
			for (int idx = 0; idx < cut->var_indices.size(); idx++) {
				auto coef = cut->lhs.at(idx);
				auto col_idx = cut->var_indices.at(idx);
				auto col_name = sub_model->var_names.at(col_idx);
				if (coef > 0) {
					cout << setprecision(2) << fixed << "+" << coef;
				}
				else
				{
					cout << setprecision(2) << fixed << coef;
				}
				cout << "*" << col_name << "	";
			}
			cout << setprecision(2) << fixed << "=" << cut->rhs << endl;*/

			//vector<double*> cols;
			//for (int j = 0; j < col_num; j++) {
			//	auto col_vals = new double[row_num];
			//	CPXbinvacol(env, model, j, col_vals);
			//	cols.push_back(col_vals);
			//}

			/*cout << "cut for " << var_name << ": " << endl;*/
			cout << setprecision(4) << fixed << "cut for " << var_name << ": " << bc_val << endl;
			//check cut
			double lhs = 0;
			for (int k = 0; k < cut->var_indices.size(); k++) {
				auto var_id = cut->var_indices.at(k);
				auto temp_name = sub_model->var_names.at(var_id);
				if (temp_name != slack_var_name) {
					auto var_val = sub_model->var_values.at(temp_name);
					auto coeff = cut->lhs.at(k);
					string str = "";
					if (cstat[var_id] == 0) {
						str = "lb";
					}
					else if (cstat[var_id] == 1) {
						str = "bs";
					}
					else if (cstat[var_id] == 2) {
						str = "ub";
					}
					lhs += coeff * var_val;
				}
			}
			bool flags = false;		
			if (lhs <= cut->rhs) {
				string file_name = "error_gomory.lp";
				CPXwriteprob(env, sub_model->model, file_name.data(), NULL);
				vector<int> temp_bc_heads;
				for (int mm = 0; mm < row_num; mm++) {
					temp_bc_heads.push_back(bc_heads[mm]);
				}
				flags = true;
				int kkksd = 0;
			}
			if (lhs != bc_val) {
				string file_name = "error_gomory.lp";
				CPXwriteprob(env, sub_model->model, file_name.data(), NULL);
				vector<int> temp_bc_heads;
				for (int mm = 0; mm < row_num; mm++) {
					temp_bc_heads.push_back(bc_heads[mm]);
				}
				flags = true;
				int kkksd = 0;
			}
		}
	}

	//add slack variables
	for (auto sk_iter : sk_map) {
		auto sk_idx = sk_iter.first;
		auto sk_name = sk_iter.second;
		//add to the model
		const int ccnt = 1;
		int nzcnt = 0;
		auto lb = new double[ccnt] {0};
		auto ub = new double[ccnt] {INFINITY};
		auto obj = new double[ccnt] {0};
		auto var_type = new char[ccnt] {'C'};
		auto colname = new char* [ccnt] {(char*)sub_model->var_names.at(sk_idx).data()};
		CPXnewcols(env, model, ccnt, obj, lb, ub, var_type, colname);
	}

	//add cuts
	CPXHelper::AddRows(&temp_map, sub_model->env, sub_model->model);

	/*string file_name = "test_sub.lp";
	int error_code = CPXwriteprob(env, model, file_name.data(), NULL);
	int kksd = 0;*/

	return ncut;
}

int SMPS_Reader::AddCoverCut(CPXMasModel* mas_model, CPXSubModel* sub_model) {
	int ncut = 0;
	auto env = sub_model->env;
	auto model = sub_model->model;
	int col_num = sub_model->var_num;
	int row_num = sub_model->cons_num;
	//for each constraint, we check and get the standard form of the cover inequality
	for (auto iter : sub_model->constraints) {
		auto cons_idx = iter.first;
		auto cons = iter.second;
		auto cons_name = cons->cons_name;
		//if the constraint is a gomory cut, 0-1 knapsack cover cut, binding constraint and bounding constraint, then skip it
		if (cons_name.find("gc_") != string::npos || cons_name.find("kc_") != string::npos ||
			cons_name.find("bd_") != string::npos || cons_name.find("ub_") != string::npos ||
			cons_name.find("lb_") != string::npos) {
			continue;
		}

		//normalize the form of the constraint to keep accordance with the context of cover cut
		CoverInequalitySeparator cover_sp;
		cover_sp.normalizeCoverConstraint(env, sub_model, cons);

		//generate the cover cut
		if (!cover_sp.is_cover_cons) {
			continue;
		}
		map<string, double> variableValues;//the values of the optimal lp solution of subproblem
		for (auto iter : sub_model->var_names) {
			auto var_idx = iter.first;
			auto var_name = iter.second;
			//exclude slack varibles and binding variables
			if (sub_model->slack_map.find(var_name) == sub_model->slack_map.end() &&
				sub_model->binding_var_map.find(var_name) == sub_model->binding_var_map.end()) {
				variableValues.insert(make_pair(var_name, sub_model->var_values.at(var_name)));
			}
		}
		cover_sp.separateMinimalCover(&variableValues);
		//only the cover exists and is violated by the current solution
		if (cover_sp.coverInequalityExists && cover_sp.minimalCoverIsViolated) {
			ncut++;
			auto cut_idx = sub_model->cons_num;
			auto cut = cover_sp.generateCut();
			//add the cut
			map<int, inequality*> temp_map;
			temp_map.insert(make_pair(cut_idx, cut));
			CPXHelper::AddRows(&temp_map, env, sub_model->model);
		}
	}
	return ncut;
}