#pragma once
#include <string>
#include <io.h>
#include <direct.h>
#include <vector>
#include "StringHelper.h"
using namespace std;
#ifndef FILEHELPER_H
#define FILEHELPER_H
class FileHelper
{
public:
	/// <summary>
	/// whether the folder is exist
	/// </summary>
	/// <param name="strName"></param>
	/// <returns></returns>
	static bool IsFolderExist(string path);

	/// <summary>
	/// create a folder
	/// </summary>
	/// <param name="strName"></param>
	/// <returns></returns>
	static bool CreateFolder(string path);

	/// <summary>
	/// create a file
	/// </summary>
	/// <param name="path"></param>
	/// <returns></returns>
	static bool CreateFile(string path);

	/// <summary>
	/// delete all files in this folder
	/// </summary>
	/// <param name="path"></param>
	/// <returns></returns>
	static bool ClearFolder(string path);

	static bool CreatFolderIfNotExist(string path);

	static bool CreatFileIfNotExist(string path);

	static string GetParentPath(string path);

	static string GetRootPath(void);

	/// <summary>
	/// get file names
	/// </summary>
	/// <param name=""></param>
	/// <param name=""></param>
	static void GetFiles(string path, vector<string>& files, vector<string>& ownname);
};
#endif // !FILEHELPER_H
