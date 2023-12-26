#include "stdafx.h"
#include "GAS_V2_Streamer.h"
#include "JSONFileWriter.h"
#include "TextureMap.h"
#include "Material.h"
#include "NeonateVertexCompression_V4.h"
#include "JsonToBin_V4.h"

GAS_V2_Streamer::GAS_V2_Streamer()
	: mTotalTriangleCount(0)
	, mTotalPolygonCount(0)
	, mTotalVertexCount(0)
{

}

GAS_V2_Streamer::~GAS_V2_Streamer()
{

}

bool GAS_V2_Streamer::save
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

	if (mMeshes != NULL)
	{
		saveMeshes(directoryPath, modelFileName, generateGZFile, meshOptimization);
	}

	if (mMaterials != NULL)
	{
		saveMaterials(directoryPath, modelFileName, generateGZFile, jsonBeautify);
	}

	if (mHierarchy != NULL)
	{
		std::string structureFile = directoryPath + modelFileName + ".structure.json";
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

	if (mAnimations != NULL)
	{
		saveAnimations(directoryPath, modelFileName, generateGZFile, animationOptimization);
	}

	std::string outputLogFile = directoryPath + modelFileName + ".convertedFiles";
	mOutputFiles.push_back(outputLogFile);

	if (mTextures != NULL)
	{
		for (size_t s = 0; s < (*mTextures).size(); ++s)
		{
			mOutputFiles.push_back((*mTextures)[s]);
		}
	}

	saveOutputLog(outputLogFile);

	return true;
}

void GAS_V2_Streamer::saveStructure(const std::string& structureFilePath, const std::string& fileName, bool beautified)
{
	JSONFileWriter* writer = new JSONFileWriter(beautified);
	writer->openExportFile(structureFilePath);

	Node* root = NULL;
	if ((*mHierarchy).size() > 0)
	{
		root = (*mHierarchy)[0];
		if (root->parent != -1)
		{
			return;
		}
	}

	int padding = 0;

	writer->writeObjectInfo("{", padding);
	writer->writeObjectInfo("\"version\":5,", padding + 1);
	writer->writeObjectInfo("\"srcVersion\":\"FBX\",", padding + 1);
	writer->writeObjectInfo("\"name\":\"" + fileName + "\",", padding + 1);
	writer->writeObjectInfo("\"setting\":{},", padding + 1);
	writer->writeObjectInfo("\"nodeTree\":", padding + 1);

	writeHierarchy_r(writer, root, padding + 2, true);

	writer->writeObjectInfo("}", padding);
	writer->closeExportFile();
	delete writer;
	writer = NULL;
}

void GAS_V2_Streamer::writeHierarchy_r(JSONFileWriter* writer, Node* node, int padding, bool isLast)
{
	char buffer[1024];

	writer->writeObjectInfo("{", padding - 1);

	sprintf(buffer, "\"uniqueID\":%d,", node->uniuqeID);
	writer->writeObjectInfo(buffer, padding);

	sprintf(buffer, "\"guid\":\"%s\",", node->guid.c_str());
	writer->writeObjectInfo(buffer, padding);

	writer->writeObjectInfo("\"name\":\"" + node->name + "\",", padding);

	writer->writeObjectInfo("\"skeletonName\":\"" + node->skeletonName + "\",", padding);

	FbxVector4 tr = node->translation;
	FbxVector4 rt = node->rotation;
	FbxVector4 sc = node->scaling;

	memset(buffer, 0, __TEMP_BUFFER_FLOAT__);
	sprintf(buffer, "\"translation\":[%f,%f,%f],",
		(isnan(tr[0]) ? 0.0f : tr[0]),
		(isnan(tr[1]) ? 0.0f : tr[1]),
		(isnan(tr[2]) ? 0.0f : tr[2]));
	writer->writeObjectInfo(buffer, padding);

	memset(buffer, 0, __TEMP_BUFFER_FLOAT__);
	sprintf(buffer, "\"rotation\":[%f,%f,%f],",
		(isnan(rt[0]) ? 0.0f : rt[0]),
		(isnan(rt[1]) ? 0.0f : rt[1]),
		(isnan(rt[2]) ? 0.0f : rt[2]));
	writer->writeObjectInfo(buffer, padding);

	memset(buffer, 0, __TEMP_BUFFER_FLOAT__);

	sprintf(buffer, "\"scaling\":[%f,%f,%f],",
		(isnan(sc[0]) ? 0.0f : sc[0]),
		(isnan(sc[1]) ? 0.0f : sc[1]),
		(isnan(sc[2]) ? 0.0f : sc[2]));
	writer->writeObjectInfo(buffer, padding);

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

	writer->writeObjectInfo("\"components\":", padding);
	writer->writeObjectInfo("{", padding);
	//Animation
	if (node->parent == -1)
	{
		//size_t boneCount = (*mEffectiveBones).size();
		size_t animationClipCount = (*mAnimations).size();
		if (animationClipCount > 0)
		{
			writer->writeObjectInfo("\"animator\":", padding + 1);
			writer->writeObjectInfo("{", padding + 1);
			writer->writeObjectInfo("\"syncLoading\":false,", padding + 2);
			writer->writeObjectInfo("\"uniqueID\":-1,", padding + 2);
			writer->writeObjectInfo("\"clips\":", padding + 2);
			writer->writeObjectInfo("[", padding + 2);
			//size_t animationClipCount = (*mAnimations).size();
			//if (animationClipCount > 0)
			//{
			for (size_t i = 0; i < animationClipCount; ++i)
			{
				KeyframeAnimation& kfa = (*mAnimations)[i];

				sprintf(buffer, "%s.%s.%d.animation.bin", mModelFileName.c_str(), kfa.clipName.c_str(), kfa.clipID);
				std::string clipFileName = buffer;
				clipFileName = standardizeFileName(clipFileName);

				if (i == animationClipCount - 1)
				{
					sprintf(buffer, "\"%s\"", clipFileName.c_str());
				}
				else
				{
					sprintf(buffer, "\"%s\",", clipFileName.c_str());
				}
				writer->writeObjectInfo(buffer, padding + 3);
			}
			//}
			writer->writeObjectInfo("]", padding + 2);
			if (node->meshInfo != NULL)
			{
				writer->writeObjectInfo("},", padding + 1);
			}
			else
			{
				writer->writeObjectInfo("}", padding + 1);
			}
		}
	}

	//Mesh
	if (node->meshInfo != NULL)
	{
		writer->writeObjectInfo("\"meshFilter\":", padding + 1);
		writer->writeObjectInfo("{", padding + 1);
		writer->writeObjectInfo("\"syncLoading\":false,", padding + 2);
		sprintf(buffer, "\"uniqueID\":%d,", node->meshInfo->uniqueID);
		writer->writeObjectInfo(buffer, padding + 2);
		
		sprintf(buffer, "%s.%s.%d.mesh.bin", mModelFileName.c_str(), node->meshInfo->meshName.c_str(), node->meshInfo->uniqueID);
		std::string standardizedName = buffer;
		standardizedName = standardizeFileName(standardizedName);
		sprintf(buffer, "\"mesh\":\"%s\",", standardizedName.c_str());
		writer->writeObjectInfo(buffer, padding + 2);
		//mesh bounding box
		FBXSDK_NAMESPACE::FbxDouble3& bboxMin = node->meshInfo->bboxMin;
		FBXSDK_NAMESPACE::FbxDouble3& bboxMax = node->meshInfo->bboxMax;
		writer->writeObjectInfo("\"bbox\":", padding + 2);
		writer->writeObjectInfo("[", padding + 2);
		sprintf(buffer, "[%0.3f,%0.3f,%0.3f],", bboxMin[0], bboxMin[1], bboxMin[2]);
		writer->writeObjectInfo(buffer, padding + 3);
		sprintf(buffer, "[%0.3f,%0.3f,%0.3f]", bboxMax[0], bboxMax[1], bboxMax[2]);
		writer->writeObjectInfo(buffer, padding + 3);
		writer->writeObjectInfo("]", padding + 2);
		writer->writeObjectInfo("},", padding + 1);

		writer->writeObjectInfo("\"meshRenderer\":", padding + 1);
		writer->writeObjectInfo("{", padding + 1);
		writer->writeObjectInfo("\"syncLoading\":false,", padding + 2);
		writer->writeObjectInfo("\"uniqueID\":-1,", padding + 2);
		writer->writeObjectInfo("\"materials\":", padding + 2);
		writer->writeObjectInfo("[", padding + 2);
		size_t materialCount = node->materials.size();
		for (size_t i = 0; i < materialCount; ++i)
		{
			unsigned int index = node->materials[i];
			Material* material = (*mMaterials)[index];

			std::string materialName = (const char*)material->mMaterialName;
			if(materialName.length() == 0)
			{
				materialName = "unknown";
			}

			sprintf(buffer, "%s.%s.%d.mat.json", mModelFileName.c_str(), materialName.c_str(), material->mUniqueID);
			std::string standardizedName = buffer;
			standardizedName = standardizeFileName(standardizedName);

			if(i == materialCount - 1)
			{
				sprintf(buffer, "\"%s\"", standardizedName.c_str());
			}
			else
			{
				sprintf(buffer, "\"%s\",", standardizedName.c_str());
			}			
			writer->writeObjectInfo(buffer, padding + 3);
		}
		writer->writeObjectInfo("]", padding + 2);
		writer->writeObjectInfo("}", padding + 1);
	}
	writer->writeObjectInfo("},", padding);

	writer->writeObjectInfo("\"children\":", padding);
	writer->writeObjectInfo("[", padding);
	size_t childCount = node->children.size();
	for (size_t i = 0; i < childCount; ++i)
	{
		int childIndex = node->children[i];
		Node* child = (*mHierarchy)[childIndex];
		writeHierarchy_r(writer, child, padding + 2, i == childCount - 1);
	}
	writer->writeObjectInfo("]", padding);
	if (isLast)
	{
		writer->writeObjectInfo("}", padding - 1);
	}
	else
	{
		writer->writeObjectInfo("},", padding - 1);
	}
}

void GAS_V2_Streamer::saveMeshes
(
	const std::string& directoryPath,
	const std::string& modelFileName,
	bool generateGZFile,
	unsigned int meshOptimization
)
{
	std::map<int, MeshInfo>::iterator iter = (*mMeshes).begin();
	for (; iter != (*mMeshes).end(); ++iter)
	{
		FBXSDK_NAMESPACE::FbxMesh* mesh = iter->second.mesh;

		int polygonCount = mesh->GetPolygonCount();
		if (polygonCount == 0)
		{
			break;
		}

		int nodeCount = mesh->GetNodeCount();
		if (nodeCount > 1)
		{
			FBXSDK_printf("Error: Detected a mesh associates with multiple nodes!\n");
		}

		int maxMaterialCount = 0;
		for (int k = 0; k < nodeCount; ++k)
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

		if (objectName.length() == 0)
		{
			objectName = "unknown";
		}
		objectName = standardizeFileName(objectName);
		if (objectName.size() > 64)
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
		meshDetail.meshID = (int)mesh->GetUniqueID();
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
		if (generateGZFile)
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

void GAS_V2_Streamer::saveAnimations
(
	const std::string& directoryPath,
	const std::string& modelFileName,
	bool generateGZFile,
	unsigned int animationOptimization
)
{
	for (size_t s = 0; s < (*mAnimations).size(); ++s)
	{
		KeyframeAnimation& kfa = (*mAnimations)[s];
		ANIMATION_DETAIL animationDetail = {
			(int)kfa.keyframeCount,
			kfa.fps,
			kfa.endFrame - kfa.startFrame
		};
		mAnimationDetails.push_back(animationDetail);
		//assetCount aniAssetCount =
		//{ 0,0,0,0,0,
		//	kfa.keyframeCount,
		//	kfa.fps,
		//	kfa.endFrame - kfa.startFrame,
		//};
		//mAssetCounts.push_back(aniAssetCount);
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
		if (generateGZFile)
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

void GAS_V2_Streamer::saveMaterials
(
	const std::string& directoryPath,
	const std::string& modelFileName,
	bool generateGZFile,
	bool jsonBeautify
)
{	
	char buffer[__TEMP_BUFFER_FLOAT__];

	std::vector<Material*>& materials = (*mMaterials);

	for (int i = 0; i < (int)materials.size(); ++i)
	{
		Material* material = materials[i];
		std::string materialName = (const char*)material->mMaterialName;
		if (materialName.length() == 0)
		{
			materialName = "unknown";
		}
		sprintf(buffer, "%s.%s.%d.mat.json", modelFileName.c_str(), materialName.c_str(), material->mUniqueID);
		std::string materialFileName = buffer;
		materialFileName = standardizeFileName(materialFileName);

		JSONFileWriter* jsonWriter = new JSONFileWriter(jsonBeautify);
		jsonWriter->openExportFile(directoryPath + materialFileName);
		writeCompoundMaterial(jsonWriter, material, mTextureMaps, mTextures, true, 0);
		jsonWriter->closeExportFile();
		#ifdef _MSC_VER
		if (generateGZFile)
		{
			std::string compressedFileName = compressFile(materialFileName);
			mOutputFiles.push_back(compressedFileName);
		}
		else
		{
			mOutputFiles.push_back(materialFileName);
		}
		#else
		mOutputFiles.push_back(materialFileName);
		#endif
	}
}

void GAS_V2_Streamer::writeCompoundMaterial
(
	JSONFileWriter* jsonWriter,
	Material* material,
	std::vector<TextureMap*>* textureMaps,
	std::vector<std::string>* textureFiles,
	bool isLastItem,
	int jsonLevel
)
{
	char buffer[__TEMP_BUFFER_FLOAT__];
	jsonWriter->writeObjectInfo("{", jsonLevel);
	string materialName = standardizeFileName((const char*)material->mMaterialName);
	sprintf(buffer, "\"name\":\"%s\",", materialName.c_str());
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);
	sprintf(buffer, "\"uniqueID\":\"%d\",", material->mUniqueID);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"type\":\"compound\",", jsonLevel + 1);
	//blinnPhong  dielectric  electric  matcap  compound
	jsonWriter->writeObjectInfo("\"activeMaterial\":\"dielectric\",", jsonLevel + 1);

	jsonWriter->writeObjectInfo("\"blinnPhong\":", jsonLevel + 1);
	writeBlinnPhongMaterial(jsonWriter, material, textureMaps, textureFiles, false, jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"dielectric\":", jsonLevel + 1);
	writeDielectricMaterial(jsonWriter, material, textureMaps, textureFiles, false, jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"electric\":", jsonLevel + 1);
	writeElectricMaterial(jsonWriter, material, textureMaps, textureFiles, false, jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"matcap\":", jsonLevel + 1);
	writeMatCapMaterial(jsonWriter, material, textureMaps, textureFiles, true, jsonLevel + 1);
	if (isLastItem)
		jsonWriter->writeObjectInfo("}", jsonLevel);
	else
		jsonWriter->writeObjectInfo("},", jsonLevel);
}

void GAS_V2_Streamer::writeBlinnPhongMaterial
(
	JSONFileWriter* jsonWriter,
	Material* material,
	std::vector<TextureMap*>* textureMaps,
	std::vector<std::string>* textureFiles,
	bool isLastItem,
	int jsonLevel
)
{
	char buffer[__TEMP_BUFFER_FLOAT__];
	jsonWriter->writeObjectInfo("{", jsonLevel);
	string materialName = standardizeFileName((const char*)material->mMaterialName);
	sprintf(buffer, "\"name\":\"%s\",", materialName.c_str());
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);
	sprintf(buffer, "\"uniqueID\":\"%d\",", material->mUniqueID);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"type\":\"blinnPhong\",", jsonLevel + 1);	
	sprintf(buffer, "\"culling\":%d,", material->mCulling);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"vertexShaderFile\":\"/system/shaders/BlinnPhongVertex.glsl\",", jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"fragmentShaderFile\":\"/system/shaders/BlinnPhongFragment.glsl\",", jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"reflectiveRatio\":0.8,", jsonLevel + 1);
	writeAlbedoMap(jsonWriter, material, textureMaps, textureFiles, false, jsonLevel + 1);
	writeSpecularMap(jsonWriter, material, textureMaps, textureFiles, false, jsonLevel + 1);
	writeGlossinessMap(jsonWriter, material, textureMaps, textureFiles, false, jsonLevel + 1);
	writeDisplacementMap(jsonWriter, material, textureMaps, textureFiles, false, jsonLevel + 1);
	writeNormalMap(jsonWriter, material, textureMaps, textureFiles, false, jsonLevel + 1);
	writeLightMap(jsonWriter, material, textureMaps, textureFiles, false, jsonLevel + 1);
	writeTransparencyMap(jsonWriter, material, textureMaps, textureFiles, false, jsonLevel + 1);
	writeEmissiveMap(jsonWriter, material, textureMaps, textureFiles, true, jsonLevel + 1);
	if (isLastItem)
		jsonWriter->writeObjectInfo("}", jsonLevel);
	else
		jsonWriter->writeObjectInfo("},", jsonLevel);
}

void GAS_V2_Streamer::writeDielectricMaterial
(
	JSONFileWriter* jsonWriter,
	Material* material,
	std::vector<TextureMap*>* textureMaps,
	std::vector<std::string>* textureFiles,
	bool isLastItem,
	int jsonLevel
)
{
	char buffer[__TEMP_BUFFER_FLOAT__];
	jsonWriter->writeObjectInfo("{", jsonLevel);
	string materialName = standardizeFileName((const char*)material->mMaterialName);
	sprintf(buffer, "\"name\":\"%s\",", materialName.c_str());
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);
	sprintf(buffer, "\"uniqueID\":\"%d\",", material->mUniqueID);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"type\":\"dielectric\",", jsonLevel + 1);
	sprintf(buffer, "\"culling\":%d,", material->mCulling);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"vertexShaderFile\":\"/system/shaders/PBRVertex.glsl\",", jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"fragmentShaderFile\":\"/system/shaders/PBRFragment.glsl\",", jsonLevel + 1);
	writeAlbedoMap(jsonWriter, material, textureMaps, textureFiles, false, jsonLevel + 1);
	writeMetalnessMap(jsonWriter, material, textureMaps, textureFiles, false, jsonLevel + 1);
	writeSpecularF0Map(jsonWriter, material, textureMaps, textureFiles, false, jsonLevel + 1);
	writeRoughnessMap(jsonWriter, material, textureMaps, textureFiles, false, jsonLevel + 1);
	writeDisplacementMap(jsonWriter, material, textureMaps, textureFiles, false, jsonLevel + 1);
	writeNormalMap(jsonWriter, material, textureMaps, textureFiles, false, jsonLevel + 1);
	writeAoMap(jsonWriter, material, textureMaps, textureFiles, false, jsonLevel + 1);
	writeCavityMap(jsonWriter, material, textureMaps, textureFiles, false, jsonLevel + 1);
	writeTransparencyMap(jsonWriter, material, textureMaps, textureFiles, false, jsonLevel + 1);
	writeEmissiveMap(jsonWriter, material, textureMaps, textureFiles, true, jsonLevel + 1);
	if (isLastItem)
		jsonWriter->writeObjectInfo("}", jsonLevel);
	else
		jsonWriter->writeObjectInfo("},", jsonLevel);
}

void GAS_V2_Streamer::writeElectricMaterial
(
	JSONFileWriter* jsonWriter,
	Material* material,
	std::vector<TextureMap*>* textureMaps,
	std::vector<std::string>* textureFiles,
	bool isLastItem,
	int jsonLevel
)
{
	char buffer[__TEMP_BUFFER_FLOAT__];
	jsonWriter->writeObjectInfo("{", jsonLevel);
	string materialName = standardizeFileName((const char*)material->mMaterialName);
	sprintf(buffer, "\"name\":\"%s\",", materialName.c_str());
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);
	sprintf(buffer, "\"uniqueID\":\"%d\",", material->mUniqueID);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"type\":\"electric\",", jsonLevel + 1);
	sprintf(buffer, "\"culling\":%d,", material->mCulling);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"vertexShaderFile\":\"/system/shaders/PBRVertex.glsl\",", jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"fragmentShaderFile\":\"/system/shaders/PBRFragment.glsl\",", jsonLevel + 1);
	writeAlbedoMap(jsonWriter, material, textureMaps, textureFiles, false, jsonLevel + 1);
	writeSpecularMap(jsonWriter, material, textureMaps, textureFiles, false, jsonLevel + 1);
	writeRoughnessMap(jsonWriter, material, textureMaps, textureFiles, false, jsonLevel + 1);
	writeDisplacementMap(jsonWriter, material, textureMaps, textureFiles, false, jsonLevel + 1);
	writeNormalMap(jsonWriter, material, textureMaps, textureFiles, false, jsonLevel + 1);
	writeAoMap(jsonWriter, material, textureMaps, textureFiles, false, jsonLevel + 1);
	writeCavityMap(jsonWriter, material, textureMaps, textureFiles, false, jsonLevel + 1);
	writeTransparencyMap(jsonWriter, material, textureMaps, textureFiles, false, jsonLevel + 1);
	writeEmissiveMap(jsonWriter, material, textureMaps, textureFiles, true, jsonLevel + 1);
	if (isLastItem)
		jsonWriter->writeObjectInfo("}", jsonLevel);
	else
		jsonWriter->writeObjectInfo("},", jsonLevel);
}

void GAS_V2_Streamer::writeMatCapMaterial
(
	JSONFileWriter* jsonWriter,
	Material* material,
	std::vector<TextureMap*>* textureMaps,
	std::vector<std::string>* textureFiles,
	bool isLastItem,
	int jsonLevel
)
{
	char buffer[__TEMP_BUFFER_FLOAT__];
	jsonWriter->writeObjectInfo("{", jsonLevel);
	string materialName = standardizeFileName((const char*)material->mMaterialName);
	sprintf(buffer, "\"name\":\"%s\",", materialName.c_str());
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);
	sprintf(buffer, "\"uniqueID\":\"%d\",", material->mUniqueID);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"type\":\"matcap\",", jsonLevel + 1);
	sprintf(buffer, "\"culling\":%d,", material->mCulling);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"vertexShaderFile\":\"/system/shaders/MatCapVertex.glsl\",", jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"fragmentShaderFile\":\"/system/shaders/MatCapFragment.glsl\",", jsonLevel + 1);
	writeMatCapMap(jsonWriter, material, textureMaps, textureFiles, false, jsonLevel + 1);

	writeDisplacementMap(jsonWriter, material, textureMaps, textureFiles, false, jsonLevel + 1);
	writeNormalMap(jsonWriter, material, textureMaps, textureFiles, false, jsonLevel + 1);
	writeTransparencyMap(jsonWriter, material, textureMaps, textureFiles, true, jsonLevel + 1);
	if (isLastItem)
	{
		jsonWriter->writeObjectInfo("}", jsonLevel);
	}
	else
	{
		jsonWriter->writeObjectInfo("},", jsonLevel);
	}
}

void GAS_V2_Streamer::writeAlbedoMap
(
	JSONFileWriter*					jsonWriter,
	Material*						material,
	std::vector<TextureMap*>*		textureMaps,
	std::vector<std::string>*		textureFiles,
	bool							isLastItem,
	int								jsonLevel
)
{
	TextureMap* map = (material->mDiffuseMapIndexInDepot >= 0) ? (*textureMaps)[material->mDiffuseMapIndexInDepot] : NULL;
	std::string textureFileName = "";
	if (map != NULL)
	{
		//std::string innerTextureFileName = map->GetFileName();
		//innerTextureFileName = extractFileName(innerTextureFileName);
		//int index = (int)reinterpret_cast<unsigned long long>(map->GetUserDataPtr());
		//textureFileName = (*textureFiles)[index];
		//if (innerTextureFileName != textureFileName)
		//{
		//	FBXSDK_printf("Error: Albedo map error!");
		//	return;
		//}
		textureFileName = map->fileName;
	}
	char buffer[__TEMP_BUFFER_FLOAT__];
	jsonWriter->writeObjectInfo("\"albedo\":", jsonLevel);
	jsonWriter->writeObjectInfo("{", jsonLevel);
	jsonWriter->writeObjectInfo("\"enable\":true,", jsonLevel + 1);
	sprintf(buffer, "\"tint\":[%f,%f,%f],", material->mDiffuseColor[0], material->mDiffuseColor[1], material->mDiffuseColor[2]);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);
	sprintf(buffer, "\"factor\":%f,", material->mDiffuseFactor);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"default\":[1.0,1.0,1.0],", jsonLevel + 1);
	writeMapParameters(jsonWriter, map, textureFileName, false, jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"srgb\":true", jsonLevel + 1);
	if (isLastItem)
		jsonWriter->writeObjectInfo("}", jsonLevel);
	else
		jsonWriter->writeObjectInfo("},", jsonLevel);
}

void GAS_V2_Streamer::writeSpecularMap
(
	JSONFileWriter*					jsonWriter,
	Material*						material,
	std::vector<TextureMap*>*		textureMaps,
	std::vector<std::string>*		textureFiles,
	bool							isLastItem,
	int								jsonLevel
)
{
	TextureMap* map = (material->mSpecularMapIndexInDepot >= 0) ? (*textureMaps)[material->mSpecularMapIndexInDepot] : NULL;
	std::string textureFileName = "";
	if (map != NULL)
	{
		//std::string innerTextureFileName = map->GetFileName();
		//innerTextureFileName = extractFileName(innerTextureFileName);
		//int index = (int)reinterpret_cast<unsigned long long>(map->GetUserDataPtr());
		//textureFileName = (*textureFiles)[index];
		//if (innerTextureFileName != textureFileName)
		//{
		//	FBXSDK_printf("Error: Normal map error!");
		//	return;
		//}
		textureFileName = map->fileName;
	}
	char buffer[__TEMP_BUFFER_FLOAT__];
	jsonWriter->writeObjectInfo("\"specular\":", jsonLevel);
	jsonWriter->writeObjectInfo("{", jsonLevel);
	jsonWriter->writeObjectInfo("\"enable\":true,", jsonLevel + 1);
	sprintf(buffer, "\"tint\":[%f,%f,%f],", material->mSpecularColor[0], material->mSpecularColor[1], material->mSpecularColor[2]);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);
	sprintf(buffer, "\"factor\":%f,", material->mSpecularFactor);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"default\":[1.0,1.0,1.0],", jsonLevel + 1);
	writeMapParameters(jsonWriter, map, textureFileName, false, jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"srgb\":true", jsonLevel + 1);
	if (isLastItem)
		jsonWriter->writeObjectInfo("}", jsonLevel);
	else
		jsonWriter->writeObjectInfo("},", jsonLevel);
}

void GAS_V2_Streamer::writeGlossinessMap
(
	JSONFileWriter*					jsonWriter,
	Material*						material,
	std::vector<TextureMap*>*		textureMaps,
	std::vector<std::string>*		textureFiles,
	bool							isLastItem,
	int								jsonLevel
)
{
	char buffer[__TEMP_BUFFER_FLOAT__];
	jsonWriter->writeObjectInfo("\"glossiness\":", jsonLevel);
	jsonWriter->writeObjectInfo("{", jsonLevel);
	sprintf(buffer, "\"factor\":%f,", material->mGlossinessValue / 100.0);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"default\":[1.0],", jsonLevel + 1);
	writeMapParameters(jsonWriter, NULL, "", false, jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"srgb\":false", jsonLevel + 1);
	if (isLastItem)
		jsonWriter->writeObjectInfo("}", jsonLevel);
	else
		jsonWriter->writeObjectInfo("},", jsonLevel);
}

void GAS_V2_Streamer::writeLightMap
(
	JSONFileWriter*					jsonWriter,
	Material*						material,
	std::vector<TextureMap*>*		textureMaps,
	std::vector<std::string>*		textureFiles,
	bool							isLastItem,
	int								jsonLevel
)
{
	jsonWriter->writeObjectInfo("\"light\":", jsonLevel);
	jsonWriter->writeObjectInfo("{", jsonLevel);
	jsonWriter->writeObjectInfo("\"enable\":true,", jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"tint\":[1.0,1.0,1.0],", jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"factor\":1.0,", jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"default\":[1.0,1.0,1.0],", jsonLevel + 1);
	writeMapParameters(jsonWriter, NULL, "", false, jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"srgb\":true", jsonLevel + 1);
	if (isLastItem)
		jsonWriter->writeObjectInfo("}", jsonLevel);
	else
		jsonWriter->writeObjectInfo("},", jsonLevel);
}

void GAS_V2_Streamer::writeNormalMap
(
	JSONFileWriter*					jsonWriter,
	Material*						material,
	std::vector<TextureMap*>*		textureMaps,
	std::vector<std::string>*		textureFiles,
	bool							isLastItem,
	int								jsonLevel
)
{
	TextureMap* map = (material->mBumpMapIndexInDepot >= 0) ? (*textureMaps)[material->mBumpMapIndexInDepot] : NULL;
	std::string textureFileName = "";
	if (map != NULL)
	{
		//std::string innerTextureFileName = map->GetFileName();
		//innerTextureFileName = extractFileName(innerTextureFileName);
		//int index = (int)reinterpret_cast<unsigned long long>(map->GetUserDataPtr());
		//textureFileName = (*textureFiles)[index];
		//if (innerTextureFileName != textureFileName)
		//{
		//	FBXSDK_printf("Error: Normal map error!");
		//	return;
		//}
		textureFileName = map->fileName;
	}
	char buffer[__TEMP_BUFFER_FLOAT__];
	jsonWriter->writeObjectInfo("\"normal\":", jsonLevel);
	jsonWriter->writeObjectInfo("{", jsonLevel);
	jsonWriter->writeObjectInfo("\"enable\":false,", jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"flipY\":false,", jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"factor\":1.0,", jsonLevel + 1);
	sprintf(buffer, "\"default\":[%f,%f,%f],", 0.5, 0.5, 1.0);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);
	writeMapParameters(jsonWriter, map, textureFileName, false, jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"srgb\":false", jsonLevel + 1);
	if (isLastItem)
		jsonWriter->writeObjectInfo("}", jsonLevel);
	else
		jsonWriter->writeObjectInfo("},", jsonLevel);
}

void GAS_V2_Streamer::writeTransparencyMap
(
	JSONFileWriter*					jsonWriter,
	Material*						material,
	std::vector<TextureMap*>*		textureMaps,
	std::vector<std::string>*		textureFiles,
	bool							isLastItem,
	int								jsonLevel
)
{
	TextureMap* map = (material->mTransparencyIndexInDepot >= 0) ? (*textureMaps)[material->mTransparencyIndexInDepot] : NULL;
	std::string textureFileName = "";
	if (map != NULL)
	{
		//std::string innerTextureFileName = map->GetFileName();
		//innerTextureFileName = extractFileName(innerTextureFileName);
		//int index = (int)reinterpret_cast<unsigned long long>(map->GetUserDataPtr());
		//textureFileName = (*textureFiles)[index];
		//if (innerTextureFileName != textureFileName)
		//{
		//	FBXSDK_printf("Error: Transparency map error!");
		//	return;
		//}
		textureFileName = map->fileName;
	}
	char buffer[__TEMP_BUFFER_FLOAT__];
	jsonWriter->writeObjectInfo("\"transparency\":", jsonLevel);
	jsonWriter->writeObjectInfo("{", jsonLevel);
	//jsonWriter->writeObjectInfo("\"enable\":false,", jsonLevel + 1);
	if (material->mTransparencyEnable)
	{
		jsonWriter->writeObjectInfo("\"enable\":true,", jsonLevel + 1);
	}
	else
	{
		jsonWriter->writeObjectInfo("\"enable\":false,", jsonLevel + 1);
	}
	jsonWriter->writeObjectInfo("\"mode\":0,", jsonLevel + 1); //blend additive dither mask
	jsonWriter->writeObjectInfo("\"invert\":false,", jsonLevel + 1);
	sprintf(buffer, "\"factor\":%f,", material->mTransparencyFactor);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);
	//jsonWriter->writeObjectInfo("\"factor\":1.0,", jsonLevel + 1);
	//sprintf(buffer, "\"color\":[%f,%f,%f],", material->mTransparencyColor[0], material->mTransparencyColor[1], material->mTransparencyColor[2]);
	//sprintf(buffer, "\"color\":[%f],", (1.0 - material->mOpacity));
	jsonWriter->writeObjectInfo("\"default\":[1.0],", jsonLevel + 1);
	writeMapParameters(jsonWriter, map, textureFileName, false, jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"srgb\":true", jsonLevel + 1);
	if (isLastItem)
		jsonWriter->writeObjectInfo("}", jsonLevel);
	else
		jsonWriter->writeObjectInfo("},", jsonLevel);
}

void GAS_V2_Streamer::writeEmissiveMap
(
	JSONFileWriter*					jsonWriter,
	Material*						material,
	std::vector<TextureMap*>*		textureMaps,
	std::vector<std::string>*		textureFiles,
	bool							isLastItem,
	int								jsonLevel
)
{
	TextureMap* map = (material->mEmissiveMapIndexInDepot >= 0) ? (*textureMaps)[material->mEmissiveMapIndexInDepot] : NULL;
	std::string textureFileName = "";
	if (map != NULL)
	{
		//std::string innerTextureFileName = map->GetFileName();
		//innerTextureFileName = extractFileName(innerTextureFileName);
		//int index = (int)reinterpret_cast<unsigned long long>(map->GetUserDataPtr());
		//textureFileName = (*textureFiles)[index];
		//if (innerTextureFileName != textureFileName)
		//{
		//	FBXSDK_printf("Error: Emissive map error!");
		//	return;
		//}
		textureFileName = map->fileName;
	}
	char buffer[__TEMP_BUFFER_FLOAT__];
	jsonWriter->writeObjectInfo("\"emissive\":", jsonLevel);
	jsonWriter->writeObjectInfo("{", jsonLevel);
	jsonWriter->writeObjectInfo("\"enable\":false,", jsonLevel + 1);
	sprintf(buffer, "\"tint\":[%f,%f,%f],", material->mEmissiveColor[0], material->mEmissiveColor[1], material->mEmissiveColor[2]);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);
	sprintf(buffer, "\"factor\":%f,", material->mEmissiveFactor);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"default\":[1.0, 1.0, 1.0],", jsonLevel + 1);
	writeMapParameters(jsonWriter, map, textureFileName, false, jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"srgb\":true,", jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"multiplicative\":false", jsonLevel + 1);
	if (isLastItem)
		jsonWriter->writeObjectInfo("}", jsonLevel);
	else
		jsonWriter->writeObjectInfo("},", jsonLevel);
}

void GAS_V2_Streamer::writeDisplacementMap
(
	JSONFileWriter*					jsonWriter,
	Material*						material,
	std::vector<TextureMap*>*		textureMaps,
	std::vector<std::string>*		textureFiles,
	bool							isLastItem,
	int								jsonLevel
)
{
	jsonWriter->writeObjectInfo("\"displacement\":", jsonLevel);
	jsonWriter->writeObjectInfo("{", jsonLevel);
	jsonWriter->writeObjectInfo("\"enable\":false,", jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"factor\":0.0,", jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"default\":[0.5],", jsonLevel + 1);
	writeMapParameters(jsonWriter, NULL, "", false, jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"srgb\":false", jsonLevel + 1);
	if (isLastItem)
		jsonWriter->writeObjectInfo("}", jsonLevel);
	else
		jsonWriter->writeObjectInfo("},", jsonLevel);
}

void GAS_V2_Streamer::writeAoMap
(
	JSONFileWriter*					jsonWriter,
	Material*						material,
	std::vector<TextureMap*>*		textureMaps,
	std::vector<std::string>*		textureFiles,
	bool							isLastItem,
	int								jsonLevel
)
{
	jsonWriter->writeObjectInfo("\"ao\":", jsonLevel);
	jsonWriter->writeObjectInfo("{", jsonLevel);
	jsonWriter->writeObjectInfo("\"enable\":false,", jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"factor\":1.0,", jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"default\":[1.0],", jsonLevel + 1);
	writeMapParameters(jsonWriter, NULL, "", false, jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"occludeSpecular\":false,", jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"srgb\":false", jsonLevel + 1);
	if (isLastItem)
		jsonWriter->writeObjectInfo("}", jsonLevel);
	else
		jsonWriter->writeObjectInfo("},", jsonLevel);
}

void GAS_V2_Streamer::writeCavityMap
(
	JSONFileWriter*					jsonWriter,
	Material*						material,
	std::vector<TextureMap*>*		textureMaps,
	std::vector<std::string>*		textureFiles,
	bool							isLastItem,
	int								jsonLevel
)
{
	jsonWriter->writeObjectInfo("\"cavity\":", jsonLevel);
	jsonWriter->writeObjectInfo("{", jsonLevel);
	jsonWriter->writeObjectInfo("\"enable\":false,", jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"factor\":1.0,", jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"default\":[1.0],", jsonLevel + 1);
	writeMapParameters(jsonWriter, NULL, "", false, jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"srgb\":false", jsonLevel + 1);
	if (isLastItem)
		jsonWriter->writeObjectInfo("}", jsonLevel);
	else
		jsonWriter->writeObjectInfo("},", jsonLevel);
}

void GAS_V2_Streamer::writeMetalnessMap
(
	JSONFileWriter*					jsonWriter,
	Material*						material,
	std::vector<TextureMap*>*		textureMaps,
	std::vector<std::string>*		textureFiles,
	bool							isLastItem,
	int								jsonLevel
)
{
	jsonWriter->writeObjectInfo("\"metalness\":", jsonLevel);
	jsonWriter->writeObjectInfo("{", jsonLevel);
	jsonWriter->writeObjectInfo("\"enable\":false,", jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"factor\":0.0,", jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"default\":[1.0],", jsonLevel + 1);
	writeMapParameters(jsonWriter, NULL, "", false, jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"srgb\":false", jsonLevel + 1);
	if (isLastItem)
		jsonWriter->writeObjectInfo("}", jsonLevel);
	else
		jsonWriter->writeObjectInfo("},", jsonLevel);
}

void GAS_V2_Streamer::writeSpecularF0Map
(
	JSONFileWriter*					jsonWriter,
	Material*						material,
	std::vector<TextureMap*>*		textureMaps,
	std::vector<std::string>*		textureFiles,
	bool							isLastItem,
	int								jsonLevel
)
{
	jsonWriter->writeObjectInfo("\"specularF0\":", jsonLevel);
	jsonWriter->writeObjectInfo("{", jsonLevel);
	jsonWriter->writeObjectInfo("\"enable\":false,", jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"factor\":0.5,", jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"default\":[1.0],", jsonLevel + 1);
	writeMapParameters(jsonWriter, NULL, "", false, jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"srgb\":false", jsonLevel + 1);
	if (isLastItem)
		jsonWriter->writeObjectInfo("}", jsonLevel);
	else
		jsonWriter->writeObjectInfo("},", jsonLevel);
}

void GAS_V2_Streamer::writeRoughnessMap
(
	JSONFileWriter*					jsonWriter,
	Material*						material,
	std::vector<TextureMap*>*		textureMaps,
	std::vector<std::string>*		textureFiles,
	bool							isLastItem,
	int								jsonLevel
)
{
	jsonWriter->writeObjectInfo("\"roughness\":", jsonLevel);
	jsonWriter->writeObjectInfo("{", jsonLevel);
	jsonWriter->writeObjectInfo("\"enable\":true,", jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"factor\":0.6,", jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"default\":[1.0],", jsonLevel + 1);
	writeMapParameters(jsonWriter, NULL, "", false, jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"srgb\":false", jsonLevel + 1);
	if (isLastItem)
		jsonWriter->writeObjectInfo("}", jsonLevel);
	else
		jsonWriter->writeObjectInfo("},", jsonLevel);
}

void GAS_V2_Streamer::writeMatCapMap
(
	JSONFileWriter*					jsonWriter,
	Material*						material,
	std::vector<TextureMap*>*		textureMaps,
	std::vector<std::string>*		textureFiles,
	bool							isLastItem,
	int								jsonLevel
)
{
	jsonWriter->writeObjectInfo("\"matsphere\":", jsonLevel);
	jsonWriter->writeObjectInfo("{", jsonLevel);
	jsonWriter->writeObjectInfo("\"enable\":false,", jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"tint\":[1.0,1.0,1.0],", jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"curvature\":0.0,", jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"factor\":1.0,", jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"default\":[0.8,0.8,0.8],", jsonLevel + 1);
	writeMapParameters(jsonWriter, NULL, "", false, jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"srgb\":true", jsonLevel + 1);
	if (isLastItem)
		jsonWriter->writeObjectInfo("}", jsonLevel);
	else
		jsonWriter->writeObjectInfo("},", jsonLevel);
}

void GAS_V2_Streamer::writeMapParameters
(
	JSONFileWriter* jsonWriter,
	TextureMap* map,
	const std::string& textureName,
	bool isLastItem,
	int jsonLevel
)
{
	char buffer[__TEMP_BUFFER_FLOAT__];
	jsonWriter->writeObjectInfo("\"map\":", jsonLevel);
	jsonWriter->writeObjectInfo("{", jsonLevel);
	FBXSDK_NAMESPACE::FbxDouble3 T(0.0, 0.0, 0.0);
	FBXSDK_NAMESPACE::FbxDouble3 R(0.0, 0.0, 0.0);
	FBXSDK_NAMESPACE::FbxDouble3 S(1.0, 1.0, 1.0);
	FBXSDK_NAMESPACE::FbxDouble3 Rp(0.0, 0.0, 0.0);
	FBXSDK_NAMESPACE::FbxDouble3 Sp(0.0, 0.0, 0.0);
	FBXSDK_NAMESPACE::FbxFileTexture::EWrapMode wrapU = FBXSDK_NAMESPACE::FbxFileTexture::eRepeat;
	FBXSDK_NAMESPACE::FbxFileTexture::EWrapMode wrapV = FBXSDK_NAMESPACE::FbxFileTexture::eRepeat;
	FbxBool uvSwapFlag = false;
	if (map)
	{
		T = map->T;
		R = map->R;
		S = map->S;
		Rp = map->Rp;
		Sp = map->Sp;
		wrapU = map->wrapU;
		wrapV = map->wrapV;
		uvSwapFlag = map->uvSwapFlag;
	}

	if (textureName.size() > 0)
	{
		sprintf(buffer, "\"texture\":\"%s\",", textureName.c_str());
		jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);
	}
	else
	{
		jsonWriter->writeObjectInfo("\"texture\":\"\",", jsonLevel + 1);
	}

	jsonWriter->writeObjectInfo("\"pixelChannels\": 0,", jsonLevel + 1);

	if (wrapU == FBXSDK_NAMESPACE::FbxFileTexture::eRepeat)
	{
		jsonWriter->writeObjectInfo("\"wrapModeU\":\"REPEAT\",", jsonLevel + 1);
	}
	else
	{
		jsonWriter->writeObjectInfo("\"wrapModeU\":\"CLAMP_TO_EDGE\",", jsonLevel + 1);
	}

	if (wrapV == FBXSDK_NAMESPACE::FbxFileTexture::eRepeat)
	{
		jsonWriter->writeObjectInfo("\"wrapModeV\":\"REPEAT\",", jsonLevel + 1);
	}
	else
	{
		jsonWriter->writeObjectInfo("\"wrapModeV\":\"CLAMP_TO_EDGE\",", jsonLevel + 1);
	}

	jsonWriter->writeObjectInfo("\"minFilter\":\"LINEAR_MIPMAP_LINEAR\",", jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"maxFilter\":\"LINEAR\",", jsonLevel + 1);
	sprintf(buffer, "\"uvSwap\":%s,", uvSwapFlag ? "true" : "false");
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);
	sprintf(buffer, "\"T\":[%f,%f,%f],", T[0], T[1], T[2]);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);
	sprintf(buffer, "\"R\":[%f,%f,%f],", R[0], R[1], R[2]);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);
	sprintf(buffer, "\"S\":[%f,%f,%f],", S[0], S[1], S[2]);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);
	sprintf(buffer, "\"Rp\":[%f,%f,%f],", Rp[0], Rp[1], Rp[2]);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);
	sprintf(buffer, "\"Sp\":[%f,%f,%f]", Sp[0], Sp[1], Sp[2]);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);

	if (isLastItem)
	{
		jsonWriter->writeObjectInfo("}", jsonLevel);
	}
	else
	{
		jsonWriter->writeObjectInfo("},", jsonLevel);
	}
}

void GAS_V2_Streamer::saveOutputLog(const std::string& path)
{
	FBXSDK_printf("***************** Output log *****************\n");
	JSONFileWriter* resultFile = new JSONFileWriter(false);
	resultFile->openExportFile(path);

	resultFile->writeObjectInfo("{", 0);
	resultFile->writeObjectInfo("\"version\":\"gas2\",", 1);
	resultFile->writeObjectInfo("\"files\":", 1);
	resultFile->writeObjectInfo("[", 1);

	const int FILE_PATH_MAX_LENGTH = 2048;
	char buffer[FILE_PATH_MAX_LENGTH * 2];

	size_t fileCount = mOutputFiles.size();
	for (size_t i = 0; i < fileCount; ++i)
	{
		std::string fileName = getFileName(mOutputFiles[i]);
		FBXSDK_printf("%s\n", fileName.c_str());
		if (fileName.length() >= FILE_PATH_MAX_LENGTH)
		{
			FBXSDK_printf("Error: output file path exceeds the up limit!\n");
			fileName = "Error: output file path exceeds the up limit!";
		}
		if (i == fileCount - 1)
			sprintf(buffer, "\"%s\"", fileName.c_str());
		else
			sprintf(buffer, "\"%s\",", fileName.c_str());

		resultFile->writeObjectInfo(buffer, 2);
	}

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