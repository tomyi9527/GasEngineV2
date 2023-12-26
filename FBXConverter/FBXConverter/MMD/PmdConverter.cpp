#include "stdafx.h"
#include "PmdConverter.h"
#include "PmxConverter.h"
#include "JsonToBin_V4.h"
#include "Material.h"
#include "FBXSceneStructureExporter_V4.h"
#include "JSONFileWriter.h"
#include "Common/Utils.h"
#include "SceneNode.h"
#include "NeonateVertexCompression_V4.h"

PmdConverter::PmdConverter()
{
}


PmdConverter::~PmdConverter()
{
}

bool PmdConverter::convert(
	const std::string& workingDirectory,
	const std::string& filePath,
	unsigned int optimizationFlag,
	std::string& background)
{
	mVersion = 4;

	pmd::PmdModel* pmd = pmd::PmdModel::LoadFromFile(filePath.c_str());

	std::string nameUTF8, commentUTF8;
	Platform_SHIFTJIS_UTF8(pmd->header.name, nameUTF8);
	Platform_SHIFTJIS_UTF8(pmd->header.comment, commentUTF8);

	unsigned int vertexCount = (unsigned int)pmd->vertices.size();
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

	for (size_t i = 0; i < pmd->vertices.size(); ++i)
	{
		const pmd::PmdVertex& vertex = pmd->vertices[i];

		((float*)outPosEncodingBuffer)[3 * i + 0] = vertex.position[0];
		((float*)outPosEncodingBuffer)[3 * i + 1] = vertex.position[1];
		((float*)outPosEncodingBuffer)[3 * i + 2] = -vertex.position[2];

		((float*)outNormalEncodingBuffer)[3 * i + 0] = vertex.normal[0];
		((float*)outNormalEncodingBuffer)[3 * i + 1] = vertex.normal[1];
		((float*)outNormalEncodingBuffer)[3 * i + 2] = -vertex.normal[2]; //left handness to right

		((float*)outUVEncodingBuffer)[2 * i + 0] = vertex.uv[0];
		((float*)outUVEncodingBuffer)[2 * i + 1] = 1.0f - vertex.uv[1];

		float weight = (float)vertex.bone_weight / 100.0f;
		((float*)outBWEncodingBuffer)[4 * i + 0] = weight;
		((float*)outBWEncodingBuffer)[4 * i + 1] = 1.0f - weight;
		((float*)outBWEncodingBuffer)[4 * i + 2] = 0.0f;
		((float*)outBWEncodingBuffer)[4 * i + 3] = 0.0f;

		((float*)outBIEncodingBuffer)[4 * i + 0] = (float)vertex.bone_index[0];
		((float*)outBIEncodingBuffer)[4 * i + 1] = (float)vertex.bone_index[1];
		((float*)outBIEncodingBuffer)[4 * i + 2] = 0.0f;
		((float*)outBIEncodingBuffer)[4 * i + 3] = 0.0f;
	}

	//Index
	unsigned int indexCount = (unsigned int)pmd->indices.size();
	unsigned int  triangleCount = indexCount / 3;
	uint8_t* outIndexEncodingBuffer = new uint8_t[indexCount * sizeof(unsigned int)];
	unsigned int outIndexEncodingSize = indexCount * (unsigned int)sizeof(unsigned int);
	unsigned int outIndexEncodingFlag = 0;
	for (unsigned int s = 0; s < triangleCount; ++s)
	{
		((unsigned int*)outIndexEncodingBuffer)[3 * s + 2] = pmd->indices[s * 3 + 0];
		((unsigned int*)outIndexEncodingBuffer)[3 * s + 1] = pmd->indices[s * 3 + 1];
		((unsigned int*)outIndexEncodingBuffer)[3 * s + 0] = pmd->indices[s * 3 + 2];
	}

	FBXSDK_NAMESPACE::FbxDouble3 uvBboxMin(FLT_MAX, FLT_MAX, FLT_MAX);
	FBXSDK_NAMESPACE::FbxDouble3 uvBboxMax(FLT_MIN, FLT_MIN, FLT_MIN);
	for (unsigned int i = 0; i < indexCount; ++i)
	{
		int index = pmd->indices[i];
		const pmd::PmdVertex& vertex = pmd->vertices[index];

		if (vertex.position[0] < posBboxMin.mData[0])
			posBboxMin.mData[0] = vertex.position[0];

		if (vertex.position[1] < posBboxMin.mData[1])
			posBboxMin.mData[1] = vertex.position[1];

		if (-vertex.position[2] < posBboxMin.mData[2])
			posBboxMin.mData[2] = -vertex.position[2];

		if (vertex.position[0] > posBboxMax.mData[0])
			posBboxMax.mData[0] = vertex.position[0];

		if (vertex.position[1] > posBboxMax.mData[1])
			posBboxMax.mData[1] = vertex.position[1];

		if (-vertex.position[2] > posBboxMax.mData[2])
			posBboxMax.mData[2] = -vertex.position[2];

		if (vertex.uv[0] < uvBboxMin.mData[0])
			uvBboxMin.mData[0] = vertex.uv[0];

		if (1.0f - vertex.uv[1] < uvBboxMin.mData[1])
			uvBboxMin.mData[1] = 1.0f - vertex.uv[1];

		if (vertex.uv[0] > uvBboxMax.mData[0])
			uvBboxMax.mData[0] = vertex.uv[0];

		if (1.0f - vertex.uv[1] > uvBboxMax.mData[1])
			uvBboxMax.mData[1] = 1.0f - vertex.uv[1];
	}

	//Topological Index
	std::vector<unsigned int> topologicalIndices;
	topologicalIndices.reserve(indexCount);
	PmxConverter::getTopologicalIndices(pmd->indices, topologicalIndices);

	uint8_t* outTopologicalIndexEncodingBuffer = new uint8_t[topologicalIndices.size() * sizeof(int)];
	unsigned int outTopologicalIndexEncodingSize = (unsigned int)(topologicalIndices.size() * sizeof(int));
	memcpy(outTopologicalIndexEncodingBuffer, &topologicalIndices[0], outTopologicalIndexEncodingSize);
	unsigned int outTopologicalIndexEncodingFlag = 0;

	//Submesh
	unsigned int subMeshCount = (unsigned int)pmd->materials.size();
	unsigned int outSubMeshEncodingSize = sizeof(unsigned int) * 16 * subMeshCount;
	uint8_t* outSubMeshEncodingBuffer = new uint8_t[outSubMeshEncodingSize];
	unsigned int outSubMeshEncodingFlag = 1;

	unsigned int indexStart = 0;
	for (size_t materialIndex = 0; materialIndex < pmd->materials.size(); ++materialIndex)
	{
		pmd::PmdMaterial& material = pmd->materials[materialIndex];

		((unsigned int*)outSubMeshEncodingBuffer)[16 * materialIndex + 0] = indexStart;
		((unsigned int*)outSubMeshEncodingBuffer)[16 * materialIndex + 1] = material.index_count / 3;

		unsigned int indexEnd = indexStart + material.index_count;

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
				bboxMax.mData[0] = x;

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

	buildSceneStructure(pmd);

	std::vector<BONE_>* effectiveBones = createSkeleton(pmd->bones);

	//Blend shape
	std::vector<MORPH_DATA_V2>* morphTargetsData = new std::vector<MORPH_DATA_V2>();
	getMorph(pmd->morphs, (float*)outPosEncodingBuffer, vertexCount, posBboxMin, posBboxMax, morphTargetsData);

	FbxUInt64 parentNodeID = 1;
	FbxUInt64 nodeID = 2;

	//Write file
	JsonToBin_V4* writer = new JsonToBin_V4();
	writer->openExportFile(workingDirectory + "MMD_Mesh.2.mesh.bin");

	float posBoundingBoxMin[3] = { (float)posBboxMin[0], (float)posBboxMin[1], (float)posBboxMin[2] };
	float posBoundingBoxMax[3] = { (float)posBboxMax[0], (float)posBboxMax[1], (float)posBboxMax[2] };

	#ifdef _MSC_VER
	writer->writeObjectBin(
		(unsigned int)parentNodeID,
		(unsigned int)nodeID,
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

		subMeshCount,
		outSubMeshEncodingBuffer,
		outSubMeshEncodingSize,
		outSubMeshEncodingFlag,

		(unsigned int)(indexCount / 3),
		outIndexEncodingBuffer,
		outIndexEncodingSize,
		outIndexEncodingFlag,

		(unsigned int)(topologicalIndices.size() / 2),
		outTopologicalIndexEncodingBuffer,
		outTopologicalIndexEncodingSize,
		outTopologicalIndexEncodingFlag,

		posBoundingBoxMin,
		posBoundingBoxMax,
		outPosEncodingBuffer,
		outPosEncodingSize,
		outPosEncodingFlag,

		outNormalEncodingBuffer,
		outNormalEncodingSize,
		outNormalEncodingFlag,

		NULL, //outTangentEncodingBuffer
		NULL, //outTangentEncodingSize
		NULL, //outTangentEncodingFlag

		NULL, //outVertexColorEncodingBuffer
		NULL, //outVertexColorEncodingSize
		NULL, //outVertexColorEncodingFlag

		uvBboxMin,
		uvBboxMax,
		outUVEncodingBuffer,
		outUVEncodingSize,
		outUVEncodingFlag,

		uv1Bbox,
		uv1Bbox,
		NULL, 0, 0,

		outBWEncodingBuffer,
		outBWEncodingSize,
		outBWEncodingFlag,

		outBIEncodingBuffer,
		outBIEncodingSize,
		outBIEncodingFlag);	
	#endif


	int morphTargetCount = (unsigned int)morphTargetsData->size();
	if (morphTargetCount > 0)
	{
		NeonateVertexCompression_V4::optimizationAndWriteMorphTarget(
			writer,
			(unsigned int)nodeID,
			(unsigned int)vertexCount,
			optimizationFlag,
			morphTargetsData);
	}

	writer->closeExportFile();
	delete writer;

	std::vector<std::string> convertedFiles;
	convertedFiles.push_back("scene.json");
	convertedFiles.push_back("structure.json");
	convertedFiles.push_back("MMD_Mesh.2.mesh.bin");

	std::string convertedFilePath = workingDirectory + "converted.json";
	convertedFiles.push_back(convertedFilePath);

	PmxConverter::convertStructureFile(
		filePath, 
		workingDirectory,
		(int)pmd->materials.size(), 
		sceneRoot, 
		posBboxMin, 
		posBboxMax, 
		mVersion);

	convertSceneFile(workingDirectory, pmd, convertedFiles, background, mVersion);

	writeConvertedFile(convertedFilePath, convertedFiles, triangleCount, triangleCount, vertexCount);

	return true;
}

void PmdConverter::getMorph(
	std::vector<pmd::PmdMorph>& morphs, 
	float* originalPositions, 
	unsigned int vertexCount,
	FBXSDK_NAMESPACE::FbxDouble3 posBboxMin,
	FBXSDK_NAMESPACE::FbxDouble3 posBboxMax,
	std::vector<MORPH_DATA_V2>* morphTargetsData)
{
	for (size_t s = 1; s < morphs.size(); ++s)
	{
		MORPH_DATA_V2 morphData;
		Platform_SHIFTJIS_UTF8(morphs[s].name, morphData.morphTargetName);
		morphData.uniqueID = 20000 + (unsigned int)s;
		morphData.vertexCount = vertexCount;

		morphData.bboxMin = posBboxMin;
		morphData.bboxMax = posBboxMax;

		morphData.positions.resize(3 * vertexCount);
		memcpy(&morphData.positions[0], originalPositions, 3 * vertexCount * sizeof(float));

		std::vector<pmd::PmdMorphVertex>& baseMorph = morphs[0].vertices;

		for (size_t i = 0; i < morphs[s].vertices.size(); ++i)
		{
			pmd::PmdMorphVertex& vertex = morphs[s].vertices[i];

			unsigned int index = baseMorph[vertex.vertex_index].vertex_index;

			float x = morphData.positions[index * 3 + 0] + vertex.position[0];
			float y = morphData.positions[index * 3 + 1] + vertex.position[1];
			float z = morphData.positions[index * 3 + 2] - vertex.position[2];

			morphData.positions[index * 3 + 0] = x;
			morphData.positions[index * 3 + 1] = y;
			morphData.positions[index * 3 + 2] = z;

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

		morphTargetsData->push_back(morphData);
	}
}

void PmdConverter::buildSceneStructure(pmd::PmdModel* pmd)
{
	std::vector<pmd::PmdBone>& bones = pmd->bones;
	std::vector<pmd::PmdIk>& iks = pmd->iks;
	std::vector<pmd::PmdRigidBody>& rigidBodies = pmd->rigid_bodies;
	std::vector<pmd::PmdJoint>& joints = pmd->joints;

	sceneRoot = new SceneNode();
	sceneRoot->name = "MMD_Skeleton_Root";
	sceneRoot->id = 3;

	nodes.clear();

	nodes.resize(bones.size());
	for (size_t s = 0; s < bones.size(); ++s)
	{
		nodes[s] = new SceneNode();

		const pmd::PmdBone& bone = bones[s];
		Platform_SHIFTJIS_UTF8(bone.name, nodes[s]->name);
		nodes[s]->id = (int)s + 10000;

		if (bone.parent_bone_index != -1)
		{
			const pmd::PmdBone& parentBone = bones[bone.parent_bone_index];
			float x = bone.bone_head_pos[0] - parentBone.bone_head_pos[0];
			float y = bone.bone_head_pos[1] - parentBone.bone_head_pos[1];
			float z = (-bone.bone_head_pos[2]) - (-parentBone.bone_head_pos[2]);
			nodes[s]->position.Set(x, y, z);
		}
		else
		{
			nodes[s]->position.Set(bone.bone_head_pos[0], bone.bone_head_pos[1], -bone.bone_head_pos[2]);
		}

		if (bone.parent_bone_index == -1)
		{
			sceneRoot->addChild(nodes[s]);
		}
	}

	for (size_t s = 0; s < bones.size(); ++s)
	{
		const pmd::PmdBone& bone = bones[s];
		if (bone.parent_bone_index != -1)
		{
			SceneNode* parent = nodes[(int)bone.parent_bone_index];
			nodes[s]->setParent(parent);
		}
	}

	sceneRoot->update();

	//IK
	sceneRoot->ikChains = NULL;
	if (iks.size() > 0)
	{
		std::vector<IKChain*>* ikChains = new std::vector<IKChain*>();

		for (size_t s = 0; s < iks.size(); ++s)
		{
			const pmd::PmdIk& pmdIKChain = iks[s];
			IKChain* chain = new IKChain();
			chain->effector = nodes[pmdIKChain.ik_bone_index];
			chain->target = nodes[pmdIKChain.target_bone_index];
			chain->interation = pmdIKChain.interations;
			chain->maxAngle = pmdIKChain.angle_limit * 4.0f;
			
			for (size_t k = 0; k < pmdIKChain.ik_child_bone_index.size(); ++k)
			{
				int index = (int)pmdIKChain.ik_child_bone_index[k];
				SceneNode* chainNode = nodes[index];

				chainNode->ikParam = new IKParam();
				chainNode->ikParam->enable = true;

				if (chainNode->name.find("ひざ") != string::npos) //knee, source code must be utf8 encoding.
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

			ikChains->push_back(chain);
		}
		sceneRoot->ikChains = ikChains;
	}

	//RigidBodies
	sceneRoot->physicsRigidBodies = NULL;
	if (rigidBodies.size() > 0)
	{
		std::vector<PhysicsRigidBody*>* physicsRigidBodies = new std::vector<PhysicsRigidBody*>();

		for (size_t s = 0; s < rigidBodies.size(); ++s)
		{
			pmd::PmdRigidBody& rb = rigidBodies[s];
			PhysicsRigidBody* prb = new PhysicsRigidBody();

			Platform_SHIFTJIS_UTF8(rb.name, prb->name);

			if (rb.related_bone_index >= 0 && rb.related_bone_index < nodes.size())
			{
				prb->boneLinked = nodes[rb.related_bone_index];
			}
			else
			{
				prb->boneLinked = NULL;
			}

			prb->groupIndex = rb.group_index;
			prb->groupTarget = rb.group_target;

			switch (rb.shapeType)
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

			prb->width = rb.size[0];
			prb->height = rb.size[1];
			prb->depth = rb.size[2];
			prb->position.Set(rb.position[0], rb.position[1], -rb.position[2]); //left to right
			prb->rotation.Set(-rb.rotation[0], -rb.rotation[1], rb.rotation[2]); //left to right
			prb->mass = rb.weight;
			prb->positionDamping = rb.position_damping;
			prb->rotationDamping = rb.rotation_damping;
			prb->restitution = rb.restitution;
			prb->friction = rb.friction;

			switch (rb.rigid_type)
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
			pmd::PmdJoint& jointData = joints[s];
			PhysicsJoint* joint = new PhysicsJoint();

			Platform_SHIFTJIS_UTF8(jointData.name, joint->name);
			joint->rigidBodyIndexA = jointData.rigid_body_index_a;
			joint->rigidBodyIndexB = jointData.rigid_body_index_b;
			joint->position.Set(jointData.position[0], jointData.position[1], -jointData.position[2]);
			joint->rotation.Set(-jointData.rotation[0], -jointData.rotation[1], jointData.rotation[2]);
			joint->positionLowerLimitation.Set(jointData.position_lower_limit[0], jointData.position_lower_limit[1], -jointData.position_upper_limit[2]);
			joint->positionUpperLimitation.Set(jointData.position_upper_limit[0], jointData.position_upper_limit[1], -jointData.position_lower_limit[2]);
			joint->rotationLowerLimitation.Set(-jointData.rotation_upper_limit[0], -jointData.rotation_upper_limit[1], jointData.rotation_lower_limit[2]);
			joint->rotationUpperLimitation.Set(-jointData.rotation_lower_limit[0], -jointData.rotation_lower_limit[1], jointData.rotation_upper_limit[2]);
			joint->positionSpringStiffness.Set(jointData.position_stiffness[0], jointData.position_stiffness[1], jointData.position_stiffness[2]);
			joint->rotationSpringStiffness.Set(jointData.rotation_stiffness[0], jointData.rotation_stiffness[1], jointData.rotation_stiffness[2]);

			PhysicsRigidBody* rbA = (*sceneRoot->physicsRigidBodies)[joint->rigidBodyIndexA];
			PhysicsRigidBody* rbB = (*sceneRoot->physicsRigidBodies)[joint->rigidBodyIndexB];

			if(rbA->dynamicType != "FollowBone" && rbB->dynamicType == "PhysicsAndBone") 
			{
				//model.bones[bodyB.boneIndex].parentIndex == bodyA.boneIndex
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

std::vector<BONE_>* PmdConverter::createSkeleton(std::vector<pmd::PmdBone>& bones)
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

void PmdConverter::convertSceneFile(
	const std::string& workingDirectory,
	void* pmd,
	std::vector<std::string>& convertedFiles,
	const std::string& background,
	int version)
{
	std::vector<std::string> textureFiles;
	std::vector<Material*> materials;
	std::vector<TextureMap*> textureMaps;

	pmd::PmdModel* pmd0 = (pmd::PmdModel*)pmd;

	for (size_t materialIndex = 0; materialIndex < pmd0->materials.size(); ++materialIndex)
	{
		pmd::PmdMaterial& material = pmd0->materials[materialIndex];
		Material* cache = new Material();
		cache->parsePMDMaterial((unsigned int)materialIndex, &material, textureMaps, textureFiles);
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