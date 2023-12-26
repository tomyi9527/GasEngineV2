#include "stdafx.h"
#include "AssistantFunctions.h"


AssistantFunctions::AssistantFunctions()
{
}


AssistantFunctions::~AssistantFunctions()
{
}

void AssistantFunctions::displayBool(const char* pHeader, bool pValue, const char* pSuffix /* = "" */)
{
	FbxString lString;

	lString = pHeader;
	lString += pValue ? "true" : "false";
	lString += pSuffix;
	lString += "\n";
	FBXSDK_printf("%s", (const char*)lString);
}


void AssistantFunctions::displayInt(const char* pHeader, int pValue, const char* pSuffix /* = "" */)
{
	FbxString lString;

	lString = pHeader;
	lString += pValue;
	lString += pSuffix;
	lString += "\n";
	FBXSDK_printf("%s", (const char*)lString);
}


void AssistantFunctions::displayDouble(const char* pHeader, double pValue, const char* pSuffix /* = "" */)
{
	FbxString lString;
	FbxString lFloatValue = (float)pValue;

	lFloatValue = pValue <= -HUGE_VAL ? "-INFINITY" : lFloatValue.Buffer();
	lFloatValue = pValue >= HUGE_VAL ? "INFINITY" : lFloatValue.Buffer();

	lString = pHeader;
	lString += lFloatValue;
	lString += pSuffix;
	lString += "\n";
	FBXSDK_printf("%s", (const char*)lString);
}

void AssistantFunctions::displayString(const char* header, const char* value, const char* suffix)
{
	FbxString lString;

	lString = header;
	lString += value;
	lString += suffix;
	lString += "\n";
	FBXSDK_printf("%s", (const char*)lString);
}

void AssistantFunctions::display2DVector(const char* pHeader, FbxVector2 pValue, const char* pSuffix  /* = "" */)
{
	FbxString lString;
	FbxString lFloatValue1 = (float)pValue[0];
	FbxString lFloatValue2 = (float)pValue[1];

	lFloatValue1 = pValue[0] <= -HUGE_VAL ? "-INFINITY" : lFloatValue1.Buffer();
	lFloatValue1 = pValue[0] >= HUGE_VAL ? "INFINITY" : lFloatValue1.Buffer();
	lFloatValue2 = pValue[1] <= -HUGE_VAL ? "-INFINITY" : lFloatValue2.Buffer();
	lFloatValue2 = pValue[1] >= HUGE_VAL ? "INFINITY" : lFloatValue2.Buffer();

	lString = pHeader;
	lString += lFloatValue1;
	lString += ", ";
	lString += lFloatValue2;
	lString += pSuffix;
	lString += "\n";
	FBXSDK_printf("%s", (const char*)lString);
}

void AssistantFunctions::display3DVector(const char* pHeader, FbxVector4 pValue, const char* pSuffix)
{
	FbxString lString;
	FbxString lFloatValue1 = (float)pValue[0];
	FbxString lFloatValue2 = (float)pValue[1];
	FbxString lFloatValue3 = (float)pValue[2];

	lFloatValue1 = pValue[0] <= -HUGE_VAL ? "-INFINITY" : lFloatValue1.Buffer();
	lFloatValue1 = pValue[0] >= HUGE_VAL ? "INFINITY" : lFloatValue1.Buffer();
	lFloatValue2 = pValue[1] <= -HUGE_VAL ? "-INFINITY" : lFloatValue2.Buffer();
	lFloatValue2 = pValue[1] >= HUGE_VAL ? "INFINITY" : lFloatValue2.Buffer();
	lFloatValue3 = pValue[2] <= -HUGE_VAL ? "-INFINITY" : lFloatValue3.Buffer();
	lFloatValue3 = pValue[2] >= HUGE_VAL ? "INFINITY" : lFloatValue3.Buffer();

	lString = pHeader;
	lString += lFloatValue1;
	lString += ", ";
	lString += lFloatValue2;
	lString += ", ";
	lString += lFloatValue3;
	lString += pSuffix;
	lString += "\n";
	FBXSDK_printf("%s", (const char*)lString);
}

void AssistantFunctions::displayColor(const char* pHeader, FbxColor pValue, const char* pSuffix)
{
	FbxString lString;

	lString = pHeader;
	lString += (float)pValue.mRed;

	lString += " (red), ";
	lString += (float)pValue.mGreen;

	lString += " (green), ";
	lString += (float)pValue.mBlue;

	lString += " (blue)";
	lString += pSuffix;
	lString += "\n";
	FBXSDK_printf("%s", (const char*)lString);
}

//const char* AssistantFunctions::getUUID()
//{
//	//UUID uuid;
//	//UuidCreate(&uuid);
//	//char* uuidStr = NULL;
//	//UuidToStringA(&uuid, (RPC_CSTR*)&uuidStr);
//
//	//return uuidStr;
//	return "";
//}

string& AssistantFunctions::replace_all(string& str, const string& old_value, const string& new_value)
{
	while (true) 
	{
		string::size_type pos(0);
		if ((pos = str.find(old_value)) != string::npos)
			str.replace(pos, old_value.length(), new_value);
		else   
			break;
	}
	return str;
}
string& AssistantFunctions::replace_all_distinct(string& str, const string& old_value, const string& new_value)
{
	for(string::size_type pos(0); pos != string::npos; pos += new_value.length()) 
	{
		if ((pos = str.find(old_value, pos)) != string::npos)
			str.replace(pos, old_value.length(), new_value);
		else   
			break;
	}
	return str;
}