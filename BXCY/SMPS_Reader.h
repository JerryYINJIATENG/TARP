#pragma once
/********************************************************
 Copyright: https://github.com/772700563
 Author: Fan Pu
 Date:
 Description: reads SMPS files
*******************************************************/
#ifndef SMPS_READER_H
#define SMPS_READER_H
#include <map>
#include <string>
#include <vector>
#include "FileHelper.h"
#include "StringHelper.h"
#include "CommonHelper.h"
#include "CSVParser.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "boollinq.h"
#include <ilcplex/cplex.h>
#include "instance.h"
#include "CPXMeanModel.h"
#include "CPXMasModel.h"
#include "CPXSubModel.h"
#include "CPXHelper.h"
#include "CoverInequalitySeparator.h"
using namespace std;
using namespace boolinq;

class SMPS_Reader
{
public:
	SMPS_Reader();
	string folder;
	map<string, instance*> instances_map;
	map<string, CPXMeanModel*> mean_models; // [ins_name,model]
	//map<string, CPXMasModel*> master_models;// [ins_name,model]
	//map<pair<string, string>, CPXSubModel*> sub_models;// [<ins_name,scen_name>,sub_model]
	map<string, vector<inequality*>*> cuts_waitted_map;// [ins_name,cuts]
	void ReadFiles(string folder_path);
	void SolveMeanModels();
	void RunBxCy();
	//transfer the model to its LP relaxation
	template<typename T>
	void TurnLP(T model);
	//transfer to IP from its LP relaxation
	template<typename T>
	void TurnIP(T model);
	//add a Benders optimality cut to the master problem
	void AddOptCut(CPXMasModel* mas_model, CPXSubModel* sub_model);
	//add Gomory fractional cuts to the subproblem
	int AddGomoryCut(CPXMasModel* mas_model, CPXSubModel* sub_model);
	//add 0-1 extended cover inequilities to the subproblem
	int AddCoverCut(CPXMasModel* mas_model, CPXSubModel* sub_model);
}; 

#endif // !SMPS_READER_H