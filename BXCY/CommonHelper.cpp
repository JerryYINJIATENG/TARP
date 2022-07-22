#include "CommonHelper.h"

string CommonHelper::TimestampToString(int time_stamp) {
	int hours = time_stamp / 3600;
	int rest = time_stamp % 3600;
	int minutes = rest / 60;
	rest = rest % 60;
	int seconds = rest;
	stringstream str_stream;
	str_stream << setfill('0') << setw(2) << hours << ":" << setw(2) << minutes << ":" << setw(2) << seconds;
	return str_stream.str();
}

double CommonHelper::round(double number, unsigned int bits) {
	stringstream ss;
	ss << fixed << setprecision(bits) << number;
	ss >> number;
	return number;
}

int CommonHelper::Floor(double val) {
	if (val > 0) {
		return (int)floor(val);
	}
	else if (val < 0) {
		return (int)(-ceil(-val));
	}
	else
	{
		return val;
	}
}