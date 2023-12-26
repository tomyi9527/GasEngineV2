#pragma once
#include <stdint.h>
#include "../stdafx.h"
#include "../Common/Utils.h"
#include "../CommonStruct.h"
#include "./AnimationsWriter.h"
#include "BinData.h"

class glTFBin
{
public:
	glTFBin();
	~glTFBin();

    void openExportFile(const string& filePath);
	void closeExportFile();
    void openAnimationFile(const string& filePath);
	void closeAnimationFile();

    void writeSkinMatricesBin(
        const vector<FbxAMatrix>& vBindMat,
        Accessor_data & accessor
        );
	void writeObjectBin(
        vector<Accessor_data>& vBinAccessorData,
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

	void writeAnimationBin(
        vector<glTFAnimationChannelData>& channelArray,
		unsigned int clipID,
		const std::string& clipName,
		float fps,
		float startFrame,
		float endFrame,
		std::vector<animationClipData>* clipData);

private:
	FILE* mBinFile;
	FILE* mAnimationBinFile;
    string mBinFileName;
    string mAnimationBinFileName;
    vector<Accessor_data> mData;
};
