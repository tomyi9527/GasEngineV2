#pragma once
#include <string>
using std::string;

class AssistantFunctions
{
public:
	AssistantFunctions();
	~AssistantFunctions();

	static void displayBool(const char* pHeader, bool pValue, const char* pSuffix = "");
	static void displayInt(const char* pHeader, int pValue, const char* pSuffix = "");
	static void displayDouble(const char* pHeader, double pValue, const char* pSuffix = "");
	static void displayString(const char* header, const char* value = "", const char* suffix = "");
	static void display3DVector(const char* pHeader, FbxVector4 pValue, const char* pSuffix = "");
	static void display2DVector(const char* pHeader, FbxVector2 pValue, const char* pSuffix = "");
	static void displayColor(const char* pHeader, FbxColor pValue, const char* pSuffix = "");
	//static const char* getUUID();
	static string& replace_all(string& str, const string& old_value, const string& new_value);
	static string& replace_all_distinct(string& str, const string& old_value, const string& new_value);
};

