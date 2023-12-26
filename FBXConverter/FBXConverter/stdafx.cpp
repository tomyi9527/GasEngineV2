// stdafx.cpp : source file that includes just the standard includes
// FBXConverter.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"
#include "JSONFileWriter.h"
// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

void writeConvertedFile(
	std::string& fileName,
	std::vector<std::string>& convertedFiles,
	unsigned int triangleCount,
	unsigned int polygonCount,
	unsigned int vertexCount)
{
	FBXSDK_printf("***************** Converted Files *****************\n");
	JSONFileWriter* resultFile = new JSONFileWriter(false);
	resultFile->openExportFile(fileName);

	resultFile->writeObjectInfo("{", 0);
	resultFile->writeObjectInfo("\"files\":", 1);
	resultFile->writeObjectInfo("[", 1);

	const int FILE_PATH_MAX_LENGTH = 2048;
	char buffer[FILE_PATH_MAX_LENGTH * 2];

	size_t fileCount = convertedFiles.size();
	for (size_t i = 0; i < fileCount; ++i)
	{
		std::string fileName = getFileName(convertedFiles[i]);
		FBXSDK_printf("%s\n", fileName.c_str());
		if (fileName.length() >= FILE_PATH_MAX_LENGTH)
		{
			FBXSDK_printf("Error: output file path exceeds the up limit!\n");
			fileName = "Error: output file path exceeds the up limit!";
		}
		if (i == fileCount - 1)
			sprintf(buffer, "\"%s\"", fileName.c_str());
		else
			sprintf(buffer, "\"%s\",", fileName.c_str());

		resultFile->writeObjectInfo(buffer, 2);
	}

	resultFile->writeObjectInfo("],", 1);

	resultFile->writeObjectInfo("\"statistics\":", 1);
	resultFile->writeObjectInfo("{", 1);

	sprintf(buffer, "\"triangleCount\":%d,", triangleCount);
	resultFile->writeObjectInfo(buffer, 2);

	sprintf(buffer, "\"polygonCount\":%d,", polygonCount);
	resultFile->writeObjectInfo(buffer, 2);

	sprintf(buffer, "\"vertexCount\":%d", vertexCount);
	resultFile->writeObjectInfo(buffer, 2);

	resultFile->writeObjectInfo("}", 1);
	resultFile->writeObjectInfo("}", 0);
	resultFile->closeExportFile();

	FBXSDK_printf("***************** End Of Converted Files *****************\n");
}