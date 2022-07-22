#include "CPXSubModel.h"
#include "CommonHelper.h"

CPXSubModel::CPXSubModel()
{
	env = nullptr;
	model = nullptr;
}

CPXSubModel::~CPXSubModel()
{
	CPXfreeprob(env, &model);
	CommonHelper::Free(constraints);
	//CommonHelper::Free(binding_cons);
}

CPXSubModel* CPXSubModel::deepcopy() {
	auto result = new CPXSubModel();
	result->env = env;
	int status;
	result->model = CPXcloneprob(env, model, &status);
	CommonHelper::DeepCopy(constraints, &result->constraints);
	CommonHelper::DeepCopy(binding_cons, &result->binding_cons);
	result->name = name;
	result->var_map = var_map;
	result->var_types = var_types;
	result->var_names = var_names;
	result->var_bounds = var_bounds;
	result->slack_map = slack_map;
	result->binding_var_map = binding_var_map;
	result->objective = objective;
	result->scen_name = scen_name;
	result->ins_name = ins_name;
	result->model_type = model_type;
	result->var_num = var_num;
	result->cons_num = cons_num;
	return result;
}

void CPXSubModel::UpdateModel() {
	CPXfreeprob(env, &model);

}