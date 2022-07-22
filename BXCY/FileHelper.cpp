#include "FileHelper.h"

bool FileHelper::IsFolderExist(string path) {
	int flag = _access(path.c_str(), 0);
	return flag == -1 ? false : true;
}

bool FileHelper::CreateFolder(string path) {
	int len = path.length();
	char tmpDirPath[256] = { 0 };
	for (int i = 0; i < len; i++)
	{
		tmpDirPath[i] = path[i];
		if (tmpDirPath[i] == '\\' || tmpDirPath[i] == '/')
		{
			if (_access(tmpDirPath, 0) == -1)
			{
				int ret = _mkdir(tmpDirPath);
				if (ret == -1) return false;
			}
		}
	}
	return true;
}

bool FileHelper::CreateFile(string path) {
	FILE* file = nullptr;
	fopen_s(&file, path.c_str(), "wb");
	if (nullptr == file)
	{
		return false;
	}
	fclose(file);
	return true;
}

bool FileHelper::ClearFolder(string path) {
	string strFolder = "rd /s/q ";
	strFolder += path;
	if (system(strFolder.c_str())) {
		return true;
	}
	return false;
}

bool FileHelper::CreatFolderIfNotExist(string path) {
	bool flag = false;
	if (!FileHelper::IsFolderExist(path)) {
		if (!FileHelper::CreateFolder(path)) {
			flag = false;
			throw "create folder " + path + " failed!";
		}
		else
		{
			flag = true;
		}
	}
	else
	{
		flag = false;
	}
	return flag;
}

bool FileHelper::CreatFileIfNotExist(string path) {
	bool flag = false;
	if (!FileHelper::IsFolderExist(path)) {
		if (!FileHelper::CreateFile(path)) {
			flag = false;
			throw "create file " + path + " failed!";
		}
		else
		{
			flag = true;
		}
	}
	else
	{
		flag = false;
	}
	return flag;
}

string FileHelper::GetParentPath(string path) {
	int idx = 0;
	for (int i = path.size() - 1; i > 0; i--) {
		if (path[i] == '\\') {
			idx = i;
			break;
		}
	}
	string result = path.substr(0, idx);
	return result;
}

string FileHelper::GetRootPath(void) {
	string root_path = _getcwd(NULL, 0);
	vector<string> temp;
	return GetParentPath(root_path) + "\\";
}

void FileHelper::GetFiles(string path, vector<string>& files, vector<string>& ownname) {
	intptr_t hFile = 0;
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			if ((fileinfo.attrib & _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					GetFiles(p.assign(path).append("\\").append(fileinfo.name), files, ownname);
			}
			else
			{
				files.push_back(path + "\\" + fileinfo.name);
				ownname.push_back(fileinfo.name);
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}