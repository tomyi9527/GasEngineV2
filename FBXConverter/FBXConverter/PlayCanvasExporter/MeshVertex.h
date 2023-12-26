#pragma once
#include "../stdafx.h"
#include "../CommonStruct.h"
#include "../TootleLib/include/tootlelib.h"
#include "./BinData.h"
#include "./SkinWriter.h"
#include "JSONFileWriter.h"
#include "JsonToBin_V4.h"
#include "tri_stripper.h"
#include "NvTriStrip.h"
#include "Material.h"
#include "meshLoader.h"

class glTFBin;
class JsonToBin_V4;
class meshLoader;

class MeshVertex
{
public:
	MeshVertex();
	~MeshVertex();

	static const char* getElementNameByType(VERTEX_LAYER_TYPE type)
	{
		if (type < VL_POSITION || type > VL_BLENDINDEX)
			return NULL;
		else
			return VERTEX_ELEMENT_NAME[type];
	}

	std::string compressVertices(
        vector<Accessor_data>& vBinAccessorData,
        stSkinInfo& skinInfo,
        vector<stSubMeshInfo>& vSubMeshInfo,
		FBXSDK_NAMESPACE::FbxManager* sdkManager,
		unsigned int optimizationFlag,
		FBXSDK_NAMESPACE::FbxMesh* mesh,
		const int materialCount,
		FBXSDK_NAMESPACE::FbxDouble3& bboxMin,
		FBXSDK_NAMESPACE::FbxDouble3& bboxMax,
		std::string& diretory,
		unsigned int& triangleCount,
		unsigned int& polygonCount,
		unsigned int& vertexCount,
		bool& isSkinned);

	int naiveTriangulate(FbxMesh* mesh, \
		std::vector<int>& triangleStartPerPolygon, std::vector<int>& triangleCountPerPolygon, std::vector<int>& triangleIndices);

	void getTopologicalIndices(FbxMesh* mesh, std::vector<unsigned int>& topologyIndices);

	int getPositionIndices(FBXSDK_NAMESPACE::FbxMesh* mesh, std::vector<int>& indices, std::vector<int>& triangleCountPerPolygon, std::vector<int>& triangles);

	int getUVIndices(FBXSDK_NAMESPACE::FbxMesh* mesh, std::vector<int>& indices, int uvIndex, std::vector<int>& triangleCountPerPolygon, std::vector<int>& triangles);

	int getNormalIndices(
		FBXSDK_NAMESPACE::FbxMesh* mesh, 
		std::vector<int>& indices, 
		int normalIndex, 
		std::vector<int>& triangleCountPerPolygon, 
		std::vector<int>& triangles);

	int getTangentIndices(
		FBXSDK_NAMESPACE::FbxMesh* mesh, 
		std::vector<int>& indices, 
		int tangentIndex, 
		std::vector<int>& triangleCountPerPolygon, 
		std::vector<int>& triangles);

	int getBinormalIndices(
		FBXSDK_NAMESPACE::FbxMesh* mesh,
		std::vector<int>& indices,
		int tangentIndex,
		std::vector<int>& triangleCountPerPolygon,
		std::vector<int>& triangles);

	int getVertexColorIndices(FBXSDK_NAMESPACE::FbxMesh* mesh, std::vector<int>& indices, int vertexColorIndex, std::vector<int>& triangleCountPerPolygon, std::vector<int>& triangles);

	void createSubmeshByMaterials(FBXSDK_NAMESPACE::FbxMesh* mesh, const int materialCount, int layerIndex, std::vector<std::vector<int> >& subMeshes, \
		std::vector<int>& triangleStartPerPolygon, std::vector<int>& triangleCountPerPolygon);

	static void getEffectiveBones(
		FbxMesh* mesh,
		std::vector<FBXSDK_NAMESPACE::FbxUInt64>& effectiveBones);

	static void getSkinning(
		FBXSDK_NAMESPACE::FbxMesh* mesh, 
		std::vector<BONE_>* effectiveBones, 
		std::vector<BW_>* bwValues, 
		std::vector<BI_>* biValues);

	FBXSDK_NAMESPACE::FbxVector4* computeVertexNormals(FBXSDK_NAMESPACE::FbxVector4* position, FBXSDK_NAMESPACE::FbxUInt32 positionCount, std::vector<int>& indices);

	void getMorph(
		FBXSDK_NAMESPACE::FbxMesh* mesh,
		std::vector<int>* positionDataIndices,
		std::vector<int>* normalDataIndices,
		std::vector<int>* uvDataIndices,
		std::vector<int>& indices,
		bool isSkinned,
		std::vector<MORPH_DATA_V2>* morphTargetsData);

	//bool appendNewLayer(std::vector<std::vector<int>*>& vb, std::vector<int>& newLayerIndices, std::vector<int>& indices, VERTEX_LAYER_TYPE layerType);

	bool appendNewLayer_V2(std::vector<std::vector<int>*>& vb, std::vector<int>& newLayerIndices, std::vector<int>& indices, VERTEX_LAYER_TYPE layerType);

	static void computeTangents(
		vector<float>& positions,
		vector<float>& uvs,
		vector<float>& normals,
		vector<int>& indices,
		vector<float>& outTangents);

	static void Orthonormlize(
		const FBXSDK_NAMESPACE::FbxVector4& N,
		FBXSDK_NAMESPACE::FbxVector4& B,
		FBXSDK_NAMESPACE::FbxVector4& T);

	static bool ComputeParity(
		FBXSDK_NAMESPACE::FbxVector4& N,
		FBXSDK_NAMESPACE::FbxVector4& B,
		FBXSDK_NAMESPACE::FbxVector4& T);

	static double ComputeTriangleArea(
		FBXSDK_NAMESPACE::FbxVector4& A,
		FBXSDK_NAMESPACE::FbxVector4& B,
		FBXSDK_NAMESPACE::FbxVector4& C);

	static void computeTangents_V2(
		const std::vector<float>& positions,
		const std::vector<float>& uvs,
		const std::vector<float>& normals,
		const std::vector<unsigned int>& smoothingGroup,
		const std::vector<float>& outTangents);

	void rearrangeTrianglesByMaterial(const std::vector<std::vector<int> >& subMeshes, const std::vector<int>& indices, \
		std::vector<unsigned int>& reorderedIndices, std::vector<unsigned int>& triangleClusters);

	bool ReorderTriangles(
		unsigned int vertexCount,
		std::vector<float>& vertexInputPrefetchOptimization,
		std::vector<unsigned int>& indices,
		std::vector<unsigned int>& triangleClusters,
		std::vector<float>& outputPositions,
		std::vector<unsigned int>& outputIndices,
		std::vector<unsigned int>& vertexRemapping);

	void stripifyTriangles_Nvidia(const vector<unsigned int>& indices);
	void stripifyTriangles_TriStripper(const vector<unsigned int>& indices);

	bool ShuffleVertexIndexOrder(std::vector<std::vector<int>*>& vertexLayers, std::vector<unsigned int>& vertexRemapping);
	bool ShuffleTopologicalIndexOrder(std::vector<unsigned int>& topologicalIndices, std::vector<unsigned int>& vertexRemapping);

	void meshOptimization(
		FbxMesh* mesh,
		unsigned int optimizationFlag,
		std::vector<VERTEX_LAYER_TYPE>& vertexElement,
		std::vector<std::vector<int>*>& vertexLayers,
		std::vector<BONE_>* bones,
		std::vector<BW_>* bwValues,
		std::vector<BI_>* biValues,
		std::vector<int>& indices,
		std::vector<unsigned int>& topologicalIndices,
		std::vector<std::vector<int> >& subMeshes,
		FBXSDK_NAMESPACE::FbxDouble3& bboxMin,
		FBXSDK_NAMESPACE::FbxDouble3& bboxMax,
		\
		uint8_t* outIndexEncodingBuffer,
		unsigned int& outIndexEncodingSize,
		unsigned int& outIndexEncodingFlag,
		\
		uint8_t* outTopologicalIndexEncodingBuffer,
		unsigned int& outTopologicalIndexEncodingSize,
		unsigned int& outTopologicalIndexEncodingFlag,
		\
		uint8_t* outPosEncodingBuffer,
		unsigned int& outPosEncodingSize,
		unsigned int& outPosEncodingFlag,
		\
		uint8_t* outNormalEncodingBuffer,
		unsigned int& outNormalEncodingSize,
		unsigned int& outNormalEncodingFlag,
		\
		uint8_t* outTangentEncodingBuffer,
		unsigned int& outTangentEncodingSize,
		unsigned int& outTangentEncodingFlag,
		\
		uint8_t* outVertexColorEncodingBuffer,
		unsigned int& outVertexColorEncodingSize,
		unsigned int& outVertexColorEncodingFlag,
		\
		FBXSDK_NAMESPACE::FbxDouble3& uvBboxMin,
		FBXSDK_NAMESPACE::FbxDouble3& uvBboxMax,
		uint8_t* outUVEncodingBuffer,
		unsigned int& outUVEncodingSize,
		unsigned int& outUVEncodingFlag,
		\
		uint8_t* outBWEncodingBuffer,
		unsigned int& outBWEncodingSize,
		unsigned int& outBWEncodingFlag,
		uint8_t* outBIEncodingBuffer,
		unsigned int& outBIEncodingSize,
		unsigned int& outBIEncodingFlag);

	static void optimizationAndWriteMorphTarget(
        uint8_t * meshPositionBuffer,
        uint8_t * meshNormalBuffer,
        vector<Accessor_data>& vBinAccessorData,
		glTFBin* writer,
		FbxUInt64 parentMeshUniqueID,
		unsigned int vertexCount,
		unsigned int optimizationFlag,
		std::vector<MORPH_DATA_V2>* morphTargetsData);

	static void getPositions(
		FbxMesh* mesh,
		std::vector<int>* dataIndices,
		bool isSkinned,
		std::vector<float>& output,
		FBXSDK_NAMESPACE::FbxDouble3& bboxMin,
		FBXSDK_NAMESPACE::FbxDouble3& bboxMax);

	static void getUVs(
		FBXSDK_NAMESPACE::FbxMesh* mesh,
		std::vector<int>* dataIndices,
		std::vector<float>& output,
		FBXSDK_NAMESPACE::FbxDouble3& bboxMin,
		FBXSDK_NAMESPACE::FbxDouble3& bboxMax);

	static void getNormals(
		FbxMesh* mesh,
		std::vector<int>* dataIndices,
		bool isSkinned,
		std::vector<float>& output);

	static void getTangents(
		FbxMesh* mesh,
		std::vector<int>* dataIndices,
		bool isSkinned,
		std::vector<float>& output);

	static void getBinormals(
		FbxMesh* mesh,
		std::vector<int>* dataIndices,
		bool isSkinned,
		std::vector<float>& output);

	static std::string mergeMeshes(
		const std::string& outputDirectory, 
		std::vector<meshLoader*> loaders,
		int& parentUniqueID,
		int& uniqueID,
		FBXSDK_NAMESPACE::FbxDouble3& bboxMin,
		FBXSDK_NAMESPACE::FbxDouble3& bboxMax);

	//void getTangents_V1(
	//	FbxMesh* mesh,
	//	const std::vector<float>& tangentArray,
	//	std::vector<int>* dataIndices,
	//	bool isSkinned,
	//	std::vector<float>& output);

	static void getVertexColors(
		FbxMesh* mesh,
		std::vector<int>* dataIndices,
		bool& unifiedColor,
		bool& unifiedAlpha,
		std::vector<uint8_t>& output);

	static bool getSmoothing(
		FBXSDK_NAMESPACE::FbxManager* sdkManager,
		FBXSDK_NAMESPACE::FbxMesh* mesh,
		bool computing,
		bool convertToSmoothingGroup,
		std::vector<int>& triangleCountPerPolygon,
		std::vector<unsigned int>& outSmoothingGroupByPolygon);

	static void getBlendWeightsAndIndices(
		std::vector<int>* dataIndices,
		std::vector<BW_>* bwValues,
		std::vector<BI_>* biValues,
		std::vector<float>& outputBW,
		std::vector<uint16_t>& outputBI);

	// Encoding functions
	static int uintEncoding(
		std::vector<float>& element,
		int count, int bias,
		float rangeLow,
		float rangeHigh,
		float epsilon,
		unsigned int& outEncodingFlag,
		uint8_t* output);

	static unsigned int positionEncoding(
		std::vector<float>& positions,
		unsigned int& outPosEncodingFlag,
		FBXSDK_NAMESPACE::FbxDouble3& bboxMin,
		FBXSDK_NAMESPACE::FbxDouble3& bboxMax,
		uint8_t* output);

	static unsigned int encodeUVs(
		unsigned int vertexCount,
		const std::vector<float>& input,
		const FBXSDK_NAMESPACE::FbxDouble3& bboxMin,
		const FBXSDK_NAMESPACE::FbxDouble3& bboxMax,
		uint8_t* outBuffer);

	static unsigned int encodeNormals(
		unsigned int vertexCount,
		const std::vector<float>& input,
		uint8_t* outBuffer);

	static unsigned int encodeTangents(
		unsigned int vertexCount,
		const std::vector<float>& input,
		uint8_t* outBuffer);

	static unsigned int encodeVertexColors(
		unsigned int vertexCount,
		const std::vector<FBXSDK_NAMESPACE::FbxUInt8>& input,
		bool unifiedColor,
		bool unifiedAlpha,
		uint8_t* output);

	static void encodeBlendWeightsAndIndices(
		std::vector<int>* dataIndices,
		std::vector<BW_>* bwValues,
		std::vector<BI_>* biValues,
		uint8_t* outBWEncodingBuffer,
		unsigned int& outBWEncodingSize,
		uint8_t* outBIEncodingBuffer,
		unsigned int& outBIEncodingSize);

	static std::vector<int>* getVertexLayer(
		std::vector<VERTEX_LAYER_TYPE>& vertexElement, 
		std::vector<std::vector<int>*>& vertexLayers,
		VERTEX_LAYER_TYPE element);

	static int varintEncoding(const std::vector<unsigned int>& input, uint8_t* output);
	static bool varintDecoding(uint8_t* input, int inputBufferSize, int elementCount, std::vector<unsigned int>& output);
	static void highWatermarkEncoding(std::vector<unsigned int>& input);
	static void highWatermarkDecoding(std::vector<unsigned int>& input);
	static void deltaAndZigzagEncoding(std::vector<unsigned int>& input);
	static void zigzagAndDeltaDecoding(std::vector<unsigned int>& input);
	//static unsigned short Float32ToFloat16(float f);
	static unsigned int ARGB8565_ARGB8888(unsigned short color, unsigned char alpha);
	static unsigned short RGB888_RGB565(unsigned char red, unsigned char green, unsigned char blue);

	static void initNormalCompressor();
	static void mapNormal(
		double x,
		double y,
		double z,
		double w,
		unsigned int& encodedPhi,
		unsigned int& encodedTheta);
public:
	static FbxVector4 transformPositionWithGeometricTransform(const FBXSDK_NAMESPACE::FbxAMatrix& meshGeometry, FbxDouble x, FbxDouble y, FbxDouble z);
	static FbxVector4 transformNormalWithGeometricTransform(const FBXSDK_NAMESPACE::FbxAMatrix& meshGeometry, FbxDouble x, FbxDouble y, FbxDouble z);
	static FbxAMatrix getGeometryMatrix(FBXSDK_NAMESPACE::FbxNode* node);
	static FbxAMatrix getWorldToBoneLocalMatrixAtBindPose(FBXSDK_NAMESPACE::FbxCluster* cluster, FBXSDK_NAMESPACE::FbxNode* meshNode);

private:
	FBXSDK_NAMESPACE::FbxLayerElementArrayTemplate<FBXSDK_NAMESPACE::FbxVector2>* mUVArray0;
	FBXSDK_NAMESPACE::FbxLayerElementArrayTemplate<FBXSDK_NAMESPACE::FbxVector2>* mUVArray1;
	FBXSDK_NAMESPACE::FbxLayerElementArrayTemplate<FBXSDK_NAMESPACE::FbxVector4>* mNormalArray;
	FBXSDK_NAMESPACE::FbxLayerElementArrayTemplate<FBXSDK_NAMESPACE::FbxVector4>* mTangentArray;
	FBXSDK_NAMESPACE::FbxLayerElementArrayTemplate<FBXSDK_NAMESPACE::FbxVector4>* mBinormalArray;
	FBXSDK_NAMESPACE::FbxLayerElementArrayTemplate<FBXSDK_NAMESPACE::FbxColor>* mVertexColorArray;

	std::vector<VERTEX_LAYER_TYPE> mVertexElements;

	static const int mscVeretexCacheSize;
	static const float mscPositionPrecision;
	static const double mscEpsilonDegree;
	static const double mscEpsilon;
	static const unsigned int msc_N_phi;
	static const double msc_MIN_N_phi;
	static std::vector<unsigned int> ms_N_theta_series;
	static double msPhiDecodingFactor;
};

