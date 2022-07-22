#pragma once
#ifndef NODE_H
#define NODE_H
#include "CPXMasModel.h"
#include <vector>

class node
{
public:
	node(CPXMasModel* _model, int idx);
	~node();
	CPXMasModel* model;
	double local_LB = -INFINITY; //the local lower bound of this node
	double local_UB = INFINITY;
	bool is_integer = true; //whether is an integer solution
	int node_idx;
	vector<string> branch_var_list; //store branch variables
};

node::node(CPXMasModel* _model, int idx)
{
	model = _model;
	node_idx = idx;
}

node::~node()
{
	delete model;
	model = nullptr;
}
#endif // !NODE_H
