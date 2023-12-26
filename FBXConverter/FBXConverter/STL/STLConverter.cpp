#include "stdafx.h"
#include "STLConverter.h"
#include "JsonToBin_V4.h"
#include "Material.h"
#include "FBXSceneStructureExporter_V4.h"
#include "JSONFileWriter.h"
#include "NeonateVertexCompression_V4.h"
#include "ParsingUtility.h"

STLConverter::STLConverter()
{
}


STLConverter::~STLConverter()
{
}

bool STLConverter::convert(
	const std::string& workingDirectory,
	const std::string& filePath,
	unsigned int optimizationFlag,
	std::string& background)
{
#ifdef _MSC_VER
	std::wstring unicodePath = UTF8_To_UCS16(filePath.c_str());
	FILE* fp = _wfopen(unicodePath.c_str(), L"rb");
#else
	FILE* fp = fopen(filePath.c_str(), "rb");
#endif

	mVersion = 4;

	if (fp)
	{
		fseek(fp, 0, SEEK_END);
		size_t fileSize = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		void* binaryData = malloc(fileSize);
		fread(binaryData, fileSize, 1, fp);
		fclose(fp);

		bool result = isBinarySTL((const char*)binaryData, (unsigned int)fileSize);
		if (result)
		{
			convertBinaryFile((unsigned char*)binaryData, (unsigned int)fileSize, workingDirectory, filePath, optimizationFlag, background);
		}
		else
		{
			result = isAsciiSTL((const char*)binaryData, (unsigned int)fileSize);
			if (result)
			{
				std::vector<float> positions;
				FBXSDK_NAMESPACE::FbxDouble3 posBboxMin;
				FBXSDK_NAMESPACE::FbxDouble3 posBboxMax;
				std::vector<float> normals;
				std::vector<int> indices;
				std::vector<int> topologicalIndices;
				unsigned int faceCount = 0;

				bool result = convertASCIIFile(
					(unsigned char*)binaryData, 
					(unsigned int)fileSize, 
					positions, 
					posBboxMin, 
					posBboxMax, 
					normals, 
					indices, 
					topologicalIndices, 
					faceCount);

				if (!result)
				{
					return false;
				}

				unsigned int vertexCount = faceCount * 3;

				std::vector<VERTEX_LAYER_TYPE> vertexElements;
				vertexElements.push_back(VL_POSITION);
				vertexElements.push_back(VL_NORMAL0);

				//Position
				unsigned int outPosEncodingSize = vertexCount * (unsigned int)sizeof(float) * 3;
				uint8_t* outPosEncodingBuffer = new uint8_t[outPosEncodingSize];
				unsigned int outPosEncodingFlag = 0;
				vertexElements.push_back(VL_POSITION);
				memcpy(outPosEncodingBuffer, &positions[0], outPosEncodingSize);

				//Normal
				unsigned int outNormalEncodingSize = vertexCount * (unsigned int)sizeof(float) * 3;
				uint8_t* outNormalEncodingBuffer = new uint8_t[outNormalEncodingSize];
				unsigned int outNormalEncodingFlag = 0;
				vertexElements.push_back(VL_NORMAL0);
				memcpy(outNormalEncodingBuffer, &normals[0], outNormalEncodingSize);

				//Index
				unsigned int outIndexEncodingSize = faceCount * 3 * (unsigned int)sizeof(unsigned int);
				uint8_t* outIndexEncodingBuffer = new uint8_t[outIndexEncodingSize];
				unsigned int outIndexEncodingFlag = 0;
				memcpy(outIndexEncodingBuffer, &indices[0], outIndexEncodingSize);

				//Topological Index
				unsigned int outTopologicalIndexEncodingSize = 6 * faceCount * sizeof(int);
				uint8_t* outTopologicalIndexEncodingBuffer = new uint8_t[outTopologicalIndexEncodingSize];
				unsigned int outTopologicalIndexEncodingFlag = 0;
				memcpy(outTopologicalIndexEncodingBuffer, &topologicalIndices[0], outTopologicalIndexEncodingSize);

				//Sub Mesh
				unsigned int subMeshCount = 1;
				unsigned int outSubMeshEncodingSize = sizeof(unsigned int) * 16 * subMeshCount;	//start1 count1, min_x_y_z3, max_x_y_z3
				uint8_t* outSubMeshEncodingBuffer = new uint8_t[outSubMeshEncodingSize];
				unsigned int outSubMeshEncodingFlag = 1;
				memset(outSubMeshEncodingBuffer, 0, outSubMeshEncodingSize);
				((unsigned int*)outSubMeshEncodingBuffer)[0] = 0;
				((unsigned int*)outSubMeshEncodingBuffer)[1] = faceCount;

				FbxUInt64 parentNodeID = 1;
				FbxUInt64 nodeID = 2;

				JsonToBin_V4* writer = new JsonToBin_V4();
				writer->openExportFile(workingDirectory + "STL_Mesh.2.mesh.bin");

				FBXSDK_NAMESPACE::FbxDouble3 uvBboxMin(0.0, 0.0, 0.0);
				FBXSDK_NAMESPACE::FbxDouble3 uvBboxMax(0.0, 0.0, 0.0);

				float posBoundingBoxMin[3] = { (float)posBboxMin[0], (float)posBboxMin[1], (float)posBboxMin[2] };
				float posBoundingBoxMax[3] = { (float)posBboxMax[0], (float)posBboxMax[1], (float)posBboxMax[2] };

				writer->writeObjectBin(
					parentNodeID,
					nodeID,
					MESH_TYPE,
					"STL_Mesh",
					(unsigned int)vertexCount,
					vertexElements,
					NULL,			//Bones
					\
					subMeshCount,
					outSubMeshEncodingBuffer,
					outSubMeshEncodingSize,
					outSubMeshEncodingFlag,
					\
					faceCount,
					outIndexEncodingBuffer,
					outIndexEncodingSize,
					outIndexEncodingFlag,
					\
					3 * faceCount,
					outTopologicalIndexEncodingBuffer,
					outTopologicalIndexEncodingSize,
					outTopologicalIndexEncodingFlag,
					\
					posBoundingBoxMin,
					posBoundingBoxMax,
					outPosEncodingBuffer,
					outPosEncodingSize,
					outPosEncodingFlag,
					\
					outNormalEncodingBuffer,
					outNormalEncodingSize,
					outNormalEncodingFlag,
					\
					NULL, 0, 0,		//Tangent
					\
					NULL, 0, 0,		//Vertex Color
					\
					uvBboxMin, uvBboxMax, NULL, 0, 0, //UV0
					uvBboxMin, uvBboxMax, NULL, 0, 0, //UV1
					\
					NULL, 0, 0, //BW & BI
					\
					NULL, 0, 0);

				writer->closeExportFile();
				delete writer;

				std::vector<std::string> convertedFiles;
				convertedFiles.push_back("scene.json");
				convertedFiles.push_back(filePath + ".structure.json");
				convertedFiles.push_back("STL_Mesh.2.mesh.bin");

				std::string convertedFilePath = filePath + ".convertedFiles";
				convertedFiles.push_back(convertedFilePath);

				convertStructureFile(filePath, 1, posBboxMin, posBboxMax, mVersion);

				FBXSDK_NAMESPACE::FbxVector4 diffuse(1.0, 1.0, 1.0, 1.0);
				FBXSDK_NAMESPACE::FbxVector4 specular(1.0, 1.0, 1.0, 1.0);
				FBXSDK_NAMESPACE::FbxVector4 ambient(1.0, 1.0, 1.0, 1.0);
				convertSceneFile(workingDirectory, diffuse, specular, ambient, convertedFiles, background, mVersion);

				writeConvertedFile(convertedFilePath, convertedFiles, faceCount, faceCount, vertexCount);

				return true;
			}
			else
			{
				FBXSDK_printf("Error: Unknown STL format.");

				free(binaryData);
				binaryData = NULL;
				return false;
			}
		}

		free(binaryData);
		binaryData = NULL;

		return true;
	}
	else
	{
		return false;
	}
}

bool STLConverter::isBinarySTL(const char* buffer, unsigned int fileSize)
{
	if (fileSize < 84) 
	{
		return false;
	}

	const char *facecount_pos = buffer + 80;
	unsigned int faceCount(0);
	::memcpy(&faceCount, facecount_pos, sizeof(unsigned int));
	const unsigned int expectedBinaryFileSize = faceCount * 50 + 84;

	return expectedBinaryFileSize == fileSize;
}

bool STLConverter::isAsciiSTL(const char* buffer, unsigned int fileSize)
{
	if (isBinarySTL(buffer, fileSize))
		return false;

	const char* bufferEnd = buffer + fileSize;

	if (!ParsingUtility::SkipSpaces(&buffer))
		return false;

	if (buffer + 5 >= bufferEnd)
		return false;

	bool isASCII(strncmp(buffer, "solid", 5) == 0);

	if (isASCII) 
	{
		// A lot of importers are write solid even if the file is binary. So we have to check for ASCII-characters.
		if (fileSize >= 500) 
		{
			isASCII = true;
			for (unsigned int i = 0; i < 500; i++) 
			{
				if (buffer[i] > 127) 
				{
					isASCII = false;
					break;
				}
			}
		}
	}

	return isASCII;
}

unsigned int STLConverter::BGRV5551_RGBA8888(unsigned short color)
{
	// changed order of variable names
	unsigned char b = (((color) & 0x001F) << 3);
	unsigned char g = (((color) & 0x03E0) >> 2);
	unsigned char r = (((color) & 0x7C00) >> 7);

	return ((255 << 24) | (b << 16) | (g << 8) | (r << 0));
}

unsigned int STLConverter::RGBV5551_RGBA8888(unsigned short color)
{
	// changed order of variable names
	unsigned char r = (((color) & 0x001F) << 3);
	unsigned char g = (((color) & 0x03E0) >> 2);
	unsigned char b = (((color) & 0x7C00) >> 7);

	return ((255 << 24) | (b << 16) | (g << 8) | (r << 0));
}

bool STLConverter::convertBinaryFile(
	unsigned char* buffer,
	unsigned int size,
	const std::string& workingDirectory,
	const std::string& filePath,
	unsigned int optimizationFlag,
	std::string& background)
{
	// https://en.wikipedia.org/wiki/STL_(file_format)
	//Skip the first 80 bytes
	if (size < 84) 
	{
		FBXSDK_printf("Error: STL binary file is too small for the header.");
		return false;
	}

	bool isMaterialise = false;

	FBXSDK_NAMESPACE::FbxVector4 diffuse(1.0, 1.0, 1.0, 1.0);
	FBXSDK_NAMESPACE::FbxVector4 specular(1.0, 1.0, 1.0, 1.0);
	FBXSDK_NAMESPACE::FbxVector4 ambient(1.0, 1.0, 1.0, 1.0);

	//Search for an occurrence of "COLOR=" in the header
	unsigned int defaultColor = 0xffffffff;
	const unsigned char* sz2 = (const unsigned char*)buffer;
	const unsigned char* const szEnd = sz2 + 80;
	while (sz2 < szEnd) 
	{
		if ('C' == *sz2++ && 'O' == *sz2++ && 'L' == *sz2++ &&
			'O' == *sz2++ && 'R' == *sz2++ && '=' == *sz2++)
		{
			//Read the default vertex color for facets
			isMaterialise = true;
			defaultColor = *((unsigned int*)sz2); //RGBA L->H

			break;
		}
	}

	unsigned char* sz = (unsigned char*)buffer + 80;
	unsigned int faceCount = *((unsigned int*)sz);

	sz += 4;

	//84 = 80(Header) + 4(Face Count)
	//50 = 3float(Normal) + 3float(PositonX) + 3float(PositonY) + 3float(PositonZ) + 1Short(Attribute)
	if (size < 84 + faceCount * 50) 
	{
		FBXSDK_printf("Error: STL binary file is too small to hold all facets.");
		return false;
	}

	if (faceCount == 0)
	{
		FBXSDK_printf("Error: STL binary file is empty. There are no facets defined.");
		return false;
	}

	unsigned int vertexCount = faceCount * 3;

	std::vector<VERTEX_LAYER_TYPE> vertexElements;

	//Position
	uint8_t* outPosEncodingBuffer = new uint8_t[vertexCount * sizeof(float) * 3];
	unsigned int outPosEncodingSize = vertexCount * (unsigned int)sizeof(float) * 3;
	unsigned int outPosEncodingFlag = 0;
	vertexElements.push_back(VL_POSITION);

	//Normal
	uint8_t* outNormalEncodingBuffer = new uint8_t[vertexCount * sizeof(float) * 3];
	unsigned int outNormalEncodingSize = vertexCount * (unsigned int)sizeof(float) * 3;
	unsigned int outNormalEncodingFlag = 0;
	vertexElements.push_back(VL_NORMAL0);

	//Vertex Color
	uint8_t* outVertexColorEncodingBuffer = new uint8_t[vertexCount * sizeof(unsigned int)];
	unsigned int outVertexColorEncodingSize = vertexCount * sizeof(unsigned int);
	unsigned int outVertexColorEncodingFlag = 0;
	vertexElements.push_back(VL_VERTEXCOLOR0);

	//Index
	unsigned int indexCount = faceCount * 3;
	uint8_t* outIndexEncodingBuffer = new uint8_t[indexCount * sizeof(unsigned int)];
	unsigned int outIndexEncodingSize = indexCount * (unsigned int)sizeof(unsigned int);
	unsigned int outIndexEncodingFlag = 0;

	//Topological Index
	unsigned int outTopologicalIndexEncodingSize = 6 * faceCount * sizeof(int);
	uint8_t* outTopologicalIndexEncodingBuffer = new uint8_t[outTopologicalIndexEncodingSize];
	unsigned int outTopologicalIndexEncodingFlag = 0;

	FBXSDK_NAMESPACE::FbxDouble3 posBboxMin(FLT_MAX, FLT_MAX, FLT_MAX);
	FBXSDK_NAMESPACE::FbxDouble3 posBboxMax(FLT_MIN, FLT_MIN, FLT_MIN);
	for (unsigned int i = 0; i < faceCount; ++i)
	{
		uint8_t* src = sz + 50*i;

		uint8_t* dest = outPosEncodingBuffer + 36 * i;
		memcpy(dest, src + sizeof(float) * 3, sizeof(float) * 3);
		memcpy(dest + sizeof(float) * 3, src + sizeof(float) * 6, sizeof(float) * 3);
		memcpy(dest + sizeof(float) * 6, src + sizeof(float) * 9, sizeof(float) * 3);

		float x0 = *((float*)dest + 0);
		float y0 = *((float*)dest + 1);
		float z0 = *((float*)dest + 2);

		float x1 = *((float*)dest + 3);
		float y1 = *((float*)dest + 4);
		float z1 = *((float*)dest + 5);

		float x2 = *((float*)dest + 6);
		float y2 = *((float*)dest + 7);
		float z2 = *((float*)dest + 8);

		if (x0 < posBboxMin[0])
			posBboxMin[0] = x0;

		if (y0 < posBboxMin[1])
			posBboxMin[1] = y0;

		if (z0 < posBboxMin[2])
			posBboxMin[2] = z0;

		if (x0 > posBboxMax[0])
			posBboxMax[0] = x0;

		if (y0 > posBboxMax[1])
			posBboxMax[1] = y0;

		if (z0 > posBboxMax[2])
			posBboxMax[2] = z0;

		dest = outNormalEncodingBuffer + 36 * i;
		float nx = *((float*)dest + 0);
		float ny = *((float*)dest + 1);
		float nz = *((float*)dest + 2);
		float sLength = nx*nx + ny*ny + nz*nz;
		if (sLength < 0.999f || sLength > 1.001f)
		{
			FBXSDK_NAMESPACE::FbxVector4 v10(x1 - x0, y1 - y0, z1 - z0);
			FBXSDK_NAMESPACE::FbxVector4 v20(x2 - x0, y2 - y0, z2 - z0);
			FBXSDK_NAMESPACE::FbxVector4 n = v10.CrossProduct(v20);
			n.Normalize();
			*((float*)src + 0) = (float)n[0];
			*((float*)src + 1) = (float)n[1];
			*((float*)src + 2) = (float)n[2];
		}

		memcpy(dest, src, sizeof(float) * 3);
		memcpy(dest + sizeof(float) * 3, src, sizeof(float) * 3);
		memcpy(dest + sizeof(float) * 6, src, sizeof(float) * 3);		

		dest = outVertexColorEncodingBuffer + 12 * i;
		unsigned short color = *((unsigned short*)(src + sizeof(float) * 12));
		if (isMaterialise)
		{
			//Materialise Magics
			if (color & (1 << 15))
			{
				*((unsigned int*)dest + 0) = defaultColor;
				*((unsigned int*)dest + 1) = defaultColor;
				*((unsigned int*)dest + 2) = defaultColor;
			}
			else
			{
				unsigned int facetColor = RGBV5551_RGBA8888(color);
				*((unsigned int*)dest + 0) = facetColor;
				*((unsigned int*)dest + 1) = facetColor;
				*((unsigned int*)dest + 2) = facetColor;
			}
		}
		else
		{
			//VisCAM SolidView
			if (color & (1 << 15))
			{
				unsigned int facetColor = BGRV5551_RGBA8888(color);
				*((unsigned int*)dest + 0) = defaultColor;
				*((unsigned int*)dest + 1) = defaultColor;
				*((unsigned int*)dest + 2) = defaultColor;
			}
			else
			{
				*((unsigned int*)dest + 0) = 0xffffffff;
				*((unsigned int*)dest + 1) = 0xffffffff;
				*((unsigned int*)dest + 2) = 0xffffffff;
			}
		}

		//Index
		dest = outIndexEncodingBuffer + 12 * i;
		*((unsigned int*)dest + 0) = i * 3 + 0;
		*((unsigned int*)dest + 1) = i * 3 + 1;
		*((unsigned int*)dest + 2) = i * 3 + 2;

		//Topological Index
		dest = outTopologicalIndexEncodingBuffer + 24 * i;
		*((unsigned int*)dest + 0) = i * 3 + 0;
		*((unsigned int*)dest + 1) = i * 3 + 1;
		*((unsigned int*)dest + 2) = i * 3 + 1;
		*((unsigned int*)dest + 3) = i * 3 + 2;
		*((unsigned int*)dest + 4) = i * 3 + 2;
		*((unsigned int*)dest + 5) = i * 3 + 0;
	}
	
	//Sub Mesh
	unsigned int subMeshCount = 1;
	unsigned int outSubMeshEncodingSize = sizeof(unsigned int) * 16 * subMeshCount;	//start1 count1, min_x_y_z3, max_x_y_z3
	uint8_t* outSubMeshEncodingBuffer = new uint8_t[outSubMeshEncodingSize];
	unsigned int outSubMeshEncodingFlag = 1;
	memset(outSubMeshEncodingBuffer, 0, outSubMeshEncodingSize);
	((unsigned int*)outSubMeshEncodingBuffer)[0] = 0;
	((unsigned int*)outSubMeshEncodingBuffer)[1] = faceCount;

	FbxUInt64 parentNodeID = 1;
	FbxUInt64 nodeID = 2;

	JsonToBin_V4* writer = new JsonToBin_V4();
	writer->openExportFile(workingDirectory + "STL_Mesh.2.mesh.bin");

	FBXSDK_NAMESPACE::FbxDouble3 uvBboxMin(0.0, 0.0, 0.0);
	FBXSDK_NAMESPACE::FbxDouble3 uvBboxMax(0.0, 0.0, 0.0);

	float posBoundingBoxMin[3] = { (float)posBboxMin[0], (float)posBboxMin[1], (float)posBboxMin[2] };
	float posBoundingBoxMax[3] = { (float)posBboxMax[0], (float)posBboxMax[1], (float)posBboxMax[2] };

	writer->writeObjectBin(
		parentNodeID,
		nodeID,
		MESH_TYPE,
		"STL_Mesh",
		(unsigned int)vertexCount,
		vertexElements,
		NULL,			//Bones
		\
		subMeshCount,
		outSubMeshEncodingBuffer,
		outSubMeshEncodingSize,
		outSubMeshEncodingFlag,
		\
		faceCount,
		outIndexEncodingBuffer,
		outIndexEncodingSize,
		outIndexEncodingFlag,
		\
		3 * faceCount,
		outTopologicalIndexEncodingBuffer,
		outTopologicalIndexEncodingSize,
		outTopologicalIndexEncodingFlag,
		\
		posBoundingBoxMin,
		posBoundingBoxMax,
		outPosEncodingBuffer,
		outPosEncodingSize,
		outPosEncodingFlag,
		\
		outNormalEncodingBuffer,
		outNormalEncodingSize,
		outNormalEncodingFlag,
		\
		NULL, 0, 0,		//Tangent
		\
		outVertexColorEncodingBuffer,
		outVertexColorEncodingSize,
		outVertexColorEncodingFlag,
		\
		uvBboxMin, uvBboxMax, NULL, 0, 0, //UV0
		uvBboxMin, uvBboxMax, NULL, 0, 0, //UV1
		\
		NULL, 0, 0, //BW & BI
		\
		NULL, 0, 0);
	
	writer->closeExportFile();
	delete writer;

	std::vector<std::string> convertedFiles;
	convertedFiles.push_back("scene.json");
	convertedFiles.push_back(filePath + ".structure.json");
	convertedFiles.push_back("STL_Mesh.2.mesh.bin");

	std::string convertedFilePath = filePath + ".convertedFiles";
	convertedFiles.push_back(convertedFilePath);

	convertStructureFile(filePath, 1, posBboxMin, posBboxMax, mVersion);

	convertSceneFile(workingDirectory, diffuse, specular, ambient, convertedFiles, background, mVersion);

	writeConvertedFile(convertedFilePath, convertedFiles, faceCount, faceCount, vertexCount);

	return true;
}

void STLConverter::convertStructureFile(
	const std::string& fullFilePath,
	int materialCount,
	FBXSDK_NAMESPACE::FbxDouble3& posBboxMin,
	FBXSDK_NAMESPACE::FbxDouble3& posBboxMax,
	int version)
{
	std::string filePath = fullFilePath + ".structure.json";

	JSONFileWriter* sceneStructureJSONWriter = new JSONFileWriter(false);
	sceneStructureJSONWriter->openExportFile(filePath);

	int jsonLevel = 0;
	sceneStructureJSONWriter->writeObjectInfo("{", jsonLevel);

	string newPath = fullFilePath;
	newPath = AssistantFunctions::replace_all_distinct(newPath, "\\", "\\\\");

	char buffer[__TEMP_BUFFER_FLOAT__];
	sprintf(buffer, "\"version\":%d,", version);
	sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 1);

	sceneStructureJSONWriter->writeObjectInfo("\"srcVersion\":\"STL\",", jsonLevel + 1);
	sceneStructureJSONWriter->writeObjectInfo("\"name\":\"" + newPath + "\",", jsonLevel + 1);
	sceneStructureJSONWriter->writeObjectInfo("\"setting\":{},", jsonLevel + 1);

	sceneStructureJSONWriter->writeObjectInfo("\"nodeTree\":", jsonLevel + 1);
	sceneStructureJSONWriter->writeObjectInfo("{", jsonLevel + 1);

	//////////////////////////////////////////////////////
	sceneStructureJSONWriter->writeObjectInfo("\"uniqueID\":0,", jsonLevel + 2);

	std::string guidString = newUUID();
	sprintf(buffer, "\"guid\":\"%s\",", guidString.c_str());
	sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 2);

	sceneStructureJSONWriter->writeObjectInfo("\"name\":\"RootNode\",", jsonLevel + 2);

	sceneStructureJSONWriter->writeObjectInfo("\"translation\":[0.0,0.0,0.0],", jsonLevel + 2);
	sceneStructureJSONWriter->writeObjectInfo("\"rotation\":[-90.0,0.0,0.0],", jsonLevel + 2);
	sceneStructureJSONWriter->writeObjectInfo("\"scaling\":[1.0,1.0,1.0],", jsonLevel + 2);

	sceneStructureJSONWriter->writeObjectInfo("\"materials\":", jsonLevel + 2);
	sceneStructureJSONWriter->writeObjectInfo("[", jsonLevel + 2);
	sceneStructureJSONWriter->writeObjectInfo("],", jsonLevel + 2);

	sceneStructureJSONWriter->writeObjectInfo("\"nodeAttr\":", jsonLevel + 2);
	sceneStructureJSONWriter->writeObjectInfo("{", jsonLevel + 2);
	sceneStructureJSONWriter->writeObjectInfo("},", jsonLevel + 2);

	sceneStructureJSONWriter->writeObjectInfo("\"children\":", jsonLevel + 2);
	sceneStructureJSONWriter->writeObjectInfo("[", jsonLevel + 2);

	writeMeshNode(sceneStructureJSONWriter, materialCount, posBboxMin, posBboxMax, jsonLevel + 3, true);

	sceneStructureJSONWriter->writeObjectInfo("]", jsonLevel + 2);
	////////////////////////////////////////////////////////////
	sceneStructureJSONWriter->writeObjectInfo("}", jsonLevel + 1);
	sceneStructureJSONWriter->writeObjectInfo("}", jsonLevel);

	//<
	sceneStructureJSONWriter->closeExportFile();
	delete sceneStructureJSONWriter;
	sceneStructureJSONWriter = NULL;
}

void STLConverter::writeMeshNode(
	JSONFileWriter* sceneStructureJSONWriter,
	int materialCount,
	FBXSDK_NAMESPACE::FbxDouble3& posBboxMin,
	FBXSDK_NAMESPACE::FbxDouble3& posBboxMax,
	int jsonLevel,
	bool lastObject)
{
	sceneStructureJSONWriter->writeObjectInfo("{", jsonLevel);

	sceneStructureJSONWriter->writeObjectInfo("\"uniqueID\":1,", jsonLevel + 1);
	char buffer[__TEMP_BUFFER_FLOAT__];
	std::string guidString = newUUID();
	sprintf(buffer, "\"guid\":\"%s\",", guidString.c_str());
	sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 1);

	sceneStructureJSONWriter->writeObjectInfo("\"name\":\"MeshNode\",", jsonLevel + 1);

	sceneStructureJSONWriter->writeObjectInfo("\"translation\":[0.0,0.0,0.0],", jsonLevel + 1);
	sceneStructureJSONWriter->writeObjectInfo("\"rotation\":[0.0,0.0,0.0],", jsonLevel + 1);
	sceneStructureJSONWriter->writeObjectInfo("\"scaling\":[1.0,1.0,1.0],", jsonLevel + 1);

	sceneStructureJSONWriter->writeObjectInfo("\"materials\":", jsonLevel + 1);
	sceneStructureJSONWriter->writeObjectInfo("[", jsonLevel + 1);
	for (size_t materialIndex = 0; materialIndex < materialCount; ++materialIndex)
	{
		if (materialIndex == materialCount - 1)
			sprintf(buffer, "%d", (int)materialIndex);
		else
			sprintf(buffer, "%d,", (int)materialIndex);

		sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 2);
	}
	sceneStructureJSONWriter->writeObjectInfo("],", jsonLevel + 1);

	sceneStructureJSONWriter->writeObjectInfo("\"nodeAttr\":", jsonLevel + 1);
	sceneStructureJSONWriter->writeObjectInfo("{", jsonLevel + 1);

	sceneStructureJSONWriter->writeObjectInfo("\"uniqueID\":2,", jsonLevel + 2);
	sceneStructureJSONWriter->writeObjectInfo("\"name\":\"STL_Mesh.2.mesh.bin\",", jsonLevel + 2);
	sceneStructureJSONWriter->writeObjectInfo("\"type\":\"mesh\",", jsonLevel + 2);
	sceneStructureJSONWriter->writeObjectInfo("\"bbox\":", jsonLevel + 2);
	sceneStructureJSONWriter->writeObjectInfo("[", jsonLevel + 2);
	sprintf(buffer, "[%0.3f,%0.3f,%0.3f],", posBboxMin[0], posBboxMin[1], posBboxMin[2]);
	sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 3);
	sprintf(buffer, "[%0.3f,%0.3f,%0.3f]", posBboxMax[0], posBboxMax[1], posBboxMax[2]);
	sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 3);
	sceneStructureJSONWriter->writeObjectInfo("]", jsonLevel + 2);

	sceneStructureJSONWriter->writeObjectInfo("},", jsonLevel + 1);

	sceneStructureJSONWriter->writeObjectInfo("\"children\":", jsonLevel + 1);
	sceneStructureJSONWriter->writeObjectInfo("[", jsonLevel + 1);
	sceneStructureJSONWriter->writeObjectInfo("]", jsonLevel + 1);
	////////////////////////////////////////////////////////////
	if (lastObject)
	{
		sceneStructureJSONWriter->writeObjectInfo("}", jsonLevel);
	}
	else
	{
		sceneStructureJSONWriter->writeObjectInfo("},", jsonLevel);
	}
}

void STLConverter::convertSceneFile(
	const std::string& workingDirectory,
	const FbxVector4& diffuse,
	const FbxVector4& specular,
	const FbxVector4& ambient,
	std::vector<std::string>& convertedFiles,
	const std::string& background,
	int version)
{
	std::vector<std::string> textureFiles;
	std::vector<Material*> materials;
	std::vector<TextureMap*> textureMaps;

	Material* cache = new Material();
	cache->parseSTLMaterial(0, diffuse, specular, ambient, textureMaps, textureFiles);
	materials.push_back(cache);

	//////////////////////////////////////////////////////
	std::string sceneConfigFile = workingDirectory + "scene.json";
	//FBXSceneStructureExporter_V4::writeSceneConfigFile(sceneConfigFile, version, materials, textureMaps, textureFiles, background);

	int textureCount = (int)textureFiles.size();
	for (int i = 0; i < textureCount; ++i)
	{
		convertedFiles.push_back(textureFiles[i]);
	}
}

bool STLConverter::convertASCIIFile(
	unsigned char* buffer, 
	unsigned int fileSize, 
	std::vector<float>& positions,
	FBXSDK_NAMESPACE::FbxDouble3& posBboxMin,
	FBXSDK_NAMESPACE::FbxDouble3& posBboxMax,
	std::vector<float>& normals,
	std::vector<int>& indices,
	std::vector<int>& topologicalIndices,
	unsigned int& faceCount)
{
	const char* sz = (const char*)buffer;
	const char* bufferEnd = sz + fileSize;
	// try to guess how many vertices we could have
	// assume we'll need 160 bytes for each face
	size_t sizeEstimate = max_0(1u, fileSize / 160u) * 3;

	positions.reserve(sizeEstimate*3);
	normals.reserve(sizeEstimate*3);

	posBboxMin[0] = posBboxMin[1] = posBboxMin[2] = FLT_MAX;
	posBboxMax[0] = posBboxMax[1] = posBboxMax[2] = FLT_MIN;

	bool recalculateNormal = false;

	while (isAsciiSTL(sz, static_cast<unsigned int>(bufferEnd - sz))) 
	{
		ParsingUtility::SkipSpaces(&sz);
		sz += 5; // skip the "solid"
		ParsingUtility::SkipSpaces(&sz);
		const char* szMe = sz;
		while (!ParsingUtility::IsSpaceOrNewLine(*sz)) 
		{
			sz++;
		}

		size_t temp;
		// setup the name of the node
		if ((temp = (size_t)(sz - szMe))) 
		{
			if (temp >= MAXLEN)
			{
				FBXSDK_printf("Error: STL node name too long.\n");
				return false;
			}
			std::string name(szMe, temp);

		}
		else 
		{
			
		}

		unsigned int faceVertexCounter = 3;
		for (;;) 
		{
			// go to the next token
			if (!ParsingUtility::SkipSpacesAndLineEnd(&sz))
			{
				// seems we're finished although there was no end marker
				FBXSDK_printf("Error: STL parsing unexpected EOF. \"endsolid\" keyword was expected.\n");
				break;
			}

			if (!strncmp(sz, "facet", 5) && ParsingUtility::IsSpaceOrNewLine(*(sz + 5)) && *(sz + 5) != '\0')
			{
				if (faceVertexCounter != 3) 
				{
					FBXSDK_printf("Error: STL face must be composed with three vertex.\n");
					return false;
				}
				faceVertexCounter = 0;

				sz += 6;
				ParsingUtility::SkipSpaces(&sz);
				if (strncmp(sz, "normal", 6)) 
				{
					FBXSDK_printf("Error: STL parsing a facet normal vector was expected but not found.\n");
				}
				else 
				{
					if (sz[6] == '\0') 
					{
						FBXSDK_printf("Error: STL parsing unexpected EOF while parsing facet.\n");
						return false;
					}

					sz += 7;
					ParsingUtility::SkipSpaces(&sz);

					double nx = 0.0;
					double ny = 0.0;
					double nz = 0.0;
					sz = ParsingUtility::fast_atoreal_move(sz, (double&)nx);
					ParsingUtility::SkipSpaces(&sz);
					sz = ParsingUtility::fast_atoreal_move(sz, (double&)ny);
					ParsingUtility::SkipSpaces(&sz);
					sz = ParsingUtility::fast_atoreal_move(sz, (double&)nz);

					double sLength = nx*nx + ny*ny + nz*nz;
					if (sLength < 0.999 || sLength > 1.001)
					{
						recalculateNormal = true;
					}

					normals.push_back((float)nx);
					normals.push_back((float)ny);
					normals.push_back((float)nz);

					normals.push_back((float)nx);
					normals.push_back((float)ny);
					normals.push_back((float)nz);

					normals.push_back((float)nx);
					normals.push_back((float)ny);
					normals.push_back((float)nz);
				}
			}
			else if (!strncmp(sz, "vertex", 6) && ParsingUtility::IsSpaceOrNewLine(*(sz + 6)))
			{
				if (faceVertexCounter >= 3) 
				{

					FBXSDK_printf("Error: STL parsing a facet with more than 3 vertices has been found.\n");
					++sz;
				}
				else 
				{
					if (sz[6] == '\0') 
					{
						FBXSDK_printf("Error: STL parsing unexpected EOF while parsing facet.\n");
						return false;
					}

					sz += 7;

					ParsingUtility::SkipSpaces(&sz);

					double x = 0.0;
					double y = 0.0;
					double z = 0.0;
					sz = ParsingUtility::fast_atoreal_move(sz, (double&)x);
					ParsingUtility::SkipSpaces(&sz);
					sz = ParsingUtility::fast_atoreal_move(sz, (double&)y);
					ParsingUtility::SkipSpaces(&sz);
					sz = ParsingUtility::fast_atoreal_move(sz, (double&)z);

					if (x < posBboxMin[0])
						posBboxMin[0] = x;

					if (y < posBboxMin[1])
						posBboxMin[1] = y;

					if (z < posBboxMin[2])
						posBboxMin[2] = z;

					if (x > posBboxMax[0])
						posBboxMax[0] = x;

					if (y > posBboxMax[1])
						posBboxMax[1] = y;

					if (z > posBboxMax[2])
						posBboxMax[2] = z;

					positions.push_back((float)x);
					positions.push_back((float)y);
					positions.push_back((float)z);

					faceVertexCounter++;
				}
			}
			else if (!::strncmp(sz, "endsolid", 8))
			{
				do 
				{
					++sz;
				} 
				while (!ParsingUtility::IsLineEnd(*sz));

				ParsingUtility::SkipSpacesAndLineEnd(&sz);
				// finished!
				break;
			}
			else
			{ 
				// else skip the whole identifier
				do 
				{
					++sz;
				} 
				while (!ParsingUtility::IsSpaceOrNewLine(*sz));
			}
		}//for
	}

	faceCount = (unsigned int)positions.size() / 9;
	indices.reserve(faceCount * 3);
	topologicalIndices.reserve(faceCount * 6);
	for (unsigned int i = 0; i < faceCount; ++i)
	{
		indices.push_back(i * 3 + 0);
		indices.push_back(i * 3 + 1);
		indices.push_back(i * 3 + 2);

		topologicalIndices.push_back(i * 3 + 0);
		topologicalIndices.push_back(i * 3 + 1);
		topologicalIndices.push_back(i * 3 + 1);
		topologicalIndices.push_back(i * 3 + 2);
		topologicalIndices.push_back(i * 3 + 2);
		topologicalIndices.push_back(i * 3 + 0);
	}

	if (recalculateNormal)
	{
		for (unsigned int i = 0; i < faceCount; ++i)
		{
			float x0 = positions[i * 9 + 0], y0 = positions[i * 9 + 1], z0 = positions[i * 9 + 2];
			float x1 = positions[i * 9 + 3], y1 = positions[i * 9 + 4], z1 = positions[i * 9 + 5];
			float x2 = positions[i * 9 + 6], y2 = positions[i * 9 + 7], z2 = positions[i * 9 + 8];

			FBXSDK_NAMESPACE::FbxVector4 v10(x1 - x0, y1 - y0, z1 - z0);
			FBXSDK_NAMESPACE::FbxVector4 v20(x2 - x0, y2 - y0, z2 - z0);
			FBXSDK_NAMESPACE::FbxVector4 n = v10.CrossProduct(v20);
			n.Normalize();

			normals[i * 9 + 0] = (float)n[0];
			normals[i * 9 + 1] = (float)n[1];
			normals[i * 9 + 2] = (float)n[2];

			normals[i * 9 + 3] = (float)n[0];
			normals[i * 9 + 4] = (float)n[1];
			normals[i * 9 + 5] = (float)n[2];

			normals[i * 9 + 6] = (float)n[0];
			normals[i * 9 + 7] = (float)n[1];
			normals[i * 9 + 8] = (float)n[2];
		}
	}
	
	return true;
}