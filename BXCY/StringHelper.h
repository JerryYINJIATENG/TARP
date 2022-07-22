#pragma once
#include <string>
#include <vector>
using namespace std;
#ifndef STRINGHELPER_H
#define STRINGHELPER_H
class StringHelper
{
public:
	static string GenVarName(string input_str, int idx);
	static string GenVarNameMultiDim(string input_str, vector<int> idx_list);//multi dimension
	static vector<string> GenVarName(string input_str, vector<int> idx_list);
	static vector<string> GenVarName(string input_str, int first_idx, vector<int> idx_list);
	static int VarNameToIdx(string name);
	static pair<int, int> TwoDimeVarNameToIdxPair(string name);
	static void SplitString(const std::string& s, std::vector<std::string>& v, const std::string& c);
	static void EraseSpace(std::vector<std::string>& v);
	static char* StringToChars(string str);
};
#endif // !STRINGHELPER_H
