#include "CPXHelper.h"

void CPXHelper::AddRows(map<int, inequality*>* rows, CPXENVptr env, CPXLPptr model) {
	int row_num = rows->size();
	auto rhs = new double[row_num];
	auto sense = new char[row_num];
	auto row_names = new char* [row_num];
	int cnt = 0;
	for (auto iter : *rows) {
		auto row = iter.second;
		rhs[cnt] = row->rhs;
		sense[cnt] = row->sense;
		row_names[cnt] = (char*)row->cons_name.data();
		cnt++;
	}
	//add new rows
	CPXnewrows(env, model, row_num, rhs, sense, NULL, row_names);
	//set values
	for (auto iter : *rows) {
		auto i = iter.first;
		auto row = iter.second;
		for (int _iter = 0; _iter < row->var_indices.size(); _iter++) {
			int j = row->var_indices[_iter];
			CPXchgcoef(env, model, i, j, row->lhs[_iter]);
		}
	}
}