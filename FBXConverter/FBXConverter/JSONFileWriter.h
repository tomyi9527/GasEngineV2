#pragma once
#include "CommonStruct.h"
#include <string>

class JSONFileWriter
{
public:
	JSONFileWriter(bool beautified);
	~JSONFileWriter();

	void openExportFile(const std::string& filePath);
	void closeExportFile();
	void writeObjectInfo(const std::string& text, int level);	
private:
	FILE* mJSONFile;
	bool mOptimization;
};