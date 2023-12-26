#include "stdafx.h"
#include "JSONFileWriter.h"

JSONFileWriter::JSONFileWriter(bool optimization)
	: mJSONFile(NULL)
	, mOptimization(optimization)
{
}


JSONFileWriter::~JSONFileWriter()
{
}

void JSONFileWriter::openExportFile(const string& filePath)
{
#ifdef _MSC_VER
	std::wstring unicodePath = UTF8_To_UCS16(filePath.c_str());
	mJSONFile = _wfopen(unicodePath.c_str(), L"wb+");
	if (!mJSONFile)
	{
		FBXSDK_printf("Error: Open file: %ws failed!\n", unicodePath.c_str());
	}
#else
	mJSONFile = fopen(filePath.c_str(), "wb+");
	if (!mJSONFile)
	{
		FBXSDK_printf("Error: Open file: %s failed!\n", filePath.c_str());
	}
#endif
}

void JSONFileWriter::closeExportFile()
{
	if (mJSONFile != NULL)
	{
		fclose(mJSONFile);
		mJSONFile = NULL;
	}
}

void JSONFileWriter::writeObjectInfo(const string& text, int level)
{
	if (mJSONFile == 0)
		return;
	if (mOptimization)	
	{
		fwrite(text.c_str(), 1, text.length(), mJSONFile);
	}
	else
	{
		int bufferSize = (int)text.length() + 4 * level + 1;
		char* p = new char[bufferSize];
		memset(p, 0x20, 4 * level);
		sprintf(&p[4 * level], "%s", text.c_str());
		p[bufferSize - 1] = '\n'; //UNIX STYLE
								  //p[bufferSize - 1] = 0;
		size_t s = fwrite(p, 1, bufferSize, mJSONFile);
		delete[] p;
	}
}