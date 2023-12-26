#include "./MeshVertex.h"
#include "glTFBin.h"

MeshVertex::MeshVertex()
{
}


MeshVertex::~MeshVertex()
{
}

std::string MeshVertex::compressVertices(
    vector<Accessor_data>& vBinAccessorData,
    stSkinInfo& skinInfo,
    vector<stSubMeshInfo>& vSubMeshInfo,
	FBXSDK_NAMESPACE::FbxManager* sdkManager,
	unsigned int optimizationFlag,
	FBXSDK_NAMESPACE::FbxMesh* mesh,
	const int materialCount,
	FBXSDK_NAMESPACE::FbxDouble3& bboxMin,
	FBXSDK_NAMESPACE::FbxDouble3& bboxMax,
	std::string& outputFile,
	unsigned int& triangleCountOfMesh, 
	unsigned int& polygonCountOfMesh, 
	unsigned int& vertexCountOfMesh,
	bool& isSkinned)
{
	triangleCountOfMesh = 0;
	polygonCountOfMesh = 0;
	vertexCountOfMesh = 0;

	mUVArray0 = NULL;
	mUVArray1 = NULL;
	mNormalArray = NULL;
	mTangentArray = NULL;
	mBinormalArray = NULL;
	mVertexColorArray = NULL;

	mVertexElements.clear();

	//Tangents
    mesh->GenerateTangentsData(0, false);

	//
	int polygonCount = mesh->GetPolygonCount();
	polygonCountOfMesh = polygonCount;

	std::vector<int> triangleStartPerPolygon;
	triangleStartPerPolygon.reserve(polygonCount);

	std::vector<int> triangleCountPerPolygon;
	triangleCountPerPolygon.reserve(polygonCount);

	std::vector<int> triangleIndices;
	triangleIndices.reserve(3 * polygonCount);

	std::vector<unsigned int> topologicalIndices;
	topologicalIndices.reserve(polygonCount);
	getTopologicalIndices(mesh, topologicalIndices);

	int totalTriangleCount = naiveTriangulate(mesh, triangleStartPerPolygon, triangleCountPerPolygon, triangleIndices);
	triangleCountOfMesh = totalTriangleCount;

	std::vector<std::vector<int>*> vertexLayers;

	//Skinning
    std::vector<BONE_>* effectiveBones = new std::vector<BONE_>();
	std::vector<BW_>* bwValues = new std::vector<BW_>();
	std::vector<BI_>* biValues = new std::vector<BI_>();
	getSkinning(mesh, effectiveBones, bwValues, biValues);
	isSkinned = effectiveBones->size() != 0;

	//Position layer
	int positionCount = mesh->GetControlPointsCount();
	vertexCountOfMesh = positionCount;

	std::vector<int>* positionLayer = new std::vector<int>();
	positionLayer->reserve(positionCount);
	for (int i = 0; i < positionCount; ++i)
	{
		positionLayer->push_back(i);
	}
	vertexLayers.push_back(positionLayer);
	mVertexElements.push_back(VL_POSITION);

	int vertexElementCount = 0;
	//Initial indices
	std::vector<int> nextIndices(totalTriangleCount * 3);
	std::vector<int> indices(totalTriangleCount * 3);
	vertexElementCount = getPositionIndices(mesh, indices, triangleCountPerPolygon, triangleIndices);

	////Positions for tangent calculation
	//FBXSDK_NAMESPACE::FbxDouble3 dummyMinPos(FLT_MAX, FLT_MAX, FLT_MAX);
	//FBXSDK_NAMESPACE::FbxDouble3 dummyMaxPos(FLT_MIN, FLT_MIN, FLT_MIN);
	//std::vector<float> positionsForTangent;
	//getPositions(mesh, &indices, isSkinned, positionsForTangent, dummyMinPos, dummyMaxPos);

	//Append Normal
	vertexElementCount = getNormalIndices(mesh, nextIndices, 0, triangleCountPerPolygon, triangleIndices);
	////Normals for tangent calculation
	//std::vector<float> normalsForTangent;
	//getNormals(mesh, &nextIndices, isSkinned, normalsForTangent);

	if (vertexElementCount > 0)
	{
		appendNewLayer_V2(vertexLayers, nextIndices, indices, VL_NORMAL0);
	}

	//Append UV0
	vertexElementCount = getUVIndices(mesh, nextIndices, 0, triangleCountPerPolygon, triangleIndices);
	////UVs for tangent calculation	
	//FBXSDK_NAMESPACE::FbxDouble3 dummyMinUV(FLT_MAX, FLT_MAX, FLT_MAX);
	//FBXSDK_NAMESPACE::FbxDouble3 dummyMaxUV(FLT_MIN, FLT_MIN, FLT_MIN);
	//std::vector<float> uvsForTangent;
	//getUVs(mesh, &nextIndices, uvsForTangent, dummyMinUV, dummyMaxUV);

	if (vertexElementCount > 0)
	{
		appendNewLayer_V2(vertexLayers, nextIndices, indices, VL_UV0);
	}

	//std::vector<unsigned int> smoothingGroup;
	//getSmoothing(sdkManager, mesh, false, true, triangleCountPerPolygon, smoothingGroup);

	//Compute Tangents
	//std::vector<float> outTangents;
	//computeTangents_V2(positionsForTangent, uvsForTangent, normalsForTangent, smoothingGroup, outTangents);

	//Append Tangent
	vertexElementCount = getTangentIndices(mesh, nextIndices, 0, triangleCountPerPolygon, triangleIndices);
	if (vertexElementCount > 0)
	{
		appendNewLayer_V2(vertexLayers, nextIndices, indices, VL_TANGENT0);
	}

	////Append Binormal
	//vertexElementCount = getBinormalIndices(mesh, nextIndices, 0, triangleCountPerPolygon, triangleIndices);
	//if (vertexElementCount > 0)
	//{
	//	appendNewLayer_V2(vertexLayers, nextIndices, indices, VL_BINORMAL0);
	//}

	////WE DO NOT GET TANGENT FROM MESH BUT CALCULTE
	std::vector<int>* positionIndices = MeshVertex::getVertexLayer(mVertexElements, vertexLayers, VL_POSITION);
	std::vector<int>* normalIndices = MeshVertex::getVertexLayer(mVertexElements, vertexLayers, VL_NORMAL0);
	std::vector<int>* uvIndices = MeshVertex::getVertexLayer(mVertexElements, vertexLayers, VL_UV0);
	std::vector<int>* tangentIndices = MeshVertex::getVertexLayer(mVertexElements, vertexLayers, VL_TANGENT0);

	//mTangentArray_V1 = NULL;
	//if (positionIndices != NULL && normalIndices != NULL && uvIndices != NULL)
	//{
	//	std::vector<float> positions;
	//	FBXSDK_NAMESPACE::FbxDouble3 bboxMin0(FLT_MAX, FLT_MAX, FLT_MAX);
	//	FBXSDK_NAMESPACE::FbxDouble3 bboxMax0(FLT_MIN, FLT_MIN, FLT_MIN);
	//	getPositions(mesh, positionIndices, isSkinned, positions, bboxMin0, bboxMax0);

	//	std::vector<float> normals;
	//	getNormals(mesh, normalIndices, isSkinned, normals);

	//	std::vector<float> uvs;
	//	FBXSDK_NAMESPACE::FbxDouble3 uvBboxMin(FLT_MAX, FLT_MAX, FLT_MAX);
	//	FBXSDK_NAMESPACE::FbxDouble3 uvBboxMax(FLT_MIN, FLT_MIN, FLT_MIN);
	//	getUVs(mesh, uvIndices, uvs, uvBboxMin, uvBboxMax);
	//	
	//	mTangentArray_V1 = new std::vector<float>();

	//	computeTangents(positions, uvs, normals, indices, *mTangentArray_V1);

	//	std::vector<int>* tangentIndices = new std::vector<int>(positionIndices->size());
	//	for (size_t i = 0; i < positionIndices->size(); ++i)
	//	{
	//		(*tangentIndices)[i] = (int)i;
	//	}
	//	vertexLayers.push_back(tangentIndices);
	//	mVertexElements.push_back(VL_TANGENT0);
	//}

	//Append Vertex Color
	vertexElementCount = getVertexColorIndices(mesh, nextIndices, 0, triangleCountPerPolygon, triangleIndices);
	if (vertexElementCount > 0)
	{
		appendNewLayer_V2(vertexLayers, nextIndices, indices, VL_VERTEXCOLOR0);
	}	

	//SubMesh
	vector<vector<int>> subMeshes;
	createSubmeshByMaterials(mesh, materialCount, 0, subMeshes, triangleStartPerPolygon, triangleCountPerPolygon);	
    if (subMeshes.size() > 0) {
        //dzlog_info("mesh has diffent submesh %ld and total material count %d",subMeshes.size(),materialCount);
    }

	size_t totalVertexCount = vertexLayers[0]->size();

	//Start optimization
	//Position outputs
	size_t posBufferSize = totalVertexCount*sizeof(float)*3 + sizeof(unsigned int)*6; //later for xyz component length
	uint8_t* outPosEncodingBuffer = new uint8_t[posBufferSize];
	unsigned int outPosEncodingSize = 0;
	unsigned int outPosEncodingFlag = 0;

	//Index outputs
	uint8_t* outIndexEncodingBuffer = new uint8_t[indices.size()*sizeof(unsigned int)];
	unsigned int outIndexEncodingSize = 0;
	unsigned int outIndexEncodingFlag = 0;

	//Topological Index outputs
	uint8_t* outTopologicalIndexEncodingBuffer = new uint8_t[topologicalIndices.size()*sizeof(int)];
	unsigned int outTopologicalIndexEncodingSize = 0;
	unsigned int outTopologicalIndexEncodingFlag = 0;

	//UV outputs
	FBXSDK_NAMESPACE::FbxDouble3 uvBboxMin(FLT_MAX, FLT_MAX, FLT_MAX);
	FBXSDK_NAMESPACE::FbxDouble3 uvBboxMax(FLT_MIN, FLT_MIN, FLT_MIN);
	uint8_t* outUVEncodingBuffer = NULL;
	unsigned int outUVEncodingSize = 0;
	unsigned int outUVEncodingFlag = 0;
	if (uvIndices != NULL)
	{
		outUVEncodingBuffer = new uint8_t[totalVertexCount*sizeof(float) * 2];
	}

	//Normal outputs
	uint8_t* outNormalEncodingBuffer = NULL;
	unsigned int outNormalEncodingSize = 0;
	unsigned int outNormalEncodingFlag = 0;
	if (normalIndices != NULL)
	{
		outNormalEncodingBuffer = new uint8_t[totalVertexCount*sizeof(float) * 3];
	}

	uint8_t* outTangentEncodingBuffer = NULL;
	unsigned int outTangentEncodingSize = 0;
	unsigned int outTangentEncodingFlag = 0;
	if (tangentIndices != NULL)
	{
		outTangentEncodingBuffer = new uint8_t[totalVertexCount*sizeof(float) * 4];
	}

	//VertexColor outputs
	uint8_t* outVertexColorEncodingBuffer = NULL;
	unsigned int outVertexColorEncodingSize = 0;
	unsigned int outVertexColorEncodingFlag = 0;
	std::vector<int>* vertexColorIndices = MeshVertex::getVertexLayer(mVertexElements, vertexLayers, VL_VERTEXCOLOR0);
	if (vertexColorIndices != NULL)
	{
		outVertexColorEncodingBuffer = new uint8_t[(totalVertexCount + 4)*sizeof(unsigned int)];
	}

	//BW & BI outputs
	uint8_t* outBWEncodingBuffer = NULL;
	unsigned int outBWEncodingSize = 0;
	unsigned int outBWEncodingFlag = 0;
	uint8_t* outBIEncodingBuffer = NULL;
	unsigned int outBIEncodingSize = 0;
	unsigned int outBIEncodingFlag = 0;
	if (bwValues->size() > 0 && biValues->size() > 0)
	{
		outBWEncodingBuffer = new uint8_t[totalVertexCount*4*sizeof(float)];
		outBIEncodingBuffer = new uint8_t[totalVertexCount*4*sizeof(float)];
	}

	uint8_t* outSubMeshEncodingBuffer = NULL;
	unsigned int outSubMeshEncodingSize = 0;
	unsigned int outSubMeshEncodingFlag = 0;
	unsigned int subMeshCount = (unsigned int)subMeshes.size();
	if (subMeshCount > 0)
	{
		outSubMeshEncodingSize = sizeof(unsigned int)*2*subMeshCount;
		outSubMeshEncodingBuffer = new uint8_t[outSubMeshEncodingSize];
		unsigned int* output = (unsigned int*)outSubMeshEncodingBuffer;
		unsigned int offset = 0;
		for (unsigned int subMeshIndex = 0; subMeshIndex < subMeshCount; ++subMeshIndex)
		{
			unsigned int triangleCount = (unsigned int)subMeshes[subMeshIndex].size();
			output[2*subMeshIndex + 0] = offset;			//indexOffset
			output[2*subMeshIndex + 1] = triangleCount;	//triangleCount
			offset += triangleCount * 3;
            stSubMeshInfo smInfo;
            smInfo.iStartTriangleIndiceIndex = offset;
            smInfo.iTriangleIndiceCount = triangleCount * 3;
            smInfo.iMaterialIndex = subMeshIndex;
            vSubMeshInfo.push_back(smInfo);
		}
	}

	
	meshOptimization(
		mesh,
		optimizationFlag,
		mVertexElements, 
		vertexLayers, 
		effectiveBones, 
		bwValues, 
		biValues, 
		indices,
		topologicalIndices,
		subMeshes,
		bboxMin,
		bboxMax,
		\
		outIndexEncodingBuffer,
		outIndexEncodingSize,
		outIndexEncodingFlag,
		\
		outTopologicalIndexEncodingBuffer,
		outTopologicalIndexEncodingSize,
		outTopologicalIndexEncodingFlag,
		\
		outPosEncodingBuffer,
		outPosEncodingSize,
		outPosEncodingFlag,
		\
		outNormalEncodingBuffer,
		outNormalEncodingSize,
		outNormalEncodingFlag,
		\
		outTangentEncodingBuffer,
		outTangentEncodingSize,
		outTangentEncodingFlag,
		\
		outVertexColorEncodingBuffer,
		outVertexColorEncodingSize,
		outVertexColorEncodingFlag,
		\
		uvBboxMin,
		uvBboxMax,
		outUVEncodingBuffer,
		outUVEncodingSize,
		outUVEncodingFlag,
		\
		outBWEncodingBuffer,
		outBWEncodingSize,
		outBWEncodingFlag,
		\
		outBIEncodingBuffer,
		outBIEncodingSize,
		outBIEncodingFlag);

	//Blend shape	
	std::vector<MORPH_DATA_V2>* morphTargetsData = new std::vector<MORPH_DATA_V2>();
	getMorph(
		mesh, 
		positionIndices,
		normalIndices,
		uvIndices,
		indices,
		isSkinned,
		morphTargetsData);

	//Write file
	//std::string objectName = mesh->GetName();
	FBXSDK_NAMESPACE::FbxNode* parentNode = mesh->GetNode();
	std::string objectName = parentNode->GetName();

	if (objectName.length() == 0)
	{
		objectName = "unknown";
	}
	objectName = standardizeFileName(objectName);

	glTFBin* writer2 = new glTFBin();
	writer2->openExportFile(outputFile);

	FbxNode* node = mesh->GetNode();
	FbxUInt64 parentNodeID = node->GetUniqueID();
	FbxUInt64 nodeID = mesh->GetUniqueID();

	//writer2->writeObjectBin(
 //       vBinAccessorData,
	//	parentNodeID,
	//	nodeID,
	//	MESH_TYPE,
	//	objectName,
	//	(unsigned int)totalVertexCount,
	//	mVertexElements,
	//	effectiveBones,
	//	\
	//	subMeshCount,
	//	outSubMeshEncodingBuffer,
	//	outSubMeshEncodingSize,
	//	outSubMeshEncodingFlag,
	//	\
	//	(unsigned int)(indices.size() / 3),
	//	outIndexEncodingBuffer,
	//	outIndexEncodingSize,
	//	outIndexEncodingFlag,
	//	\
	//	(unsigned int)(topologicalIndices.size() / 2),
	//	outTopologicalIndexEncodingBuffer,
	//	outTopologicalIndexEncodingSize,
	//	outTopologicalIndexEncodingFlag,
	//	\
	//	bboxMin,
	//	bboxMax,
	//	outPosEncodingBuffer,
	//	outPosEncodingSize,
	//	outPosEncodingFlag,
	//	\
	//	outNormalEncodingBuffer,
	//	outNormalEncodingSize,
	//	outNormalEncodingFlag,
	//	\
	//	outTangentEncodingBuffer,
	//	outTangentEncodingSize,
	//	outTangentEncodingFlag,
	//	\
	//	outVertexColorEncodingBuffer,
	//	outVertexColorEncodingSize,
	//	outVertexColorEncodingFlag,
	//	\
	//	uvBboxMin,
	//	uvBboxMax,
	//	outUVEncodingBuffer,
	//	outUVEncodingSize,
	//	outUVEncodingFlag,
	//	\
	//	outBWEncodingBuffer,
	//	outBWEncodingSize,
	//	outBWEncodingFlag,
	//	\
	//	outBIEncodingBuffer,
	//	outBIEncodingSize,
	//	outBIEncodingFlag);

    int morphTargetCount = (unsigned int)morphTargetsData->size();
    if (morphTargetCount > 0)
    {
        optimizationAndWriteMorphTarget(
            outPosEncodingBuffer,
            outNormalEncodingBuffer,
            vBinAccessorData,
            writer2,
            nodeID,
            (unsigned int)totalVertexCount,
            optimizationFlag,
            morphTargetsData);
    }

    if (isSkinned) {
        skinInfo.name = "skin001";
        for (auto it = effectiveBones->cbegin(); it != effectiveBones->cend(); ++it) {
            skinInfo.vBoneNodeID.push_back(it->id);
            skinInfo.vBindMat.push_back(it->modelWorldToBoneLocal);
        }
    }

	writer2->closeExportFile();
	delete writer2;

	//Clear
	delete morphTargetsData;
	delete biValues;
	delete bwValues;
    delete effectiveBones;

	for (unsigned int i = 0; i < vertexLayers.size(); ++i)
	{
		std::vector<int>* layer = vertexLayers[i];
		delete layer;
	}

	return "";
}

std::vector<int>* MeshVertex::getVertexLayer(
	std::vector<VERTEX_LAYER_TYPE>& vertexElement, 
	std::vector<std::vector<int>*>& vertexLayers, 
	VERTEX_LAYER_TYPE element)
{
	if (vertexElement.size() != vertexLayers.size())
	{
		return NULL;
	}

	std::vector<int>* p = NULL;
	for (size_t s = 0; s < vertexElement.size(); ++s)
	{
		if (element == vertexElement[s])
		{
			p = vertexLayers[s];
			break;
		}
	}

	return p;
}

void MeshVertex::optimizationAndWriteMorphTarget(
    uint8_t * meshPositionBuffer,
    uint8_t * meshNormalBuffer,
    vector<Accessor_data>& vBinAccessorData,
	glTFBin* writer,
	FbxUInt64 parentMeshUniqueID,
	unsigned int vertexCount,
	unsigned int optimizationFlag, 
	std::vector<MORPH_DATA_V2>* morphTargetsData)
{
	int morphTargetCount = (unsigned int)morphTargetsData->size();
	for (int i = 0; i < morphTargetCount; ++i)
	{
		MORPH_DATA_V2& data = (*morphTargetsData)[i];

		std::vector<VERTEX_LAYER_TYPE> vertexElements;
		vertexElements.push_back(VL_POSITION);

		//Encode positions for morph displace
        size_t iPosCount = data.positions.size();
        //dzlog_info("iPosCount %d vertexCount %d",(int)iPosCount,vertexCount);
        unsigned int outPosEncodingSize = 0;
        outPosEncodingSize = iPosCount * sizeof(float);

        float *meshOriginPositionBuffer = (float *)reinterpret_cast<float *>(meshPositionBuffer);
		size_t posBufferSize = data.positions.size()*sizeof(float);
		uint8_t* outPosEncodingBuffer = new uint8_t[posBufferSize];

        displaceMemcpy((float *)reinterpret_cast<float *>(outPosEncodingBuffer),data.positions,meshOriginPositionBuffer,iPosCount);

        //showFloatBufferString(outPosEncodingBuffer,iPosCount);

		//Encode normals
		unsigned int outNormalEncodingSize = 0;
		uint8_t* outNormalEncodingBuffer = NULL;

		if (data.normals.size() > 0 && data.normals.size() == data.positions.size())
		{
			vertexElements.push_back(VL_NORMAL0);

            //TODO test this code
            size_t iNormalCount = data.normals.size();
            //dzlog_info("iNormalCount %d vertexCount %d",(int)iNormalCount,vertexCount);
            outNormalEncodingSize = (unsigned int)(vertexCount * sizeof(float) * 3);
			outNormalEncodingBuffer = new uint8_t[data.normals.size() * sizeof(float)];
            float *meshOriginNormalBuffer = (float *)reinterpret_cast<float *>(meshNormalBuffer);
            displaceMemcpy((float *)reinterpret_cast<float *>(outNormalEncodingBuffer),data.normals,meshOriginNormalBuffer,iNormalCount);
		}

		FBXSDK_NAMESPACE::FbxDouble3 uvBboxMin(FLT_MAX, FLT_MAX, FLT_MAX);
		FBXSDK_NAMESPACE::FbxDouble3 uvBboxMax(FLT_MIN, FLT_MIN, FLT_MIN);

		writer->writeObjectBin(
			vBinAccessorData,
			parentMeshUniqueID,
			data.uniqueID,
			MORPHTARGET_TYPE,
			data.morphTargetName,
			vertexCount,
			vertexElements,
			NULL,			//Bones
			\
			0, NULL, 0, 0,	//Sub mesh
			\
			0, NULL, 0, 0,	//Index
			\
			0, NULL, 0, 0,	//Topology
			\
			data.bboxMin,
			data.bboxMax,
			outPosEncodingBuffer,
			outPosEncodingSize,
			0,
			\
			outNormalEncodingBuffer,
			outNormalEncodingSize,
			0,
			\
			NULL, 0, 0,		//Tangent
			\
			NULL, 0, 0,		//VertexColor
			\
			uvBboxMin, uvBboxMax, NULL, 0, 0, //UV
			\
			NULL, 0, 0, //BW & BI
			\
			NULL, 0, 0);
	}
}

void MeshVertex::meshOptimization(
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
	\
	uint8_t* outBIEncodingBuffer,
	unsigned int& outBIEncodingSize,
	unsigned int& outBIEncodingFlag)
{
	bool isSkinned = bones->size() != 0;
	unsigned int vertexCount = (unsigned int)vertexLayers[0]->size();

	//Reorder indices by material
	std::vector<unsigned int> reorderedIndicesByMaterial;
	std::vector<unsigned int> triangleClustersByMaterial;
	rearrangeTrianglesByMaterial(subMeshes, indices, reorderedIndicesByMaterial, triangleClustersByMaterial);	
	
	//Get original positions
	std::vector<float> outputOriginalPositions;

	getPositions(mesh, vertexLayers[0], isSkinned, outputOriginalPositions, bboxMin, bboxMax);
	
	//Reorder indices and vertex buffer
	std::vector<unsigned int> outVertexRemapping;
	std::vector<float> outputReorderedPositions;	
	std::vector<unsigned int> outIndices;
	ReorderTriangles(
		vertexCount,
		outputOriginalPositions,
		reorderedIndicesByMaterial,
		triangleClustersByMaterial,
		outputReorderedPositions,
		outIndices,
		outVertexRemapping);

	//Reorder other vertex index, like normal, vertex color etc.
	ShuffleVertexIndexOrder(vertexLayers, outVertexRemapping);

	//Reorder topological indices
	ShuffleTopologicalIndexOrder(topologicalIndices, outVertexRemapping);

	//Encoding positions
	if (optimizationFlag & (1 << VL_POSITION))
	{
		outPosEncodingSize = positionEncoding(outputReorderedPositions, outPosEncodingFlag, bboxMin, bboxMax, outPosEncodingBuffer);
		outPosEncodingSize = roundUp(outPosEncodingSize, 4);
	}
	else
	{
		outPosEncodingSize = (unsigned int)(outputReorderedPositions.size()*sizeof(float));
		memcpy(outPosEncodingBuffer, &outputReorderedPositions[0], outPosEncodingSize);
		outPosEncodingFlag = 0;
	}

	//Encoding Indices
	if (optimizationFlag & (1 << VL_INDEX))
	{
		MeshVertex::deltaAndZigzagEncoding(outIndices);
		outIndexEncodingSize = MeshVertex::varintEncoding(outIndices, outIndexEncodingBuffer);
		outIndexEncodingSize = roundUp(outIndexEncodingSize, 4);
		outIndexEncodingFlag = 1;
	}
	else
	{
		outIndexEncodingSize = (unsigned int)(outIndices.size()*sizeof(int));
		memcpy(outIndexEncodingBuffer, &outIndices[0], outIndexEncodingSize);
		outIndexEncodingFlag = 0;
	}

	//Encoding topological indices
	if (optimizationFlag & (1 << VL_TOPOLOGICALINDEX))
	{
		//TODO: further optimization should be taken, like rearrange vertex cache order, can make the indices advance progressively
		MeshVertex::deltaAndZigzagEncoding(topologicalIndices);
		outTopologicalIndexEncodingSize = MeshVertex::varintEncoding(topologicalIndices, outTopologicalIndexEncodingBuffer);
		outTopologicalIndexEncodingSize = roundUp(outTopologicalIndexEncodingSize, 4);
		outTopologicalIndexEncodingFlag = 1;
	}
	else
	{
		outTopologicalIndexEncodingSize = (unsigned int)(topologicalIndices.size()*sizeof(int));
		memcpy(outTopologicalIndexEncodingBuffer, &topologicalIndices[0], outTopologicalIndexEncodingSize);
		outTopologicalIndexEncodingFlag = 0;
	}

	//Encoding UVs
	std::vector<int>* uvIndices = MeshVertex::getVertexLayer(vertexElement, vertexLayers, VL_UV0);
	std::vector<float> orginalUVData;
	if (uvIndices != NULL)
	{		
		getUVs(mesh, uvIndices, orginalUVData, uvBboxMin, uvBboxMax);
		if (optimizationFlag & (1 << VL_UV0))
		{		
			outUVEncodingSize = encodeUVs(vertexCount, orginalUVData, uvBboxMin, uvBboxMax, outUVEncodingBuffer);
			outUVEncodingSize = roundUp(outUVEncodingSize, 4);
			outUVEncodingFlag = 1;
		}
		else
		{
			outUVEncodingSize = (unsigned int)(vertexCount*sizeof(float) * 2);
			memcpy(outUVEncodingBuffer, &orginalUVData[0], outUVEncodingSize);
			outUVEncodingFlag = 0;
		}
	}

	//Encoding Normals
	std::vector<int>* normalDataIndices = MeshVertex::getVertexLayer(vertexElement, vertexLayers, VL_NORMAL0);
	std::vector<float> orginalNormalData;
	if (normalDataIndices != NULL)
	{
		getNormals(mesh, normalDataIndices, isSkinned, orginalNormalData);
		if (optimizationFlag & (1 << VL_NORMAL0))
		{			
			outNormalEncodingSize = encodeNormals(vertexCount, orginalNormalData, outNormalEncodingBuffer);
			outNormalEncodingSize = roundUp(outNormalEncodingSize, 4);
			outNormalEncodingFlag = 1;
		}
		else
		{
			outNormalEncodingSize = (unsigned int)(vertexCount*sizeof(float) * 3);
			memcpy(outNormalEncodingBuffer, &orginalNormalData[0], outNormalEncodingSize);
			outNormalEncodingFlag = 0;
		}
	}

	////Encoding Tangents
	//std::vector<int>* tangentDataIndices = MeshVertex::getVertexLayer(vertexElement, vertexLayers, VL_TANGENT0);
	//if (tangentDataIndices != NULL && mTangentArray_V1 != NULL)
	//{
	//	std::vector<float> orginalTangentData;
	//	getTangents_V1(mesh, *mTangentArray_V1, tangentDataIndices, isSkinned, orginalTangentData);
	//	if (optimizationFlag & (1 << VL_TANGENT0))
	//	{
	//		outTangentEncodingSize = encodeTangents(vertexCount, orginalTangentData, outTangentEncodingBuffer);
	//		outTangentEncodingSize = roundUp(outTangentEncodingSize, 4);
	//		outTangentEncodingFlag = 1;
	//	}
	//	else
	//	{
	//		outTangentEncodingSize = (unsigned int)(vertexCount*sizeof(float) * 4);
	//		memcpy(outTangentEncodingBuffer, &orginalTangentData[0], outTangentEncodingSize);
	//		outTangentEncodingFlag = 0;
	//	}
	//}
	//Encoding Tangents
	std::vector<int>* tangentDataIndices = MeshVertex::getVertexLayer(vertexElement, vertexLayers, VL_TANGENT0);
	//std::vector<int>* binormalDataIndices = MeshVertex::getVertexLayer(vertexElement, vertexLayers, VL_BINORMAL0);
	if (tangentDataIndices != NULL)
	{
		std::vector<float> orginalTangentData;
		std::vector<float> orginalBinormalData;
		getTangents(mesh, tangentDataIndices, isSkinned, orginalTangentData);
		//getBinormals(mesh, binormalDataIndices, isSkinned, orginalBinormalData);

		//size_t vertexCount = orginalTangentData.size() / 4;
		//for (size_t k = 0; k < vertexCount; ++k)
		//{
		//	FBXSDK_NAMESPACE::FbxVector4 t(orginalTangentData[k * 4], orginalTangentData[k * 4 + 1], orginalTangentData[k * 4 + 2]);
		//	FBXSDK_NAMESPACE::FbxVector4 b(orginalBinormalData[k * 4], orginalBinormalData[k * 4 + 1], orginalBinormalData[k * 4 + 2]);
		//	FBXSDK_NAMESPACE::FbxVector4 n(orginalNormalData[k * 3], orginalNormalData[k * 3 + 1], orginalNormalData[k * 3 + 2]);

		//	double binormalProjection = n.CrossProduct(t).DotProduct(b);
		//	orginalTangentData[k * 4 + 3] = (float)((binormalProjection < 0.0) ? -1.0 : 1.0);
		//}

		if (optimizationFlag & (1 << VL_TANGENT0))
		{
			outTangentEncodingSize = encodeTangents(vertexCount, orginalTangentData, outTangentEncodingBuffer);
			outTangentEncodingSize = roundUp(outTangentEncodingSize, 4);
			outTangentEncodingFlag = 1;
		}
		else
		{
			outTangentEncodingSize = (unsigned int)(vertexCount*sizeof(float) * 4);
			memcpy(outTangentEncodingBuffer, &orginalTangentData[0], outTangentEncodingSize);
			outTangentEncodingFlag = 0;
		}
	}

	//Encoding Vertex Colors
	std::vector<int>* vertexColorDataIndices = MeshVertex::getVertexLayer(vertexElement, vertexLayers, VL_VERTEXCOLOR0);
	if (vertexColorDataIndices != NULL)
	{
		bool unifiedColor = true;
		bool unifiedAlpha = true;
		std::vector<FBXSDK_NAMESPACE::FbxUInt8> originalVertexColors;
		getVertexColors(mesh, vertexColorDataIndices, unifiedColor, unifiedAlpha, originalVertexColors);
		if (optimizationFlag & (1 << VL_VERTEXCOLOR0))
		{
			outVertexColorEncodingSize = encodeVertexColors(vertexCount, originalVertexColors, unifiedColor, unifiedAlpha, outVertexColorEncodingBuffer);
			outVertexColorEncodingSize = roundUp(outVertexColorEncodingSize, 4);
			outVertexColorEncodingFlag = 1;
		}
		else
		{
			outVertexColorEncodingSize = (unsigned int)(vertexCount*sizeof(FBXSDK_NAMESPACE::FbxUInt8) * 4);
			memcpy(outVertexColorEncodingBuffer, &originalVertexColors[0], outVertexColorEncodingSize);
			outVertexColorEncodingFlag = 0;
		}
	}

	//Encoding BW & BI
	if (bwValues->size() > 0 && biValues->size() > 0)
	{
		if (optimizationFlag & (1 << VL_BLENDWEIGHT))
		{
			encodeBlendWeightsAndIndices(vertexLayers[0], bwValues, biValues, outBWEncodingBuffer, outBWEncodingSize, outBIEncodingBuffer, outBIEncodingSize);
			outBWEncodingSize = roundUp(outBWEncodingSize, 4);
			outBIEncodingSize = roundUp(outBIEncodingSize, 4);
			outBWEncodingFlag = 1;
			outBIEncodingFlag = 1;
		}
		else
		{
			std::vector<float> orginalBWData;
			std::vector<uint16_t> orginalBIData;
			getBlendWeightsAndIndices(vertexLayers[0], bwValues, biValues, orginalBWData, orginalBIData);
			outBWEncodingSize = (unsigned int)(vertexCount*sizeof(float) * 4);
			outBIEncodingSize = (unsigned int)(vertexCount*sizeof(uint16_t) * 4);
			memcpy(outBWEncodingBuffer, &orginalBWData[0], outBWEncodingSize);
			memcpy(outBIEncodingBuffer, &orginalBIData[0], outBIEncodingSize);
			outBWEncodingFlag = 0;
			outBIEncodingFlag = 0;
		}
	}
}

unsigned int MeshVertex::encodeTangents(
	unsigned int vertexCount,
	const std::vector<float>& input,
	uint8_t* outBuffer)
{
	unsigned int encodedPhi, encodeTheta;
	for (unsigned int i = 0; i < vertexCount; ++i)
	{
		float x = input[i * 4 + 0];
		float y = input[i * 4 + 1];
		float z = input[i * 4 + 2];
		float w = input[i * 4 + 3];
		MeshVertex::mapNormal(x, y, z, (w - 1.0) / 2.0, encodedPhi, encodeTheta);

		outBuffer[i * 2 + 0] = encodedPhi;
		outBuffer[i * 2 + 1] = encodeTheta;
	}

	return vertexCount * 2;
}

unsigned int MeshVertex::encodeNormals(
	unsigned int vertexCount,
	const std::vector<float>& input,
	uint8_t* outBuffer)
{	
	unsigned int encodedPhi, encodeTheta;
	for (unsigned int i = 0; i < vertexCount; ++i)
	{
		float x = input[i * 3 + 0];
		float y = input[i * 3 + 1];
		float z = input[i * 3 + 2];
		MeshVertex::mapNormal(x, y, z, 0, encodedPhi, encodeTheta);

		outBuffer[i * 2 + 0] = encodedPhi;
		outBuffer[i * 2 + 1] = encodeTheta;
	}

	return vertexCount * 2;
	
	//Algorithm Moving Frame Version
	//FBXSDK_NAMESPACE::FbxVector4 lastFrameX(1, 0, 0);
	//FBXSDK_NAMESPACE::FbxVector4 lastFrameY(0, 1, 0);
	//FBXSDK_NAMESPACE::FbxVector4 lastFrameZ(0, 0, 1);
	//for (unsigned int elementIndex = 0; elementIndex < vertexCount; ++elementIndex)
	//{
	//	int k = (*elementIndices)[elementIndex];
	//	FBXSDK_NAMESPACE::FbxVector4 v = normalArray[k];

	//	v.Normalize();
	//	double mappedX = v.DotProduct(lastFrameX);
	//	double mappedY = v.DotProduct(lastFrameY);
	//	double mappedZ = v.DotProduct(lastFrameZ);

	//	//MeshVertex::mapNormal(v[0], v[1], v[2], encodedPhi, encodeTheta);
	//	FBXSDK_NAMESPACE::FbxVector4 encodedSubFrameVector = MeshVertex::mapNormal(mappedX, mappedY, mappedZ, encodedPhi, encodeTheta);

	//	double unmappedX = encodedSubFrameVector[0] * lastFrameX[0] + encodedSubFrameVector[1] * lastFrameY[0] + encodedSubFrameVector[2] * lastFrameZ[0];
	//	double unmappedY = encodedSubFrameVector[0] * lastFrameX[1] + encodedSubFrameVector[1] * lastFrameY[1] + encodedSubFrameVector[2] * lastFrameZ[1];
	//	double unmappedZ = encodedSubFrameVector[0] * lastFrameX[2] + encodedSubFrameVector[1] * lastFrameY[2] + encodedSubFrameVector[2] * lastFrameZ[2];
	//	FBXSDK_NAMESPACE::FbxVector4 newFrameZ(unmappedX, unmappedY, unmappedZ);
	//	if (lastFrameZ != newFrameZ)
	//	{
	//		double f = lastFrameZ.DotProduct(newFrameZ);
	//		FBXSDK_NAMESPACE::FbxVector4 newFrameX = newFrameZ*f - lastFrameZ;
	//		newFrameX[3] = 1.0;
	//		newFrameX.Normalize();
	//		lastFrameX = newFrameX;
	//		lastFrameZ = newFrameZ;
	//		lastFrameY = lastFrameZ.CrossProduct(lastFrameX);
	//	}
	//	//if (encodedPhi > 255 || encodeTheta > 255)
	//	//{
	//	//	FBXSDK_printf("%s", "Encode normal error!\n");
	//	//}

	//	output[elementIndex * 2 + 0] = encodedPhi;
	//	output[elementIndex * 2 + 1] = encodeTheta;
	//}
}

unsigned int MeshVertex::encodeUVs(
	unsigned int vertexCount,
	const std::vector<float>& input,
	const FBXSDK_NAMESPACE::FbxDouble3& bboxMin,
	const FBXSDK_NAMESPACE::FbxDouble3& bboxMax,
	uint8_t* outBuffer)
{
	//Max texture resolution is 4096 = 2 ** 12
	float precisionX = (float)(bboxMax.mData[0] - bboxMin.mData[0]) / (pow(2.0f, 12.0f) - 1.0f);
	float precisionY = (float)(bboxMax.mData[1] - bboxMin.mData[1]) / (pow(2.0f, 12.0f) - 1.0f);
	for (unsigned int i = 0; i < vertexCount; ++i)
	{
		float uvX = input[i*2 + 0];
		float uvY = input[i*2 + 1];

		unsigned int encodedX = (unsigned int)((uvX - bboxMin.mData[0]) / precisionX);
		unsigned int encodedY = (unsigned int)((uvY - bboxMin.mData[1]) / precisionY);

		unsigned int encoded = encodedX | (encodedY << 12);

		outBuffer[i * 3 + 0] = *(reinterpret_cast<uint8_t*>(&encoded));
		outBuffer[i * 3 + 1] = *(reinterpret_cast<uint8_t*>(&encoded) + 1);
		outBuffer[i * 3 + 2] = *(reinterpret_cast<uint8_t*>(&encoded) + 2);
	}

	return vertexCount*3;
}

unsigned int MeshVertex::encodeVertexColors(
	unsigned int vertexCount,
	const std::vector<FBXSDK_NAMESPACE::FbxUInt8>& input,
	bool unifiedColor,
	bool unifiedAlpha,
	uint8_t* output)
{
	unsigned int encodedSize = 0;
	if (unifiedColor)
	{
		output[0] = 1;
		output[1] = input[0];
		output[2] = input[1];
		output[3] = input[2];
		encodedSize += 4;
	}
	else
	{
		output[0] = 0; output[1] = 0;
		encodedSize += 2;
		for (unsigned int elementIndex = 0; elementIndex < vertexCount; ++elementIndex)
		{
			FBXSDK_NAMESPACE::FbxUInt8 red = input[elementIndex * 4 + 0];
			FBXSDK_NAMESPACE::FbxUInt8 green = input[elementIndex * 4 + 1];
			FBXSDK_NAMESPACE::FbxUInt8 blue = input[elementIndex * 4 + 2];
			unsigned short e = RGB888_RGB565(red, green, blue);

			*((unsigned short*)(&(output[encodedSize + elementIndex * 2]))) = e;
		}
		encodedSize += vertexCount * 2;
	}

	if (unifiedAlpha)
	{
		output[encodedSize + 0] = 1;
		output[encodedSize + 1] = input[3];
		encodedSize += 2;
	}
	else
	{
		output[encodedSize + 0] = 0;
		encodedSize += 1;
		for (unsigned int elementIndex = 0; elementIndex < vertexCount; ++elementIndex)
		{
			output[encodedSize + elementIndex * 2] = input[elementIndex * 4 + 3];
		}
		encodedSize += vertexCount;
	}

	return encodedSize;
}

void MeshVertex::encodeBlendWeightsAndIndices(
	std::vector<int>* dataIndices,
	std::vector<BW_>* bwValues,
	std::vector<BI_>* biValues,
	uint8_t* outBWEncodingBuffer,
	unsigned int& outBWEncodingSize,
	uint8_t* outBIEncodingBuffer,
	unsigned int& outBIEncodingSize)
{
	unsigned int vertexCount = (unsigned int)dataIndices->size();
	std::vector<unsigned int> blendWeights;
	blendWeights.resize(vertexCount * 4);
	for (unsigned int i = 0; i < vertexCount; ++i)
	{
		int k = (*dataIndices)[i];
		BW_& bw = (*bwValues)[k];
		blendWeights[i * 4 + 0] = (unsigned int)floor(bw.data[0] * 65535.0);
		blendWeights[i * 4 + 1] = (unsigned int)floor(bw.data[1] * 65535.0);
		blendWeights[i * 4 + 2] = (unsigned int)floor(bw.data[2] * 65535.0);
		blendWeights[i * 4 + 3] = (unsigned int)floor(bw.data[3] * 65535.0);
	}
	outBWEncodingSize = MeshVertex::varintEncoding(blendWeights, outBWEncodingBuffer);

	//<
	std::vector<unsigned int> blendIndices;
	blendIndices.resize(vertexCount * 4);
	for (unsigned int i = 0; i < vertexCount; ++i)
	{
		int k = (*dataIndices)[i];
		BI_& bi = (*biValues)[k];

		blendIndices[i * 4 + 0] = (unsigned int)bi.data[0];
		blendIndices[i * 4 + 1] = (unsigned int)bi.data[1];
		blendIndices[i * 4 + 2] = (unsigned int)bi.data[2];
		blendIndices[i * 4 + 3] = (unsigned int)bi.data[3];
	}

	outBIEncodingSize = MeshVertex::varintEncoding(blendIndices, outBIEncodingBuffer);
}

void MeshVertex::rearrangeTrianglesByMaterial(const std::vector<std::vector<int> >& subMeshes, const std::vector<int>& indices, \
							std::vector<unsigned int>& reorderedIndices, std::vector<unsigned int>& triangleClusters)
{
	std::vector<int> polygonIDs;
	unsigned int subMeshCount = (unsigned int)subMeshes.size();
	for (unsigned int i = 0; i < subMeshCount; ++i)
	{
		polygonIDs.insert(polygonIDs.end(), subMeshes[i].begin(), subMeshes[i].end());
		triangleClusters.insert(triangleClusters.end(), subMeshes[i].size(), i);
	}

	reorderedIndices.resize(indices.size());
	int triangleCount = (int)polygonIDs.size();
	if (triangleCount > 0)
	{
		//unsigned int indexAttributeCount = 3;
		//unsigned int indexBufferSize = sizeof(unsigned int) * indexAttributeCount * triangleCount;

		for (int triangleIndex = 0; triangleIndex < triangleCount; ++triangleIndex)
		{
			int a = indices[polygonIDs[triangleIndex] * 3];
			int b = indices[polygonIDs[triangleIndex] * 3 + 1];
			int c = indices[polygonIDs[triangleIndex] * 3 + 2];

			reorderedIndices[triangleIndex * 3 + 0] = (unsigned int)a;
			reorderedIndices[triangleIndex * 3 + 1] = (unsigned int)b;
			reorderedIndices[triangleIndex * 3 + 2] = (unsigned int)c;
		}
	}
}

bool MeshVertex::ShuffleVertexIndexOrder(std::vector<std::vector<int>*>& vertexLayers, std::vector<unsigned int>& vertexRemapping)
{
	unsigned int layerCount = (unsigned int)vertexLayers.size();
	std::vector<int> temp;
	temp.resize(vertexRemapping.size());
	for (unsigned int i = 0; i < layerCount; ++i)
	{
		std::vector<int>& elementIndices = *(vertexLayers[i]);
		if (elementIndices.size() != vertexRemapping.size() || vertexRemapping.size() == 0)
		{
			FBXSDK_printf("%s", "Shuffle vertex order failed!\n");
			return false;
		}

		for (unsigned int j = 0; j < (unsigned int)elementIndices.size(); ++j)
		{
			unsigned int newSlotIndex = vertexRemapping[j];
			int vertexIndex = elementIndices[j];
			temp[newSlotIndex] = vertexIndex;
		}

		memcpy(&elementIndices[0], &temp[0], elementIndices.size()*sizeof(int));
	}

	return true;
}

bool MeshVertex::ShuffleTopologicalIndexOrder(std::vector<unsigned int>& topologicalIndices, std::vector<unsigned int>& vertexRemapping)
{
	std::vector<std::vector<unsigned int> > database;
	database.resize(vertexRemapping.size());

	for (unsigned int i = 0; i < (unsigned int)topologicalIndices.size(); ++i)
	{
		unsigned int pos = topologicalIndices[i];
		database[pos].push_back(i);
	}

	for (unsigned int j = 0; j < (unsigned int)vertexRemapping.size(); ++j)
	{
		unsigned int newIndex = vertexRemapping[j];
		std::vector<unsigned int>& slots = database[j];
		for (unsigned int k = 0; k < (unsigned int)slots.size(); ++k)
		{
			unsigned int s = slots[k];
			topologicalIndices[s] = newIndex;
		}
	}

	return true;
}

void MeshVertex::getPositions(
		FbxMesh* mesh,
		std::vector<int>* dataIndices,
		bool isSkinned, 
		std::vector<float>& output,
		FBXSDK_NAMESPACE::FbxDouble3& bboxMin,
		FBXSDK_NAMESPACE::FbxDouble3& bboxMax)
{
	FBXSDK_NAMESPACE::FbxVector4* positions = mesh->GetControlPoints();
	int positionCount = mesh->GetControlPointsCount();

	FBXSDK_NAMESPACE::FbxVector4* temp = new FBXSDK_NAMESPACE::FbxVector4[positionCount];

	FBXSDK_NAMESPACE::FbxAMatrix geometricMatrix;
	FbxNode* node = mesh->GetNode();
	if (node != NULL && !isSkinned)
	{
		geometricMatrix = MeshVertex::getGeometryMatrix(node);
	}

	bool identicalMatrix = geometricMatrix.IsIdentity();
	if (!identicalMatrix)
	{
		for (int i = 0; i < positionCount; ++i)
		{
			FBXSDK_NAMESPACE::FbxVector4 p = positions[i];
			temp[i] = MeshVertex::transformPositionWithGeometricTransform(geometricMatrix, p[0], p[1], p[2]);
		}
	}

	unsigned int vertexCount = (unsigned int)dataIndices->size();
	output.resize(3 * vertexCount);

	for (unsigned int elementIndex = 0; elementIndex < vertexCount; ++elementIndex)
	{
		int k = (*dataIndices)[elementIndex];
		FBXSDK_NAMESPACE::FbxVector4 v = identicalMatrix ? positions[k] : temp[k];

        //dzlog_debug("%8.3f %8.3f %8.3f",v[0],v[1],v[2]);

		if (v[0] < bboxMin.mData[0])
			bboxMin.mData[0] = v[0];

		if (v[1] < bboxMin.mData[1])
			bboxMin.mData[1] = v[1];

		if (v[2] < bboxMin.mData[2])
			bboxMin.mData[2] = v[2];

		if (v[0] > bboxMax.mData[0])
			bboxMax.mData[0] = v[0];

		if (v[1] > bboxMax.mData[1])
			bboxMax.mData[1] = v[1];

		if (v[2] > bboxMax.mData[2])
			bboxMax.mData[2] = v[2];

		output[elementIndex*3 + 0] = (float)v[0];
		output[elementIndex*3 + 1] = (float)v[1];
		output[elementIndex*3 + 2] = (float)v[2];
	}

	delete[] temp;
}

void MeshVertex::getUVs(
	FBXSDK_NAMESPACE::FbxMesh* mesh,
	std::vector<int>* dataIndices,
	std::vector<float>& output,
	FBXSDK_NAMESPACE::FbxDouble3& bboxMin,
	FBXSDK_NAMESPACE::FbxDouble3& bboxMax)
{
	unsigned int vertexCount = (unsigned int)dataIndices->size();
	FBXSDK_NAMESPACE::FbxLayer* layer = mesh->GetLayer(0, FBXSDK_NAMESPACE::FbxLayerElement::eUV);
	FBXSDK_NAMESPACE::FbxLayerElementUV* meshUV = layer->GetUVs();
	FBXSDK_NAMESPACE::FbxLayerElementArrayTemplate<FBXSDK_NAMESPACE::FbxVector2>& uvArray = meshUV->GetDirectArray();

	output.resize(2*vertexCount);
	for (unsigned int elementIndex = 0; elementIndex < vertexCount; ++elementIndex)
	{
		int k = (*dataIndices)[elementIndex];
		FBXSDK_NAMESPACE::FbxVector2 v = uvArray[k];

		output[elementIndex*2 + 0] = (float)v[0];
        //flip y axis for texture UV
		//output[elementIndex*2 + 1] = 1 - (float)v[1];
		output[elementIndex*2 + 1] = (float)v[1];

		if (v[0] < bboxMin.mData[0])
			bboxMin.mData[0] = v[0];

		if (v[1] < bboxMin.mData[1])
			bboxMin.mData[1] = v[1];

		if (v[0] > bboxMax.mData[0])
			bboxMax.mData[0] = v[0];

		if (v[1] > bboxMax.mData[1])
			bboxMax.mData[1] = v[1];
	}
}

void MeshVertex::getNormals(
	FbxMesh* mesh,
	std::vector<int>* dataIndices,
	bool isSkinned,
	std::vector<float>& output)
{
	FBXSDK_NAMESPACE::FbxLayer* layer = mesh->GetLayer(0, FBXSDK_NAMESPACE::FbxLayerElement::eNormal);
	FBXSDK_NAMESPACE::FbxLayerElementNormal* meshNormal = layer->GetNormals();
	FBXSDK_NAMESPACE::FbxLayerElementArrayTemplate<FBXSDK_NAMESPACE::FbxVector4>& normalArray = meshNormal->GetDirectArray();
	//int normalCount = normalArray.GetCount();

	FBXSDK_NAMESPACE::FbxAMatrix geometricMatrix;
	FbxNode* node = mesh->GetNode();
	if (node != NULL && !isSkinned)
	{
		geometricMatrix = MeshVertex::getGeometryMatrix(node);
	}

	bool identicalMatrix = geometricMatrix.IsIdentity();
	//if (!identicalMatrix)
	//{
	//	for (int i = 0; i < normalCount; ++i)
	//	{
	//		FBXSDK_NAMESPACE::FbxVector4 n = normalArray[i];
	//		normalArray[i] = MeshVertex::transformNormalWithGeometricTransform(geometricMatrix, n[0], n[1], n[2]);
	//	}
	//}

	unsigned int vertexCount = (unsigned int)dataIndices->size();
	output.resize(vertexCount * 3);
	for (unsigned int elementIndex = 0; elementIndex < vertexCount; ++elementIndex)
	{
		int i = (*dataIndices)[elementIndex];
		FBXSDK_NAMESPACE::FbxVector4 v = normalArray[i];
		if (!identicalMatrix)
		{
			v = MeshVertex::transformNormalWithGeometricTransform(geometricMatrix, v[0], v[1], v[2]);
		}

		v.Normalize();

		output[elementIndex * 3 + 0] = (float)v[0];
		output[elementIndex * 3 + 1] = (float)v[1];
		output[elementIndex * 3 + 2] = (float)v[2];
	}
}

void MeshVertex::getTangents(
	FBXSDK_NAMESPACE::FbxMesh* mesh,
	std::vector<int>* dataIndices,
	bool isSkinned,
	std::vector<float>& output)
{
	FBXSDK_NAMESPACE::FbxLayer* layer = mesh->GetLayer(0, FBXSDK_NAMESPACE::FbxLayerElement::eTangent);
	FBXSDK_NAMESPACE::FbxLayerElementTangent* meshTangent = layer->GetTangents();
	FBXSDK_NAMESPACE::FbxLayerElementArrayTemplate<FBXSDK_NAMESPACE::FbxVector4>& tangentArray = meshTangent->GetDirectArray();

	FBXSDK_NAMESPACE::FbxAMatrix geometricMatrix;
	FbxNode* node = mesh->GetNode();
	if (node != NULL && !isSkinned)
	{
		geometricMatrix = MeshVertex::getGeometryMatrix(node);
	}

	bool identicalMatrix = geometricMatrix.IsIdentity();
	unsigned int vertexCount = (unsigned int)dataIndices->size();
	output.resize(vertexCount * 4);
	for (unsigned int elementIndex = 0; elementIndex < vertexCount; ++elementIndex)
	{
		int i = (*dataIndices)[elementIndex];
		FBXSDK_NAMESPACE::FbxVector4 v = tangentArray[i];
		if (!identicalMatrix)
		{
			v = MeshVertex::transformNormalWithGeometricTransform(geometricMatrix, v[0], v[1], v[2]);
		}

		v.Normalize();

		output[elementIndex * 4 + 0] = (float)v[0];
		output[elementIndex * 4 + 1] = (float)v[1];
		output[elementIndex * 4 + 2] = (float)v[2];
		output[elementIndex * 4 + 3] = (float)v[3];
	}
}

void MeshVertex::getBinormals(
	FbxMesh* mesh,
	std::vector<int>* dataIndices,
	bool isSkinned,
	std::vector<float>& output)
{
	FBXSDK_NAMESPACE::FbxLayer* layer = mesh->GetLayer(0, FBXSDK_NAMESPACE::FbxLayerElement::eBiNormal);
	FBXSDK_NAMESPACE::FbxLayerElementBinormal* meshBinormal = layer->GetBinormals();
	FBXSDK_NAMESPACE::FbxLayerElementArrayTemplate<FBXSDK_NAMESPACE::FbxVector4>& binormalArray = meshBinormal->GetDirectArray();

	FBXSDK_NAMESPACE::FbxAMatrix geometricMatrix;
	FbxNode* node = mesh->GetNode();
	if (node != NULL && !isSkinned)
	{
		geometricMatrix = MeshVertex::getGeometryMatrix(node);
	}

	bool identicalMatrix = geometricMatrix.IsIdentity();
	unsigned int vertexCount = (unsigned int)dataIndices->size();
	output.resize(vertexCount * 4);
	for (unsigned int elementIndex = 0; elementIndex < vertexCount; ++elementIndex)
	{
		int i = (*dataIndices)[elementIndex];
		FBXSDK_NAMESPACE::FbxVector4 v = binormalArray[i];
		if (!identicalMatrix)
		{
			v = MeshVertex::transformNormalWithGeometricTransform(geometricMatrix, v[0], v[1], v[2]);
		}

		v.Normalize();

		output[elementIndex * 4 + 0] = (float)v[0];
		output[elementIndex * 4 + 1] = (float)v[1];
		output[elementIndex * 4 + 2] = (float)v[2];
		output[elementIndex * 4 + 3] = 0.0f;
	}
}

//void MeshVertex::getTangents_V1(
//	FbxMesh* mesh,
//	const std::vector<float>& tangentArray,
//	std::vector<int>* dataIndices,
//	bool isSkinned,
//	std::vector<float>& output)
//{
//	FBXSDK_NAMESPACE::FbxAMatrix geometricMatrix;
//	FbxNode* node = mesh->GetNode();
//	if (node != NULL && !isSkinned)
//	{
//		geometricMatrix = MeshVertex::getGeometryMatrix(node);
//	}
//
//	bool identicalMatrix = geometricMatrix.IsIdentity();
//
//	unsigned int vertexCount = (unsigned int)dataIndices->size();
//	output.resize(vertexCount * 4);
//	for (unsigned int elementIndex = 0; elementIndex < vertexCount; ++elementIndex)
//	{
//		int i = (*dataIndices)[elementIndex];
//		FBXSDK_NAMESPACE::FbxVector4 v(tangentArray[i * 4], tangentArray[i * 4 + 1], tangentArray[i * 4 + 2], tangentArray[i * 4 + 3]);
//		if (!identicalMatrix)
//		{
//			v = MeshVertex::transformNormalWithGeometricTransform(geometricMatrix, v[0], v[1], v[2]);
//		}
//
//		v.Normalize();
//
//		output[elementIndex * 4 + 0] = (float)v[0];
//		output[elementIndex * 4 + 1] = (float)v[1];
//		output[elementIndex * 4 + 2] = (float)v[2];
//		output[elementIndex * 4 + 3] = (float)v[3];
//	}
//}

void MeshVertex::getVertexColors(
	FbxMesh* mesh,
	std::vector<int>* dataIndices,
	bool& unifiedColor,
	bool& unifiedAlpha,
	std::vector<uint8_t>& output)
{
	FBXSDK_NAMESPACE::FbxLayer* layer = mesh->GetLayer(0, FBXSDK_NAMESPACE::FbxLayerElement::eVertexColor);
	FBXSDK_NAMESPACE::FbxLayerElementVertexColor* meshVertexColor = layer->GetVertexColors();
	FBXSDK_NAMESPACE::FbxLayerElementArrayTemplate<FBXSDK_NAMESPACE::FbxColor>& vcArray = meshVertexColor->GetDirectArray();

	unsigned int vertexCount = (unsigned int)dataIndices->size();
	unifiedColor = unifiedAlpha = true;
	FBXSDK_NAMESPACE::FbxUInt8 lastRed = 0;
	FBXSDK_NAMESPACE::FbxUInt8 lastGreen = 0;
	FBXSDK_NAMESPACE::FbxUInt8 lastBlue = 0;
	FBXSDK_NAMESPACE::FbxUInt8 lastAlpha = 0;

	output.resize(vertexCount*4);
	for (unsigned int elementIndex = 0; elementIndex < vertexCount; ++elementIndex)
	{
		int k = (*dataIndices)[elementIndex];
		FBXSDK_NAMESPACE::FbxColor v = vcArray[k];

		FBXSDK_NAMESPACE::FbxUInt8 red = (FBXSDK_NAMESPACE::FbxUInt8(v.mRed*255.0));
		FBXSDK_NAMESPACE::FbxUInt8 green = (FBXSDK_NAMESPACE::FbxUInt8(v.mGreen*255.0));
		FBXSDK_NAMESPACE::FbxUInt8 blue = (FBXSDK_NAMESPACE::FbxUInt8(v.mBlue*255.0));
		FBXSDK_NAMESPACE::FbxUInt8 alpha = (FBXSDK_NAMESPACE::FbxUInt8(v.mAlpha*255.0));

		output[elementIndex * 4 + 0] = red;
		output[elementIndex * 4 + 1] = green;
		output[elementIndex * 4 + 2] = blue;
		output[elementIndex * 4 + 3] = alpha;
		if (elementIndex > 0)
		{
			if (alpha != lastAlpha)
			{
				unifiedAlpha = false;
			}

			if (red != lastRed || green != lastGreen || blue != lastBlue)
			{
				unifiedColor = false;
			}
		}

		lastRed = red;
		lastGreen = green;
		lastBlue = blue;
		lastAlpha = alpha;
	}
}

//set computing true to compute smoothing from normals by default 
//set convertToSmoothingGroup true to convert hard/soft edge info to smoothing group info by default
bool MeshVertex::getSmoothing(
	FBXSDK_NAMESPACE::FbxManager* sdkManager,
	FBXSDK_NAMESPACE::FbxMesh* mesh,
	bool computing,
	bool convertToSmoothingGroup,
	std::vector<int>& triangleCountPerPolygon,
	std::vector<unsigned int>& outSmoothingGroupByPolygon)
{
	if(computing)
	{
		FBXSDK_NAMESPACE::FbxGeometryConverter geometryConverter(sdkManager);
		geometryConverter.ComputeEdgeSmoothingFromNormals(mesh);
		if (convertToSmoothingGroup)
		{
			geometryConverter.ComputePolygonSmoothingFromEdgeSmoothing(mesh);
		}
	}

	FBXSDK_NAMESPACE::FbxGeometryElementSmoothing* smoothingElement = mesh->GetElementSmoothing();
	if (smoothingElement)
	{
		if (smoothingElement->GetMappingMode() == FbxGeometryElement::eByEdge)
		{
			FBXSDK_printf("%s", "Do not supported eByEdge mode. This mode is usually come from Maya.\n");
			return false;

			//for (int edgeIndex = 0; edgeIndex < mesh->GetMeshEdgeCount(); edgeIndex++)
			//{
			//	int smoothingIndex = 0;

			//	if (smoothingElement->GetReferenceMode() == FbxGeometryElement::eDirect)
			//		smoothingIndex = edgeIndex;

			//	if (smoothingElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
			//		smoothingIndex = smoothingElement->GetIndexArray().GetAt(edgeIndex);

			//	int smoothingGroupID = smoothingElement->GetDirectArray().GetAt(smoothingIndex);

			//	outSmoothingGroupByPolygon.push_back(smoothingGroupID);
			//}
		}
		else if (smoothingElement->GetMappingMode() == FbxGeometryElement::eByPolygon)
		{
			int polygonCount = (int)triangleCountPerPolygon.size();
			//int polygonCount0 = mesh->GetPolygonCount();
			for (int polygonIndex = 0; polygonIndex < polygonCount; polygonIndex++)
			{
				int smoothingIndex = 0;
				if (smoothingElement->GetReferenceMode() == FbxGeometryElement::eDirect)
					smoothingIndex = polygonIndex;

				if (smoothingElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
					smoothingIndex = smoothingElement->GetIndexArray().GetAt(polygonIndex);

				//smoothing group id are actually 32 bit flags.
				int smoothingGroupID = smoothingElement->GetDirectArray().GetAt(smoothingIndex);				
				unsigned int uValue = *(reinterpret_cast<unsigned int*>(&smoothingGroupID));
				int triangleCountOfPolygon = triangleCountPerPolygon[polygonIndex];
				for (int i = 0; i < triangleCountOfPolygon; ++i)
				{
					outSmoothingGroupByPolygon.push_back(uValue);
				}
			}
		}
	}

	return true;
}

void MeshVertex::getBlendWeightsAndIndices(
	std::vector<int>* dataIndices,
	std::vector<BW_>* bwValues,
	std::vector<BI_>* biValues,
	std::vector<float>& outputBW,
	std::vector<uint16_t>& outputBI)
{
	unsigned int vertexCount = (unsigned int)dataIndices->size();
	outputBW.resize(vertexCount * 4);
	for (unsigned int i = 0; i < vertexCount; ++i)
	{
		int k = (*dataIndices)[i];
		BW_& bw = (*bwValues)[k];
		outputBW[i * 4 + 0] = (float)bw.data[0];
		outputBW[i * 4 + 1] = (float)bw.data[1];
		outputBW[i * 4 + 2] = (float)bw.data[2];
		outputBW[i * 4 + 3] = (float)bw.data[3];
	}

	//<
	std::vector<unsigned int> blendIndices;
	outputBI.resize(vertexCount * 4);
	for (unsigned int i = 0; i < vertexCount; ++i)
	{
		int k = (*dataIndices)[i];
		BI_& bi = (*biValues)[k];
		outputBI[i * 4 + 0] = (uint16_t)bi.data[0];
        outputBI[i * 4 + 1] = (uint16_t)bi.data[1];
		outputBI[i * 4 + 2] = (uint16_t)bi.data[2];
		outputBI[i * 4 + 3] = (uint16_t)bi.data[3];
	}
}

bool MeshVertex::ReorderTriangles(
	unsigned int vertexCount,
	std::vector<float>& vertexInputPrefetchOptimization,
	std::vector<unsigned int>& indices, 
	std::vector<unsigned int>& triangleClusters,
	std::vector<float>& outputPositions, 
	std::vector<unsigned int>& outputIndices, 
	std::vector<unsigned int>& vertexRemapping)
{
	// initialize Tootle
	TootleResult result = TootleInit();
	
	unsigned int vertexStride = 3*sizeof(float);
	unsigned int meshTriangleCount = (unsigned int)indices.size()/3;
	if ((unsigned int)indices.size() % 3 != 0)
	{
		FBXSDK_printf("%s", "The vertex count of a mesh must be a multiplier of 3!\n");
		return false;
	}

	//Triangle Order Optimization
	//Triangle Order Optimization for Graphics Hardware Computation Culling
	//http://gameangst.com/?p=9

	//Triangle Order Optimization for Efficient Graphics Hardware Computation Culling
	//https://pdfs.semanticscholar.org/330d/b96cc13b3ca3bb259a8eacc40e7b0ec55b32.pdf

	//Fast Triangle Reordering for Vertex Locality and Reduced Overdraw
	//http://gfx.cs.princeton.edu/pubs/Sander_2007_%3ETR/tipsy.pdf

	//Linear-Speed Vertex Cache Optimization
	//https://tomforsyth1000.github.io/papers/fast_vert_cache_opt.html

	std::vector<unsigned int> indicesVtxCacheOptimization;
	indicesVtxCacheOptimization.resize(indices.size());
	result = TootleVCacheClusters(
		&indices[0], 
		meshTriangleCount, 
		vertexCount, 
		mscVeretexCacheSize, 
		&triangleClusters[0],
		&indicesVtxCacheOptimization[0], 
		NULL/*&reorderMap[0]*/, 
		TOOTLE_VCACHE_TIPSY);

	if (result != TOOTLE_OK)
	{
		FBXSDK_printf("%s", "Vertex cache optimization failed\n");
		return false;
	}	

	//Prefetch Optimization
	vertexRemapping.resize(vertexCount);
	outputIndices.resize(indices.size());
	outputPositions.resize(3*vertexCount);

	//mapping is the new position of the original one.
	result = TootleOptimizeVertexMemory(&vertexInputPrefetchOptimization[0], &indicesVtxCacheOptimization[0], vertexCount, meshTriangleCount, vertexStride, \
		&outputPositions[0], &outputIndices[0], &vertexRemapping[0]);

	if (result != TOOTLE_OK)
	{
		FBXSDK_printf("%s", "Vertex prefetch optimization failed\n");
		return false;
	}

#ifdef USE_NVIDIA_STRIPPER
	stripifyTriangles_Nvidia(indicesVtxPrefetchOptimization);
#else
	#ifdef USE_TRI_STRIPPER
		stripifyTriangles_TriStripper(indicesVtxPrefetchOptimization);
	#endif
#endif

	TootleCleanup();

	return true;
}

inline int getPrecision(float extent, int elementCount, float epsilon)
{
	//double m = pow((float)elementCount, 0.33333f);
	//double a = extent / epsilon;
	if (elementCount < pow(2, 12))
	{
		return 1;
	}
	else if (elementCount < pow(2, 21))
	{
		return 2;
	}
	else
	{
		return 4;
	}
}

void MeshVertex::highWatermarkEncoding(std::vector<unsigned int>& input)
{
	size_t count = input.size();
	long long highWatermark = 0;
	for (size_t s = 0; s < count; ++s)
	{
		long long o = (long long)input[s];
		int v = (int)(o - highWatermark);
		int* p0 = reinterpret_cast<int*>(&input[s]);
		*p0 = v;

		highWatermark = (o > highWatermark) ? (o + 1) : highWatermark;
	}
}

void MeshVertex::highWatermarkDecoding(std::vector<unsigned int>& input)
{
	size_t count = input.size();
	long long highWatermark = 0;
	for (size_t s = 0; s < count; ++s)
	{
		int* p0 = reinterpret_cast<int*>(&input[s]);
		long long o = (long long)(*p0);
		unsigned int v = (unsigned int)(highWatermark - o);
		input[s] = v;

		highWatermark = (v >= highWatermark) ? (v + 1) : highWatermark;
	}
}

void MeshVertex::deltaAndZigzagEncoding(std::vector<unsigned int>& input)
{
	size_t count = input.size();
	long long lastValue = 0;
	for (size_t s = 0; s < count; ++s)
	{
		long long o = (long long)input[s];
		int delta = (int)(o - lastValue);
		unsigned int zigzagValue = (delta << 1) ^ (delta >> 31); //zigzag
		input[s] = zigzagValue;

		lastValue = o;
	}
}

void MeshVertex::zigzagAndDeltaDecoding(std::vector<unsigned int>& input)
{
	size_t count = input.size();
	long long lastValue = 0;
	for (size_t s = 0; s < count; ++s)
	{
		unsigned int n = input[s];
		int i = ((n >> 1) ^ (0 - (n & 1)));
		long long o = (long long)(lastValue + i);
		
		input[s] = (unsigned int)o;

		lastValue = o;
	}
}

int MeshVertex::varintEncoding(const std::vector<unsigned int>& input, uint8_t* output)
{
	size_t outputSize = 0;
	
	for (size_t s = 0; s < input.size(); ++s)
	{
		unsigned int value = input[s];
		while (value > 127)
		{
			output[outputSize] = ((uint8_t)(value & 127)) | 128;
			value >>= 7;
			outputSize++;
		}
		output[outputSize++] = ((uint8_t)value) & 127;
	}

	return (int)outputSize;
}

bool MeshVertex::varintDecoding(uint8_t* input, int inputBufferSize, int elementCount, std::vector<unsigned int>& output)
{
	int s = 0;
	for (int index = 0; index != elementCount;)
	{
		unsigned int o = 0;
		unsigned int l = 0;
		do
		{
			o |= (127 & input[s]) << l;
			l += 7;
		} while (0 != (128 & input[s++]));

		output[index++] = o;
	}

	if (s != inputBufferSize)
	{
		return false;
	}
	else
	{
		return true;
	}
}

int MeshVertex::uintEncoding(
	std::vector<float>& element, 
	int count, 
	int bias, 
	float rangeLow, 
	float rangeHigh, 
	float epsilon, 
	unsigned int& outEncodingFlag, 
	uint8_t* output)
{
	size_t elementCount = element.size() / count;

	int precisionType = getPrecision(rangeHigh - rangeLow, (int)elementCount, epsilon);

	outEncodingFlag |= (precisionType << (3 * bias));

	int outputLength = 0;
	if (precisionType == 1)
	{
		float precision = (rangeHigh - rangeLow) / (pow(2.0f, precisionType*8.0f) - 1.0f);
		for (size_t s = 0; s < elementCount; ++s)
		{
			int v = (int)((element[3 * s + bias] - rangeLow) / precision);
			if (v > 255 || v < 0)
			{
				FBXSDK_printf("%s", "Varint encoding failed! Byte value is too big!\n");
			}
			output[s] = (uint8_t)v;
		}
		outputLength = (int)elementCount;
	}
	else if (precisionType == 2)
	{
		std::vector<unsigned int> lowPrecisionArray;
		lowPrecisionArray.resize(elementCount);

		float precision = (rangeHigh - rangeLow) / (pow(2.0f, precisionType*8.0f) - 1.0f);
		for (size_t s = 0; s < elementCount; ++s)
		{
			int v = (int)((element[3 * s + bias] - rangeLow) / precision);
			if (v > 65535 || v < 0)
			{
				FBXSDK_printf("%s", "Varint encoding failed! Short value is too big!\n");
			}
			lowPrecisionArray[s] = (unsigned int)v;
			//uint16_t* p0 = reinterpret_cast<uint16_t*>(&output[s*sizeof(uint16_t)]);
			//*p0 = (uint16_t)v;
		}		
		MeshVertex::deltaAndZigzagEncoding(lowPrecisionArray);
		outputLength = MeshVertex::varintEncoding(lowPrecisionArray, output);
		//outputLength = ((int)elementCount*sizeof(uint16_t));
	}
	else if(precisionType == 4)
	{
		for (size_t s = 0; s < elementCount; ++s)
		{
			float* p0 = reinterpret_cast<float*>(&output[s*sizeof(float)]);
			*p0 = element[3 * s + bias];
		}
		outputLength = ((int)elementCount*sizeof(float));
	}
	else
	{
		FBXSDK_printf("%s", "Varint encoding failed!\n");
	}

	return outputLength;
}

unsigned int MeshVertex::positionEncoding(std::vector<float>& positions, unsigned int& outPosEncodingFlag,
	FBXSDK_NAMESPACE::FbxDouble3& bboxMin, FBXSDK_NAMESPACE::FbxDouble3& bboxMax, uint8_t* output)
{
	unsigned int index = 12;

	unsigned int* xsize = reinterpret_cast<unsigned int*>(&output[0]);
	*xsize = (unsigned int)uintEncoding(positions, 3, 0, (float)bboxMin[0], (float)bboxMax[0], mscPositionPrecision, outPosEncodingFlag, &output[index]);
	*xsize = roundUp(*xsize, 4);
	index += *xsize;

	unsigned int* ysize = reinterpret_cast<unsigned int*>(&output[4]);
	*ysize = (unsigned int)uintEncoding(positions, 3, 1, (float)bboxMin[1], (float)bboxMax[1], mscPositionPrecision, outPosEncodingFlag, &output[index]);
	*ysize = roundUp(*ysize, 4);
	index += *ysize;

	unsigned int* zsize = reinterpret_cast<unsigned int*>(&output[8]);
	*zsize = (unsigned int)uintEncoding(positions, 3, 2, (float)bboxMin[2], (float)bboxMax[2], mscPositionPrecision, outPosEncodingFlag, &output[index]);
	*zsize = roundUp(*zsize, 4);
	index += *zsize;

	return index;
}

//void MeshVertex::getSmoothingGroup()
//{
	//https://forums.autodesk.com/t5/fbx-forum/how-to-create-a-smoothing-group/td-p/4245673
	//FBXSDK_NAMESPACE::FbxLayerElementSmoothing
//KFbxLayer* lLayer0 = pMesh->GetLayer(0);
//if (lLayer0 == NULL)
//{
//	pMesh->CreateLayer();
//	lLayer0 = pMesh->GetLayer(0);
//}
//
//// Set material mapping.
//KFbxLayerElementMaterial* lLayerElementMaterial = lLayer0->GetMaterials();
//
//if (!lLayerElementMaterial)
//{
//	lLayerElementMaterial = KFbxLayerElementMaterial::Create(pMesh, "Material Layer");
//}
//
//lLayerElementMaterial->SetMappingMode(KFbxLayerElement::eBY_POLYGON);
//lLayerElementMaterial->SetReferenceMode(KFbxLayerElement::eINDEX_TO_DIRECT);
//lLayerElementMaterial->SetMappingMode(KFbxLayerElement::eALL_SAME);
//lLayerElementMaterial->SetReferenceMode(KFbxLayerElement::eDIRECT);
//lLayer0->SetMaterials(lLayerElementMaterial);
	//FbxLayerElementSmoothing* lLayerElementSmoothing = KFbxLayerElementSmoothing::Create(pMesh, "Smoothing");
	//lLayerElementSmoothing->SetMappingMode(FbxLayerElement::eBY_POLYGON);
	//lLayerElementSmoothing->SetReferenceMode(FbxLayerElement::eDIRECT);    // must be direct
	//lLayer0->SetSmoothing(lLayerElementSmoothing);

//then set the group mask per triangle.In my code, a chart is a separate smoothing group.It contains the smoothing group mask.

//lLayerElementSmoothing->GetDirectArray().SetCount(polygonCount);
//int triIndex = 0;
//for (size_t fid = 0; fid < mesh.triList.size(); fid++)
//{
//	lpIndexedTriangle & tri = mesh.triList;
//	if (!tri.tags.active)
//		continue;
//	assert(tri.materialID < (int)mesh.materialList.size() && tri.materialID != -1);
//
//	lLayerElementMaterial->GetIndexArray().SetAt(triIndex, tri.materialID);
//
//	int chartID = tri.chartID;
//	lpChart & chart = mesh.chartList;
//
//	lLayerElementSmoothing->GetDirectArray().SetAt(triIndex, chart.smoothingGroupMask);  // 
//	triIndex++;
//
//}
//}

//void MeshVertex::mergeTriangleStrips(osg::Geometry::PrimitiveSetList& primitives)
//{
//	int nbtristrip = 0;
//	int nbtristripVertexes = 0;
//
//	for (unsigned int i = 0; i < primitives.size(); ++i)
//	{
//		osg::PrimitiveSet* ps = primitives[i].get();
//		osg::DrawElements* de = ps->getDrawElements();
//		if (de && de->getMode() == osg::PrimitiveSet::TRIANGLE_STRIP)
//		{
//			++nbtristrip;
//			nbtristripVertexes += de->getNumIndices();
//		}
//	}
//
//	if (nbtristrip > 0) {
//		osg::notify(osg::NOTICE) << "found " << nbtristrip << " tristrip, "
//			<< "total indices " << nbtristripVertexes
//			<< " should result to " << nbtristripVertexes + nbtristrip * 2
//			<< " after connection" << std::endl;
//
//		osg::DrawElementsUInt* ndw = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLE_STRIP);
//		for (unsigned int i = 0; i < primitives.size(); ++i)
//		{
//			osg::PrimitiveSet* ps = primitives[i].get();
//			if (ps && ps->getMode() == osg::PrimitiveSet::TRIANGLE_STRIP)
//			{
//				osg::DrawElements* de = ps->getDrawElements();
//				if (de)
//				{
//					// if connection needed insert degenerate triangles
//					if (ndw->getNumIndices() != 0 && ndw->back() != de->getElement(0))
//					{
//						// duplicate last vertex
//						ndw->addElement(ndw->back());
//						// insert first vertex of next strip
//						ndw->addElement(de->getElement(0));
//					}
//
//					if (ndw->getNumIndices() % 2 != 0) {
//						// add a dummy vertex to reverse the strip
//						ndw->addElement(de->getElement(0));
//					}
//
//					for (unsigned int j = 0; j < de->getNumIndices(); j++) {
//						ndw->addElement(de->getElement(j));
//					}
//				}
//			}
//		}
//
//		for (int i = primitives.size() - 1; i >= 0; --i)
//		{
//			osg::PrimitiveSet* ps = primitives[i].get();
//			// remove null primitive sets and all primitives that have been merged
//			// (i.e. all TRIANGLE_STRIP DrawElements)
//			if (!ps || (ps && ps->getMode() == osg::PrimitiveSet::TRIANGLE_STRIP))
//			{
//				primitives.erase(primitives.begin() + i);
//			}
//		}
//		primitives.insert(primitives.begin(), ndw);
//	}
//}

void MeshVertex::stripifyTriangles_Nvidia(const vector<unsigned int>& indices)
{
	std::vector<unsigned short> shortIndices(indices.begin(), indices.end());

	high_res_timer Timer;

	SetCacheSize(mscVeretexCacheSize);
	SetStitchStrips(false);
	SetMinStripSize(2);

	unsigned short groupCount = 0;
	PrimitiveGroup* primitiveGroup = NULL;
	GenerateStrips(&(shortIndices[0]), (unsigned int)shortIndices.size(), &primitiveGroup, &groupCount);

	//double processingTime = Timer.ElapsedTime<float>() * 1000.0;

	unsigned int stripCount = 0;
	unsigned int fanCount = 0;
	unsigned int listCount = 0;
	unsigned int strippedTriangleCount = 0;

	for (int i = 0; i < (int)groupCount; ++i) 
	{
		PrimType type = primitiveGroup[i].type;
		if (type == PT_STRIP)
		{
			++stripCount;
			strippedTriangleCount += (primitiveGroup[i].numIndices) - 2;
		}
		else if (type == PT_LIST)
		{
			listCount += (primitiveGroup[i].numIndices) / 3;
		}
		else if (type == PT_FAN)
		{
			++fanCount;
		}
	}
}

void MeshVertex::stripifyTriangles_TriStripper(const vector<unsigned int>& indices)
{
	using namespace triangle_stripper;

	high_res_timer Timer;

	primitive_vector PrimitivesVector;
	{ 
		tri_stripper TriStripper(indices);

		TriStripper.SetMinStripSize(2);
		TriStripper.SetCacheSize(mscVeretexCacheSize);
		TriStripper.SetBackwardSearch(false);

		TriStripper.Strip(&PrimitivesVector);
	}

	//double processingTime				= Timer.ElapsedTime<float>() * 1000.0;	
	unsigned int stripsCount			= 0;
	unsigned int listedTriangleCount	= 0;
	unsigned int strippedTriangleCount	= 0;

	for (size_t i = 0; i < PrimitivesVector.size(); ++i) 
	{
		if (PrimitivesVector[i].Type == TRIANGLE_STRIP) 
		{
			++stripsCount;
			strippedTriangleCount += ((unsigned int)(PrimitivesVector[i].Indices.size())) - 2;
		}
		else
		{
			listedTriangleCount += ((unsigned int)(PrimitivesVector[i].Indices.size())) / 3;
		}
	}	
}

int MeshVertex::naiveTriangulate(FbxMesh* mesh, \
	std::vector<int>& triangleStartPerPolygon, std::vector<int>& triangleCountPerPolygon, std::vector<int>& triangleIndices)
{
	int totalTriangleCount = 0;
	int polygonCount = mesh->GetPolygonCount();
	for (int polygonIndex = 0; polygonIndex < polygonCount; ++polygonIndex)
	{
		int polygonSize = mesh->GetPolygonSize(polygonIndex);
		int triangleCount = 0;
		if (polygonSize < 3)
		{
			FBXSDK_printf("%s", "Unsupported polygon with less than 3 positions!\n");
		}
		else
		{
			//Fan split, clockwise
			for (int vertexIndex = 2; vertexIndex < polygonSize; ++vertexIndex)
			{
				triangleIndices.push_back(0);
				triangleIndices.push_back(vertexIndex - 1);
				triangleIndices.push_back(vertexIndex);
				++triangleCount;
			}
		}

		triangleCountPerPolygon.push_back(triangleCount);
		triangleStartPerPolygon.push_back(totalTriangleCount);

		totalTriangleCount += triangleCount;		
	}

	return totalTriangleCount;
}

void MeshVertex::getTopologicalIndices(FbxMesh* mesh, std::vector<unsigned int>& topologyIndices)
{
	std::map<int, vector<int> > dictionary;

	int polygonCount = mesh->GetPolygonCount();
	for (int polygonIndex = 0; polygonIndex < polygonCount; ++polygonIndex)
	{
		int polygonSize = mesh->GetPolygonSize(polygonIndex);
		if (polygonSize < 3)
		{
			FBXSDK_printf("%s", "Unsupported polygon with less than 3 positions!\n");
		}
		else
		{
			for (int vertexIndex = 1; vertexIndex <= polygonSize; ++vertexIndex)
			{
				int controlPointIndex0 = mesh->GetPolygonVertex(polygonIndex, (vertexIndex - 1) % polygonSize);
				int controlPointIndex1 = mesh->GetPolygonVertex(polygonIndex, vertexIndex % polygonSize);

				bool foundInSet0 = false;
				std::map<int, vector<int> >::iterator iter0 = dictionary.find(controlPointIndex0);
				if (iter0 != dictionary.end())
				{
					for (unsigned int i = 0; i < iter0->second.size(); ++i)
					{
						if (iter0->second[i] == controlPointIndex1)
						{
							foundInSet0 = true;
							break;
						}
					}

					if (!foundInSet0)
					{
						iter0->second.push_back(controlPointIndex1);
					}
				}
				else
				{
					vector<int> items;
					items.push_back(controlPointIndex1);
					dictionary[controlPointIndex0] = items;
				}

				//<
				bool foundInSet1 = false;
				std::map<int, vector<int> >::iterator iter1 = dictionary.find(controlPointIndex1);
				if (iter1 != dictionary.end())
				{
					for (unsigned int i = 0; i < iter1->second.size(); ++i)
					{
						if (iter1->second[i] == controlPointIndex0)
						{
							foundInSet1 = true;
							break;
						}
					}

					if (!foundInSet1)
					{
						iter1->second.push_back(controlPointIndex0);
					}
				}
				else
				{
					vector<int> items;
					items.push_back(controlPointIndex0);
					dictionary[controlPointIndex1] = items;
				}

				if ((!foundInSet0 && foundInSet1) || (foundInSet0 && !foundInSet1))
				{
					//FBXSDK_printf("%s", "Find topological indices failed!\n");
				}

				if (!foundInSet0 && !foundInSet1)
				{
					topologyIndices.push_back((unsigned int)controlPointIndex0);
					topologyIndices.push_back((unsigned int)controlPointIndex1);
				}				
				//<
			}
		}
	}
}

int MeshVertex::getPositionIndices(FbxMesh* mesh, std::vector<int>& indices, std::vector<int>& triangleCountPerPolygon, std::vector<int>& triangles)
{
	int vertexID = 0;
	int polygonCount = (int)triangleCountPerPolygon.size();

	int triangleIndex = 0;
	for (int polygonIndex = 0; polygonIndex < polygonCount; ++polygonIndex)
	{
		int tc = triangleCountPerPolygon[polygonIndex];
		for (int i = 0; i < tc; ++i)
		{
			int j = (triangleIndex + i) * 3;
			for (int k = 0; k < 3; ++k)
			{
				int vertexOrder = triangles[j + k];
				int controlPointIndex = mesh->GetPolygonVertex(polygonIndex, vertexOrder);
				indices[vertexID] = controlPointIndex;
				vertexID += 1;
			}
		}
		triangleIndex += tc;
	}
	
	int controlPointCount = mesh->GetControlPointsCount();
	return controlPointCount;
}


int MeshVertex::getUVIndices(
	FBXSDK_NAMESPACE::FbxMesh* mesh, 
	std::vector<int>& indices, 
	int uvIndex, 
	std::vector<int>& triangleCountPerPolygon, 
	std::vector<int>& triangles)
{
	uvIndex = 0;

	FBXSDK_NAMESPACE::FbxLayer* layer = mesh->GetLayer(uvIndex, FBXSDK_NAMESPACE::FbxLayerElement::eUV);
	if (layer == NULL)
		return 0;

	FBXSDK_NAMESPACE::FbxLayerElementUV* meshUV = layer->GetUVs();
	if (meshUV == NULL)
		return 0;

	int vertexID = 0;
	int vertexIndex = 0;
	int polygonCount = (int)triangleCountPerPolygon.size();
	int triangleIndex = 0;
	for (int polygonIndex = 0; polygonIndex < polygonCount; ++polygonIndex)
	{
		int tc = triangleCountPerPolygon[polygonIndex];

		for (int i = 0; i < tc; ++i)
		{
			int j = (triangleIndex + i) * 3;
			for (int k = 0; k < 3; ++k)
			{
				int vertexOrder = triangles[j + k];
				int controlPointIndex = mesh->GetPolygonVertex(polygonIndex, vertexOrder);
				FBXSDK_NAMESPACE::FbxLayerElement::EMappingMode mappingMode = meshUV->GetMappingMode();
				if (mappingMode == FBXSDK_NAMESPACE::FbxLayerElement::eByControlPoint)
				{
					FBXSDK_NAMESPACE::FbxLayerElement::EReferenceMode refMode = meshUV->GetReferenceMode();
					if (refMode == FBXSDK_NAMESPACE::FbxLayerElement::eDirect)
					{
						indices[vertexIndex] = controlPointIndex;
					}
					else if (refMode == FBXSDK_NAMESPACE::FbxLayerElement::eIndexToDirect)
					{
						FBXSDK_NAMESPACE::FbxLayerElementArrayTemplate<int>& uvIndexArray = meshUV->GetIndexArray();
						int index = uvIndexArray.GetAt(controlPointIndex);
						indices[vertexIndex] = index;
					}
					else
					{
						FBXSDK_printf("%s", "Unsupported uv reference mode for polygon vertex!\n");
					}
				}
				else if (mappingMode == FBXSDK_NAMESPACE::FbxLayerElement::eByPolygonVertex)
				{
					FBXSDK_NAMESPACE::FbxLayerElement::EReferenceMode refMode = meshUV->GetReferenceMode();
					if (refMode == FBXSDK_NAMESPACE::FbxLayerElement::eDirect)
					{
						indices[vertexIndex] = vertexID + vertexOrder;
					}
					else if (refMode == FBXSDK_NAMESPACE::FbxLayerElement::eIndexToDirect)
					{
						FBXSDK_NAMESPACE::FbxLayerElementArrayTemplate<int>& uvIndexArray = meshUV->GetIndexArray();
						int index = uvIndexArray.GetAt(vertexID + vertexOrder);
						indices[vertexIndex] = index;
					}
					else
					{
						FBXSDK_printf("%s", "Unsupported uv reference mode for polygon vertex!\n");
					}
					//<
				}
				else
				{
					FBXSDK_printf("%s", "Unsupported uv mapping mode for polygon vertex!\n");
				}
				vertexIndex += 1;
			}			
		}
		int polygonSize = mesh->GetPolygonSize(polygonIndex);
		vertexID += polygonSize;

		triangleIndex += tc;
	}

	int uvCount = 0;
	if (uvIndex == 0)
	{
		mUVArray0 = &(meshUV->GetDirectArray());
		uvCount = mUVArray0->GetCount();
	}
	else if (uvIndex == 1)
	{
		mUVArray1 = &(meshUV->GetDirectArray());
		uvCount = mUVArray1->GetCount();
	}
	else
	{
		FBXSDK_printf("%s", "Unsupported uv layer!\n");
	}

	return uvCount;
}

int MeshVertex::getNormalIndices(
	FBXSDK_NAMESPACE::FbxMesh* mesh, 
	std::vector<int>& indices, 
	int normalIndex, 
	std::vector<int>& triangleCountPerPolygon, 
	std::vector<int>& triangles)
{
	normalIndex = 0;

	FBXSDK_NAMESPACE::FbxLayer* layer = mesh->GetLayer(normalIndex, FBXSDK_NAMESPACE::FbxLayerElement::eNormal);
	if (layer == NULL)
	{
		if (mesh->GenerateNormals(true, false))
		{
			layer = mesh->GetLayer(normalIndex, FBXSDK_NAMESPACE::FbxLayerElement::eNormal);
			FBXSDK_printf("%s", "Warning: This mesh do not have any normals. Normals are generated automatically.\n");
		}
		else
		{
			FBXSDK_printf("%s", "Warning: This mesh do not have any normals. Normals calculation failed.\n");
			return 0;
		}
	}

	FBXSDK_NAMESPACE::FbxLayerElementNormal* meshNormal = layer->GetNormals();
	if (meshNormal == NULL)
		return 0;	

	int vertexID = 0;
	int vertexIndex = 0;
	int polygonCount = (int)triangleCountPerPolygon.size();
	int triangleIndex = 0;
	
	bool wrongRefMode = false;
	bool wrongMappingMode = false;
	for (int polygonIndex = 0; polygonIndex < polygonCount; ++polygonIndex)
	{
		int tc = triangleCountPerPolygon[polygonIndex];

		for (int i = 0; i < tc; ++i)
		{
			int j = (triangleIndex + i) * 3;
			for (int k = 0; k < 3; ++k)
			{
				int vertexOrder = triangles[j + k];
				int controlPointIndex = mesh->GetPolygonVertex(polygonIndex, vertexOrder);
				FBXSDK_NAMESPACE::FbxLayerElement::EMappingMode mappingMode = meshNormal->GetMappingMode();
				if (mappingMode == FBXSDK_NAMESPACE::FbxLayerElement::eByControlPoint)
				{
					FBXSDK_NAMESPACE::FbxLayerElement::EReferenceMode refMode = meshNormal->GetReferenceMode();
					if (refMode == FBXSDK_NAMESPACE::FbxLayerElement::eDirect)
					{
						indices[vertexIndex] = controlPointIndex;
					}
					else if (refMode == FBXSDK_NAMESPACE::FbxLayerElement::eIndexToDirect)
					{
						FBXSDK_NAMESPACE::FbxLayerElementArrayTemplate<int>& normalIndexArray = meshNormal->GetIndexArray();
						int index = normalIndexArray.GetAt(controlPointIndex);
						indices[vertexIndex] = index;
					}
					else
					{
						wrongRefMode = true;
					}
				}
				else if (mappingMode == FBXSDK_NAMESPACE::FbxLayerElement::eByPolygonVertex)
				{
					FBXSDK_NAMESPACE::FbxLayerElement::EReferenceMode refMode = meshNormal->GetReferenceMode();
					if (refMode == FBXSDK_NAMESPACE::FbxLayerElement::eDirect)
					{
						indices[vertexIndex] = vertexID + vertexOrder;
					}
					else if (refMode == FBXSDK_NAMESPACE::FbxLayerElement::eIndexToDirect)
					{
						FBXSDK_NAMESPACE::FbxLayerElementArrayTemplate<int>& normalIndexArray = meshNormal->GetIndexArray();
						int index = normalIndexArray.GetAt(vertexID + vertexOrder);
						indices[vertexIndex] = index;
					}
					else
					{
						wrongRefMode = true;
					}
					//<
				}
				else
				{
					wrongMappingMode = true;
				}
				vertexIndex += 1;
			}
		}
		int polygonSize = mesh->GetPolygonSize(polygonIndex);
		vertexID += polygonSize;

		triangleIndex += tc;
	}

	if (wrongRefMode)
	{
		FBXSDK_printf("%s", "Unsupported normal reference mode for polygon vertex!\n");
	}

	if (wrongMappingMode)
	{
		FBXSDK_printf("%s", "Unsupported normal mapping mode for polygon vertex!\n");
	}

	mNormalArray = &(meshNormal->GetDirectArray());
	int normalCount = mNormalArray->GetCount();

	return normalCount;
}

int MeshVertex::getTangentIndices(
	FBXSDK_NAMESPACE::FbxMesh* mesh, 
	std::vector<int>& indices, 
	int tangentIndex, 
	std::vector<int>& triangleCountPerPolygon, 
	std::vector<int>& triangles)
{
	tangentIndex = 0;

	FBXSDK_NAMESPACE::FbxLayer* layer = mesh->GetLayer(tangentIndex, FBXSDK_NAMESPACE::FbxLayerElement::eTangent);
	if (layer == NULL)
		return 0;

	FBXSDK_NAMESPACE::FbxLayerElementTangent* meshTangent = layer->GetTangents();
	if (meshTangent == NULL)
		return 0;

	int vertexID = 0;
	int vertexIndex = 0;
	int polygonCount = (int)triangleCountPerPolygon.size();
	int triangleIndex = 0;

	bool wrongRefMode = false;
	bool wrongMappingMode = false;
	for (int polygonIndex = 0; polygonIndex < polygonCount; ++polygonIndex)
	{
		int tc = triangleCountPerPolygon[polygonIndex];

		for (int i = 0; i < tc; ++i)
		{
			int j = (triangleIndex + i) * 3;
			for (int k = 0; k < 3; ++k)
			{
				int vertexOrder = triangles[j + k];
				int controlPointIndex = mesh->GetPolygonVertex(polygonIndex, vertexOrder);
				FBXSDK_NAMESPACE::FbxLayerElement::EMappingMode mappingMode = meshTangent->GetMappingMode();
				if (mappingMode == FBXSDK_NAMESPACE::FbxLayerElement::eByControlPoint)
				{
					FBXSDK_NAMESPACE::FbxLayerElement::EReferenceMode refMode = meshTangent->GetReferenceMode();
					if (refMode == FBXSDK_NAMESPACE::FbxLayerElement::eDirect)
					{
						indices[vertexIndex] = controlPointIndex;
					}
					else if (refMode == FBXSDK_NAMESPACE::FbxLayerElement::eIndexToDirect)
					{
						FBXSDK_NAMESPACE::FbxLayerElementArrayTemplate<int>& tangentIndexArray = meshTangent->GetIndexArray();
						int index = tangentIndexArray.GetAt(controlPointIndex);
						indices[vertexIndex] = index;
					}
					else
					{
						wrongRefMode = true;
					}
				}
				else if (mappingMode == FBXSDK_NAMESPACE::FbxLayerElement::eByPolygonVertex)
				{
					FBXSDK_NAMESPACE::FbxLayerElement::EReferenceMode refMode = meshTangent->GetReferenceMode();
					if (refMode == FBXSDK_NAMESPACE::FbxLayerElement::eDirect)
					{
						indices[vertexIndex] = vertexID + vertexOrder;
					}
					else if (refMode == FBXSDK_NAMESPACE::FbxLayerElement::eIndexToDirect)
					{
						FBXSDK_NAMESPACE::FbxLayerElementArrayTemplate<int>& tangentIndexArray = meshTangent->GetIndexArray();
						int index = tangentIndexArray.GetAt(vertexID + vertexOrder);
						indices[vertexIndex] = index;
					}
					else
					{
						wrongRefMode = true;
					}
					//<
				}
				else
				{
					wrongMappingMode = true;
				}
				vertexIndex += 1;
			}
		}
		int polygonSize = mesh->GetPolygonSize(polygonIndex);
		vertexID += polygonSize;

		triangleIndex += tc;
	}

	if (wrongRefMode)
	{
		FBXSDK_printf("%s", "Unsupported tangent reference mode for polygon vertex!\n");
	}

	if (wrongMappingMode)
	{
		FBXSDK_printf("%s", "Unsupported tangent mapping mode for polygon vertex!\n");
	}

	mTangentArray = &(meshTangent->GetDirectArray());
	int tangentCount = mTangentArray->GetCount();

	return tangentCount;
}

int MeshVertex::getBinormalIndices(
	FBXSDK_NAMESPACE::FbxMesh* mesh,
	std::vector<int>& indices,
	int tangentIndex,
	std::vector<int>& triangleCountPerPolygon,
	std::vector<int>& triangles)
{
	tangentIndex = 0;

	FBXSDK_NAMESPACE::FbxLayer* layer = mesh->GetLayer(tangentIndex, FBXSDK_NAMESPACE::FbxLayerElement::eBiNormal);
	if (layer == NULL)
		return 0;

	FBXSDK_NAMESPACE::FbxLayerElementBinormal* meshBinormal = layer->GetBinormals();
	if (meshBinormal == NULL)
		return 0;

	int vertexID = 0;
	int vertexIndex = 0;
	int polygonCount = (int)triangleCountPerPolygon.size();
	int triangleIndex = 0;

	bool wrongRefMode = false;
	bool wrongMappingMode = false;
	for (int polygonIndex = 0; polygonIndex < polygonCount; ++polygonIndex)
	{
		int tc = triangleCountPerPolygon[polygonIndex];

		for (int i = 0; i < tc; ++i)
		{
			int j = (triangleIndex + i) * 3;
			for (int k = 0; k < 3; ++k)
			{
				int vertexOrder = triangles[j + k];
				int controlPointIndex = mesh->GetPolygonVertex(polygonIndex, vertexOrder);
				FBXSDK_NAMESPACE::FbxLayerElement::EMappingMode mappingMode = meshBinormal->GetMappingMode();
				if (mappingMode == FBXSDK_NAMESPACE::FbxLayerElement::eByControlPoint)
				{
					FBXSDK_NAMESPACE::FbxLayerElement::EReferenceMode refMode = meshBinormal->GetReferenceMode();
					if (refMode == FBXSDK_NAMESPACE::FbxLayerElement::eDirect)
					{
						indices[vertexIndex] = controlPointIndex;
					}
					else if (refMode == FBXSDK_NAMESPACE::FbxLayerElement::eIndexToDirect)
					{
						FBXSDK_NAMESPACE::FbxLayerElementArrayTemplate<int>& indexArray = meshBinormal->GetIndexArray();
						int index = indexArray.GetAt(controlPointIndex);
						indices[vertexIndex] = index;
					}
					else
					{
						wrongRefMode = true;
					}
				}
				else if (mappingMode == FBXSDK_NAMESPACE::FbxLayerElement::eByPolygonVertex)
				{
					FBXSDK_NAMESPACE::FbxLayerElement::EReferenceMode refMode = meshBinormal->GetReferenceMode();
					if (refMode == FBXSDK_NAMESPACE::FbxLayerElement::eDirect)
					{
						indices[vertexIndex] = vertexID + vertexOrder;
					}
					else if (refMode == FBXSDK_NAMESPACE::FbxLayerElement::eIndexToDirect)
					{
						FBXSDK_NAMESPACE::FbxLayerElementArrayTemplate<int>& indexArray = meshBinormal->GetIndexArray();
						int index = indexArray.GetAt(vertexID + vertexOrder);
						indices[vertexIndex] = index;
					}
					else
					{
						wrongRefMode = true;
					}
					//<
				}
				else
				{
					wrongMappingMode = true;
				}
				vertexIndex += 1;
			}
		}
		int polygonSize = mesh->GetPolygonSize(polygonIndex);
		vertexID += polygonSize;

		triangleIndex += tc;
	}

	if (wrongRefMode)
	{
		FBXSDK_printf("%s", "Unsupported binormal reference mode for polygon vertex!\n");
	}

	if (wrongMappingMode)
	{
		FBXSDK_printf("%s", "Unsupported binormal mapping mode for polygon vertex!\n");
	}

	mBinormalArray = &(meshBinormal->GetDirectArray());
	int binormalCount = mBinormalArray->GetCount();

	return binormalCount;
}

int MeshVertex::getVertexColorIndices(FBXSDK_NAMESPACE::FbxMesh* mesh, std::vector<int>& indices, int vertexColorIndex, std::vector<int>& triangleCountPerPolygon, std::vector<int>& triangles)
{
	FBXSDK_NAMESPACE::FbxLayer* layer = mesh->GetLayer(vertexColorIndex, FBXSDK_NAMESPACE::FbxLayerElement::eVertexColor);
	if (layer == NULL)
		return 0;

	FBXSDK_NAMESPACE::FbxLayerElementVertexColor* meshVertexColor = layer->GetVertexColors();
	if (meshVertexColor == NULL)
		return 0;

	int vertexID = 0;
	int vertexIndex = 0;
	int polygonCount = (int)triangleCountPerPolygon.size();
	int triangleIndex = 0;
	for (int polygonIndex = 0; polygonIndex < polygonCount; ++polygonIndex)
	{
		int tc = triangleCountPerPolygon[polygonIndex];
		for (int i = 0; i < tc; ++i)
		{
			int j = (triangleIndex + i) * 3;
			for (int k = 0; k < 3; ++k)
			{
				int vertexOrder = triangles[j + k];
				int controlPointIndex = mesh->GetPolygonVertex(polygonIndex, vertexOrder);
				FBXSDK_NAMESPACE::FbxLayerElement::EMappingMode mappingMode = meshVertexColor->GetMappingMode();
				if (mappingMode == FBXSDK_NAMESPACE::FbxLayerElement::eByControlPoint)
				{
					FBXSDK_NAMESPACE::FbxLayerElement::EReferenceMode refMode = meshVertexColor->GetReferenceMode();
					if (refMode == FBXSDK_NAMESPACE::FbxLayerElement::eDirect)
					{
						indices[vertexIndex] = controlPointIndex;
					}
					else if (refMode == FBXSDK_NAMESPACE::FbxLayerElement::eIndexToDirect)
					{
						FBXSDK_NAMESPACE::FbxLayerElementArrayTemplate<int>& colorIndexArray = meshVertexColor->GetIndexArray();
						int index = colorIndexArray.GetAt(controlPointIndex);
						indices[vertexIndex] = index;
					}
					else
					{
						FBXSDK_printf("%s", "Unsupported vertex color reference mode for polygon vertex 0!\n");
					}
				}
				else if (mappingMode == FBXSDK_NAMESPACE::FbxLayerElement::eByPolygonVertex)
				{
					FBXSDK_NAMESPACE::FbxLayerElement::EReferenceMode refMode = meshVertexColor->GetReferenceMode();
					if (refMode == FBXSDK_NAMESPACE::FbxLayerElement::eDirect)
					{
						indices[vertexIndex] = vertexID + vertexOrder;
					}
					else if (refMode == FBXSDK_NAMESPACE::FbxLayerElement::eIndexToDirect)
					{
						FBXSDK_NAMESPACE::FbxLayerElementArrayTemplate<int>& colorIndexArray = meshVertexColor->GetIndexArray();
						int index = colorIndexArray.GetAt(vertexID + vertexOrder);
						indices[vertexIndex] = index;
					}
					else
					{
						FBXSDK_printf("%s", "Unsupported vertex color reference mode for polygon vertex!\n");
					}
					//<
				}
				else
				{
					FBXSDK_printf("%s", "Unsupported vertex color mapping mode for polygon vertex!\n");
				}
				vertexIndex += 1;
			}
		}

		int polygonSize = mesh->GetPolygonSize(polygonIndex);
		vertexID += polygonSize;

		triangleIndex += tc;
	}

	mVertexColorArray = &(meshVertexColor->GetDirectArray());
	int vertexColorCount = mVertexColorArray->GetCount();

	return vertexColorCount;
}

FbxAMatrix MeshVertex::getGeometryMatrix(FBXSDK_NAMESPACE::FbxNode* node)
{
	const FbxVector4 lT = node->GetGeometricTranslation(FBXSDK_NAMESPACE::FbxNode::eSourcePivot);
	const FbxVector4 lR = node->GetGeometricRotation(FBXSDK_NAMESPACE::FbxNode::eSourcePivot);
	const FbxVector4 lS = node->GetGeometricScaling(FBXSDK_NAMESPACE::FbxNode::eSourcePivot);

	return FbxAMatrix(lT, lR, lS);
}

FbxAMatrix MeshVertex::getWorldToBoneLocalMatrixAtBindPose(FBXSDK_NAMESPACE::FbxCluster* cluster, FBXSDK_NAMESPACE::FbxNode* meshNode)
{
	FbxAMatrix meshLocalToWorldAtBindingTime;
	FbxAMatrix boneLocalToWorldAtBindingTime;

	FbxAMatrix meshGeometry = getGeometryMatrix(meshNode);
	cluster->GetTransformMatrix(meshLocalToWorldAtBindingTime);	// The transformation of the mesh at binding time
	cluster->GetTransformLinkMatrix(boneLocalToWorldAtBindingTime);	// The transformation of the cluster(joint) at binding time from joint space to world space
    FbxAMatrix fullWorldToBoneLocalAtBindingTime = boneLocalToWorldAtBindingTime.Inverse() * meshLocalToWorldAtBindingTime * meshGeometry;
    showFBXMatrix(meshGeometry,"meshGeometry");
    showFBXMatrix(meshLocalToWorldAtBindingTime,"meshLocalToWorldAtBindingTime");
    showFBXMatrix(boneLocalToWorldAtBindingTime,"boneLocalToWorldAtBindingTime");
    showFBXMatrix(fullWorldToBoneLocalAtBindingTime,"fullWorldToBoneLocalAtBindingTime");

    return fullWorldToBoneLocalAtBindingTime;
}

FbxVector4 MeshVertex::transformPositionWithGeometricTransform(const FBXSDK_NAMESPACE::FbxAMatrix& meshGeometry, FbxDouble x, FbxDouble y, FbxDouble z)
{
	FbxVector4 position(x, y, z, 1.0);
	const FbxMatrix temp(meshGeometry);
	FbxVector4 newPosition = temp.MultNormalize(position);
	return newPosition;
}

FbxVector4 MeshVertex::transformNormalWithGeometricTransform(const FBXSDK_NAMESPACE::FbxAMatrix& meshGeometry, FbxDouble x, FbxDouble y, FbxDouble z)
{
	FbxVector4 normal(x, y, z, 1.0);
	FbxMatrix temp(meshGeometry);
	FbxVector4 v(0, 0, 0, 1);
	temp.SetRow(3, v); // ignore translation
	FbxVector4 newNormal = temp.MultNormalize(normal);
	newNormal.Normalize();
	return newNormal;
}

void MeshVertex::getEffectiveBones(FbxMesh* mesh, std::vector<FBXSDK_NAMESPACE::FbxUInt64>& vBoneID)
{
	FBXSDK_NAMESPACE::FbxSkin* skin = NULL;
	int skinningDeformerCount = mesh->GetDeformerCount(FBXSDK_NAMESPACE::FbxDeformer::eSkin);
	if (skinningDeformerCount > 0)
	{
		skin = (FBXSDK_NAMESPACE::FbxSkin*)mesh->GetDeformer(0, FBXSDK_NAMESPACE::FbxDeformer::eSkin);
		if (skinningDeformerCount > 1)
		{
			FBXSDK_printf("%s", "Error: There are more than one skinning deformer on a single mesh! Exporter only support a single skinning deformer!\n");
		}
	}
	else
	{
		return;
	}

	FbxSkin::EType skinningType = skin->GetSkinningType();
	if (skinningType != FbxSkin::eLinear && skinningType != FbxSkin::eRigid)
	{
		FBXSDK_printf("%s", "Error: Skinning is not linear or rigid type! This type is not supported by engine!\n");
	}

	//int polygonVertexCount = mesh->GetControlPointsCount();

	//FBXSDK_NAMESPACE::FbxAMatrix transformMatrix, transformLinkMatrix;
	int clusterCount = skin->GetClusterCount();
	if (clusterCount <= 0)
	{
		return;
	}

	for (int i = 0; i < clusterCount; ++i)
	{
		FBXSDK_NAMESPACE::FbxCluster* cluster = skin->GetCluster(i);

		FBXSDK_NAMESPACE::FbxCluster::ELinkMode clusterMode = cluster->GetLinkMode();
		if (clusterMode != FBXSDK_NAMESPACE::FbxCluster::eNormalize &&
			clusterMode != FBXSDK_NAMESPACE::FbxCluster::eTotalOne)
		{
			FBXSDK_printf("%s", "Error: Only support normalized and total one weights!\n");
		}

		FbxNode* linkNode = cluster->GetLink();
		if (linkNode == NULL)
			continue;

		FbxString nodeName = linkNode->GetName();
		int associatedCtrlPointCount = cluster->GetControlPointIndicesCount();
		if (associatedCtrlPointCount == 0)
			continue;

		FBXSDK_NAMESPACE::FbxUInt64 id = linkNode->GetUniqueID();
		vBoneID.push_back(id);
	}
}

void MeshVertex::getSkinning(
	FbxMesh* mesh, 
	std::vector<BONE_>* effectiveBones, 
	std::vector<BW_>* bwValues, 
	std::vector<BI_>* biValues)
{
	FbxSkin* skin = NULL;
	int skinningDeformerCount = mesh->GetDeformerCount(FbxDeformer::eSkin);
	if (skinningDeformerCount > 0)
	{
		skin = (FBXSDK_NAMESPACE::FbxSkin*)mesh->GetDeformer(0, FbxDeformer::eSkin);
		if (skinningDeformerCount > 1)
		{
			FBXSDK_printf("%s", "Error: There are more than one skinning deformer on a single mesh! Exporter only support a single skinning deformer!\n");
		}
	}
	else
	{
		return;
	}

	FBXSDK_NAMESPACE::FbxSkin::EType skinningType = skin->GetSkinningType();
	if (skinningType != FBXSDK_NAMESPACE::FbxSkin::eLinear && skinningType != FBXSDK_NAMESPACE::FbxSkin::eRigid)
	{
		FBXSDK_printf("%s", "Error: Skinning is not linear or rigid type! This type is not supported by engine!\n");
	}

	int polygonVertexCount = mesh->GetControlPointsCount();

	FBXSDK_NAMESPACE::FbxAMatrix transformMatrix, transformLinkMatrix;
	int clusterCount = skin->GetClusterCount();
	if (clusterCount <= 0)
	{
		return;
	}

	SKINNING_DATA_* skinningData = new SKINNING_DATA_[polygonVertexCount];
	memset(skinningData, 0, sizeof(SKINNING_DATA_)*polygonVertexCount);
	FBXSDK_NAMESPACE::FbxNode** bones = new FBXSDK_NAMESPACE::FbxNode*[clusterCount];
	FBXSDK_NAMESPACE::FbxAMatrix* worldToBoneLocalMatrices = new FBXSDK_NAMESPACE::FbxAMatrix[clusterCount];

	bool affectedByMoreThanFourBones = false;
	int effectiveBoneCount = 0;

	//vector<std::string> testNames;
	for (int i = 0; i < clusterCount; ++i)
	{
		FBXSDK_NAMESPACE::FbxCluster* cluster = skin->GetCluster(i);

		FBXSDK_NAMESPACE::FbxCluster::ELinkMode clusterMode = cluster->GetLinkMode();
		if (clusterMode != FBXSDK_NAMESPACE::FbxCluster::eNormalize && 
			clusterMode != FBXSDK_NAMESPACE::FbxCluster::eTotalOne)
		{
			FBXSDK_printf("%s", "Error: Only support normalized and total one weights!\n");
		}

		FBXSDK_NAMESPACE::FbxNode* linkNode = cluster->GetLink();
		if (linkNode == NULL)
			continue;

		FBXSDK_NAMESPACE::FbxString nodeName = linkNode->GetName();
		int associatedCtrlPointCount = cluster->GetControlPointIndicesCount();
		if (associatedCtrlPointCount == 0)
			continue;

		//testNames.push_back((const char*)nodeName);

		bones[effectiveBoneCount] = linkNode;
		FBXSDK_NAMESPACE::FbxNode* meshNode = mesh->GetNode();
		worldToBoneLocalMatrices[effectiveBoneCount] = getWorldToBoneLocalMatrixAtBindPose(cluster, meshNode);
		int* ctrlPointIndices = cluster->GetControlPointIndices();
		double* ctrlPointWeights = cluster->GetControlPointWeights();

		for (int k = 0; k < associatedCtrlPointCount; ++k)
		{
			int vertexIndex = ctrlPointIndices[k];

			double bi = (double)effectiveBoneCount;
			double bw = ctrlPointWeights[k];

			int count = skinningData[vertexIndex].count;
			if (count == MAX_INFLUENCE_BONE_COUNT && bw > 0.0)
			{
				affectedByMoreThanFourBones = true;
			}

			int insertSlot = -1;
			for (int y = 0; y < MAX_INFLUENCE_BONE_COUNT; ++y)
			{
				double z = skinningData[vertexIndex].weights[y];
				if (bw > z)
				{
					insertSlot = y;
					break;
				}
			}

			if (insertSlot >= 0)
			{
				count = count > (MAX_INFLUENCE_BONE_COUNT - 1) ? (MAX_INFLUENCE_BONE_COUNT - 1) : count;
				for (int mi = count; mi > insertSlot; --mi)
				{
					skinningData[vertexIndex].weights[mi] = skinningData[vertexIndex].weights[mi - 1];
					skinningData[vertexIndex].indices[mi] = skinningData[vertexIndex].indices[mi - 1];
				}

				skinningData[vertexIndex].indices[insertSlot] = bi;
				skinningData[vertexIndex].weights[insertSlot] = bw;

				if (skinningData[vertexIndex].count < MAX_INFLUENCE_BONE_COUNT)
				{
					skinningData[vertexIndex].count++;
				}				
			}
			//<
		}

		++effectiveBoneCount;
	}

	if (affectedByMoreThanFourBones)
	{
		FBXSDK_printf("Error: Some vertices are affected by more than %d bones! redundant bones would be ignored!\n", MAX_INFLUENCE_BONE_COUNT);
	}

	//std::map<int, int> effectiveBoneIDs;
	for (int j = 0; j < polygonVertexCount; ++j)
	{
		SKINNING_DATA_& data = skinningData[j];

		double totalWeight = data.weights[0] + data.weights[1] + data.weights[2] + data.weights[3];
		if (abs(totalWeight) < 1e-7)
		{
			data.weights[0] = 1.0;
			FBXSDK_printf("%s", "Error: The skinned vertex is not affected by any bone!\n");
		}
        //dzlog_info("%d: %f,%f,%f,%f",j, data.indices[0], data.indices[1], data.indices[2], data.indices[3]);
		bwValues->push_back(BW_(data.weights[0], data.weights[1], data.weights[2], data.weights[3]));
		biValues->push_back(BI_(data.indices[0], data.indices[1], data.indices[2], data.indices[3]));
	}

	delete[] skinningData;
	skinningData = NULL;

	// bones
	for (int i = 0; i < effectiveBoneCount; ++i)
	{
		BONE_ bone;
		FbxNode* node = bones[i];
		bone.id = node->GetUniqueID();
		bone.name = standardizeFileName(node->GetName());
		bone.modelWorldToBoneLocal = worldToBoneLocalMatrices[i];
		effectiveBones->push_back(bone);
	}

	delete[] bones;
	bones = NULL;

	delete[] worldToBoneLocalMatrices;
	worldToBoneLocalMatrices = NULL;
}

FBXSDK_NAMESPACE::FbxVector4* MeshVertex::computeVertexNormals(FBXSDK_NAMESPACE::FbxVector4* positions, FBXSDK_NAMESPACE::FbxUInt32 positionCount, std::vector<int>& indices)
{
	FBXSDK_NAMESPACE::FbxVector4* normals = new FBXSDK_NAMESPACE::FbxVector4[positionCount];
	memset(normals, 0, positionCount*sizeof(FBXSDK_NAMESPACE::FbxVector4));

	for (int i = 0; i < (int)indices.size(); i += 3)
	{
		int vA = indices[i + 0];
		int vB = indices[i + 1];
		int vC = indices[i + 2];

		FBXSDK_NAMESPACE::FbxVector4& pA = *(positions + vA);
		FBXSDK_NAMESPACE::FbxVector4& pB = *(positions + vB);
		FBXSDK_NAMESPACE::FbxVector4& pC = *(positions + vC);

		FBXSDK_NAMESPACE::FbxVector4 cb = pC - pB;
		FBXSDK_NAMESPACE::FbxVector4 ab = pA - pB;
		FBXSDK_NAMESPACE::FbxVector4 planeNormalAtB = cb.CrossProduct(ab);
		planeNormalAtB.Normalize();

		normals[vA] += planeNormalAtB;
		normals[vB] += planeNormalAtB;
		normals[vC] += planeNormalAtB;
	}

	for (int i = 0; i < (int)positionCount; ++i)
	{
		FBXSDK_NAMESPACE::FbxVector4& n = normals[i];
		n.Normalize();
	}

	return normals;
}

void MeshVertex::getMorph(
	FBXSDK_NAMESPACE::FbxMesh* mesh, 
	std::vector<int>* positionDataIndices, 
	std::vector<int>* normalDataIndices,
	std::vector<int>* uvDataIndices,
	std::vector<int>& indices,
	bool isSkinned, 
	std::vector<MORPH_DATA_V2>* morphTargetsData)
{
	int blendShapeCount = mesh->GetDeformerCount(FbxDeformer::eBlendShape);
	if (blendShapeCount == 0)
	{
		return;
	}
	else
	{
		if (blendShapeCount > 1)
		{
			FBXSDK_printf("%s", "Error: There are more than one blend shape deformer on a single mesh! Exporter only support a single blendshape deformer!\n");
		}
	}

	FBXSDK_NAMESPACE::FbxAMatrix geometricMatrix;
	FbxNode* node = mesh->GetNode();
	if (node != NULL && !isSkinned)
	{
		geometricMatrix = MeshVertex::getGeometryMatrix(node);
	}
	bool identicalMatrix = geometricMatrix.IsIdentity();
	
	FBXSDK_NAMESPACE::FbxBlendShape* blendShape = (FBXSDK_NAMESPACE::FbxBlendShape*)mesh->GetDeformer(0, FbxDeformer::eBlendShape);
	int blendShapeChannelCount = blendShape->GetBlendShapeChannelCount();
	for (int channelIndex = 0; channelIndex < blendShapeChannelCount; ++channelIndex)
	{
		FBXSDK_NAMESPACE::FbxBlendShapeChannel* channel = blendShape->GetBlendShapeChannel(channelIndex);
		if (channel)
		{
			int shapeCount = channel->GetTargetShapeCount();
			if (shapeCount > 1)
			{
				FBXSDK_printf("%s", "Error: Do not support more than one morph target on a single channel(progressive morph or in-between blend shape)!\n");
				//return;
			}
			const char* channelName = channel->GetName();

			FBXSDK_NAMESPACE::FbxShape* blendShape = channel->GetTargetShape(0);
			if (blendShape)
			{
				FBXSDK_NAMESPACE::FbxUInt64 uniqueID = blendShape->GetUniqueID();
				FBXSDK_NAMESPACE::FbxVector4* positions = blendShape->GetControlPoints();
				int positionCount = blendShape->GetControlPointsCount();

				FBXSDK_NAMESPACE::FbxLayerElementNormal* meshNormal = blendShape->GetElementNormal();
				FBXSDK_NAMESPACE::FbxLayerElementArrayTemplate<FBXSDK_NAMESPACE::FbxVector4>& normals = meshNormal->GetDirectArray();
				int normalCount = normals.GetCount();
				
				//TODO: Blend shape tangent
				//FBXSDK_NAMESPACE::FbxLayerElementUV* meshUV = blendShape->GetElementUV();
				//FBXSDK_NAMESPACE::FbxLayerElementArrayTemplate<FBXSDK_NAMESPACE::FbxVector2>& uvs = meshUV->GetDirectArray();
				//int uvCount = uvs.GetCount();
				
				if (positionCount > 0 && normalCount > 0)
				{
					unsigned int vertexCount = (unsigned int)positionDataIndices->size();

					MORPH_DATA_V2 morphData;					
					morphData.morphTargetName = channelName;
					morphData.uniqueID = uniqueID;
					morphData.vertexCount = vertexCount;

					morphData.bboxMin.mData[0] = morphData.bboxMin.mData[1] = morphData.bboxMin.mData[2] = FLT_MAX;
					morphData.bboxMax.mData[0] = morphData.bboxMax.mData[1] = morphData.bboxMax.mData[2] = FLT_MIN;
					//Transform position at first, because there are more positions after splitting.
					if (!identicalMatrix)
					{
						for (int i = 0; i < positionCount; ++i)
						{
							FBXSDK_NAMESPACE::FbxVector4 p = positions[i];
							positions[i] = MeshVertex::transformPositionWithGeometricTransform(geometricMatrix, p[0], p[1], p[2]);							
						}
					}

					morphData.positions.resize(vertexCount * 3);
					for (unsigned int i = 0; i < vertexCount; ++i)
					{
						int j = (*positionDataIndices)[i];
						FBXSDK_NAMESPACE::FbxVector4 p = positions[j];
						morphData.positions[i * 3 + 0] = (float)p[0];
						morphData.positions[i * 3 + 1] = (float)p[1];
						morphData.positions[i * 3 + 2] = (float)p[2];

						if (p[0] < morphData.bboxMin.mData[0])
							morphData.bboxMin.mData[0] = p[0];

						if (p[1] < morphData.bboxMin.mData[1])
							morphData.bboxMin.mData[1] = p[1];

						if (p[2] < morphData.bboxMin.mData[2])
							morphData.bboxMin.mData[2] = p[2];

						if (p[0] > morphData.bboxMax.mData[0])
							morphData.bboxMax.mData[0] = p[0];

						if (p[1] > morphData.bboxMax.mData[1])
							morphData.bboxMax.mData[1] = p[1];

						if (p[2] > morphData.bboxMax.mData[2])
							morphData.bboxMax.mData[2] = p[2];
					}

					//Normal
					morphData.normals.resize(vertexCount * 3);
					for (unsigned int i = 0; i < vertexCount; ++i)
					{
						int j = (*normalDataIndices)[i];
						FBXSDK_NAMESPACE::FbxVector4 n = normals[j];
						if (!identicalMatrix)
						{
							n = MeshVertex::transformNormalWithGeometricTransform(geometricMatrix, n[0], n[1], n[2]);
						}
						n.Normalize();

						morphData.normals[i * 3 + 0] = (float)n[0];
						morphData.normals[i * 3 + 1] = (float)n[1];
						morphData.normals[i * 3 + 2] = (float)n[2];
					}

					morphTargetsData->push_back(morphData);
				}
			}
		}
	}
}

void MeshVertex::createSubmeshByMaterials(FBXSDK_NAMESPACE::FbxMesh* mesh, const int materialCount, int layerIndex, std::vector<std::vector<int> >& subMeshes, \
	std::vector<int>& triangleStartPerPolygon, std::vector<int>& triangleCountPerPolygon)
{
	int polygonCount = (int)triangleStartPerPolygon.size();
	if (polygonCount == 0)
		return;

	FBXSDK_NAMESPACE::FbxGeometryElementMaterial* elementMaterial = NULL;
	FBXSDK_NAMESPACE::FbxLayer* layer = mesh->GetLayer(layerIndex, FBXSDK_NAMESPACE::FbxLayerElement::eMaterial);
	if (layer != NULL)
	{
		elementMaterial = layer->GetMaterials();
		FBXSDK_NAMESPACE::FbxGeometryElement::EReferenceMode materialReferenceMode = elementMaterial->GetReferenceMode();
		if (FBXSDK_NAMESPACE::FbxLayerElement::eIndex == materialReferenceMode)
		{
			// Materials are in an undefined external table instead of on this node
			FBXSDK_printf("%s", "Do not supported external material indices mapping mode!\n");
			return;
		}
	}

	int triangleCount = triangleStartPerPolygon[polygonCount - 1] + triangleCountPerPolygon[polygonCount - 1];
	if (elementMaterial == NULL || layer == NULL)
	{
		subMeshes.resize(1);
		for (int triangleIndex = 0; triangleIndex < triangleCount; ++triangleIndex)
		{
			subMeshes[0].push_back(triangleIndex);
		}
	}
	else
	{
		// Only resolve the first FbxGeometryElementMaterial
		FBXSDK_NAMESPACE::FbxLayerElementArrayTemplate<int>& materialIndice = elementMaterial->GetIndexArray();
		FBXSDK_NAMESPACE::FbxGeometryElement::EMappingMode materialMappingMode = elementMaterial->GetMappingMode();

		if (materialMappingMode == FbxGeometryElement::eByPolygon)
		{
			bool errorFlag = false;
			for (int polygonIndex = 0; polygonIndex < polygonCount; ++polygonIndex)
			{
				int materialIndex = materialIndice.GetAt(polygonIndex);
				if (materialIndex >= materialCount || materialIndex < 0)
				{
					materialIndex = 0;
					errorFlag = true;
				}

				if ((int)subMeshes.size() < materialIndex + 1)
				{
					subMeshes.resize(materialIndex + 1);
				}

				int tc = triangleCountPerPolygon[polygonIndex];
				int start = triangleStartPerPolygon[polygonIndex];
				for (int k = 0; k < tc; ++k)
				{
					subMeshes[materialIndex].push_back(start + k);
				}
			}

			if (errorFlag)
			{
				FBXSDK_printf("%s", "Detected a polygon associate with a invalid material index in a multi material!\n");
			}
		}
		else if (materialMappingMode == FbxGeometryElement::eAllSame)
		{
			subMeshes.resize(1);
			for (int triangleIndex = 0; triangleIndex < triangleCount; ++triangleIndex)
			{
				subMeshes[0].push_back(triangleIndex);
			}
		}
		else
		{
			FBXSDK_printf("%s", "Unsupported material indices mapping mode!\n");
		}
	}
}

std::string MeshVertex::mergeMeshes(
	const std::string& outputDirectory, 
	std::vector<meshLoader*> loaders,
	int& parentUniqueID,
	int& uniqueID,
	FBXSDK_NAMESPACE::FbxDouble3& bboxMin,
	FBXSDK_NAMESPACE::FbxDouble3& bboxMax
	)
{
	if (loaders.size() == 0)
		return "";

	uniqueID = loaders[0]->mUniqueID;
	parentUniqueID = loaders[0]->mParentUniqueID;

	std::string objectName = "mergedMesh";
	char buffer[1024];
	sprintf(buffer, "%s.%d.mesh.bin", objectName.c_str(), uniqueID);
	std::string meshBinaryFile = buffer;

	unsigned int totalVertexCount = 0;
	unsigned int totalTriangleCount = 0;
	unsigned int totalTopologicalLineCount = 0;

	unsigned int totalPositionBufferSize = 0;
	unsigned int totalNormalBufferSize = 0;
	unsigned int totalVertexColorBufferSize = 0;
	unsigned int totalIndexBufferSize = 0;
	unsigned int totalTopologicalIndexBufferSize = 0;

	for (size_t s = 0; s < loaders.size(); ++s)
	{
		meshLoader* loader = loaders[s];
		totalVertexCount += loader->mVertexCount;
		totalTriangleCount += loader->mTriangleCount;
		totalTopologicalLineCount += loader->mTopologicalLineCount;

		totalPositionBufferSize += loader->mPositionBufferSize;
		totalNormalBufferSize += loader->mNormalBufferSize;
		totalVertexColorBufferSize += loader->mVertexColorBufferSize;
		totalIndexBufferSize += loader->mIndexBufferSize;
		totalTopologicalIndexBufferSize += loader->mTopologicalIndexBufferSize;

		if (loader->mMinX < bboxMin[0])
			bboxMin[0] = loader->mMinX;

		if (loader->mMinY < bboxMin[1])
			bboxMin[1] = loader->mMinY;

		if (loader->mMinZ < bboxMin[2])
			bboxMin[2] = loader->mMinZ;

		if (loader->mMaxX > bboxMax[0])
			bboxMax[0] = loader->mMaxX;

		if (loader->mMaxY > bboxMax[1])
			bboxMax[1] = loader->mMaxY;

		if (loader->mMaxZ > bboxMax[2])
			bboxMax[2] = loader->mMaxZ;
	}

	//Position outputs
	//unsigned int outPosEncodingSize = totalPositionBufferSize + sizeof(unsigned int) * 6; //later for xyz component length;
	unsigned int outPosEncodingSize = totalPositionBufferSize;
	uint8_t* outPosEncodingBuffer = new uint8_t[outPosEncodingSize];
	unsigned int outPosEncodingFlag = 0;

	//Normal outputs	
	unsigned int outNormalEncodingSize = totalNormalBufferSize;
	uint8_t* outNormalEncodingBuffer = new uint8_t[outNormalEncodingSize];
	unsigned int outNormalEncodingFlag = 0;

	//VertexColor outputs
	unsigned int outVertexColorEncodingSize = totalVertexColorBufferSize;
	uint8_t* outVertexColorEncodingBuffer = new uint8_t[outVertexColorEncodingSize];
	unsigned int outVertexColorEncodingFlag = 0;

	//Index outputs
	unsigned int outIndexEncodingSize = totalIndexBufferSize;
	uint8_t* outIndexEncodingBuffer = new uint8_t[outIndexEncodingSize];
	unsigned int outIndexEncodingFlag = 0;

	//Topological Index outputs
	unsigned int outTopologicalIndexEncodingSize = totalTopologicalIndexBufferSize;
	uint8_t* outTopologicalIndexEncodingBuffer = new uint8_t[outTopologicalIndexEncodingSize];
	unsigned int outTopologicalIndexEncodingFlag = 0;

	unsigned int p0 = 0, p1 = 0, p2 = 0, p3 = 0, p4 = 0;
	unsigned int v0 = 0;
	for (size_t s = 0; s < loaders.size(); ++s)
	{
		meshLoader* loader = loaders[s];

		memcpy(outPosEncodingBuffer + p0, loader->mPositions, loader->mPositionBufferSize);
		p0 += loader->mPositionBufferSize;

		memcpy(outNormalEncodingBuffer + p1, loader->mNormals, loader->mNormalBufferSize);
		p1 += loader->mNormalBufferSize;

		memcpy(outVertexColorEncodingBuffer + p2, loader->mVertexColors, loader->mVertexColorBufferSize);
		p2 += loader->mVertexColorBufferSize;

		int* index = (int*)(outIndexEncodingBuffer + p3);
		for (int k = 0; k < loader->mTriangleCount * 3; ++k)
		{
			index[k] = loader->mIndices[k] + v0;
		}
		p3 += loader->mIndexBufferSize;

		index = (int*)(outTopologicalIndexEncodingBuffer + p4);
		for (int k = 0; k < loader->mTopologicalLineCount * 2; ++k)
		{
			index[k] = loader->mTopologicalIndices[k] + v0;
		}
		p4 += loader->mTopologicalIndexBufferSize;

		v0 += loader->mVertexCount;
	}

	//
	std::vector<VERTEX_LAYER_TYPE> vertexElements;
	vertexElements.push_back(VL_POSITION);
	vertexElements.push_back(VL_NORMAL0);
	vertexElements.push_back(VL_VERTEXCOLOR0);
	vertexElements.push_back(VL_INDEX);
	vertexElements.push_back(VL_TOPOLOGICALINDEX);

	FBXSDK_NAMESPACE::FbxDouble3 uvBboxMin(FLT_MAX, FLT_MAX, FLT_MAX);
	FBXSDK_NAMESPACE::FbxDouble3 uvBboxMax(FLT_MIN, FLT_MIN, FLT_MIN);

	unsigned int subMeshCount = 1;
	unsigned int outSubMeshEncodingSize = sizeof(unsigned int) * 2 * subMeshCount;
	uint8_t* outSubMeshEncodingBuffer = new uint8_t[outSubMeshEncodingSize];
	unsigned int outSubMeshEncodingFlag = 0;
	unsigned int* output = (unsigned int*)outSubMeshEncodingBuffer;
	output[0] = 0;//indexOffset
	output[1] = totalTriangleCount;	//triangleCount

	//
	JsonToBin_V4* writer = new JsonToBin_V4();
	writer->openExportFile(outputDirectory + meshBinaryFile);

	//writer->writeObjectBin(
	//	parentUniqueID,
	//	uniqueID,
	//	MESH_TYPE,
	//	objectName,
	//	(unsigned int)totalVertexCount,
	//	vertexElements,
	//	NULL,
	//	\
	//	subMeshCount,
	//	outSubMeshEncodingBuffer,
	//	outSubMeshEncodingSize,
	//	outSubMeshEncodingFlag,
	//	\
	//	totalTriangleCount,
	//	outIndexEncodingBuffer,
	//	outIndexEncodingSize,
	//	outIndexEncodingFlag,
	//	\
	//	totalTopologicalLineCount,
	//	outTopologicalIndexEncodingBuffer,
	//	outTopologicalIndexEncodingSize,
	//	outTopologicalIndexEncodingFlag,
	//	\
	//	bboxMin,
	//	bboxMax,
	//	outPosEncodingBuffer,
	//	outPosEncodingSize,
	//	outPosEncodingFlag,
	//	\
	//	outNormalEncodingBuffer,
	//	outNormalEncodingSize,
	//	outNormalEncodingFlag,
	//	\
	//	NULL, 0, 0,
	//	\
	//	outVertexColorEncodingBuffer,
	//	outVertexColorEncodingSize,
	//	outVertexColorEncodingFlag,
	//	\
	//	uvBboxMin, uvBboxMax, NULL, 0, 0,
	//	\
	//	NULL, 0, 0,
	//	\
	//	NULL, 0, 0);

	
	writer->closeExportFile();
	delete writer;

	return meshBinaryFile;
}

//bool MeshVertex::appendNewLayer(std::vector<std::vector<int>*>& vb, std::vector<int>& newLayerIndices, std::vector<int>& indices, VERTEX_LAYER_TYPE layerType)
//{
//	if ((newLayerIndices.size() == 0) || (newLayerIndices.size() != indices.size()) || (vb.size() == 0))
//	{
//		return false;
//	}
//
//	std::vector<int>* position = vb[0];
//	unsigned int positionCount = (unsigned int)position->size();
//
//	std::vector<int>* newBuffer = new std::vector<int>(positionCount, -1);
//
//	//
//	unsigned int vertexLayerCount = (unsigned int)vb.size();
//	unsigned int vertexCount = (unsigned int)indices.size();
//
//	FBXSDK_NAMESPACE::FbxVector4 N0, N1;
//
//	for (unsigned int i = 0; i < vertexCount; ++i)
//	{
//		int pi = indices[i];
//		int newElementIndex = newLayerIndices[i];
//
//		int ni = (*newBuffer)[pi];
//		if (ni != newElementIndex)
//		{
//			if (ni == -1)
//			{
//				(*newBuffer)[pi] = newElementIndex;
//			}
//			else
//			{
//				bool differentValue = true;
//				if (layerType == VL_POSITION)
//				{
//					FBXSDK_NAMESPACE::FbxVector4 N0 = mNormalArray->GetAt(ni);
//					FBXSDK_NAMESPACE::FbxVector4 N1 = mNormalArray->GetAt(newElementIndex);
//					if (N0 == N1)
//					{
//						differentValue = false;
//					}
//				}
//				else if (layerType == VL_UV0)
//				{
//					FBXSDK_NAMESPACE::FbxVector2 U0 = mUVArray0->GetAt(ni);
//					FBXSDK_NAMESPACE::FbxVector2 U1 = mUVArray0->GetAt(newElementIndex);
//					if (U0 == U1)
//					{
//						differentValue = false;
//					}
//				}
//				else if (layerType == VL_VERTEXCOLOR0)
//				{
//					FBXSDK_NAMESPACE::FbxColor C0 = mVertexColorArray->GetAt(ni);
//					FBXSDK_NAMESPACE::FbxColor C1 = mVertexColorArray->GetAt(newElementIndex);
//					if (C0 == C1)
//					{
//						differentValue = false;
//					}
//				}
//
//				if (differentValue)
//				{
//					//Copy vertex
//					for (unsigned int L = 0; L < vertexLayerCount; L++)
//					{
//						std::vector<int>* layer = vb[L];
//						int indexToBeCopied = (*layer)[pi];
//						(*layer).push_back(indexToBeCopied);
//					}
//
//					(*newBuffer).push_back(newElementIndex);
//
//					indices[i] = (int)((*newBuffer).size() - 1);
//				}
//			}	
//			//<
//			
//		}
//		
//	}
//
//	//
//	vb.push_back(newBuffer);
//	mVertexElements.push_back(layerType);
//
//	return true;
//}

bool MeshVertex::appendNewLayer_V2(std::vector<std::vector<int>*>& vb, std::vector<int>& newLayerIndices, std::vector<int>& indices, VERTEX_LAYER_TYPE layerType)
{
	if ((newLayerIndices.size() == 0) || (newLayerIndices.size() != indices.size()) || (vb.size() == 0))
	{
		return false;
	}

	std::vector<int>* position = vb[0];
	unsigned int newElementArrayLength = (unsigned int)position->size();

	std::vector<REF_RECORD*> records;
	records.resize(newElementArrayLength);

	//
	unsigned int vertexLayerCount = (unsigned int)vb.size();
	unsigned int vertexCount = (unsigned int)indices.size();

	FBXSDK_NAMESPACE::FbxVector4 N0, N1;

	for (unsigned int i = 0; i < vertexCount; ++i)
	{
		int pi = indices[i];
		int newElementIndex = newLayerIndices[i];

		REF_RECORD* first = records[pi];
		if (first == NULL)
		{
			REF_RECORD* newRecord = new REF_RECORD();
			newRecord->INDEX = newElementIndex;
			newRecord->WHERE = pi;
			records[pi] = newRecord;
		}
		else
		{
			bool foundSameElement = false;
			while (first != NULL)
			{
				if (first->INDEX == newElementIndex)
				{
					indices[i] = first->WHERE;
					foundSameElement = true;
					break;
				}
				else
				{
					bool identical = false;
					if (layerType == VL_NORMAL0)
					{
						FBXSDK_NAMESPACE::FbxVector4 N0 = mNormalArray->GetAt(first->INDEX);
						FBXSDK_NAMESPACE::FbxVector4 N1 = mNormalArray->GetAt(newElementIndex);
						if (N0 == N1)
						{
							identical = true;
						}
					}
					else if (layerType == VL_TANGENT0)
					{
						FBXSDK_NAMESPACE::FbxVector4 T0 = mTangentArray->GetAt(first->INDEX);
						FBXSDK_NAMESPACE::FbxVector4 T1 = mTangentArray->GetAt(newElementIndex);
						if (T0 == T1)
						{
							identical = true;
						}
					}
					else if (layerType == VL_BINORMAL0)
					{
						FBXSDK_NAMESPACE::FbxVector4 B0 = mBinormalArray->GetAt(first->INDEX);
						FBXSDK_NAMESPACE::FbxVector4 B1 = mBinormalArray->GetAt(newElementIndex);
						if (B0 == B1)
						{
							identical = true;
						}
					}
					else if (layerType == VL_UV0)
					{
						FBXSDK_NAMESPACE::FbxVector2 U0 = mUVArray0->GetAt(first->INDEX);
						FBXSDK_NAMESPACE::FbxVector2 U1 = mUVArray0->GetAt(newElementIndex);
						if (U0 == U1)
						{
							identical = true;
						}
					}
					else if (layerType == VL_VERTEXCOLOR0)
					{
						FBXSDK_NAMESPACE::FbxColor C0 = mVertexColorArray->GetAt(first->INDEX);
						FBXSDK_NAMESPACE::FbxColor C1 = mVertexColorArray->GetAt(newElementIndex);
						if (C0 == C1)
						{
							identical = true;
						}
					}

					//<
					if (identical)
					{
						indices[i] = first->WHERE;
						foundSameElement = true;
						break;
					}
					//<
				}
				first = first->NEXT;
			}

			//if cannot find the same element
			if (!foundSameElement)
			{
				REF_RECORD* newRecord = new REF_RECORD();
				newRecord->INDEX = newElementIndex;
				newRecord->WHERE = newElementArrayLength;

				REF_RECORD* head = records[pi];
				newRecord->NEXT = head;
				records[pi] = newRecord;

				indices[i] = newRecord->WHERE;

				++newElementArrayLength;
			}
		}
	}
	//<
	
	for (unsigned int L = 0; L < vertexLayerCount; L++)
	{
		std::vector<int>* layer = vb[L];
		layer->resize(newElementArrayLength);
	}

	std::vector<int>* newBuffer = new std::vector<int>(newElementArrayLength, -1);	

	for (int i = 0; i < (int)records.size(); ++i)
	{
		REF_RECORD* first = records[i];
		while (first != NULL)
		{
			(*newBuffer)[first->WHERE] = first->INDEX;

			if (first->WHERE != i)
			{
				for (unsigned int L = 0; L < vertexLayerCount; L++)
				{
					std::vector<int>* layer = vb[L];
					int indexToBeCopied = (*layer)[i];
					(*layer)[first->WHERE] = indexToBeCopied;
				}
			}

			first = first->NEXT;
		}
	}

	vb.push_back(newBuffer);
	mVertexElements.push_back(layerType);
		
	//Clear memory
	for (unsigned int i = 0; i < records.size(); ++i)
	{
		REF_RECORD* first = records[i];
		while (first != NULL)
		{
			REF_RECORD* next = first->NEXT;
			delete first;
			first = next;
		}
	}

	return true;
}
template<class T>
const T& clamp(const T& x, const T& lower, const T& upper) {
	return min_0(upper, max_0(x, lower));
}

const int MeshVertex::mscVeretexCacheSize = 24;
const float MeshVertex::mscPositionPrecision = 0.5f;
const double MeshVertex::mscEpsilonDegree = 1.05;
const double MeshVertex::mscEpsilon = mscEpsilonDegree*M_PI/180.0;
const unsigned int MeshVertex::msc_N_phi = 120; //(int)ceil(MIN_N_phi); N_theta max is 248, total points 18778
const double MeshVertex::msc_MIN_N_phi = M_PI / (2.0*mscEpsilon) + 1.0;
std::vector<unsigned int> MeshVertex::ms_N_theta_series;
double MeshVertex::msPhiDecodingFactor = 0.0;

void MeshVertex::initNormalCompressor()
{
	double cosEpsilon = cos(mscEpsilon);
	double deltaRadian = M_PI / (2.0*(msc_N_phi - 1));
	unsigned int totalPointCount = 0;
	unsigned int maxThetaCount = 0;
	for (unsigned int j = 0; j < msc_N_phi; ++j)
	{
		double phi = j*(M_PI / (msc_N_phi - 1));
		double cosPhi = cos(phi);
		double cosPhiDelta = cos(phi + deltaRadian);
		double sinPhi = sin(phi);
		double sinPhiDelta = sin(phi + deltaRadian);
		double R = (cosEpsilon - cosPhi*cosPhiDelta) / max_0(1e-5, (sinPhi*sinPhiDelta));
		R = clamp(R, -1.0, 1.0);

		unsigned int N_theta = (unsigned int)ceil(M_PI / max_0(1e-5, acos(R)));

		maxThetaCount = N_theta > maxThetaCount ? N_theta : maxThetaCount;
		totalPointCount += N_theta;
		ms_N_theta_series.push_back(N_theta);
	}

	msPhiDecodingFactor = M_PI / (ms_N_theta_series.size() - 1);
}

void MeshVertex::mapNormal(
	double x, 
	double y, 
	double z,
	double w,
	unsigned int& encodedPhi, 
	unsigned int& encodedTheta)
{
	x = clamp(x, -1.0, 1.0);
	y = clamp(y, -1.0, 1.0);
	z = clamp(z, -1.0, 1.0);

	double phi = acos(z);
	double theta = atan2(y, x);
	theta = theta < 0.0 ? (2.0*M_PI + theta) : theta;

	//double cosEpsilon = cos(mscEpsilon);
	//double deltaRadian = M_PI / (2.0*(msc_N_phi - 1));

	double j = round(phi*(msc_N_phi - 1) / M_PI);	

	unsigned int N_theta = ms_N_theta_series[(unsigned int)j];

	j -= (w * 128.0);

	double k = round(theta*N_theta / (2.0*M_PI));
	//j = {0, 1, ..., N_phi - 1}
	//k = {0, 1, ..., N_theta - 1}
	encodedPhi = (unsigned int)j;
	encodedTheta = (unsigned int)k;

	//double n = (2.0 * M_PI) / N_theta;
	//double m = sin(encodedPhi*msPhiDecodingFactor);
	//double decodeX = m*cos(n*encodedTheta);
	//double decodeY = m*sin(n*encodedTheta);
	//double decodeZ = cos(encodedPhi*msPhiDecodingFactor);

	//return FBXSDK_NAMESPACE::FbxVector4(decodeX, decodeY, decodeZ, 0.0);
}

unsigned short MeshVertex::RGB888_RGB565(unsigned char red, unsigned char green, unsigned char blue)
{
	unsigned short  B = ((unsigned short)blue >> 3) & 0x001F;
	unsigned short  G = (((unsigned short)green >> 2) << 5) & 0x07E0;
	unsigned short  R = (((unsigned short)red >> 3) << 11) & 0xF800;

	return (unsigned short)(R | G | B);
}

unsigned int MeshVertex::ARGB8565_ARGB8888(unsigned short color, unsigned char alpha)
{
	// changed order of variable names
	unsigned char b = (((color)& 0x001F) << 3);
	unsigned char g = (((color)& 0x07E0) >> 3); // Fixed: shift >> 5 and << 2
	unsigned char r = (((color)& 0xF800) >> 8); // shift >> 11 and << 3

	return ((alpha << 24) | (r << 16) | (g << 8) | (b << 0));
}

//unsigned short MeshVertex::Float32ToFloat16(float f)
//{
//	unsigned int Result;
//
//	unsigned int IValue = ((unsigned int*)(&f))[0];
//	unsigned int Sign = (IValue & 0x80000000U) >> 16U;
//	IValue = IValue & 0x7FFFFFFFU;      // Hack off the sign
//
//	if (IValue > 0x47FFEFFFU)
//	{
//		// The number is too large to be represented as a half.  Saturate to infinity.
//		Result = 0x7FFFU;
//	}
//	else
//	{
//		if (IValue < 0x38800000U)
//		{
//			// The number is too small to be represented as a normalized half.
//			// Convert it to a denormalized value.
//			unsigned int Shift = 113U - (IValue >> 23U);
//			IValue = (0x800000U | (IValue & 0x7FFFFFU)) >> Shift;
//		}
//		else
//		{
//			// Rebias the exponent to represent the value as a normalized half.
//			IValue += 0xC8000000U;
//		}
//
//		Result = ((IValue + 0x0FFFU + ((IValue >> 13U) & 1U)) >> 13U) & 0x7FFFU;
//	}
//	return (unsigned int)(Result | Sign);
//}

void MeshVertex::Orthonormlize(
	const FBXSDK_NAMESPACE::FbxVector4& N, 
	FBXSDK_NAMESPACE::FbxVector4& B, 
	FBXSDK_NAMESPACE::FbxVector4& T)
{
	//N must be normalized.
	FBXSDK_NAMESPACE::FbxVector4 T0 = T - N*(N.DotProduct(T));
	FBXSDK_NAMESPACE::FbxVector4 B0 = B - N*(N.DotProduct(B)) - T0*(T0.DotProduct(B) / T0.DotProduct(T0));

	B = B0;
	T = T0;
}

bool MeshVertex::ComputeParity(
	FBXSDK_NAMESPACE::FbxVector4& N,
	FBXSDK_NAMESPACE::FbxVector4& B,
	FBXSDK_NAMESPACE::FbxVector4& T)
{
	FBXSDK_NAMESPACE::FbxVector4 N0 = T.CrossProduct(B);
	//N0.Normalize();
	//N.Normalize();
	double angle = N.DotProduct(N0);
	return angle > 0;
}

double MeshVertex::ComputeTriangleArea(
	FBXSDK_NAMESPACE::FbxVector4& A,
	FBXSDK_NAMESPACE::FbxVector4& B,
	FBXSDK_NAMESPACE::FbxVector4& C)
{
	FBXSDK_NAMESPACE::FbxVector4 a = A - B;
	FBXSDK_NAMESPACE::FbxVector4 b = C - B;

	double area = a.CrossProduct(b).Length()*0.5;
	return area;
}

void MeshVertex::computeTangents_V2(
	const std::vector<float>& positions,
	const std::vector<float>& uvs,
	const std::vector<float>& normals,
	const std::vector<unsigned int>& smoothingGroup,
	const std::vector<float>& outTangents)
{
	size_t triangleCount = smoothingGroup.size();
	for (size_t s = 0; s < triangleCount; ++s)
	{
		FBXSDK_NAMESPACE::FbxVector4 p0(positions[s * 9 + 0], positions[s * 9 + 1], positions[s * 9 + 2]);
		FBXSDK_NAMESPACE::FbxVector4 p1(positions[s * 9 + 3], positions[s * 9 + 4], positions[s * 9 + 5]);
		FBXSDK_NAMESPACE::FbxVector4 p2(positions[s * 9 + 6], positions[s * 9 + 7], positions[s * 9 + 8]);

		FBXSDK_NAMESPACE::FbxVector2 uv0(uvs[s * 6 + 0], uvs[s * 6 + 1]);
		FBXSDK_NAMESPACE::FbxVector2 uv1(uvs[s * 6 + 2], uvs[s * 6 + 3]);
		FBXSDK_NAMESPACE::FbxVector2 uv2(uvs[s * 6 + 4], uvs[s * 6 + 5]);

		//In FBX normal is computed by CCW. CCW is front. Normal point out.
		//FBXSDK_NAMESPACE::FbxVector4 n0(normals[s * 9 + 0], normals[s * 9 + 1], normals[s * 9 + 2]);

		FBXSDK_NAMESPACE::FbxVector4 Q1 = p1 - p0;
		FBXSDK_NAMESPACE::FbxVector4 Q2 = p2 - p0;

		FBXSDK_NAMESPACE::FbxVector4 faceNormal = Q1.CrossProduct(Q2);
		
		double s1 = uv1[0] - uv0[0];
		double t1 = uv1[1] - uv0[1];
		double s2 = uv2[0] - uv0[0];
		double t2 = uv2[1] - uv0[1];

		double k = s1*t2 - s2*t1;
		FBXSDK_NAMESPACE::FbxVector4 t0 = Q1*t2 - Q2*t1;
		FBXSDK_NAMESPACE::FbxVector4 b0 = Q2*s1 - Q1*s2;

		FBXSDK_NAMESPACE::FbxVector4 Tan(1.0, 0.0, 0.0);
		FBXSDK_NAMESPACE::FbxVector4 Bin(0.0, 1.0, 0.0);
		bool rightHandness = true;
		if (k != 0)
		{
			Tan = t0 / k;
			Bin = b0 / k;
			rightHandness = ComputeParity(faceNormal, b0, t0);
		}
        //dzlog_info("rightHandness %d",rightHandness);
		
		double angleA = acos(Q1.DotProduct(Q2) / (Q1.Length()*Q2.Length()));

		Q1 = p0 - p1;
		Q2 = p2 - p1;
		double angleB = acos(Q1.DotProduct(Q2) / (Q1.Length()*Q2.Length()));

		Q1 = p0 - p2;
		Q2 = p1 - p2;
		double angleC = acos(Q1.DotProduct(Q2) / (Q1.Length()*Q2.Length()));
        //dzlog_info("angleA %f angleB %f angleC %f",angleA,angleB,angleC);
		//Only vertices with the same smooth group and the same control point(position) can be merged together.
	}
}

void MeshVertex::computeTangents(
	vector<float>& positions, 
	vector<float>& uvs,
	vector<float>& normals,
	vector<int>& indices,
	vector<float>& outTangents)
{
	size_t vertexCount = positions.size() / 3;

	outTangents.resize(vertexCount*4);

	FBXSDK_NAMESPACE::FbxDouble3* tan1 = new FBXSDK_NAMESPACE::FbxDouble3[vertexCount * 2];
	FBXSDK_NAMESPACE::FbxDouble3* tan2 = tan1 + vertexCount;
	memset(tan1, 0, vertexCount * 2 * sizeof(FBXSDK_NAMESPACE::FbxDouble3));

	size_t triangleCount = indices.size() / 3;	

	for (size_t triangleIndex = 0; triangleIndex < triangleCount; triangleIndex++)
	{
		int i1 = indices[triangleIndex * 3 + 0];
		int i2 = indices[triangleIndex * 3 + 1];
		int i3 = indices[triangleIndex * 3 + 2];

		const FBXSDK_NAMESPACE::FbxDouble3 v1(positions[i1 * 3], positions[i1 * 3 + 1], positions[i1 * 3 + 2]);
		const FBXSDK_NAMESPACE::FbxDouble3 v2(positions[i2 * 3], positions[i2 * 3 + 1], positions[i2 * 3 + 2]);
		const FBXSDK_NAMESPACE::FbxDouble3 v3(positions[i3 * 3], positions[i3 * 3 + 1], positions[i3 * 3 + 2]);

		const FBXSDK_NAMESPACE::FbxDouble2 w1(uvs[i1 * 2], uvs[i1 * 2 + 1]);
		const FBXSDK_NAMESPACE::FbxDouble2 w2(uvs[i2 * 2], uvs[i2 * 2 + 1]);
		const FBXSDK_NAMESPACE::FbxDouble2 w3(uvs[i3 * 2], uvs[i3 * 2 + 1]);

		FBXSDK_NAMESPACE::FbxDouble3 v2_v1(v2[0] - v1[0], v2[1] - v1[1], v2[2] - v1[2]);
		FBXSDK_NAMESPACE::FbxDouble3 v3_v1(v3[0] - v1[0], v3[1] - v1[1], v3[2] - v1[2]);

		FBXSDK_NAMESPACE::FbxDouble2 w2_w1(w2[0] - w1[0], w2[1] - w1[1]);
		FBXSDK_NAMESPACE::FbxDouble2 w3_w1(w3[0] - w1[0], w3[1] - w1[1]);

		double r = 1.0 / max_0((w2_w1[0] * w3_w1[1] - w3_w1[0] * w2_w1[1]), 1e-6);

		FBXSDK_NAMESPACE::FbxDouble3 sdir(
			(w3_w1[1] * v2_v1[0] - w2_w1[1] * v3_v1[0]) * r,
			(w3_w1[1] * v2_v1[1] - w2_w1[1] * v3_v1[1]) * r,
			(w3_w1[1] * v2_v1[2] - w2_w1[1] * v3_v1[2]) * r);

		FBXSDK_NAMESPACE::FbxDouble3 tdir(
			(w2_w1[0] * v3_v1[0] - w3_w1[0] * v2_v1[0]) * r,
			(w2_w1[0] * v3_v1[1] - w3_w1[0] * v2_v1[1]) * r,
			(w2_w1[0] * v3_v1[2] - w3_w1[0] * v2_v1[2]) * r);

		tan1[i1][0] += sdir[0]; tan1[i1][1] += sdir[1]; tan1[i1][2] += sdir[2];
		tan1[i2][0] += sdir[0]; tan1[i2][1] += sdir[1]; tan1[i2][2] += sdir[2];
		tan1[i3][0] += sdir[0]; tan1[i3][1] += sdir[1]; tan1[i3][2] += sdir[2];

		tan2[i1][0] += tdir[0]; tan2[i1][1] += tdir[1]; tan2[i1][2] += tdir[2];
		tan2[i2][0] += tdir[0]; tan2[i2][1] += tdir[1]; tan2[i2][2] += tdir[2];
		tan2[i3][0] += tdir[0]; tan2[i3][1] += tdir[1]; tan2[i3][2] += tdir[2];
	}

	for (size_t i = 0; i < vertexCount; i++)
	{
		const FBXSDK_NAMESPACE::FbxVector4 n = FBXSDK_NAMESPACE::FbxVector4(*(reinterpret_cast<FBXSDK_NAMESPACE::FbxDouble3*>(&normals[i * 3])));
		const FBXSDK_NAMESPACE::FbxVector4 t = FBXSDK_NAMESPACE::FbxVector4(tan1[i]);

		//Gram-Schmidt orthogonalize
		FBXSDK_NAMESPACE::FbxVector4 tangent = t - n * n.DotProduct(t);
		tangent.Normalize();

		//Calculate handedness
		double binormalProjection = n.CrossProduct(t).DotProduct(tan2[i]);
		tangent[3] = (binormalProjection < 0.0) ? -1.0 : 1.0;

		outTangents[i * 4 + 0] = (float)tangent[0];
		outTangents[i * 4 + 1] = (float)tangent[1];
		outTangents[i * 4 + 2] = (float)tangent[2];
		outTangents[i * 4 + 3] = (float)tangent[3];
	}

	delete[] tan1;
}
