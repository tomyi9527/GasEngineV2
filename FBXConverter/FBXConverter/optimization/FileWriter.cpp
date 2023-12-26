#include "FileWriter.h"

FileWriter::FileWriter()
{

}

FileWriter::~FileWriter()
{

}

void FileWriter::writeObjectBin(
	unsigned int parentNodeID,
	unsigned int nodeID,
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
	float PosBboxMin[3],
	float PosBboxMax[3],
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
	FBXSDK_NAMESPACE::FbxDouble3& uv0BboxMin,
	FBXSDK_NAMESPACE::FbxDouble3& uv0BboxMax,
	uint8_t* uv0EncodingBuffer,
	unsigned int uv0EncodingSize,
	unsigned int uv0EncodingFlag,
	\
	FBXSDK_NAMESPACE::FbxDouble3& uv1BboxMin,
	FBXSDK_NAMESPACE::FbxDouble3& uv1BboxMax,
	uint8_t* uv1EncodingBuffer,
	unsigned int uv1EncodingSize,
	unsigned int uv1EncodingFlag,
	\
	uint8_t* bwEncodingBuffer,
	unsigned int bwEncodingSize,
	unsigned int bwEncodingFlag,
	\
	uint8_t* biEncodingBuffer,
	unsigned int biEncodingSize,
	unsigned int biBIEncodingFlag)
{

}


void FileWriter::writeAnimationBin(
	unsigned int clipID,
	const std::string& clipName,
	float fps,
	float startFrame,
	float endFrame,
	std::vector<animationClipData>* clipData)
{

}