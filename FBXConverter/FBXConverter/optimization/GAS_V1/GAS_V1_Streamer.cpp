#include "stdafx.h"
#include "GAS_V1_Streamer.h"
#include "JSONFileWriter.h"
#include "TextureMap.h"
#include "Material.h"
#include "NeonateVertexCompression_V4.h"
#include "JsonToBin_V4.h"

GAS_V1_Streamer::GAS_V1_Streamer()
: mTotalTriangleCount(0)
, mTotalPolygonCount(0)
, mTotalVertexCount(0)
{

}

GAS_V1_Streamer::~GAS_V1_Streamer()
{

}

bool GAS_V1_Streamer::save
(
	const std::string& directoryPath,
	const std::string& modelFileName,
	bool jsonBeautify, 
	bool generateGZFile, 
	unsigned int meshOptimization,
	unsigned int animationOptimization
)
{
	//NOTICE��saveMeshes must be called before saveHierarchy, so as to get the bounding box of meshese.
	if (mMeshes != NULL)
	{
		saveMeshes(directoryPath, modelFileName, generateGZFile, meshOptimization);
	}

	if(mMaterials != NULL && mTextures != NULL)
	{
		std::string sceneFile = directoryPath + modelFileName + ".scene.json";
		saveScene(sceneFile, jsonBeautify);
		#ifdef _MSC_VER
		if (generateGZFile)
		{
			std::string compressedFileName = compressFile(sceneFile);
			mOutputFiles.push_back(compressedFileName);
		}
		else
		{
			mOutputFiles.push_back(sceneFile);
		}
		#else
			mOutputFiles.push_back(sceneFile);
		#endif
	}

	if(mHierarchy != NULL)
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

	if(mAnimations != NULL)
	{
		saveAnimations(directoryPath, modelFileName, generateGZFile, animationOptimization);
	}

	std::string outputLogFile = directoryPath + modelFileName + ".convertedFiles";
	mOutputFiles.push_back(outputLogFile);

	if(mTextures != NULL)
	{
		//assetCount mTextureAsset = { 0,0,0,0,0,0,0,0 };
		std::vector<std::string> textures;
		for (size_t s = 0; s < (*mTextures).size(); ++s)
		{
			mOutputFiles.push_back((*mTextures)[s]);
			textures.push_back((*mTextures)[s]);
		}
		//mTextureAsset.textures = textures;
		//mAssetCounts.push_back(mTextureAsset);
	}

	saveOutputLog(outputLogFile);

	return true;
}

void GAS_V1_Streamer::saveScene(const std::string& sceneFilePath, bool beautified)
{
	JSONFileWriter* jsonWriter = new JSONFileWriter(beautified);
	jsonWriter->openExportFile(sceneFilePath);

	int jsonLevel = 0;
	jsonWriter->writeObjectInfo("{", jsonLevel);

	char buffer[1024];
	sprintf(buffer, "\"version\":%d,", 4);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);

	//Renderer
	jsonWriter->writeObjectInfo("\"renderer\":", jsonLevel + 1);
	jsonWriter->writeObjectInfo("{", jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"version\":1,", jsonLevel + 2);
	jsonWriter->writeObjectInfo("\"shading\":0,", jsonLevel + 2);	//lighting:0   shadeless:1
	jsonWriter->writeObjectInfo("\"wireframe\":false", jsonLevel + 2);
	jsonWriter->writeObjectInfo("},", jsonLevel + 1);

	//PPE
	jsonWriter->writeObjectInfo("\"postprocesseffects\":", jsonLevel + 1);
	jsonWriter->writeObjectInfo("{", jsonLevel + 1);
	writePostProcessEffectConfig(jsonWriter, jsonLevel + 2);
	jsonWriter->writeObjectInfo("},", jsonLevel + 1);

	//Background
	jsonWriter->writeObjectInfo("\"background\":", jsonLevel + 1);
	jsonWriter->writeObjectInfo("{", jsonLevel + 1);
	jsonWriter->writeObjectInfo("\"enable\":true,", jsonLevel + 2);
	jsonWriter->writeObjectInfo("\"type\":1,", jsonLevel + 2); //Environment:0  image:1  color:2
	jsonWriter->writeObjectInfo("\"environmentType\":0,", jsonLevel + 2); //Cube:0  Panorama:1

	sprintf(buffer, "\"environment\":\"%s\",", "10_Small_Waterfall_In_Park");
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 2);

	jsonWriter->writeObjectInfo("\"environmentIndex\":4,", jsonLevel + 2);
	jsonWriter->writeObjectInfo("\"orientation\":0.0,", jsonLevel + 2);
	jsonWriter->writeObjectInfo("\"ambientEnvironment\":true,", jsonLevel + 2);
	jsonWriter->writeObjectInfo("\"environmentExposure\":1.0,", jsonLevel + 2);
	jsonWriter->writeObjectInfo("\"backgroundExposure\":1.0,", jsonLevel + 2);
	jsonWriter->writeObjectInfo("\"image\":\"BG02-dark.jpg\",", jsonLevel + 2);
	jsonWriter->writeObjectInfo("\"color\":[0.0, 0.0, 0.0]", jsonLevel + 2);
	jsonWriter->writeObjectInfo("},", jsonLevel + 1);

	//Cameras
	jsonWriter->writeObjectInfo("\"cameraConfig\":", jsonLevel + 1);
	jsonWriter->writeObjectInfo("{", jsonLevel + 1);	//
	jsonWriter->writeObjectInfo("\"defaultCamera\":0,", jsonLevel + 2);
	jsonWriter->writeObjectInfo("\"cameras\":", jsonLevel + 2);
	jsonWriter->writeObjectInfo("[", jsonLevel + 2);
	jsonWriter->writeObjectInfo("{", jsonLevel + 3);
	jsonWriter->writeObjectInfo("\"name\":\"Camera 0\",", jsonLevel + 4);
	jsonWriter->writeObjectInfo("\"FOV\":60.0", jsonLevel + 4);
	jsonWriter->writeObjectInfo("}", jsonLevel + 3);
	jsonWriter->writeObjectInfo("]", jsonLevel + 2);
	jsonWriter->writeObjectInfo("},", jsonLevel + 1);

	//Lights
	jsonWriter->writeObjectInfo("\"lights\":", jsonLevel + 1);
	jsonWriter->writeObjectInfo("[", jsonLevel + 1);
	writeLights(jsonWriter, jsonLevel + 2);
	jsonWriter->writeObjectInfo("],", jsonLevel + 1);

	//Lights
	jsonWriter->writeObjectInfo("\"hotspots\":", jsonLevel + 1);
	jsonWriter->writeObjectInfo("[", jsonLevel + 1);
	jsonWriter->writeObjectInfo("],", jsonLevel + 1);

	//Texture Files
	jsonWriter->writeObjectInfo("\"textures\":", jsonLevel + 1);
	jsonWriter->writeObjectInfo("[", jsonLevel + 1);
	writeTextures(jsonWriter, *mTextures, jsonLevel + 2);
	jsonWriter->writeObjectInfo("],", jsonLevel + 1);
	//End Texture Files

	//Material Maps
	jsonWriter->writeObjectInfo("\"maps\":", jsonLevel + 1);
	jsonWriter->writeObjectInfo("[", jsonLevel + 1);
	writeMaps(jsonWriter, *mTextureMaps, *mTextures, jsonLevel + 2);
	jsonWriter->writeObjectInfo("],", jsonLevel + 1);
	//End Material Maps

	//Materials
	jsonWriter->writeObjectInfo("\"materials\":", jsonLevel + 1);
	jsonWriter->writeObjectInfo("[", jsonLevel + 1);
	writeMaterials(jsonWriter, *mMaterials, jsonLevel + 2);
	jsonWriter->writeObjectInfo("]", jsonLevel + 1);

	jsonWriter->writeObjectInfo("}", jsonLevel);

	jsonWriter->closeExportFile();
	delete jsonWriter;
	jsonWriter = NULL;
}

void GAS_V1_Streamer::saveStructure(const std::string& structureFilePath, const std::string& modelFileName, bool beautified)
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
	writer->writeObjectInfo("\"version\":4,", padding + 1);
	writer->writeObjectInfo("\"srcVersion\":\"FBX\",", padding + 1);
	writer->writeObjectInfo("\"name\":\"" + modelFileName + "\",", padding + 1);
	writer->writeObjectInfo("\"setting\":{},", padding + 1);
	writer->writeObjectInfo("\"nodeTree\":", padding + 1);

	writeHierarchy_r(writer, root, padding + 2, true);

	writer->writeObjectInfo("}", padding);
	writer->closeExportFile();
	delete writer;
	writer = NULL;
}

void GAS_V1_Streamer::writeHierarchy_r(JSONFileWriter* writer, Node* node, int padding, bool isLast)
{
	char buffer[1024];

	writer->writeObjectInfo("{", padding - 1);

	sprintf(buffer, "\"uniqueID\":%d,", node->uniuqeID);
	writer->writeObjectInfo(buffer, padding);

	sprintf(buffer, "\"guid\":\"%s\",", node->guid.c_str());
	writer->writeObjectInfo(buffer, padding);

	writer->writeObjectInfo("\"name\":\"" + node->name + "\",", padding);

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

	writer->writeObjectInfo("\"materials\":", padding);
	writer->writeObjectInfo("[", padding);
	size_t materialCount = node->materials.size();
	for (size_t i = 0; i < materialCount; ++i)
	{
		unsigned int index = node->materials[i];
		if (i == materialCount - 1)
		{
			sprintf(buffer, "%d", (int)index);
		}
		else
		{
			sprintf(buffer, "%d,", (int)index);
		}
		writer->writeObjectInfo(buffer, padding + 1);
	}
	writer->writeObjectInfo("],", padding);

	writer->writeObjectInfo("\"nodeAttr\":", padding);
	writer->writeObjectInfo("{", padding);

	if (node->meshInfo != NULL)
	{
		sprintf(buffer, "\"uniqueID\":%d,", node->meshInfo->uniqueID);
		writer->writeObjectInfo(buffer, padding + 1);

		writer->writeObjectInfo("\"name\":\"" + node->meshInfo->meshName + "\",", padding + 1);

		writer->writeObjectInfo("\"type\":\"mesh\",", padding + 1);

		FBXSDK_NAMESPACE::FbxDouble3& bboxMin = node->meshInfo->bboxMin;
		FBXSDK_NAMESPACE::FbxDouble3& bboxMax = node->meshInfo->bboxMax;
		writer->writeObjectInfo("\"bbox\":", padding + 1);
		writer->writeObjectInfo("[", padding + 1);
		sprintf(buffer, "[%0.3f,%0.3f,%0.3f],", bboxMin[0], bboxMin[1], bboxMin[2]);
		writer->writeObjectInfo(buffer, padding + 2);
		sprintf(buffer, "[%0.3f,%0.3f,%0.3f]", bboxMax[0], bboxMax[1], bboxMax[2]);
		writer->writeObjectInfo(buffer, padding + 2);
		writer->writeObjectInfo("]", padding + 1);
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

void GAS_V1_Streamer::writeTextures(JSONFileWriter* jsonWriter, std::vector<std::string>& textures, int jsonLevel)
{
	for (int i = 0; i < (int)textures.size(); ++i)
	{
		char buffer[__TEMP_BUFFER_FLOAT__];

		jsonWriter->writeObjectInfo("{", jsonLevel);
		
		std::string fileName = getFileName(textures[i]);

		sprintf(buffer, "\"internalName\":\"%s\",", fileName.c_str());
		jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);

		sprintf(buffer, "\"displayName\":\"%s\",", fileName.c_str());
		jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);

		sprintf(buffer, "\"width\":%d,", 0);
		jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);

		sprintf(buffer, "\"height\":%d,", 0);
		jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);

		jsonWriter->writeObjectInfo("\"format\":0,", jsonLevel + 1); //RGB:0
		jsonWriter->writeObjectInfo("\"pixelType\":0", jsonLevel + 1); //UNSIGNED_BYTE:0

		if (i != (int)textures.size() - 1)
		{
			jsonWriter->writeObjectInfo("},", jsonLevel);
		}
		else
		{
			jsonWriter->writeObjectInfo("}", jsonLevel);
		}
	}
}

void GAS_V1_Streamer::writeMapParameters(
	JSONFileWriter* jsonWriter,
	TextureMap* map,
	int mapID,
	std::vector<std::string>* textures,
	bool isLastItem,
	int jsonLevel)
{
	char buffer[__TEMP_BUFFER_FLOAT__];

	jsonWriter->writeObjectInfo("{", jsonLevel);

	FBXSDK_NAMESPACE::FbxDouble3 T(0.0, 0.0, 0.0);
	FBXSDK_NAMESPACE::FbxDouble3 R(0.0, 0.0, 0.0);
	FBXSDK_NAMESPACE::FbxDouble3 S(1.0, 1.0, 1.0);
	FBXSDK_NAMESPACE::FbxDouble3 Rp(0.0, 0.0, 0.0);
	FBXSDK_NAMESPACE::FbxDouble3 Sp(0.0, 0.0, 0.0);

	FBXSDK_NAMESPACE::FbxFileTexture::EWrapMode wrapU = FBXSDK_NAMESPACE::FbxFileTexture::eRepeat;
	FBXSDK_NAMESPACE::FbxFileTexture::EWrapMode wrapV = FBXSDK_NAMESPACE::FbxFileTexture::eRepeat;
	FbxBool uvSwapFlag = false;
	std::string fileName = "";

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

		fileName = getFileName(map->fileName);
	}

	sprintf(buffer, "\"id\":%d,", mapID);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);

	sprintf(buffer, "\"texture\":\"%s\",", fileName.c_str());
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);

	sprintf(buffer, "\"wrapModeU\":%d,", wrapU);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);

	sprintf(buffer, "\"wrapModeV\":%d,", wrapV);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);

	sprintf(buffer, "\"minFilter\":%d,", 5);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);

	sprintf(buffer, "\"maxFilter\":%d,", 3);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);

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

void GAS_V1_Streamer::writeMaps(
	JSONFileWriter* jsonWriter,
	std::vector<TextureMap*>& textureMaps,
	std::vector<std::string>& textures,
	int jsonLevel)
{
	for (int i = 0; i < (int)textureMaps.size(); ++i)
	{
		TextureMap* map = textureMaps[i];

		bool isLastItem = (i == (int)textureMaps.size() - 1) ? true : false;

		writeMapParameters(jsonWriter, map, i, &textures, isLastItem, jsonLevel);
	}
}

void GAS_V1_Streamer::writePostProcessEffectConfig(JSONFileWriter* jsonWriter, int jsonLevel)
{
	jsonWriter->writeObjectInfo("\"enable\":false,", jsonLevel);
	jsonWriter->writeObjectInfo("\"glow\":", jsonLevel);
	jsonWriter->writeObjectInfo("{", jsonLevel);
	{
		jsonWriter->writeObjectInfo("\"enable\":false,", jsonLevel + 1);
		jsonWriter->writeObjectInfo("\"threshold\":0.0,", jsonLevel + 1);
		jsonWriter->writeObjectInfo("\"intensity\":1.0,", jsonLevel + 1);
		jsonWriter->writeObjectInfo("\"radius\":1.0", jsonLevel + 1);
	}
	jsonWriter->writeObjectInfo("},", jsonLevel);

	jsonWriter->writeObjectInfo("\"colorGrading\":", jsonLevel);
	jsonWriter->writeObjectInfo("{", jsonLevel);
	{
		jsonWriter->writeObjectInfo("\"enable\":false,", jsonLevel + 1);
		jsonWriter->writeObjectInfo("\"src\":null", jsonLevel + 1);
	}
	jsonWriter->writeObjectInfo("}", jsonLevel);
}

void GAS_V1_Streamer::writeLights(JSONFileWriter* jsonWriter, int jsonLevel)
{
	jsonWriter->writeObjectInfo("{", jsonLevel);
	{
		jsonWriter->writeObjectInfo("\"uniqueID\":-1001,", jsonLevel + 1);
		jsonWriter->writeObjectInfo("\"name\":\"Ambient Light 0\",", jsonLevel + 1);
		jsonWriter->writeObjectInfo("\"type\":\"ambientLight\",", jsonLevel + 1);
		jsonWriter->writeObjectInfo("\"color\":[1.0, 1.0, 1.0],", jsonLevel + 1);
		jsonWriter->writeObjectInfo("\"intensity\":0.5", jsonLevel + 1);
	}
	jsonWriter->writeObjectInfo("},", jsonLevel);

	jsonWriter->writeObjectInfo("{", jsonLevel);
	{
		jsonWriter->writeObjectInfo("\"uniqueID\":-1002,", jsonLevel + 1);
		jsonWriter->writeObjectInfo("\"name\":\"Directional Light 0\",", jsonLevel + 1);
		jsonWriter->writeObjectInfo("\"type\":\"directionalLight\",", jsonLevel + 1);
		jsonWriter->writeObjectInfo("\"color\":[1.0, 1.0, 1.0],", jsonLevel + 1);
		jsonWriter->writeObjectInfo("\"intensity\":0.8,", jsonLevel + 1);
		jsonWriter->writeObjectInfo("\"direction\":[-1.0, -1.0, 0.0]", jsonLevel + 1);
	}
	jsonWriter->writeObjectInfo("}", jsonLevel);
}

void GAS_V1_Streamer::writeMaterialParameters(
	JSONFileWriter* jsonWriter,
	bool separateFile,
	Material* cache,
	std::vector<TextureMap*>* textureMaps,
	std::vector<std::string>* textureFiles,
	bool isLastItem,
	int jsonLevel)
{
	char buffer[__TEMP_BUFFER_FLOAT__];

	jsonWriter->writeObjectInfo("{", jsonLevel);

	sprintf(buffer, "\"uniqueID\":%d,", cache->mUniqueID);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);

	jsonWriter->writeObjectInfo("\"type\":1,", jsonLevel + 1); //0:classic  1:dielectric   2:electric
	jsonWriter->writeObjectInfo("\"visible\":true,", jsonLevel + 1);

	sprintf(buffer, "\"name\":\"%s\",", (const char*)cache->mMaterialName);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);

	sprintf(buffer, "\"culling\":%d,", cache->mCulling);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);

	//NormalMap
	jsonWriter->writeObjectInfo("\"Normal\":", jsonLevel + 1);
	jsonWriter->writeObjectInfo("{", jsonLevel + 1);
	if (cache->mBumpEnable)
	{
		jsonWriter->writeObjectInfo("\"enable\":true,", jsonLevel + 2);
	}
	else
	{
		jsonWriter->writeObjectInfo("\"enable\":false,", jsonLevel + 2);
	}

	jsonWriter->writeObjectInfo("\"flipY\":false,", jsonLevel + 2);
	jsonWriter->writeObjectInfo("\"factor\":1.0,", jsonLevel + 2);
	jsonWriter->writeObjectInfo("\"channel\":1,", jsonLevel + 2); //RGB, RG
	sprintf(buffer, "\"color\":[%f,%f,%f],", 0.5, 0.5, 1.0);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 2);

	if (separateFile)
	{
		TextureMap* map = NULL;
		int mapIndex = cache->mBumpMapIndexInDepot;
		if (mapIndex >= 0)
		{
			map = (*textureMaps)[mapIndex];
		}

		jsonWriter->writeObjectInfo("\"map\":", jsonLevel + 2);
		writeMapParameters(jsonWriter, map, mapIndex, textureFiles, false, jsonLevel + 2);
	}
	else
	{
		sprintf(buffer, "\"map\":%d,", cache->mBumpMapIndexInDepot); //TODO: In 3ds max bump is normal
		jsonWriter->writeObjectInfo(buffer, jsonLevel + 2);
	}
	jsonWriter->writeObjectInfo("\"sRGB\":false", jsonLevel + 2);
	jsonWriter->writeObjectInfo("},", jsonLevel + 1);

	//Opacity & Transparency
	jsonWriter->writeObjectInfo("\"Transparency\":", jsonLevel + 1);
	jsonWriter->writeObjectInfo("{", jsonLevel + 1);
	if (cache->mTransparencyEnable)
	{
		jsonWriter->writeObjectInfo("\"enable\":true,", jsonLevel + 2);
	}
	else
	{
		jsonWriter->writeObjectInfo("\"enable\":false,", jsonLevel + 2);
	}

	sprintf(buffer, "\"mode\":%d,", 0); //blend additive dither mask
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 2);
	jsonWriter->writeObjectInfo("\"invert\":false,", jsonLevel + 2);
	sprintf(buffer, "\"factor\":%f,", cache->mTransparencyFactor);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 2);
	jsonWriter->writeObjectInfo("\"channel\":0,", jsonLevel + 2);

	//sprintf(buffer, "\"color\":[%f,%f,%f,1.0],", cache->mTransparencyColor[0], cache->mTransparencyColor[1], cache->mTransparencyColor[2]);
	//jsonWriter->writeObjectInfo(buffer, jsonLevel + 2);
	//sprintf(buffer, "\"color\":[%f],", (1.0 - cache->mOpacity));
	jsonWriter->writeObjectInfo("\"color\":[1.0, 1.0, 1.0, 1.0],", jsonLevel + 2);

	if (separateFile)
	{
		TextureMap* map = NULL;
		int mapIndex = cache->mTransparencyIndexInDepot;
		if (mapIndex >= 0)
		{
			map = (*textureMaps)[mapIndex];
		}
		jsonWriter->writeObjectInfo("\"map\":", jsonLevel + 2);
		writeMapParameters(jsonWriter, map, mapIndex, textureFiles, false, jsonLevel + 2);
	}
	else
	{
		sprintf(buffer, "\"map\":%d,", cache->mTransparencyIndexInDepot); //TODO: In 3ds max bump is normal
		jsonWriter->writeObjectInfo(buffer, jsonLevel + 2);
	}

	jsonWriter->writeObjectInfo("\"sRGB\":false", jsonLevel + 2);
	jsonWriter->writeObjectInfo("},", jsonLevel + 1);

	//Emissive
	jsonWriter->writeObjectInfo("\"Emissive\":", jsonLevel + 1);

	jsonWriter->writeObjectInfo("{", jsonLevel + 1);
	if (cache->mEmissiveFactor > 0.0 &&
		(cache->mEmissiveColor[0] > 0.0 ||
			cache->mEmissiveColor[1] > 0.0 ||
			cache->mEmissiveColor[2] > 0.0 ||
			cache->mEmissiveMapIndexInDepot >= 0))
	{
		jsonWriter->writeObjectInfo("\"enable\":true,", jsonLevel + 2);
	}
	else
	{
		jsonWriter->writeObjectInfo("\"enable\":false,", jsonLevel + 2);
	}

	sprintf(buffer, "\"colorBias\":[%f,%f,%f],", cache->mEmissiveColor[0], cache->mEmissiveColor[1], cache->mEmissiveColor[2]);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 2);
	sprintf(buffer, "\"factor\":%f,", cache->mEmissiveFactor);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 2);
	jsonWriter->writeObjectInfo("\"color\":[1.0, 1.0, 1.0],", jsonLevel + 2);
	if (separateFile)
	{
		TextureMap* map = NULL;

		int mapIndex = cache->mEmissiveMapIndexInDepot;
		if (mapIndex >= 0)
		{
			map = (*textureMaps)[mapIndex];
		}

		jsonWriter->writeObjectInfo("\"map\":", jsonLevel + 2);
		writeMapParameters(jsonWriter, map, mapIndex, textureFiles, false, jsonLevel + 2);
	}
	else
	{
		sprintf(buffer, "\"map\":%d,", cache->mEmissiveMapIndexInDepot); //TODO: In 3ds max bump is normal
		jsonWriter->writeObjectInfo(buffer, jsonLevel + 2);
	}

	jsonWriter->writeObjectInfo("\"sRGB\":true,", jsonLevel + 2);
	jsonWriter->writeObjectInfo("\"multiplicative\":false", jsonLevel + 2);
	jsonWriter->writeObjectInfo("},", jsonLevel + 1);

	//Displacement
	jsonWriter->writeObjectInfo("\"Displacement\":", jsonLevel + 1);
	jsonWriter->writeObjectInfo("{", jsonLevel + 1);
	{
		jsonWriter->writeObjectInfo("\"enable\":false,", jsonLevel + 2);
		jsonWriter->writeObjectInfo("\"factor\":0.0,", jsonLevel + 2);
		jsonWriter->writeObjectInfo("\"channel\":0,", jsonLevel + 2);
		jsonWriter->writeObjectInfo("\"color\":[0.5],", jsonLevel + 2);
		jsonWriter->writeObjectInfo("\"map\":-1,", jsonLevel + 2);
		jsonWriter->writeObjectInfo("\"sRGB\":false", jsonLevel + 2);
	}
	jsonWriter->writeObjectInfo("},", jsonLevel + 1);

	//Classic Preset
	jsonWriter->writeObjectInfo("\"Classic\":", jsonLevel + 1);
	jsonWriter->writeObjectInfo("{", jsonLevel + 1);
	//Diffuse
	jsonWriter->writeObjectInfo("\"Diffuse\":", jsonLevel + 2);
	jsonWriter->writeObjectInfo("{", jsonLevel + 2);
	jsonWriter->writeObjectInfo("\"enable\":true,", jsonLevel + 3);
	sprintf(buffer, "\"colorBias\":[%f,%f,%f],", cache->mDiffuseColor[0], cache->mDiffuseColor[1], cache->mDiffuseColor[2]);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 3);
	sprintf(buffer, "\"factor\":%f,", cache->mDiffuseFactor);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 3);
	jsonWriter->writeObjectInfo("\"color\":[1.0,1.0,1.0],", jsonLevel + 3);

	if (separateFile)
	{
		TextureMap* map = NULL;

		int mapIndex = cache->mDiffuseMapIndexInDepot;
		if (mapIndex >= 0)
		{
			map = (*textureMaps)[mapIndex];
		}

		jsonWriter->writeObjectInfo("\"map\":", jsonLevel + 3);
		writeMapParameters(jsonWriter, map, mapIndex, textureFiles, false, jsonLevel + 3);
	}
	else
	{
		sprintf(buffer, "\"map\":%d,", cache->mDiffuseMapIndexInDepot); //TODO: In 3ds max bump is normal
		jsonWriter->writeObjectInfo(buffer, jsonLevel + 3);
	}
	jsonWriter->writeObjectInfo("\"sRGB\":true", jsonLevel + 3);
	jsonWriter->writeObjectInfo("},", jsonLevel + 2);

	//Specular
	jsonWriter->writeObjectInfo("\"Specular\":", jsonLevel + 2);
	jsonWriter->writeObjectInfo("{", jsonLevel + 2);
	jsonWriter->writeObjectInfo("\"enable\":true,", jsonLevel + 3);
	sprintf(buffer, "\"colorBias\":[%f,%f,%f],", cache->mSpecularColor[0], cache->mSpecularColor[1], cache->mSpecularColor[2]);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 3);
	sprintf(buffer, "\"factor\":%f,", cache->mSpecularFactor);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 3);
	jsonWriter->writeObjectInfo("\"color\":[1.0,1.0,1.0],", jsonLevel + 3);

	if (separateFile)
	{
		TextureMap* map = NULL;

		int mapIndex = cache->mSpecularMapIndexInDepot;
		if (mapIndex >= 0)
		{
			map = (*textureMaps)[mapIndex];
		}

		jsonWriter->writeObjectInfo("\"map\":", jsonLevel + 3);
		writeMapParameters(jsonWriter, map, mapIndex, textureFiles, false, jsonLevel + 3);
	}
	else
	{
		sprintf(buffer, "\"map\":%d,", cache->mSpecularMapIndexInDepot); //TODO: In 3ds max bump is normal
		jsonWriter->writeObjectInfo(buffer, jsonLevel + 3);
	}

	jsonWriter->writeObjectInfo("\"sRGB\":true", jsonLevel + 3);
	jsonWriter->writeObjectInfo("},", jsonLevel + 2);

	//Glossiness
	jsonWriter->writeObjectInfo("\"Glossiness\":", jsonLevel + 2);
	jsonWriter->writeObjectInfo("{", jsonLevel + 2);
	sprintf(buffer, "\"factor\":%f,", cache->mGlossinessValue / 100.0);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 3);
	jsonWriter->writeObjectInfo("\"channel\":0,", jsonLevel + 3);
	jsonWriter->writeObjectInfo("\"color\":[1.0],", jsonLevel + 3);
	jsonWriter->writeObjectInfo("\"map\":-1,", jsonLevel + 3);
	jsonWriter->writeObjectInfo("\"sRGB\":false", jsonLevel + 3);
	jsonWriter->writeObjectInfo("}", jsonLevel + 2);
	jsonWriter->writeObjectInfo("},", jsonLevel + 1);

	//PBR Preset
	jsonWriter->writeObjectInfo("\"PBR\":", jsonLevel + 1);
	jsonWriter->writeObjectInfo("{", jsonLevel + 1);
	//AO
	jsonWriter->writeObjectInfo("\"AO\":", jsonLevel + 2);
	jsonWriter->writeObjectInfo("{", jsonLevel + 2);
	{
		jsonWriter->writeObjectInfo("\"enable\":false,", jsonLevel + 3);
		jsonWriter->writeObjectInfo("\"factor\":1.0,", jsonLevel + 3);
		jsonWriter->writeObjectInfo("\"channel\":0,", jsonLevel + 3);
		jsonWriter->writeObjectInfo("\"color\":[1.0],", jsonLevel + 3);
		jsonWriter->writeObjectInfo("\"map\":-1,", jsonLevel + 3);
		jsonWriter->writeObjectInfo("\"occludeSpecular\":false,", jsonLevel + 3);
		jsonWriter->writeObjectInfo("\"sRGB\":false", jsonLevel + 3);
	}
	jsonWriter->writeObjectInfo("},", jsonLevel + 2);

	//Cavity
	jsonWriter->writeObjectInfo("\"Cavity\":", jsonLevel + 2);
	jsonWriter->writeObjectInfo("{", jsonLevel + 2);
	{
		jsonWriter->writeObjectInfo("\"enable\":false,", jsonLevel + 3);
		jsonWriter->writeObjectInfo("\"factor\":1.0,", jsonLevel + 3);
		jsonWriter->writeObjectInfo("\"channel\":0,", jsonLevel + 3);
		jsonWriter->writeObjectInfo("\"color\":[1.0],", jsonLevel + 3);
		jsonWriter->writeObjectInfo("\"map\":-1,", jsonLevel + 3);
		jsonWriter->writeObjectInfo("\"sRGB\":false", jsonLevel + 3);
	}
	jsonWriter->writeObjectInfo("},", jsonLevel + 2);

	//Two types of Material
	jsonWriter->writeObjectInfo("\"Dielectric\":", jsonLevel + 2);
	jsonWriter->writeObjectInfo("{", jsonLevel + 2);
	{
		//
		jsonWriter->writeObjectInfo("\"BaseColor\":", jsonLevel + 3);
		jsonWriter->writeObjectInfo("{", jsonLevel + 3);
		{
			sprintf(buffer, "\"colorBias\":[%f,%f,%f],", cache->mDiffuseColor[0], cache->mDiffuseColor[1], cache->mDiffuseColor[2]);
			jsonWriter->writeObjectInfo(buffer, jsonLevel + 4);

			jsonWriter->writeObjectInfo("\"factor\":1.0,", jsonLevel + 4);

			jsonWriter->writeObjectInfo("\"color\":[1.0,1.0,1.0],", jsonLevel + 4);

			if (separateFile)
			{
				TextureMap* map = NULL;

				int mapIndex = cache->mDiffuseMapIndexInDepot;
				if (mapIndex >= 0)
				{
					map = (*textureMaps)[mapIndex];
				}

				jsonWriter->writeObjectInfo("\"map\":", jsonLevel + 4);
				writeMapParameters(jsonWriter, map, mapIndex, textureFiles, false, jsonLevel + 4);
			}
			else
			{
				sprintf(buffer, "\"map\":%d,", cache->mDiffuseMapIndexInDepot); //TODO: In 3ds max bump is normal
				jsonWriter->writeObjectInfo(buffer, jsonLevel + 4);
			}

			jsonWriter->writeObjectInfo("\"sRGB\":true", jsonLevel + 4);
		}
		jsonWriter->writeObjectInfo("},", jsonLevel + 3);
		//
		jsonWriter->writeObjectInfo("\"Metalness\":", jsonLevel + 3);
		jsonWriter->writeObjectInfo("{", jsonLevel + 3);
		{
			jsonWriter->writeObjectInfo("\"factor\":0.0,", jsonLevel + 4);
			jsonWriter->writeObjectInfo("\"channel\":0,", jsonLevel + 4);
			jsonWriter->writeObjectInfo("\"color\":[1.0],", jsonLevel + 4);
			jsonWriter->writeObjectInfo("\"map\":-1,", jsonLevel + 4);
			jsonWriter->writeObjectInfo("\"sRGB\":false", jsonLevel + 4);
		}
		jsonWriter->writeObjectInfo("},", jsonLevel + 3);
		//
		jsonWriter->writeObjectInfo("\"SpecularF0\":", jsonLevel + 3);
		jsonWriter->writeObjectInfo("{", jsonLevel + 3);
		{
			jsonWriter->writeObjectInfo("\"factor\":0.5,", jsonLevel + 4);
			jsonWriter->writeObjectInfo("\"channel\":0,", jsonLevel + 4);
			jsonWriter->writeObjectInfo("\"color\":[1.0],", jsonLevel + 4);
			jsonWriter->writeObjectInfo("\"map\":-1,", jsonLevel + 4);
			jsonWriter->writeObjectInfo("\"sRGB\":false", jsonLevel + 4);
		}
		jsonWriter->writeObjectInfo("},", jsonLevel + 3);
		//
		jsonWriter->writeObjectInfo("\"Roughness\":", jsonLevel + 3);
		jsonWriter->writeObjectInfo("{", jsonLevel + 3);
		{
			jsonWriter->writeObjectInfo("\"factor\":0.6,", jsonLevel + 4);
			jsonWriter->writeObjectInfo("\"channel\":0,", jsonLevel + 4);
			jsonWriter->writeObjectInfo("\"invert\":false,", jsonLevel + 4);
			jsonWriter->writeObjectInfo("\"color\":[1.0],", jsonLevel + 4);
			jsonWriter->writeObjectInfo("\"map\":-1,", jsonLevel + 4);
			jsonWriter->writeObjectInfo("\"sRGB\":false", jsonLevel + 4);
		}
		jsonWriter->writeObjectInfo("}", jsonLevel + 3);
	}
	jsonWriter->writeObjectInfo("},", jsonLevel + 2);
	//<
	jsonWriter->writeObjectInfo("\"Electric\":", jsonLevel + 2);
	jsonWriter->writeObjectInfo("{", jsonLevel + 2);

	jsonWriter->writeObjectInfo("\"Albedo\":", jsonLevel + 3);
	jsonWriter->writeObjectInfo("{", jsonLevel + 3);	
	sprintf(buffer, "\"colorBias\":[%f,%f,%f],", cache->mDiffuseColor[0], cache->mDiffuseColor[1], cache->mDiffuseColor[2]);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 4);
	jsonWriter->writeObjectInfo("\"factor\":1.0,", jsonLevel + 4);
	jsonWriter->writeObjectInfo("\"color\":[1.0,1.0,1.0],", jsonLevel + 4);

	if (separateFile)
	{
		TextureMap* map = NULL;

		int mapIndex = cache->mDiffuseMapIndexInDepot;
		if (mapIndex >= 0)
		{
			map = (*textureMaps)[mapIndex];
		}

		jsonWriter->writeObjectInfo("\"map\":", jsonLevel + 4);
		writeMapParameters(jsonWriter, map, mapIndex, textureFiles, false, jsonLevel + 4);
	}
	else
	{
		sprintf(buffer, "\"map\":%d,", cache->mDiffuseMapIndexInDepot); //TODO: In 3ds max bump is normal
		jsonWriter->writeObjectInfo(buffer, jsonLevel + 4);
	}

	jsonWriter->writeObjectInfo("\"sRGB\":true", jsonLevel + 4);
	jsonWriter->writeObjectInfo("},", jsonLevel + 3);
	//
	jsonWriter->writeObjectInfo("\"Specular\":", jsonLevel + 3);
	jsonWriter->writeObjectInfo("{", jsonLevel + 3);
	jsonWriter->writeObjectInfo("\"colorBias\":[1.0,1.0,1.0],", jsonLevel + 4);
	jsonWriter->writeObjectInfo("\"factor\":0.0,", jsonLevel + 4);
	jsonWriter->writeObjectInfo("\"color\":[1.0,1.0,1.0],", jsonLevel + 4);
	jsonWriter->writeObjectInfo("\"map\":-1,", jsonLevel + 4);
	jsonWriter->writeObjectInfo("\"sRGB\":true", jsonLevel + 4);
	jsonWriter->writeObjectInfo("},", jsonLevel + 3);
	//
	jsonWriter->writeObjectInfo("\"Roughness\":", jsonLevel + 3);
	jsonWriter->writeObjectInfo("{", jsonLevel + 3);
	jsonWriter->writeObjectInfo("\"factor\":0.6,", jsonLevel + 4);
	jsonWriter->writeObjectInfo("\"channel\":0,", jsonLevel + 4);
	jsonWriter->writeObjectInfo("\"invert\":false,", jsonLevel + 4);
	jsonWriter->writeObjectInfo("\"color\":[1.0],", jsonLevel + 4);
	jsonWriter->writeObjectInfo("\"map\":-1,", jsonLevel + 4);
	jsonWriter->writeObjectInfo("\"sRGB\":false", jsonLevel + 4);
	jsonWriter->writeObjectInfo("}", jsonLevel + 3);

	jsonWriter->writeObjectInfo("}", jsonLevel + 2);

	//
	jsonWriter->writeObjectInfo("}", jsonLevel + 1);
	//<
	if (isLastItem)
	{
		jsonWriter->writeObjectInfo("}", jsonLevel);
	}
	else
	{
		jsonWriter->writeObjectInfo("},", jsonLevel);
	}
}

void GAS_V1_Streamer::writeMaterials(JSONFileWriter* jsonWriter, std::vector<Material*>& materials, int jsonLevel)
{
	for (int i = 0; i < (int)materials.size(); ++i)
	{
		Material* cache = materials[i];

		bool isLastItem = (i == (int)materials.size() - 1) ? true : false;

		writeMaterialParameters(jsonWriter, false, cache, NULL, NULL, isLastItem, jsonLevel);
	}
}

void GAS_V1_Streamer::saveMeshes
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

void GAS_V1_Streamer::saveAnimations
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

void GAS_V1_Streamer::saveOutputLog(const std::string& path)
{
	FBXSDK_printf("***************** Output log *****************\n");
	JSONFileWriter* resultFile = new JSONFileWriter(false);
	resultFile->openExportFile(path);

	resultFile->writeObjectInfo("{", 0);
	resultFile->writeObjectInfo("\"version\":\"gas1\",", 1);
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
