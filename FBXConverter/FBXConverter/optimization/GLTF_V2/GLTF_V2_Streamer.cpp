#include "stdafx.h"
#include "GLTF_V2_Streamer.h"
#include "JSONFileWriter.h"
#include "TextureMap.h"
#include "Material.h"
#include "NeonateVertexCompression_V4.h"
#include "JsonToBin_V4.h"

GLTF_V2_Streamer::GLTF_V2_Streamer()
{

}

GLTF_V2_Streamer::~GLTF_V2_Streamer()
{

}

bool GLTF_V2_Streamer::save
(
	const std::string& directoryPath,
	const std::string& modelFileName,
	bool jsonBeautify,
	bool generateGZFile,
	unsigned int meshOptimization,
	unsigned int animationOptimization
)
{
	mModelFileName = modelFileName;

	if (mHierarchy != NULL)
	{
		std::string structureFile = directoryPath + modelFileName + ".structure.gltf";
		saveStructure(structureFile, modelFileName, jsonBeautify);
		#ifdef _MSC_VER
		if (generateGZFile)
		{
			std::string compressedFileName = compressFile(structureFile);
			mOutputFiles.push_back(compressedFileName);
		}
		else
		{
			mOutputFiles.push_back(structureFile);
		}
		#else
		mOutputFiles.push_back(structureFile);
		#endif
	}

	return true;
}


void GLTF_V2_Streamer::saveStructure(const std::string& structureFilePath, const std::string& fileName, bool beautified)
{
	JSONFileWriter* writer = new JSONFileWriter(beautified);
	writer->openExportFile(structureFilePath);	

	int padding = 0;

	writer->writeObjectInfo("{", padding);

	writer->writeObjectInfo("\"asset\":", padding + 1);
	writer->writeObjectInfo("{", padding + 1);
	writer->writeObjectInfo("\"version\":\"2.0\",", padding + 2);
	writer->writeObjectInfo("\"generator\":\"fbx2gltf@gasenginev2\",", padding + 2);
	writer->writeObjectInfo("\"copyright\":\"tomyi@tencent.com\"", padding + 2);
	writer->writeObjectInfo("},", padding + 1);

	writer->writeObjectInfo("\"nodes\":", padding + 1);
	writer->writeObjectInfo("[", padding + 1);
	writeHierarchy_r(writer, padding + 2);
	writer->writeObjectInfo("]", padding + 1);

	writer->writeObjectInfo("}", padding);
	writer->closeExportFile();
	delete writer;
	writer = NULL;
}

void GLTF_V2_Streamer::writeHierarchy_r(JSONFileWriter* writer, int padding)
{
	char buffer[1024];

	size_t totalNodeCount = (*mHierarchy).size();
	for (size_t s = 0; s < totalNodeCount; ++s)
	{
		Node* node = (*mHierarchy)[s];

		writer->writeObjectInfo("{", padding);

		sprintf(buffer, "\"id\":%d,", node->uniuqeID);
		writer->writeObjectInfo(buffer, padding + 1);

		sprintf(buffer, "\"guid\":\"%s\",", node->guid.c_str());
		writer->writeObjectInfo(buffer, padding + 1);

		writer->writeObjectInfo("\"name\":\"" + node->name + "\",", padding + 1);

		FbxVector4 tr = node->translation;
		FbxVector4 rt = node->rotation;
		FbxVector4 sc = node->scaling;

		memset(buffer, 0, __TEMP_BUFFER_FLOAT__);
		sprintf(buffer, "\"translation\":[%f,%f,%f],",
			(isnan(tr[0]) ? 0.0f : tr[0]),
			(isnan(tr[1]) ? 0.0f : tr[1]),
			(isnan(tr[2]) ? 0.0f : tr[2]));
		writer->writeObjectInfo(buffer, padding + 1);

		memset(buffer, 0, __TEMP_BUFFER_FLOAT__);
		sprintf(buffer, "\"rotation\":[%f,%f,%f],",
			(isnan(rt[0]) ? 0.0f : rt[0]),
			(isnan(rt[1]) ? 0.0f : rt[1]),
			(isnan(rt[2]) ? 0.0f : rt[2]));
		writer->writeObjectInfo(buffer, padding + 1);

		memset(buffer, 0, __TEMP_BUFFER_FLOAT__);

		sprintf(buffer, "\"scale\":[%f,%f,%f],",
			(isnan(sc[0]) ? 0.0f : sc[0]),
			(isnan(sc[1]) ? 0.0f : sc[1]),
			(isnan(sc[2]) ? 0.0f : sc[2]));
		writer->writeObjectInfo(buffer, padding + 1);

        if (!node->is_max) {
            // motion builder related properties
            FBXSDK_NAMESPACE::FbxVector4 PropSp = node->scalingPivot;
            FBXSDK_NAMESPACE::FbxVector4 PropSoff = node->scalingOffset;

            FBXSDK_NAMESPACE::FbxVector4 PropRp = node->rotationPivot;
            FBXSDK_NAMESPACE::FbxVector4 PropRpost = node->postRotation;
            FBXSDK_NAMESPACE::FbxVector4 PropRpre = node->preRotation;
            FBXSDK_NAMESPACE::FbxVector4 PropRoff = node->rotationOffset;

            writer->writeObjectInfo("\"MB_PROPS\":", padding);
            writer->writeObjectInfo("{", padding);

            sprintf(buffer, "\"ScalingPivot\":[%f,%f,%f],",
                (isnan(PropSp[0]) ? 0.0f : PropSp[0]),
                (isnan(PropSp[1]) ? 0.0f : PropSp[1]),
                (isnan(PropSp[2]) ? 0.0f : PropSp[2]));
            writer->writeObjectInfo(buffer, padding + 1);

            sprintf(buffer, "\"ScalingOffset\":[%f,%f,%f],",
                (isnan(PropSoff[0]) ? 0.0f : PropSoff[0]),
                (isnan(PropSoff[1]) ? 0.0f : PropSoff[1]),
                (isnan(PropSoff[2]) ? 0.0f : PropSoff[2]));
            writer->writeObjectInfo(buffer, padding + 1);

            sprintf(buffer, "\"RotationPivot\":[%f,%f,%f],",
                (isnan(PropRp[0]) ? 0.0f : PropRp[0]),
                (isnan(PropRp[1]) ? 0.0f : PropRp[1]),
                (isnan(PropRp[2]) ? 0.0f : PropRp[2]));
            writer->writeObjectInfo(buffer, padding + 1);

            sprintf(buffer, "\"RotationOffset\":[%f,%f,%f],",
                (isnan(PropRoff[0]) ? 0.0f : PropRoff[0]),
                (isnan(PropRoff[1]) ? 0.0f : PropRoff[1]),
                (isnan(PropRoff[2]) ? 0.0f : PropRoff[2]));
            writer->writeObjectInfo(buffer, padding + 1);

            sprintf(buffer, "\"PreRotation\":[%f,%f,%f],",
                (isnan(PropRpre[0]) ? 0.0f : PropRpre[0]),
                (isnan(PropRpre[1]) ? 0.0f : PropRpre[1]),
                (isnan(PropRpre[2]) ? 0.0f : PropRpre[2]));
            writer->writeObjectInfo(buffer, padding + 1);

            sprintf(buffer, "\"PostRotation\":[%f,%f,%f],",
                (isnan(PropRpost[0]) ? 0.0f : PropRpost[0]),
                (isnan(PropRpost[1]) ? 0.0f : PropRpost[1]),
                (isnan(PropRpost[2]) ? 0.0f : PropRpost[2]));
            writer->writeObjectInfo(buffer, padding + 1);

            sprintf(buffer, "\"RotationOrder\":\"%s\",", node->rotationOrder.c_str());
            writer->writeObjectInfo(buffer, padding + 1);

            sprintf(buffer, "\"InheritType\":\"%s\",", node->inheritType.c_str());
            writer->writeObjectInfo(buffer, padding + 1);

            sprintf(buffer, "\"Visibility\":%f,", node->visibility);
            writer->writeObjectInfo(buffer, padding + 1);

            if (node->visibilityInheritance) {
                writer->writeObjectInfo("\"VisibilityInheritance\":true", padding + 1);
            } else {
                writer->writeObjectInfo("\"VisibilityInheritance\":false", padding + 1);
            }
        } else {
            // 3ds max related properties
            FBXSDK_NAMESPACE::FbxVector4 PropSa = node->scaling_axis;
            FBXSDK_NAMESPACE::FbxVector4 PropOR = node->offset_rotation;

            FBXSDK_NAMESPACE::FbxVector4 PropOT = node->offset_translation;
            FBXSDK_NAMESPACE::FbxVector4 PropOS = node->offset_scaling;
            FBXSDK_NAMESPACE::FbxVector4 PropOSa = node->offset_scaling_axis;

            writer->writeObjectInfo("\"MAX_PROPS\":", padding);
            writer->writeObjectInfo("{", padding);

            sprintf(buffer, "\"ScalingAxis\":[%f,%f,%f,%f],",
                (isnan(PropSa[0]) ? 0.0f : PropSa[0]),
                (isnan(PropSa[1]) ? 0.0f : PropSa[1]),
                (isnan(PropSa[2]) ? 0.0f : PropSa[2]),
                (isnan(PropSa[3]) ? 0.0f : PropSa[3]));
            writer->writeObjectInfo(buffer, padding + 1);

            sprintf(buffer, "\"OffsetRotation\":[%f,%f,%f,%f],",
                (isnan(PropOR[0]) ? 0.0f : PropOR[0]),
                (isnan(PropOR[1]) ? 0.0f : PropOR[1]),
                (isnan(PropOR[2]) ? 0.0f : PropOR[2]),
                (isnan(PropOR[3]) ? 0.0f : PropOR[3]));
            writer->writeObjectInfo(buffer, padding + 1);

            sprintf(buffer, "\"OffsetTranslation\":[%f,%f,%f],",
                (isnan(PropOT[0]) ? 0.0f : PropOT[0]),
                (isnan(PropOT[1]) ? 0.0f : PropOT[1]),
                (isnan(PropOT[2]) ? 0.0f : PropOT[2]));
            writer->writeObjectInfo(buffer, padding + 1);

            sprintf(buffer, "\"OffsetScaling\":[%f,%f,%f],",
                (isnan(PropOS[0]) ? 0.0f : PropOS[0]),
                (isnan(PropOS[1]) ? 0.0f : PropOS[1]),
                (isnan(PropOS[2]) ? 0.0f : PropOS[2]));
            writer->writeObjectInfo(buffer, padding + 1);

            sprintf(buffer, "\"OffsetScalingAxis\":[%f,%f,%f,%f]",
                (isnan(PropOSa[0]) ? 0.0f : PropOSa[0]),
                (isnan(PropOSa[1]) ? 0.0f : PropOSa[1]),
                (isnan(PropOSa[2]) ? 0.0f : PropOSa[2]),
                (isnan(PropOSa[3]) ? 0.0f : PropOSa[3]));
            writer->writeObjectInfo(buffer, padding + 1);
        }

        writer->writeObjectInfo("},", padding);

		writer->writeObjectInfo("\"children\":", padding + 1);
		writer->writeObjectInfo("[", padding + 1);
		std::string childString = "";
		size_t childCount = node->children.size();
		for (size_t i = 0; i < childCount; ++i)
		{
			int childIndex = node->children[i];
			if (i == childCount - 1)
			{
				sprintf(buffer, "%d", childIndex);
			}
			else
			{
				sprintf(buffer, "%d,", childIndex);
			}
			childString += buffer;
		}
		writer->writeObjectInfo(childString.c_str(), padding + 2);
		writer->writeObjectInfo("]", padding + 1);
		if (s == totalNodeCount - 1)
		{
			writer->writeObjectInfo("}", padding);
		}
		else
		{
			writer->writeObjectInfo("},", padding);
		}
	}	
}

void GLTF_V2_Streamer::saveMeshes
(
	const std::string& directoryPath,
	const std::string& modelFileName,
	bool generateGZFile,
	unsigned int meshOptimization
)
{
	std::map<int, MeshInfo>::iterator iter = (*mMeshes).begin();
	for(; iter != (*mMeshes).end(); ++iter)
	{
		FBXSDK_NAMESPACE::FbxMesh* mesh = iter->second.mesh;

		int polygonCount = mesh->GetPolygonCount();
		if(polygonCount == 0)
		{
			break;
		}

		int nodeCount = mesh->GetNodeCount();
		if(nodeCount > 1)
		{
			FBXSDK_printf("Error: Detected a mesh associates with multiple nodes!\n");
		}

		int maxMaterialCount = 0;
		for(int k = 0; k < nodeCount; ++k)
		{
			FBXSDK_NAMESPACE::FbxNode* node = mesh->GetNode(k);
			int materialCount = node->GetMaterialCount();
			maxMaterialCount = max_0(maxMaterialCount, materialCount);
		}

		NeonateVertexCompression_V4* compressor = new NeonateVertexCompression_V4();
		unsigned int triangleCountOfMesh = 0, polygonCountOfMesh = 0, vertexCountOfMesh = 0;

		char buffer[1024];
		unsigned int id = (unsigned int)mesh->GetUniqueID();
		FBXSDK_NAMESPACE::FbxNode* parentNode = mesh->GetNode();
		std::string objectName = parentNode->GetName();

		if(objectName.length() == 0)
		{
			objectName = "unknown";
		}
		objectName = standardizeFileName(objectName);
		if(objectName.size() > 64)
		{
			objectName = objectName.substr(0, 64);
		}
		sprintf(buffer, "%s.%s.%d.mesh.bin", modelFileName.c_str(), objectName.c_str(), id);
		std::string meshFileName = buffer;
		meshFileName = standardizeFileName(meshFileName);

		JsonToBin_V4* writer = new JsonToBin_V4();
		std::string meshBinFilePath = directoryPath + meshFileName;
		writer->openExportFile(meshBinFilePath);

		compressor->compressVertices
		(
			meshOptimization,
			mesh,
			maxMaterialCount,
			iter->second.bboxMin,
			iter->second.bboxMax,
			triangleCountOfMesh,
			polygonCountOfMesh,
			vertexCountOfMesh,
			iter->second.isSkinned,
			writer
		);

		MESH_DETAIL meshDetail = compressor->getMeshDetail();
		meshDetail.meshID = mesh->GetUniqueID();
		meshDetail.meshName = objectName;
		mMeshDetails.push_back(meshDetail);

		writer->closeExportFile();
		delete writer;
		delete compressor;

		iter->second.triangleCount = triangleCountOfMesh;
		iter->second.polygonCount = polygonCountOfMesh;
		iter->second.vertexCount = vertexCountOfMesh;

		mTotalTriangleCount += triangleCountOfMesh;
		mTotalPolygonCount += polygonCountOfMesh;
		mTotalVertexCount += vertexCountOfMesh;
		#ifdef _MSC_VER
		if(generateGZFile)
		{
			std::string compressedFileName = compressFile(meshBinFilePath);
			mOutputFiles.push_back(compressedFileName);
		}
		else
		{
			mOutputFiles.push_back(meshBinFilePath);
		}
		#else
		mOutputFiles.push_back(meshBinFilePath);
		#endif
	}
}

void GLTF_V2_Streamer::saveAnimations
(
	const std::string& directoryPath,
	const std::string& modelFileName,
	bool generateGZFile,
	unsigned int animationOptimization
)
{
	for(size_t s = 0; s < (*mAnimations).size(); ++s)
	{
		KeyframeAnimation& kfa = (*mAnimations)[s];
		ANIMATION_DETAIL animationDetail = {
			(int)kfa.keyframeCount,
			kfa.fps,
			kfa.endFrame - kfa.startFrame
		};
		mAnimationDetails.push_back(animationDetail);
		char buffer[__TEMP_BUFFER_FLOAT__];
		sprintf(buffer, "%s.%s.%d.animation.bin", modelFileName.c_str(), kfa.clipName.c_str(), kfa.clipID);
		std::string clipFileName = buffer;
		clipFileName = standardizeFileName(clipFileName);

		std::string animationBinFilePath = directoryPath + clipFileName;
		JsonToBin_V4* writer = new JsonToBin_V4();
		writer->openExportFile(animationBinFilePath);
		writer->writeAnimationBin(kfa.clipID, kfa.clipName, kfa.fps, kfa.startFrame, kfa.endFrame, kfa.nodes);
		writer->closeExportFile();
		delete writer;
		#ifdef _MSC_VER
		if(generateGZFile)
		{
			std::string compressedFileName = compressFile(animationBinFilePath);
			mOutputFiles.push_back(compressedFileName);
		}
		else
		{
			mOutputFiles.push_back(animationBinFilePath);
		}
		#else
		mOutputFiles.push_back(animationBinFilePath);
		#endif
	}
}

void GLTF_V2_Streamer::saveOutputLog(const std::string& path)
{
	FBXSDK_printf("***************** Output log *****************\n");

	JSONFileWriter* resultFile = new JSONFileWriter(false);
	resultFile->openExportFile(path);

	resultFile->writeObjectInfo("{", 0);
	resultFile->writeObjectInfo("\"version\":\"gltf2\",", 1);
	resultFile->writeObjectInfo("\"files\":", 1);
	resultFile->writeObjectInfo("[", 1);

	const int FILE_PATH_MAX_LENGTH = 2048;
	char buffer[FILE_PATH_MAX_LENGTH * 2];

	//size_t fileCount = mOutputFiles.size();
	//for (size_t i = 0; i < fileCount; ++i)
	//{
	//	std::string fileName = getFileName(mOutputFiles[i]);
	//	FBXSDK_printf("%s\n", fileName.c_str());
	//	if (fileName.length() >= FILE_PATH_MAX_LENGTH)
	//	{
	//		FBXSDK_printf("Error: output file path exceeds the up limit!\n");
	//		fileName = "Error: output file path exceeds the up limit!";
	//	}
	//	if (i == fileCount - 1)
	//		sprintf(buffer, "\"%s\"", fileName.c_str());
	//	else
	//		sprintf(buffer, "\"%s\",", fileName.c_str());

	//	resultFile->writeObjectInfo(buffer, 2);
	//}

	resultFile->writeObjectInfo("],", 1);

	resultFile->writeObjectInfo("\"statistics\":", 1);
	resultFile->writeObjectInfo("{", 1);

	sprintf(buffer, "\"triangleCount\":%d,", mTotalTriangleCount);
	resultFile->writeObjectInfo(buffer, 2);

	sprintf(buffer, "\"polygonCount\":%d,", mTotalPolygonCount);
	resultFile->writeObjectInfo(buffer, 2);

	sprintf(buffer, "\"vertexCount\":%d", mTotalVertexCount);
	resultFile->writeObjectInfo(buffer, 2);

	resultFile->writeObjectInfo("}", 1);
	resultFile->writeObjectInfo("}", 0);
	resultFile->closeExportFile();

	FBXSDK_printf("***************** End Of Converted Files *****************\n");
}