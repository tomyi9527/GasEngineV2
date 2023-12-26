#include "stdafx.h"
#include "PmxConverter.h"
#include "JsonToBin_V4.h"
#include "Material.h"
#include "FBXSceneStructureExporter_V4.h"
#include "JSONFileWriter.h"
#include "SceneNode.h"
#include "NeonateVertexCompression_V4.h"
#include "Common/Utils.h"

PmxConverter::PmxConverter()
{
}


PmxConverter::~PmxConverter()
{
}

//void PmxConverter::getTopologicalIndices(const std::vector<int>& indices, std::vector<unsigned int>& topologyIndices)
//{
//	std::map<int, vector<int> > dictionary;
//
//	int polygonCount = (int)indices.size() / 3;
//	for (int polygonIndex = 0; polygonIndex < polygonCount; ++polygonIndex)
//	{
//		int polygonSize = 3;
//		
//		for (int vertexIndex = 1; vertexIndex <= polygonSize; ++vertexIndex)
//		{
//			int controlPointIndex0 = indices[polygonIndex * polygonSize + (vertexIndex - 1) % polygonSize];
//			int controlPointIndex1 = indices[polygonIndex * polygonSize + vertexIndex % polygonSize];
//
//			bool foundInSet0 = false;
//			std::map<int, vector<int> >::iterator iter0 = dictionary.find(controlPointIndex0);
//			if (iter0 != dictionary.end())
//			{
//				for (unsigned int i = 0; i < iter0->second.size(); ++i)
//				{
//					if (iter0->second[i] == controlPointIndex1)
//					{
//						foundInSet0 = true;
//						break;
//					}
//				}
//
//				if (!foundInSet0)
//				{
//					iter0->second.push_back(controlPointIndex1);
//				}
//			}
//			else
//			{
//				vector<int> items;
//				items.push_back(controlPointIndex1);
//				dictionary[controlPointIndex0] = items;
//			}
//
//			//<
//			bool foundInSet1 = false;
//			std::map<int, vector<int> >::iterator iter1 = dictionary.find(controlPointIndex1);
//			if (iter1 != dictionary.end())
//			{
//				for (unsigned int i = 0; i < iter1->second.size(); ++i)
//				{
//					if (iter1->second[i] == controlPointIndex0)
//					{
//						foundInSet1 = true;
//						break;
//					}
//				}
//
//				if (!foundInSet1)
//				{
//					iter1->second.push_back(controlPointIndex0);
//				}
//			}
//			else
//			{
//				vector<int> items;
//				items.push_back(controlPointIndex0);
//				dictionary[controlPointIndex1] = items;
//			}
//
//			if ((!foundInSet0 && foundInSet1) || (foundInSet0 && !foundInSet1))
//			{
//				FBXSDK_printf("%s", "Find topological indices failed!\n");
//			}
//
//			if (!foundInSet0 && !foundInSet1)
//			{
//				topologyIndices.push_back((unsigned int)controlPointIndex0);
//				topologyIndices.push_back((unsigned int)controlPointIndex1);
//			}
//			//<
//		}
//	}
//}

bool PmxConverter::convert(
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

	if(fp)
	{
		fseek(fp, 0, SEEK_END);
		size_t fileSize = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		void* binaryData = malloc(fileSize);
		fread(binaryData, fileSize, 1, fp);
		fclose(fp);

		PmxReader* pmx = new PmxReader(binaryData, fileSize);
		unsigned int vertexCount = (unsigned int)pmx->VertexList.size();
		/////////////////////////////////////////////////////

		std::vector<VERTEX_LAYER_TYPE> vertexElements;

		//Position
		FBXSDK_NAMESPACE::FbxDouble3 posBboxMin(FLT_MAX, FLT_MAX, FLT_MAX);
		FBXSDK_NAMESPACE::FbxDouble3 posBboxMax(FLT_MIN, FLT_MIN, FLT_MIN);
		uint8_t* outPosEncodingBuffer = new uint8_t[vertexCount * sizeof(float) * 3];
		unsigned int outPosEncodingSize = vertexCount * (unsigned int)sizeof(float) * 3;
		unsigned int outPosEncodingFlag = 0;
		vertexElements.push_back(VL_POSITION);

		//Normal
		uint8_t* outNormalEncodingBuffer = new uint8_t[vertexCount * sizeof(float) * 3];
		unsigned int outNormalEncodingSize = vertexCount * (unsigned int)sizeof(float) * 3;
		unsigned int outNormalEncodingFlag = 0;
		vertexElements.push_back(VL_NORMAL0);

		//UV		
		uint8_t* outUVEncodingBuffer = new uint8_t[vertexCount * sizeof(float) * 2];
		unsigned int outUVEncodingSize = vertexCount * (unsigned int)sizeof(float) * 2;
		unsigned int outUVEncodingFlag = 0;
		vertexElements.push_back(VL_UV0);

		//BW & BI outputs
		unsigned int skinDataSize = vertexCount * 4 * sizeof(float);
		uint8_t* outBWEncodingBuffer = new uint8_t[skinDataSize];
		unsigned int outBWEncodingSize = skinDataSize;
		unsigned int outBWEncodingFlag = 0;

		uint8_t* outBIEncodingBuffer = new uint8_t[skinDataSize];
		unsigned int outBIEncodingSize = skinDataSize;
		unsigned int outBIEncodingFlag = 0;

		for (size_t i = 0; i < pmx->VertexList.size(); ++i)
		{
			const PmxReader::PmxVertex& vertex = pmx->VertexList[i];

			((float*)outPosEncodingBuffer)[3 * i + 0] = vertex.Position.X;
			((float*)outPosEncodingBuffer)[3 * i + 1] = vertex.Position.Y;
			((float*)outPosEncodingBuffer)[3 * i + 2] = -vertex.Position.Z;

			((float*)outNormalEncodingBuffer)[3 * i + 0] = vertex.Normal.X;
			((float*)outNormalEncodingBuffer)[3 * i + 1] = vertex.Normal.Y;			
			((float*)outNormalEncodingBuffer)[3 * i + 2] = -vertex.Normal.Z; //left handness to right

			((float*)outUVEncodingBuffer)[2 * i + 0] = vertex.UV.X;
			((float*)outUVEncodingBuffer)[2 * i + 1] = 1.0f - vertex.UV.Y;

			((float*)outBWEncodingBuffer)[4 * i + 0] = (float)vertex.Weight[0].Value;
			((float*)outBWEncodingBuffer)[4 * i + 1] = (float)vertex.Weight[1].Value;
			((float*)outBWEncodingBuffer)[4 * i + 2] = (float)vertex.Weight[2].Value;
			((float*)outBWEncodingBuffer)[4 * i + 3] = (float)vertex.Weight[3].Value;

			((float*)outBIEncodingBuffer)[4 * i + 0] = (float)vertex.Weight[0].Bone < 0 ? 0.0f : (float)vertex.Weight[0].Bone;
			((float*)outBIEncodingBuffer)[4 * i + 1] = (float)vertex.Weight[1].Bone < 0 ? 0.0f : (float)vertex.Weight[1].Bone;
			((float*)outBIEncodingBuffer)[4 * i + 2] = (float)vertex.Weight[2].Bone < 0 ? 0.0f : (float)vertex.Weight[2].Bone;
			((float*)outBIEncodingBuffer)[4 * i + 3] = (float)vertex.Weight[3].Bone < 0 ? 0.0f : (float)vertex.Weight[3].Bone;
		}

		//Index
		unsigned int indexCount = (unsigned int)pmx->FaceList.size();
		unsigned int  triangleCount = indexCount / 3;
		uint8_t* outIndexEncodingBuffer = new uint8_t[indexCount * sizeof(unsigned int)];
		unsigned int outIndexEncodingSize = indexCount * (unsigned int)sizeof(unsigned int);
		unsigned int outIndexEncodingFlag = 0;
		for (unsigned int s = 0; s < triangleCount; ++s)
		{
			((unsigned int*)outIndexEncodingBuffer)[3 * s + 2] = pmx->FaceList[s * 3 + 0];
			((unsigned int*)outIndexEncodingBuffer)[3 * s + 1] = pmx->FaceList[s * 3 + 1];
			((unsigned int*)outIndexEncodingBuffer)[3 * s + 0] = pmx->FaceList[s * 3 + 2];
		}

		FBXSDK_NAMESPACE::FbxDouble3 uvBboxMin(FLT_MAX, FLT_MAX, FLT_MAX);
		FBXSDK_NAMESPACE::FbxDouble3 uvBboxMax(FLT_MIN, FLT_MIN, FLT_MIN);
		for (unsigned int i = 0; i < indexCount; ++i)
		{
			int index = pmx->FaceList[i];
			const PmxReader::PmxVertex& vertex = pmx->VertexList[index];

			if (vertex.Position.X < posBboxMin.mData[0])
				posBboxMin.mData[0] = vertex.Position.X;

			if (vertex.Position.Y < posBboxMin.mData[1])
				posBboxMin.mData[1] = vertex.Position.Y;

			if (-vertex.Position.Z < posBboxMin.mData[2])
				posBboxMin.mData[2] = -vertex.Position.Z;

			if (vertex.Position.X > posBboxMax.mData[0])
				posBboxMax.mData[0] = vertex.Position.X;

			if (vertex.Position.Y > posBboxMax.mData[1])
				posBboxMax.mData[1] = vertex.Position.Y;

			if (-vertex.Position.Z > posBboxMax.mData[2])
				posBboxMax.mData[2] = -vertex.Position.Z;

			if (vertex.UV.X < uvBboxMin.mData[0])
				uvBboxMin.mData[0] = vertex.UV.X;

			if (1.0f - vertex.UV.Y < uvBboxMin.mData[1])
				uvBboxMin.mData[1] = 1.0f - vertex.UV.Y;

			if (vertex.UV.X > uvBboxMax.mData[0])
				uvBboxMax.mData[0] = vertex.UV.X;

			if (1.0f - vertex.UV.Y > uvBboxMax.mData[1])
				uvBboxMax.mData[1] = 1.0f - vertex.UV.Y;
		}
		//memcpy(outIndexEncodingBuffer, &(pmx->FaceList[0]), indexCount * sizeof(unsigned int));

		//Topological Index
		std::vector<unsigned int> topologicalIndices;
		topologicalIndices.reserve(indexCount);
		getTopologicalIndices(pmx->FaceList, topologicalIndices);

		uint8_t* outTopologicalIndexEncodingBuffer = new uint8_t[topologicalIndices.size() * sizeof(int)];
		unsigned int outTopologicalIndexEncodingSize = (unsigned int)(topologicalIndices.size() * sizeof(int));
		memcpy(outTopologicalIndexEncodingBuffer, &topologicalIndices[0], outTopologicalIndexEncodingSize);
		unsigned int outTopologicalIndexEncodingFlag = 0;

		//Submesh
		unsigned int subMeshCount = (unsigned int)pmx->MaterialList.size();
		unsigned int outSubMeshEncodingSize = sizeof(unsigned int) * 16 * subMeshCount;	//start1 count1, min_x_y_z3, max_x_y_z3
		uint8_t* outSubMeshEncodingBuffer = new uint8_t[outSubMeshEncodingSize];
		unsigned int outSubMeshEncodingFlag = 1;

		memset(outSubMeshEncodingBuffer, 0, outSubMeshEncodingSize);

		unsigned int indexStart = 0;
		for (size_t materialIndex = 0; materialIndex < pmx->MaterialList.size(); ++materialIndex)
		{
			PmxReader::PmxMaterial& material = pmx->MaterialList[materialIndex];

			((unsigned int*)outSubMeshEncodingBuffer)[16 * materialIndex + 0] = indexStart;
			((unsigned int*)outSubMeshEncodingBuffer)[16 * materialIndex + 1] = material.FaceCount / 3;
			
			unsigned int indexEnd = indexStart + material.FaceCount;

			FBXSDK_NAMESPACE::FbxDouble3 bboxMin(FLT_MAX, FLT_MAX, FLT_MAX);
			FBXSDK_NAMESPACE::FbxDouble3 bboxMax(FLT_MIN, FLT_MIN, FLT_MIN);
			for (size_t i = indexStart; i < indexEnd; ++i)
			{
				unsigned int vertexIndex = ((unsigned int*)outIndexEncodingBuffer)[i];

				float x = ((float*)outPosEncodingBuffer)[3 * vertexIndex + 0];
				float y = ((float*)outPosEncodingBuffer)[3 * vertexIndex + 1];
				float z = ((float*)outPosEncodingBuffer)[3 * vertexIndex + 2];

				if (x < bboxMin.mData[0])
					bboxMin.mData[0] = x;

				if (y < bboxMin.mData[1])
					bboxMin.mData[1] = y;

				if (z < bboxMin.mData[2])
					bboxMin.mData[2] = z;

				if (x > bboxMax.mData[0])
					bboxMax.mData[0] = z;

				if (y > bboxMax.mData[1])
					bboxMax.mData[1] = y;

				if (z > bboxMax.mData[2])
					bboxMax.mData[2] = z;
			}

			((float*)outSubMeshEncodingBuffer)[16 * materialIndex + 2] = (float)bboxMin.mData[0];
			((float*)outSubMeshEncodingBuffer)[16 * materialIndex + 3] = (float)bboxMin.mData[1];
			((float*)outSubMeshEncodingBuffer)[16 * materialIndex + 4] = (float)bboxMin.mData[2];
			((float*)outSubMeshEncodingBuffer)[16 * materialIndex + 5] = (float)bboxMax.mData[0];
			((float*)outSubMeshEncodingBuffer)[16 * materialIndex + 6] = (float)bboxMax.mData[1];
			((float*)outSubMeshEncodingBuffer)[16 * materialIndex + 7] = (float)bboxMax.mData[2];

			indexStart = indexEnd;
		}

		buildSceneStructure(pmx);

		std::vector<BONE_>* effectiveBones = createSkeleton(pmx->BoneList);

		//Blend shape
		std::vector<MORPH_DATA_V2>* morphTargetsData = new std::vector<MORPH_DATA_V2>();
		getMorph(pmx->MorphList, (float*)outPosEncodingBuffer, vertexCount, posBboxMin, posBboxMax, morphTargetsData);

		FbxUInt64 parentNodeID = 1;
		FbxUInt64 nodeID = 2;

		JsonToBin_V4* writer = new JsonToBin_V4();
		writer->openExportFile(workingDirectory + "MMD_Mesh.2.mesh.bin");

		float posBoundingBoxMin[3] = { (float)posBboxMin[0], (float)posBboxMin[1], (float)posBboxMin[2] };
		float posBoundingBoxMax[3] = { (float)posBboxMax[0], (float)posBboxMax[1], (float)posBboxMax[2] };

		#ifdef _MSC_VER
		writer->writeObjectBin(
			parentNodeID,
			nodeID,
			MESH_TYPE,
			"MMD_Mesh",
			(unsigned int)vertexCount,
			vertexElements,
			effectiveBones,
			\
			subMeshCount,
			outSubMeshEncodingBuffer,
			outSubMeshEncodingSize,
			outSubMeshEncodingFlag,
			\
			(unsigned int)(indexCount / 3),
			outIndexEncodingBuffer,
			outIndexEncodingSize,
			outIndexEncodingFlag,
			\
			(unsigned int)(topologicalIndices.size() / 2),
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
			NULL, //outTangentEncodingBuffer
			NULL, //outTangentEncodingSize
			NULL, //outTangentEncodingFlag
			\
			NULL, //outVertexColorEncodingBuffer
			NULL, //outVertexColorEncodingSize
			NULL, //outVertexColorEncodingFlag
			\
			uvBboxMin,
			uvBboxMax,
			outUVEncodingBuffer,
			outUVEncodingSize,
			outUVEncodingFlag,
			\
			FBXSDK_NAMESPACE::FbxDouble3(0.0, 0.0, 0.0),
			FBXSDK_NAMESPACE::FbxDouble3(0.0, 0.0, 0.0),
			NULL, 0, 0,
			\
			outBWEncodingBuffer,
			outBWEncodingSize,
			outBWEncodingFlag,
			\
			outBIEncodingBuffer,
			outBIEncodingSize,
			outBIEncodingFlag
		);
		#else
		FBXSDK_NAMESPACE::FbxDouble3 uv1Bbox(0.0, 0.0, 0.0);
		writer->writeObjectBin(
			parentNodeID,
			nodeID,
			MESH_TYPE,
			"MMD_Mesh",
			(unsigned int)vertexCount,
			vertexElements,
			effectiveBones,
			\
			subMeshCount,
			outSubMeshEncodingBuffer,
			outSubMeshEncodingSize,
			outSubMeshEncodingFlag,
			\
			(unsigned int)(indexCount / 3),
			outIndexEncodingBuffer,
			outIndexEncodingSize,
			outIndexEncodingFlag,
			\
			(unsigned int)(topologicalIndices.size() / 2),
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
			NULL, //outTangentEncodingBuffer
			NULL, //outTangentEncodingSize
			NULL, //outTangentEncodingFlag
			\
			NULL, //outVertexColorEncodingBuffer
			NULL, //outVertexColorEncodingSize
			NULL, //outVertexColorEncodingFlag
			\
			uvBboxMin,
			uvBboxMax,
			outUVEncodingBuffer,
			outUVEncodingSize,
			outUVEncodingFlag,
			\
			uv1Bbox,
			uv1Bbox,
			NULL, 0, 0,
			\
			outBWEncodingBuffer,
			outBWEncodingSize,
			outBWEncodingFlag,
			\
			outBIEncodingBuffer,
			outBIEncodingSize,
			outBIEncodingFlag
		);		
		#endif

		int morphTargetCount = (unsigned int)morphTargetsData->size();
		if (morphTargetCount > 0)
		{
			NeonateVertexCompression_V4::optimizationAndWriteMorphTarget(
				writer,
				nodeID,
				(unsigned int)vertexCount,
				optimizationFlag,
				morphTargetsData);
		}

		writer->closeExportFile();
		delete writer;

		//////////////////////////////////////////////////////
		free(binaryData);
		binaryData = NULL;

		std::vector<std::string> convertedFiles;
		convertedFiles.push_back("scene.json");
		convertedFiles.push_back("structure.json");
		convertedFiles.push_back("MMD_Mesh.2.mesh.bin");

		std::string convertedFilePath = workingDirectory + "converted.json";
		convertedFiles.push_back(convertedFilePath);

		convertStructureFile(filePath, workingDirectory, (int)pmx->MaterialList.size(), sceneRoot, posBboxMin, posBboxMax, mVersion);

		convertSceneFile(workingDirectory, pmx, convertedFiles, background, mVersion);

		writeConvertedFile(convertedFilePath, convertedFiles, triangleCount, triangleCount, vertexCount);

		return true;
	}
	else
	{
		return false;
	}
}

void PmxConverter::getMorph(
	std::vector<PmxReader::PmxMorph>& morphs,
	float* originalPositions,
	unsigned int vertexCount,
	FBXSDK_NAMESPACE::FbxDouble3 posBboxMin,
	FBXSDK_NAMESPACE::FbxDouble3 posBboxMax,
	std::vector<MORPH_DATA_V2>* morphTargetsData)
{
	for (size_t s = 0; s < morphs.size(); ++s)
	{
		MORPH_DATA_V2 morphData;
		morphData.morphTargetName = morphs[s].Name;
		morphData.uniqueID = 20000 + s;
		morphData.vertexCount = vertexCount;

		morphData.bboxMin = posBboxMin;
		morphData.bboxMax = posBboxMax;

		morphData.positions.resize(3 * vertexCount);
		memcpy(&morphData.positions[0], originalPositions, 3 * vertexCount * sizeof(float));
		
		if (morphs[s].Kind == PmxReader::OffsetKind::Vertex)
		{
			for (size_t i = 0; i < morphs[s].OffsetList.size(); ++i)
			{
				PmxReader::PmxVertexMorph* vertex = (PmxReader::PmxVertexMorph*)morphs[s].OffsetList[i];

				if (vertex->Index != -1)
				{
					float x = morphData.positions[vertex->Index * 3 + 0] + vertex->Offset.X;
					float y = morphData.positions[vertex->Index * 3 + 1] + vertex->Offset.Y;
					float z = morphData.positions[vertex->Index * 3 + 2] - vertex->Offset.Z;

					morphData.positions[vertex->Index * 3 + 0] = x;
					morphData.positions[vertex->Index * 3 + 1] = y;
					morphData.positions[vertex->Index * 3 + 2] = z;

					if (x < posBboxMin.mData[0])
						posBboxMin.mData[0] = z;

					if (y < posBboxMin.mData[1])
						posBboxMin.mData[1] = y;

					if (z < posBboxMin.mData[2])
						posBboxMin.mData[2] = z;

					if (x > posBboxMax.mData[0])
						posBboxMax.mData[0] = x;

					if (y > posBboxMax.mData[1])
						posBboxMax.mData[1] = y;

					if (z > posBboxMax.mData[2])
						posBboxMax.mData[2] = z;
				}
				else
				{
					FBXSDK_printf("Error: Unsupported morph vertex index.");
				}
			}
		}
		else if (morphs[s].Kind == PmxReader::OffsetKind::Group)
		{
			FBXSDK_printf("Error: Unsupported morph vertex format.");
		}
		else
		{
			FBXSDK_printf("Error: Unsupported morph vertex format.");
		}

		morphTargetsData->push_back(morphData);
	}
}

void PmxConverter::buildSceneStructure(PmxReader* pmx)
{
	std::vector<PmxReader::PmxBone>& bones = pmx->BoneList;
	std::vector<PmxReader::PmxRigidBody>& rigidBodies = pmx->BodyList;
	std::vector<PmxReader::PmxJoint>& joints = pmx->JointList;

	sceneRoot = new SceneNode();
	sceneRoot->name = "MMD_Skeleton_Root";
	sceneRoot->id = 3;

	nodes.clear();

	nodes.resize(bones.size());
	for (size_t s = 0; s < bones.size(); ++s)
	{
		nodes[s] = new SceneNode();

		const PmxReader::PmxBone& bone = bones[s];
		nodes[s]->name = bone.Name;
		nodes[s]->id = (int)s + 10000;

		if (bone.Parent != -1)
		{
			const PmxReader::PmxBone& parentBone = bones[bone.Parent];
			float x = bone.Position.X - parentBone.Position.X;
			float y = bone.Position.Y - parentBone.Position.Y;
			float z = (-bone.Position.Z) - (-parentBone.Position.Z);
			nodes[s]->position.Set(x, y, z);
		}
		else
		{
			nodes[s]->position.Set(bone.Position.X, bone.Position.Y, -bone.Position.Z);
		}

		if (bone.Parent == -1)
		{
			sceneRoot->addChild(nodes[s]);
		}
	}

	for (size_t s = 0; s < bones.size(); ++s)
	{
		const PmxReader::PmxBone& bone = bones[s];
		if (bone.Parent != -1)
		{
			SceneNode* parent = nodes[(int)bone.Parent];
			nodes[s]->setParent(parent);
		}
	}

	sceneRoot->update();

	//IK
	//Link Constraints
	sceneRoot->ikChains = NULL;
	sceneRoot->linkConstraints = NULL;
	for (size_t s = 0; s < bones.size(); ++s)
	{
		const PmxReader::PmxBone& bone = bones[s];
		if (bone.IK.Effector != -1)
		{
			if (sceneRoot->ikChains == NULL)
			{
				sceneRoot->ikChains = new std::vector<IKChain*>();
			}

			IKChain* chain = new IKChain();
			chain->effector = nodes[bone.IK.Effector];
			chain->target = nodes[s];
			chain->interation = bone.IK.Iteration;
			chain->maxAngle = bone.IK.MaxAngle;

			for (size_t k = 0; k < bone.IK.LinkList.size(); ++k)
			{
				const PmxReader::IKLink& link = bone.IK.LinkList[k];
				int index = link.Bone;
				SceneNode* chainNode = nodes[index];

				chainNode->ikParam = new IKParam();
				chainNode->ikParam->enable = true;

				chainNode->ikParam->lowerAngles[0] = link.Low.X;
				chainNode->ikParam->lowerAngles[1] = link.Low.Y;
				chainNode->ikParam->lowerAngles[2] = link.Low.Z;

				chainNode->ikParam->upperAngles[0] = link.High.X;
				chainNode->ikParam->upperAngles[1] = link.High.Y;
				chainNode->ikParam->upperAngles[2] = link.High.Z;

				if (link.IsLimit)
				{
					chainNode->ikParam->enableAngleLimitation = true;
					chainNode->ikParam->limitation.Set(1.0, 0.0, 0.0, 0.0);
				}
				else
				{
					chainNode->ikParam->enableAngleLimitation = false;
				}
				chain->chainList.push_back(chainNode);
			}

			sceneRoot->ikChains->push_back(chain);
		}

		//Link constraint
		if (bone.LinkConstraintAffectRotation || bone.LinkConstraintAffectTranslation)
		{
			if (sceneRoot->linkConstraints == NULL)
			{
				sceneRoot->linkConstraints = new std::vector<LinkConstraint*>();
			}

			LinkConstraint* constraint = new LinkConstraint();

			constraint->childNode = nodes[s];
			constraint->parentNode = nodes[bone.LinkConstraintParent];
			constraint->LinkConstraintAffectTranslation = bone.LinkConstraintAffectTranslation;
			constraint->LinkConstraintAffectRotation = bone.LinkConstraintAffectRotation;
			constraint->LinkConstraintAffectLocal = bone.LinkConstraintAffectLocal;
			constraint->LinkConstraintRatio = bone.LinkConstraintRatio;
			constraint->Priority = bone.Priority;

			sceneRoot->linkConstraints->push_back(constraint);
		}
	}

	//RigidBodies
	sceneRoot->physicsRigidBodies = NULL;
	if (rigidBodies.size() > 0)
	{
		std::vector<PhysicsRigidBody*>* physicsRigidBodies = new std::vector<PhysicsRigidBody*>();

		for (size_t s = 0; s < rigidBodies.size(); ++s)
		{
			PmxReader::PmxRigidBody& rb = rigidBodies[s];
			PhysicsRigidBody* prb = new PhysicsRigidBody();

			prb->name = rb.Name;

			if (rb.Bone >= 0 && rb.Bone < nodes.size())
			{
				prb->boneLinked = nodes[rb.Bone];
			}
			else
			{
				prb->boneLinked = NULL;
			}

			prb->groupIndex = rb.GroupIndex;
			prb->groupTarget = rb.GroupTarget;

			switch (rb.BoxType)
			{
			case 0:
				prb->shapeType = "Sphere";
				break;
			case 1:
				prb->shapeType = "Box";
				break;
			case 2:
				prb->shapeType = "Capsule";
				break;
			default:
				break;
			}

			prb->width = rb.BoxSize.X;
			prb->height = rb.BoxSize.Y;
			prb->depth = rb.BoxSize.Z;

			if (prb->boneLinked != NULL)
			{
				/*
				* RigidBody position parameter in PMX seems global position
				* while the one in PMD seems offset from corresponding bone.
				* So unify being offset.
				*/
				float x = rb.Position.X - bones[rb.Bone].Position.X;
				float y = rb.Position.Y - bones[rb.Bone].Position.Y;
				float z = (-rb.Position.Z) - (-bones[rb.Bone].Position.Z);
				prb->position.Set(x, y, z);
			}
			else
			{
				prb->position.Set(rb.Position.X, rb.Position.Y, -rb.Position.Z); //left to right
			}

			prb->rotation.Set(-rb.Rotation.X, -rb.Rotation.Y, rb.Rotation.Z); //left to right
			prb->mass = rb.Mass;
			prb->positionDamping = rb.PositionDamping;
			prb->rotationDamping = rb.RotationDamping;
			prb->restitution = rb.Restitution;
			prb->friction = rb.Friction;

			switch (rb.Mode)
			{
			case 0:
				prb->dynamicType = "FollowBone";
				break;
			case 1:
				prb->dynamicType = "Physics";
				break;
			case 2:
				prb->dynamicType = "PhysicsAndBone";
				break;
			default:
				prb->dynamicType = "";
				break;
			}

			physicsRigidBodies->push_back(prb);
		}

		sceneRoot->physicsRigidBodies = physicsRigidBodies;
	}

	//joints
	sceneRoot->physicsJoints = NULL;
	if (joints.size() > 0)
	{
		std::vector<PhysicsJoint*>* physicsJoints = new std::vector<PhysicsJoint*>();

		for (size_t s = 0; s < joints.size(); ++s)
		{
			const PmxReader::PmxJoint& jointData = joints[s];
			PhysicsJoint* joint = new PhysicsJoint();

			joint->name = jointData.Name;
			joint->rigidBodyIndexA = jointData.BodyA;
			joint->rigidBodyIndexB = jointData.BodyB;
			joint->position.Set(jointData.Position.X, jointData.Position.Y, -jointData.Position.Z);
			joint->rotation.Set(-jointData.Rotation.X, -jointData.Rotation.Y, jointData.Rotation.Z);
			joint->positionLowerLimitation.Set(jointData.Limit_MoveLow.X, jointData.Limit_MoveLow.Y, -jointData.Limit_MoveLow.Z);
			joint->positionUpperLimitation.Set(jointData.Limit_MoveHigh.X, jointData.Limit_MoveHigh.Y, -jointData.Limit_MoveHigh.Z);
			joint->rotationLowerLimitation.Set(-jointData.Limit_AngleHigh.X, -jointData.Limit_AngleHigh.Y, jointData.Limit_AngleLow.Z); //rotation range left to right
			joint->rotationUpperLimitation.Set(-jointData.Limit_AngleLow.X, -jointData.Limit_AngleLow.Y, jointData.Limit_AngleHigh.Z);
			joint->positionSpringStiffness.Set(jointData.SpConst_Move.X, jointData.SpConst_Move.Y, jointData.SpConst_Move.Z);
			joint->rotationSpringStiffness.Set(jointData.SpConst_Rotate.X, jointData.SpConst_Rotate.Y, jointData.SpConst_Rotate.Z);

			PhysicsRigidBody* rbA = (*sceneRoot->physicsRigidBodies)[joint->rigidBodyIndexA];
			PhysicsRigidBody* rbB = (*sceneRoot->physicsRigidBodies)[joint->rigidBodyIndexB];

			if (rbA->dynamicType != "FollowBone" && rbB->dynamicType == "PhysicsAndBone")
			{
				if (rbA->boneLinked != NULL && rbB->boneLinked != NULL && rbB->boneLinked->parent == rbA->boneLinked)
				{
					rbB->dynamicType = "Physics";
				}
			}

			physicsJoints->push_back(joint);
		}

		sceneRoot->physicsJoints = physicsJoints;
	}
}

std::vector<BONE_>* PmxConverter::createSkeleton(std::vector<PmxReader::PmxBone>& bones)
{
	std::vector<BONE_>* effectiveBones = new std::vector<BONE_>();
	for (size_t i = 0; i < nodes.size(); i++)
	{
		const SceneNode& sceneNode = *(nodes[i]);

		BONE_ bone;
		bone.id = sceneNode.id;
		bone.name = sceneNode.name;
		bone.modelWorldToBoneLocal = sceneNode.world.Inverse();

		effectiveBones->push_back(bone);
	}

	return effectiveBones;
}

void PmxConverter::convertStructureFile(
	const std::string& fullFilePath,
	const std::string& workingDirectory,
	int materialCount,
	SceneNode* sceneRoot,
	FBXSDK_NAMESPACE::FbxDouble3& posBboxMin,
	FBXSDK_NAMESPACE::FbxDouble3& posBboxMax,
	int version)
{
	std::string filePath = workingDirectory + "structure.json";

	JSONFileWriter* sceneStructureJSONWriter = new JSONFileWriter(false);
	sceneStructureJSONWriter->openExportFile(filePath);

	int jsonLevel = 0;
	sceneStructureJSONWriter->writeObjectInfo("{", jsonLevel);

	string newPath = fullFilePath;
	newPath = AssistantFunctions::replace_all_distinct(newPath, "\\", "\\\\");

	char buffer[__TEMP_BUFFER_FLOAT__];
	sprintf(buffer, "\"version\":%d,", version);
	sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 1);

	sceneStructureJSONWriter->writeObjectInfo("\"srcVersion\":\"MMD\",", jsonLevel + 1);
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
	sceneStructureJSONWriter->writeObjectInfo("\"rotation\":[0.0,0.0,0.0],", jsonLevel + 2);
	sceneStructureJSONWriter->writeObjectInfo("\"scaling\":[1.0,1.0,1.0],", jsonLevel + 2);

	sceneStructureJSONWriter->writeObjectInfo("\"materials\":", jsonLevel + 2);
	sceneStructureJSONWriter->writeObjectInfo("[", jsonLevel + 2);
	sceneStructureJSONWriter->writeObjectInfo("],", jsonLevel + 2);

	sceneStructureJSONWriter->writeObjectInfo("\"nodeAttr\":", jsonLevel + 2);
	sceneStructureJSONWriter->writeObjectInfo("{", jsonLevel + 2);
	sceneStructureJSONWriter->writeObjectInfo("},", jsonLevel + 2);

	sceneStructureJSONWriter->writeObjectInfo("\"children\":", jsonLevel + 2);
	sceneStructureJSONWriter->writeObjectInfo("[", jsonLevel + 2);

	writeMeshNode(sceneStructureJSONWriter, materialCount, posBboxMin, posBboxMax, jsonLevel + 3, sceneRoot == NULL);

	if (sceneRoot != NULL)
	{
		writeSceneNode_r(sceneStructureJSONWriter, sceneRoot, jsonLevel + 3, true);
	}

	sceneStructureJSONWriter->writeObjectInfo("]", jsonLevel + 2);
	////////////////////////////////////////////////////////////
	sceneStructureJSONWriter->writeObjectInfo("}", jsonLevel + 1);
	sceneStructureJSONWriter->writeObjectInfo("}", jsonLevel);

	//<
	sceneStructureJSONWriter->closeExportFile();
	delete sceneStructureJSONWriter;
	sceneStructureJSONWriter = NULL;
}

void PmxConverter::writeSceneNode_r(
	JSONFileWriter* sceneStructureJSONWriter,
	SceneNode* node, 
	int jsonLevel, 
	bool lastObject)
{
	char buffer[__TEMP_BUFFER_FLOAT__];

	sceneStructureJSONWriter->writeObjectInfo("{", jsonLevel);

	sprintf(buffer, "\"uniqueID\":%d,", node->id);
	sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 1);

	std::string guidString = newUUID();
	sprintf(buffer, "\"guid\":\"%s\",", guidString.c_str());
	sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 1);

	string nodeName = standardizeFileName(node->name);
	sceneStructureJSONWriter->writeObjectInfo("\"name\":\"" + nodeName + "\",", jsonLevel + 1);

	FbxVector4 tr = node->position;
	memset(buffer, 0, __TEMP_BUFFER_FLOAT__);
	sprintf(buffer, "\"translation\":[%f,%f,%f],", (isnan(tr[0]) ? 0.0f : tr[0]), (isnan(tr[1]) ? 0.0f : tr[1]), (isnan(tr[2]) ? 0.0f : tr[2]));
	sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 1);
	sceneStructureJSONWriter->writeObjectInfo("\"rotation\":[0.0,0.0,0.0],", jsonLevel + 1);
	sceneStructureJSONWriter->writeObjectInfo("\"scaling\":[1.0,1.0,1.0],", jsonLevel + 1);

	if (node->ikChains != NULL)
	{
		sceneStructureJSONWriter->writeObjectInfo("\"IKChains\":", jsonLevel + 1);
		sceneStructureJSONWriter->writeObjectInfo("[", jsonLevel + 1);
		for (size_t s = 0; s < node->ikChains->size(); ++s)
		{
			sceneStructureJSONWriter->writeObjectInfo("{", jsonLevel + 2);

			IKChain* chain = (*node->ikChains)[s];

			sprintf(buffer, "\"effector\":\"%s\",", standardizeFileName(chain->effector->name).c_str());
			sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 3);

			sprintf(buffer, "\"target\":\"%s\",", standardizeFileName(chain->target->name).c_str());
			sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 3);

			sprintf(buffer, "\"iteration\":%d,", chain->interation);
			sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 3);

			sprintf(buffer, "\"maxAngle\":%f,", chain->maxAngle);
			sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 3);

			sceneStructureJSONWriter->writeObjectInfo("\"chainLinks\":", jsonLevel + 3);
			sceneStructureJSONWriter->writeObjectInfo("[", jsonLevel + 3);

			std::string strChain;
			for (size_t k = 0; k < chain->chainList.size(); ++k)
			{
				SceneNode* node = chain->chainList[k];
				strChain += ("\"" + standardizeFileName(node->name) + "\"");
				if (k < chain->chainList.size() - 1)
					strChain += ",";
			}
			sceneStructureJSONWriter->writeObjectInfo(strChain, jsonLevel + 4);

			sceneStructureJSONWriter->writeObjectInfo("]", jsonLevel + 3);

			if (s == node->ikChains->size() - 1)
				sceneStructureJSONWriter->writeObjectInfo("}", jsonLevel + 2);
			else
				sceneStructureJSONWriter->writeObjectInfo("},", jsonLevel + 2);
		}
		sceneStructureJSONWriter->writeObjectInfo("],", jsonLevel + 1);
	}

	if (node->linkConstraints != NULL)
	{
		sceneStructureJSONWriter->writeObjectInfo("\"LinkConstraints\":", jsonLevel + 1);
		sceneStructureJSONWriter->writeObjectInfo("[", jsonLevel + 1);

		for (size_t s = 0; s < node->linkConstraints->size(); ++s)
		{
			sceneStructureJSONWriter->writeObjectInfo("{", jsonLevel + 2);

			LinkConstraint* constraint = (*node->linkConstraints)[s];

			sprintf(buffer, "\"child\":\"%s\",", standardizeFileName(constraint->childNode->name).c_str());
			sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 3);

			sprintf(buffer, "\"parent\":\"%s\",", standardizeFileName(constraint->parentNode->name).c_str());
			sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 3);

			sprintf(buffer, "\"ratio\":%f,", constraint->LinkConstraintRatio);
			sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 3);

			sprintf(buffer, "\"affectTranslation\":%s,", constraint->LinkConstraintAffectTranslation ? "true" : "false");
			sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 3);

			sprintf(buffer, "\"affectRotation\":%s,", constraint->LinkConstraintAffectRotation ? "true" : "false");
			sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 3);

			sprintf(buffer, "\"affectLocal\":%s,", constraint->LinkConstraintAffectLocal ? "true" : "false");
			sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 3);

			sprintf(buffer, "\"priority\":%d", constraint->Priority);
			sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 3);

			if (s == node->linkConstraints->size() - 1)
				sceneStructureJSONWriter->writeObjectInfo("}", jsonLevel + 2);
			else
				sceneStructureJSONWriter->writeObjectInfo("},", jsonLevel + 2);
		}

		sceneStructureJSONWriter->writeObjectInfo("],", jsonLevel + 1);
	}


	if (node->ikParam != NULL)
	{
		sceneStructureJSONWriter->writeObjectInfo("\"ikParam\":", jsonLevel + 1);
		sceneStructureJSONWriter->writeObjectInfo("{", jsonLevel + 1);

		sprintf(buffer, "\"enable\":%s,", node->ikParam->enable ? "true" : "false");
		sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 2);

		sprintf(buffer, "\"enableAngleLimitation\":%s,", node->ikParam->enableAngleLimitation ? "true" : "false");
		sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 2);

		sprintf(buffer, "\"limitation\":[%f,%f,%f],", node->ikParam->limitation[0], node->ikParam->limitation[1], node->ikParam->limitation[2]);
		sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 2);

		sprintf(buffer, "\"lowerAngles\":[%f,%f,%f],", node->ikParam->lowerAngles[0], node->ikParam->lowerAngles[1], node->ikParam->lowerAngles[2]);
		sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 2);

		sprintf(buffer, "\"upperAngles\":[%f,%f,%f]", node->ikParam->upperAngles[0], node->ikParam->upperAngles[1], node->ikParam->upperAngles[2]);
		sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 2);

		sceneStructureJSONWriter->writeObjectInfo("},", jsonLevel + 1);
	}

	if (node->physicsRigidBodies != NULL || node->physicsJoints != NULL)
	{
		sceneStructureJSONWriter->writeObjectInfo("\"physics\":", jsonLevel + 1);
		sceneStructureJSONWriter->writeObjectInfo("{", jsonLevel + 1);

		sceneStructureJSONWriter->writeObjectInfo("\"rigidBodies\":", jsonLevel + 2);
		sceneStructureJSONWriter->writeObjectInfo("[", jsonLevel + 2);
		if (node->physicsRigidBodies != NULL)
		{
			for (size_t s0 = 0; s0 < node->physicsRigidBodies->size(); ++s0)
			{
				PhysicsRigidBody* prb = (*(node->physicsRigidBodies))[s0];
				sceneStructureJSONWriter->writeObjectInfo("{", jsonLevel + 3);

				sprintf(buffer, "\"name\":\"%s\",", standardizeFileName(prb->name).c_str());
				sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 4);

				if (prb->boneLinked != NULL)
				{
					sprintf(buffer, "\"objectLinked\":\"%s\",", standardizeFileName(prb->boneLinked->name).c_str());
					sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 4);
				}
				else
				{
					sceneStructureJSONWriter->writeObjectInfo("\"objectLinked\":null,", jsonLevel + 4);
				}

				sprintf(buffer, "\"groupIndex\":%d,", prb->groupIndex);
				sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 4);

				sprintf(buffer, "\"groupTarget\":%d,", prb->groupTarget);
				sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 4);

				sprintf(buffer, "\"shapeType\":\"%s\",", prb->shapeType.c_str());
				sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 4);

				sprintf(buffer, "\"width\":%f,", prb->width);
				sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 4);

				sprintf(buffer, "\"height\":%f,", prb->height);
				sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 4);

				sprintf(buffer, "\"depth\":%f,", prb->depth);
				sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 4);

				sprintf(buffer, "\"position\":[%f,%f,%f],", prb->position[0], prb->position[1], prb->position[2]);
				sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 4);

				sprintf(buffer, "\"rotation\":[%f,%f,%f],", prb->rotation[0], prb->rotation[1], prb->rotation[2]);
				sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 4);

				sprintf(buffer, "\"mass\":%f,", prb->mass);
				sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 4);

				sprintf(buffer, "\"positionDamping\":%f,", prb->positionDamping);
				sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 4);

				sprintf(buffer, "\"rotationDamping\":%f,", prb->rotationDamping);
				sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 4);

				sprintf(buffer, "\"restitution\":%f,", prb->restitution);
				sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 4);

				sprintf(buffer, "\"friction\":%f,", prb->friction);
				sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 4);

				sprintf(buffer, "\"dynamicType\":\"%s\"", prb->dynamicType.c_str());
				sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 4);

				if (s0 == node->physicsRigidBodies->size() - 1)
				{
					sceneStructureJSONWriter->writeObjectInfo("}", jsonLevel + 3);
				}
				else
				{
					sceneStructureJSONWriter->writeObjectInfo("},", jsonLevel + 3);
				}
			}
		}
		sceneStructureJSONWriter->writeObjectInfo("],", jsonLevel + 2);

		sceneStructureJSONWriter->writeObjectInfo("\"joints\":", jsonLevel + 2);
		sceneStructureJSONWriter->writeObjectInfo("[", jsonLevel + 2);
		if (node->physicsJoints != NULL)
		{
			for (size_t s0 = 0; s0 < node->physicsJoints->size(); ++s0)
			{
				PhysicsJoint* joint = (*(node->physicsJoints))[s0];
				sceneStructureJSONWriter->writeObjectInfo("{", jsonLevel + 3);

				sprintf(buffer, "\"name\":\"%s\",", standardizeFileName(joint->name).c_str());
				sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 4);

				sprintf(buffer, "\"rigidBodyIndexA\":%d,", joint->rigidBodyIndexA);
				sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 4);

				sprintf(buffer, "\"rigidBodyIndexB\":%d,", joint->rigidBodyIndexB);
				sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 4);

				sprintf(buffer, "\"position\":[%f,%f,%f],", joint->position[0], joint->position[1], joint->position[2]);
				sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 4);

				sprintf(buffer, "\"rotation\":[%f,%f,%f],", joint->rotation[0], joint->rotation[1], joint->rotation[2]);
				sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 4);

				sprintf(buffer, "\"positionLowerLimitation\":[%f,%f,%f],", 
					joint->positionLowerLimitation[0], joint->positionLowerLimitation[1], joint->positionLowerLimitation[2]);
				sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 4);

				sprintf(buffer, "\"positionUpperLimitation\":[%f,%f,%f],", 
					joint->positionUpperLimitation[0], joint->positionUpperLimitation[1], joint->positionUpperLimitation[2]);
				sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 4);

				sprintf(buffer, "\"rotationLowerLimitation\":[%f,%f,%f],",
					joint->rotationLowerLimitation[0], joint->rotationLowerLimitation[1], joint->rotationLowerLimitation[2]);
				sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 4);

				sprintf(buffer, "\"rotationUpperLimitation\":[%f,%f,%f],",
					joint->rotationUpperLimitation[0], joint->rotationUpperLimitation[1], joint->rotationUpperLimitation[2]);
				sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 4);

				sprintf(buffer, "\"positionSpringStiffness\":[%f,%f,%f],",
					joint->positionSpringStiffness[0], joint->positionSpringStiffness[1], joint->positionSpringStiffness[2]);
				sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 4);

				sprintf(buffer, "\"rotationSpringStiffness\":[%f,%f,%f]",
					joint->rotationSpringStiffness[0], joint->rotationSpringStiffness[1], joint->rotationSpringStiffness[2]);
				sceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 4);

				if (s0 == node->physicsJoints->size() - 1)
				{
					sceneStructureJSONWriter->writeObjectInfo("}", jsonLevel + 3);
				}
				else
				{
					sceneStructureJSONWriter->writeObjectInfo("},", jsonLevel + 3);
				}
			}
		}
		sceneStructureJSONWriter->writeObjectInfo("]", jsonLevel + 2);

		sceneStructureJSONWriter->writeObjectInfo("},", jsonLevel + 1);
	}


	sceneStructureJSONWriter->writeObjectInfo("\"materials\":", jsonLevel + 1);
	sceneStructureJSONWriter->writeObjectInfo("[", jsonLevel + 1);
	sceneStructureJSONWriter->writeObjectInfo("],", jsonLevel + 1);

	sceneStructureJSONWriter->writeObjectInfo("\"nodeAttr\":", jsonLevel + 1);
	sceneStructureJSONWriter->writeObjectInfo("{", jsonLevel + 1);
	sceneStructureJSONWriter->writeObjectInfo("},", jsonLevel + 1);

	sceneStructureJSONWriter->writeObjectInfo("\"children\":", jsonLevel + 1);
	sceneStructureJSONWriter->writeObjectInfo("[", jsonLevel + 1);
	int childCount = node->getChildCount();
	for (int i = 0; i < childCount; ++i)
	{
		SceneNode* child = node->getChild(i);
		writeSceneNode_r(sceneStructureJSONWriter, child, jsonLevel + 2, i == childCount - 1);
	}
	sceneStructureJSONWriter->writeObjectInfo("]", jsonLevel + 1);

	if (lastObject)
	{
		sceneStructureJSONWriter->writeObjectInfo("}", jsonLevel);
	}
	else
	{
		sceneStructureJSONWriter->writeObjectInfo("},", jsonLevel);
	}
}

void PmxConverter::writeMeshNode(
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
	sceneStructureJSONWriter->writeObjectInfo("\"name\":\"MMD_Mesh.2.mesh.bin\",", jsonLevel + 2);
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

void PmxConverter::convertSceneFile(
	const std::string& workingDirectory, 
	PmxReader* pmx,
	std::vector<std::string>& convertedFiles,
	const std::string& background,
	int version)
{
	std::vector<std::string> textureFiles;
	std::vector<Material*> materials;
	std::vector<TextureMap*> textureMaps;

	for (size_t materialIndex = 0; materialIndex < pmx->MaterialList.size(); ++materialIndex)
	{
		PmxReader::PmxMaterial& material = pmx->MaterialList[materialIndex];
		Material* cache = new Material();
		cache->parsePMXMaterial((unsigned int)materialIndex, &material, textureMaps, textureFiles);
		materials.push_back(cache);
	}

	//////////////////////////////////////////////////////
	std::string sceneConfigFile = workingDirectory + "scene.json";
	//FBXSceneStructureExporter_V4::writeSceneConfigFile(sceneConfigFile, version, materials, textureMaps, textureFiles, background);

	int textureCount = (int)textureFiles.size();
	for (int i = 0; i < textureCount; ++i)
	{
		convertedFiles.push_back(textureFiles[i]);
	}
}