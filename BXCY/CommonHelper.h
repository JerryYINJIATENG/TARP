#pragma once
#include <string>
#include <map>
#include <vector>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <math.h>
using namespace std;
#ifndef COMMONHELPER_H
#define COMMONHELPER_H

class CommonHelper
{
public:
	CommonHelper() = default;
	//sum the elements for map
	template <class T_1, class T_2>
	static T_2 SumMap(map<T_1, T_2> temp_map) {
		T_2 result = 0;
		for (auto iter : temp_map) {
			result += iter.second;
		}
		return result;
	}

	static string TimestampToString(int time_stamp);

	static double round(double number, unsigned int bits);

	template <typename T_1,typename T_2>
	//free a map which saves pointers
	static void Free(map<T_1, T_2> temp_map) {
		static_assert(is_pointer<T_2>::value, "elements are not pointers");
		for (auto iter : temp_map) {
			delete(iter.second);
		}
	}

	template <typename T>
	//free a vector which saves pointers
	static void Free(vector<T> temp_vec) {
		static_assert(is_pointer<T>::value, "elements are not pointers");
		for (auto iter : temp_vec) {
			delete(iter);
		}
	}

	template <typename T_1, typename T_2>
	//copy map of which the elements implements deepcopy funtion
	static void DeepCopy(map<T_1, T_2> temp_map, map<T_1, T_2>* result) {
		static_assert(is_pointer<T_2>::value, "elements are not pointers");
		//copy elements
		for (auto iter : temp_map) {
			result->insert(make_pair(iter.first, iter.second->deepcopy()));
		}
	}

	template <typename T>
	//copy vector of which the elements implements deepcopy funtion
	static void DeepCopy(vector<T> temp_vec, vector<T>* result) {
		static_assert(is_pointer<T>::value, "elements are not pointers");
		//copy elements
		for (auto iter : temp_vec) {
			result->push_back(iter->deepcopy());
		}
	}

	//round down the given value, e.g., Floor(-1.5)=-2
	static int Floor(double val);
};

/// <summary>
/// use for map search, give a map value, return the corresponding key
/// </summary>
class finder
{
public:
	finder(const string& cmp_string) :s_(cmp_string) {}
	bool operator ()(const map<int, string>::value_type& item)
	{
		return item.second == s_;
	}
private:
	const string& s_;
};
#endif // !COMMONHELPER_H
