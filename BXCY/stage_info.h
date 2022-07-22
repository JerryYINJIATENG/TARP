#pragma once
#ifndef STAGE_INFO_H
#define STAGE_INFO_H
#include <map>
#include <string>
#include <vector>
using namespace std;

/// <summary>
/// stage infomation
/// </summary>
class stage_info
{
public:
	stage_info() = default;
	string stage_name;
	vector<string> var_names;
	vector<string> cons_names;
};
#endif // !STAGE_INFO_H
