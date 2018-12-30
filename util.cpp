#include "util.h"
using namespace std;
int parse_command(string command)
{
	string theString = command;
	if (theString.substr(0, 2) == "ls")
		return LS;
	else if (theString.substr(0, 4) == "down")
		return DOWN;
	else if (theString.substr(0, 2) == "up")
		return UP;
	else if (theString.substr(0, 4) == "data")
		return DATA;
	else if (theString.substr(0, 4) == "quit")
		return QUIT;
	else return UNKOWN;
}

string get_all_files_names_within_folder(string folder)
{
	vector<string> names;
	string search_path = folder + "/*.*";
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(search_path.c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			// read all (real) files in current folder
			// , delete '!' read other 2 default folder . and ..
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				names.push_back(fd.cFileName);
			}
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}
	string str_names;
	for (int i = 0; i < names.size(); i++)
		str_names = str_names + names[i] + '\n';
	return str_names;
}
