#ifndef COMMAND
#define COMMAND
#include <string>
#include <Windows.h>
#include <vector>
#include <iostream>
using namespace std;

enum Command {LS, DOWN, UP, DATA, QUIT, UNKOWN};
int parse_command(std::string command);
string get_all_files_names_within_folder(string folder);
#endif