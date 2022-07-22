#include "inequality.h"

inequality* inequality::deepcopy() {
	auto result = new inequality();
	result->rhs = rhs;
	result->sense = sense;
	result->var_indices = vector<int>(var_indices);
	result->lhs = vector<double>(lhs);
	result->cons_name = cons_name;
	return result;
}