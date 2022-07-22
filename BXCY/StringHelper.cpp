#include "StringHelper.h"

string StringHelper::GenVarName(string input_str, int idx) {
	return input_str += '{' + to_string(idx) + '}';
}

string StringHelper::GenVarNameMultiDim(string input_str, vector<int> idx_list) {
	string output = input_str + '{';
	for (int i = 0; i < idx_list.size(); i++) {
		int idx = idx_list.at(i);
		output += to_string(idx) + ",";
	}
	output.erase(output.end() - 1);
	output += '}';
	return output;
}

vector<string> StringHelper::GenVarName(string input_str, vector<int> idx_list) {
	vector<string> str_list = vector<string>();
	for (int i = 0; i < idx_list.size(); i++) {
		str_list.push_back(input_str + '{' + to_string(idx_list[i]) + '}');
	}
	return str_list;
}

vector<string> StringHelper::GenVarName(string input_str, int first_idx, vector<int> idx_list) {
	vector<string> str_list = vector<string>();
	for (int i = 0; i < idx_list.size(); i++) {
		str_list.push_back(input_str + '{' + to_string(first_idx) + '}' + '{' + to_string(idx_list[i]) + '}');
	}
	return str_list;
}

int StringHelper::VarNameToIdx(string name) {
	string num_str;
	for (int i = 0; i < name.size(); i++) {
		if (name[i] >= '0' && name[i] <= '9') {
			num_str += name[i];
		}
	}
	return atoi(num_str.c_str());
}

pair<int, int> StringHelper::TwoDimeVarNameToIdxPair(string name) {
	int first_num, last_num;
	first_num = last_num = -1;
	string temp;
	for (int i = 0; i < name.size(); i++) {
		if (name[i] >= '0' && name[i] <= '9' || name[i] == ',') {
			temp += name[i];
		}
	}
	vector<string> num_list;
	SplitString(temp, num_list, ",");
	return make_pair(atoi(num_list.front().c_str()), atoi(num_list.back().c_str()));
}

void StringHelper::SplitString(const string& s, vector<std::string>& v, const string& c) {
	string::size_type pos1, pos2;
	pos2 = s.find(c);
	pos1 = 0;
	while (string::npos != pos2)
	{
		v.push_back(s.substr(pos1, pos2 - pos1));

		pos1 = pos2 + c.size();
		pos2 = s.find(c, pos1);
	}
	if (pos1 != s.length())
		v.push_back(s.substr(pos1));
}

void StringHelper::EraseSpace(vector<std::string>& v) {
	auto iter = v.begin();
	for (; iter != v.end();) {
		if (*iter == " " || *iter == "") {
			iter = v.erase(iter);
		}
		else
		{
			++iter;
		}
	}
}

char* StringHelper::StringToChars(string str) {
	int num = strlen(str.c_str()) + 1;
	char* strchar = new char[num];
	strcpy(strchar, str.c_str());
	return strchar;
}