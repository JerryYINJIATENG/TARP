#include "CPXMasModel.h"
#include "CommonHelper.h"

CPXMasModel::CPXMasModel()
{
	env = nullptr;
	model = nullptr;
}

CPXMasModel::~CPXMasModel()
{
	CPXfreeprob(env, &model);
	//free pointers
	CommonHelper::Free(constraints);
	/*CommonHelper::Free(opt_cuts);*/
	/*CommonHelper::Free(feas_cuts);
	CommonHelper::Free(valid_cuts);*/
}

CPXMasModel* CPXMasModel::deepcopy() {
	auto result = new CPXMasModel();
	result->env = env;
	int status;
	result->model = CPXcloneprob(env, model, &status);
	CommonHelper::DeepCopy(constraints, &result->constraints);
	/*CommonHelper::DeepCopy(opt_cuts, &result->opt_cuts);*/
	/*CommonHelper::DeepCopy(feas_cuts, &result->feas_cuts);
	CommonHelper::DeepCopy(valid_cuts, &result->valid_cuts);*/
	result->name = name;
	result->var_map = var_map;
	result->var_types = var_types;
	result->var_names = var_names;
	result->var_bounds = var_bounds;
	result->estimator_map = estimator_map;
	result->objective = objective;
	result->ins_name = ins_name;
	result->model_type = model_type;
	result->var_num = var_num;
	result->cons_num = cons_num;
	return result;
}