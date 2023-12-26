#pragma once
#include "CommonStruct.h"

class JSONFileWriter;

class STLConverter
{
public:
	STLConverter();
	~STLConverter();

	bool convert(
		const std::string& workingDirectory,
		const std::string& filePath,
		unsigned int optimizationFlag,
		std::string& background);

	bool isBinarySTL(const char* buffer, unsigned int fileSize);
	bool isAsciiSTL(const char* buffer, unsigned int fileSize);

	bool convertASCIIFile(
		unsigned char* buffer,
		unsigned int fileSize,
		std::vector<float>& positions,
		FBXSDK_NAMESPACE::FbxDouble3& posBboxMin,
		FBXSDK_NAMESPACE::FbxDouble3& posBboxMax,
		std::vector<float>& normals,
		std::vector<int>& indices,
		std::vector<int>& topologicalIndices,
		unsigned int& faceCount);

	bool convertBinaryFile(
		unsigned char* buffer,
		unsigned int fileSize,
		const std::string& workingDirectory,
		const std::string& filePath,
		unsigned int optimizationFlag,
		std::string& background);

	void convertStructureFile(
		const std::string& fullFilePath,
		int materialCount,
		FBXSDK_NAMESPACE::FbxDouble3& posBboxMin,
		FBXSDK_NAMESPACE::FbxDouble3& posBboxMax,
		int version);

	void writeMeshNode(
		JSONFileWriter* sceneStructureJSONWriter,
		int materialCount,
		FBXSDK_NAMESPACE::FbxDouble3& posBboxMin,
		FBXSDK_NAMESPACE::FbxDouble3& posBboxMax,
		int jsonLevel,
		bool lastObject);

	void convertSceneFile(
		const std::string& workingDirectory,
		const FbxVector4& diffuse,
		const FbxVector4& specular,
		const FbxVector4& ambient,
		std::vector<std::string>& convertedFiles,
		const std::string& background,
		int version);

	inline unsigned int BGRV5551_RGBA8888(unsigned short color);
	inline unsigned int RGBV5551_RGBA8888(unsigned short color);

	int mVersion;
};