#pragma once
#include "FileWriter.h"
#include "../rapidjson/writer.h"
#include "../rapidjson/filewritestream.h"

using namespace rapidjson;

class PlayCanvasWriter : public FileWriter
{
public:
	PlayCanvasWriter();
	virtual ~PlayCanvasWriter();

	void setRapidJsonWriter(Writer<FileWriteStream>* writer);

	virtual void writeObjectBin(
		FbxUInt64 parentNodeID,
		FbxUInt64 nodeID,
		const char* objectType,
		const std::string& objectName,
		unsigned int vertexCount,
		std::vector<VERTEX_LAYER_TYPE>& vertexElement,
		std::vector<BONE_>* bones,
		\
		unsigned int subMeshCount,
		uint8_t* subMeshEncodingBuffer,
		unsigned int subMeshEncodingSize,
		unsigned int subMeshEncodingFlag,
		\
		unsigned int triangleCount,
		uint8_t* indexEncodingBuffer,
		unsigned int indexEncodingSize,
		unsigned int indexEncodingFlag,
		\
		unsigned int topologicalLineCount,
		uint8_t* topologicalIndexEncodingBuffer,
		unsigned int topologicalIndexEncodingSize,
		unsigned int topologicalIndexEncodingFlag,
		\
		FBXSDK_NAMESPACE::FbxDouble3& PosBboxMin,
		FBXSDK_NAMESPACE::FbxDouble3& PosBboxMax,
		uint8_t* posistionEncodingBuffer,
		unsigned int posistionEncodingSize,
		unsigned int posistionEncodingFlag,
		\
		uint8_t* normalEncodingBuffer,
		unsigned int normalEncodingSize,
		unsigned int normalEncodingFlag,
		\
		uint8_t* tangentEncodingBuffer,
		unsigned int tangentEncodingSize,
		unsigned int tangentEncodingFlag,
		\
		uint8_t* vcEncodingBuffer,
		unsigned int vcEncodingSize,
		unsigned int vcEncodingFlag,
		\
		FBXSDK_NAMESPACE::FbxDouble3& uvBboxMin,
		FBXSDK_NAMESPACE::FbxDouble3& uvBboxMax,
		uint8_t* uvEncodingBuffer,
		unsigned int uvEncodingSize,
		unsigned int uvEncodingFlag,
		\
		uint8_t* bwEncodingBuffer,
		unsigned int bwEncodingSize,
		unsigned int bwEncodingFlag,
		\
		uint8_t* biEncodingBuffer,
		unsigned int biEncodingSize,
		unsigned int biBIEncodingFlag);

	virtual void writeAnimationBin(
		FbxUInt64 clipID,
		const std::string& clipName,
		float fps,
		float startFrame,
		float endFrame,
		std::vector<animationClipData>* clipData);

private:
	Writer<FileWriteStream>* mJSONWriter;
};