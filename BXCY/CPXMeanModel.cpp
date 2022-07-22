#include "CPXMeanModel.h"

CPXMeanModel::CPXMeanModel()
{
	env = nullptr;
	model = nullptr;
}

CPXMeanModel::~CPXMeanModel()
{
	CPXfreeprob(env, &model);
}