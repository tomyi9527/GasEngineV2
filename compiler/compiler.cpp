//Compiler.cpp: this file contains the "main" function. Program execution will start and end here.
//
#ifdef _MSC_VER
#include "pch.h"
#include <Windows.h>
#include <direct.h>
#include <regex>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <sys/io.h>

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

using namespace std;
vector<string> lines;
vector<string> copylines;

string baseUrl = "./";

string trim(string s) {
	if (s.empty())
		return s;

	s.erase(0, s.find_first_not_of(" \t"));
	s.erase(s.find_last_not_of(" \t") + 1);
	return s;
}

string trimRight(string s) {
	if (s.empty())
		return s;

	s.erase(s.find_last_not_of(" \t") + 1);
	return s;
}

#ifdef _MSC_VER
string parseCode(regex_constants::error_type etype) {
	switch (etype) {
	case regex_constants::error_collate:
		return "error_collate: invalid collating element request";
	case regex_constants::error_ctype:
		return "error_ctype: invalid character class";
	case regex_constants::error_escape:
		return "error_escape: invalid escape character or trailing escape";
	case regex_constants::error_backref:
		return "error_backref: invalid back reference";
	case regex_constants::error_brack:
		return "error_brack: mismatched bracket([ or ])";
	case regex_constants::error_paren:
		return "error_paren: mismatched parentheses(( or ))";
	case regex_constants::error_brace:
		return "error_brace: mismatched brace({ or })";
	case regex_constants::error_badbrace:
		return "error_badbrace: invalid range inside a { }";
	case regex_constants::error_range:
		return "erro_range: invalid character range(e.g., [z-a])";
	case regex_constants::error_space:
		return "error_space: insufficient memory to handle this regular expression";
	case regex_constants::error_badrepeat:
		return "error_badrepeat: a repetition character (*, ?, +, or {) was not preceded by a valid regular expression";
	case regex_constants::error_complexity:
		return "error_complexity: the requested match is too complex";
	case regex_constants::error_stack:
		return "error_stack: insufficient memory to evaluate a match";
	default:
		return "";
	}
}

void readGASEngine(const string& path)
{
	//read GASEngine.js
	string temp;
	ifstream input;
	input.open(baseUrl + path, ios::in); // open file

	//file does NOT exist
	if (!input.is_open()) {
		baseUrl = "../";

		input.open(baseUrl + path, ios::in);

		if (!input.is_open()) {
			return;
		}
	}

	stringstream stext;

	while (getline(input, temp))
	{
		stext << trim(temp);
	}
	input.close();

	string text = stext.str();

	//file is empty
	if (text.empty()) 
		return;

	ofstream output;
	output.open("files.txt", ios::out);

	try {
		smatch results;

		if (regex_search(text, results, regex("var\\s*sourceFiles\\s*=\\[", regex_constants::ECMAScript)))
		{
			string prefix = results.begin()->str();
			int startPos = (int)text.find(prefix);
			startPos += (int)prefix.length();
			int endPos = (int)text.find(']', startPos);

			if (endPos > startPos) {

				int lineBegin = (int)text.find_first_of("'\"", startPos);
				while (lineBegin >= 0) {
					int lineEnd = (int)text.find_first_of("'\"", lineBegin + 1);
					if (lineEnd < 0 || lineEnd > endPos)
						break;

					string line = text.substr(lineBegin + 1, lineEnd - lineBegin - 1);
					output << line << endl;
					lines.push_back(line);

					lineBegin = (int)text.find_first_of("'\"", lineEnd + 1);
				}
			}
			else {
				cout << "read failure: lack of ] at the end of " << prefix << " ... " << endl;
			}
		}
	}
	catch (regex_error & e) {
		cout << "what: " << e.what() << ";code: " << parseCode(e.code()) << endl;
	}
	output.close();
}
#else
void readGASEngineLinux(const string& path)
{
	string temp;
	ifstream input;
	input.open((baseUrl + path).c_str(), ios::in); // open file

	//file does NOT exist
	if (!input.is_open()) {
		baseUrl = "../";

		input.open((baseUrl + path).c_str(), ios::in);

		if (!input.is_open()) {
			return;
		}
	}

	stringstream stext;

	while (getline(input, temp))
	{
		stext << trim(temp);
	}
	input.close();

	string text = stext.str();

	//file is empty
	if (text.empty()) 
		return;

	ofstream output;
	output.open("files.txt", ios::out);
	//output << text << endl;
	int startPos = text.find("'GASEngineSource/");
	while(startPos!=string::npos){
		int endPos = text.find(".js'");
		string line = text.substr(startPos + 1, endPos - startPos + 2);
		output << line << endl;
		lines.push_back(line);
		text = text.substr(endPos + 4);
		startPos = text.find("'GASEngineSource/");
	}

	output.close();
}
#endif

void readCopyFiles(const string& path) {
	//read copyfiles.txt
	string temp;
	ifstream input;
	#ifdef _MSC_VER
	input.open(path, ios::in); // open file
	#else
	input.open(path.c_str(), ios::in);
	#endif

	if (input.is_open()) {

		while (getline(input, temp))
		{
			copylines.push_back(trim(temp));
		}
		input.close();
	}
}

#define MAX_BLOCK 16000
char memblock[MAX_BLOCK] = { 0 };

#ifdef _MSC_VER
int makeDirWin(string base, string path)
{
	int len = path.length();
	if (len == 0) {
		if (_access(base.c_str(), 0) == -1) {
			_mkdir(base.c_str());
		}
	}
	else {
		char dpath[512] = { 0 };
		for (int ii = 0; ii < len; ii++)
		{
			dpath[ii] = path[ii];

			if (dpath[ii] == '\\' || dpath[ii] == '/')
			{
				string dir = base + dpath;
				if (_access(dir.c_str(), 0) == -1)
				{
					int ret = _mkdir(dir.c_str());
					if (ret < 0)
						return ret;
				}
			}
		}
	}
	return 0;
}
#else
int makeDirLinux(string base, string path, bool isAbsolute)
{
	int len = path.length();
	if(len == 0)
	{
		if(access(base.c_str(), F_OK) == -1){
			mkdir(base.c_str(), 0777);
		}
	}
	else
	{
		char dpath[512] = {0};

		for (int ii = 0; ii < len;ii++)
		{
			dpath[ii] = path[ii];
			if (dpath[ii] == '\\' || dpath[ii] == '/')
			{
				string dir = base + dpath;
				if(isAbsolute)
				{
					if(ii==0)
						continue;
					else
						dir = dpath;
				}
				if (access(dir.c_str(), F_OK) == -1)
				{
					int ret = mkdir(dir.c_str(), 0777);
					if (ret < 0)
						return ret;
				}
			}
		}
	}
	return 0;
}
#endif
// int copyOneFile(const string base, const string src)
// {
// 	if (src.length() == 0)
// 		return -3;

// 	ifstream input;
// 	input.open(baseUrl + src, ios::in | ios::binary); // open file
// 	int ret = 0;

// 	if (input.is_open()) {

// 		makeDirWin(base, src); //make dir if not exist

// 		ofstream output;
// 		output.open(base + src, ios::out | ios::binary);

// 		if (output.is_open()) {

// 			//get length of input file
// 			input.seekg(0, ios::end);
// 			int leftBytes = (int)input.tellg();
// 			input.seekg(0, ios::beg);

// 			while (leftBytes > 0) {

// 				int readBytes = min(leftBytes, MAX_BLOCK);
// 				input.read(memblock, readBytes);

// 				if (readBytes > 0) {
// 					output.write(memblock, readBytes);
// 					leftBytes -= readBytes;
// 				}

// 				if (readBytes == 0 || input.eof())
// 					break;
// 			}
// 			output.close();
// 			cout << "copy  success: " << src << endl;
// 			ret = 0;
// 		}
// 		else {
// 			cout << "write failure: " << src << endl;
// 			ret = -1;
// 		}
// 		input.close();
// 	}
// 	else {
// 		cout << "read  failure: " << src << endl;
// 		ret = -2;
// 	}
// 	return ret;
// }

// int copyOneDirectoryWin(const string base, const string src) {
// 	if (src.length() == 0)
// 		return -3;

// 	string srcdir = baseUrl + src + "\\*.*";
// 	vector<string> vDir;

// 	WIN32_FIND_DATA data;
// 	HANDLE finder = FindFirstFile(srcdir.c_str(), &data);
// 	if (finder != INVALID_HANDLE_VALUE) {
// 		BOOL bExist = TRUE;
// 		while (bExist) {

// 			string filename(data.cFileName);
// 			string fullname = src + "\\" + filename;

// 			if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
// 				if (filename != "." && filename != "..")
// 					vDir.push_back(fullname);
// 			}
// 			else {
// 				copyOneFile(base, fullname);
// 			}
// 			bExist = FindNextFile(finder, &data);
// 		}
// 		FindClose(finder);
// 	}

// 	for (size_t n = 0; n < vDir.size(); n++) {
// 		copyOneDirectoryWin(base, vDir.at(n));
// 	}
// 	return 0;
// }

int main(int argc, char *argv[])
{
	#ifdef _MSC_VER
	readGASEngine("GASEngineSource/GASEngine.js");
	#else
	readGASEngineLinux("GASEngineSource/GASEngine.js");
	#endif
	
	string destUrl = baseUrl + "build/";
	string destFileName = destUrl + "GASEngine.js";
	bool isAbsolutePath = true;
	if (argc == 2)
	{
		string usrUrl = argv[1];
		if(usrUrl[0] == '/')
			isAbsolutePath = true;
		int endpoint = usrUrl.find_last_of("/\\");
		if(endpoint != string::npos){
			destUrl = usrUrl.substr(0, endpoint) + "/";
		}
		destFileName = usrUrl;
	}
	else if(argc > 2)
	{
		cout << "Too many arguments supplied." << endl;
		return -1;
	}

	if (lines.size() > 0) { //merge files
		#ifdef _MSC_VER
		makeDirWin(destUrl, "");
		#else
		makeDirLinux(destUrl, "", isAbsolutePath);
		#endif

		ofstream output;
		#ifdef _MSC_VER
		output.open(destFileName, ios::out);
		#else
		output.open((destFileName).c_str(), ios::out);
		#endif
		output << "window.GASEngine = " << endl;
		output << "{" << endl;
		output << "   _RendererRevision_: '1'," << endl;
		output << "   _EngineRevision_ : '1'" << endl;
		output << "};" << endl;
		output << endl;

		string temp;

		for (size_t i = 0; i < lines.size(); i++) {

			ifstream input;
			#ifdef _MSC_VER
			input.open(baseUrl + lines[i], ios::in); // open file
			#else
			input.open((baseUrl + lines[i]).c_str(), ios::in);
			#endif
			if (input.is_open()) {
				while (getline(input, temp))
				{
					output << trimRight(temp) << endl;
				}
				output << endl;
				input.close();

				cout << "merge success: " << lines[i] << endl;
			}
			else {
				cout << "merge failure: " << lines[i] << endl;
			}

		}
		output << "function _loadGASEngineSourceFiles_(sourceRootDirectory) {" << endl;
		output << "};" << endl;

		output.close();
	}
	else {
		cout << "There is no sourceFile from GASEngine.js" << endl;
	}

	//copy files
	// readCopyFiles("copylist.txt");

	// if (copylines.size() > 0) {
	// 	makeDirWin(destUrl, "");

	// 	for (size_t i = 0; i < copylines.size(); i++) {

	// 		string line = copylines[i];

	// 		if (line.find("/D,") == 0) { //copy directory recursively
	// 			line = line.substr(3, line.length() - 3);

	// 			cout << " == copy recursively: " + line << endl;
	// 			copyOneDirectoryWin(destUrl, line);
	// 		}
	// 		else {
	// 			copyOneFile(destUrl, line);
	// 		}
	// 	}
	// }

}
