#include <ilcplex/cplex.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "SMPS_Reader.h"

int main() {
	SMPS_Reader reader;
	string root_path = FileHelper::GetRootPath();
	string bench_folder = root_path + "Benchmarks\\small(1scen)\\";
	reader.ReadFiles(bench_folder);
	/*reader.SolveMeanModels();*/
	reader.RunBxCy();

	return 0;
}