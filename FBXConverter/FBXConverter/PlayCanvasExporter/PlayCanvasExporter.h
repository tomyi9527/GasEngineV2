#pragma once
#include "../stdafx.h"
#include "../CommonStruct.h"
#include "../Common/Common.h"
#include "../Common/Utils.h"
#include "../rapidjson/writer.h"
#include "../rapidjson/filewritestream.h"
#include "./AnimationsWriter.h"
#include "../AssistantFunctions.h"
#include "../JSONFileWriter.h"
#include "../Material.h"
#include "../optimization/FBXAnimationExporter_V4.h"
#include "../TextureMap.h"
#include "../optimization/meshLoader.h"
#include "MeshVertex.h"
#include "fbxShow.h"
#include "AnimationsWriter.h"
#include "SkinWriter.h"
#include "AnimationsExport.h"

using namespace rapidjson;

class BinData;
class JSONFileWriter;
class Material;
class TextureMap;

class PlayCanvasExporter
{
public:
	PlayCanvasExporter();
	~PlayCanvasExporter();

	void init(
		const string& fullFilePath,
		const string& workingDirectory,
		FbxManager* manager, 
		FbxScene* scene);

private:
	void writeSceneFile(
		std::vector<Material*>& materials,
		std::vector<TextureMap*>& textureMaps,
		std::vector<std::string>& textureFiles);

	void gatherAllUsedMaterils_r(
		FBXSDK_NAMESPACE::FbxNode* node,
		std::vector<FBXSDK_NAMESPACE::FbxUInt64>& effectiveBones);

	void writeTextures(vector<TextureMap *>,vector<string>& textures);

	static void writeMapParameters(
		JSONFileWriter* jsonWriter,
		TextureMap* map,
		int mapID,
		std::vector<std::string>* textures,
		bool isLastItem,
		int jsonLevel);

	static void writeMaps(
		JSONFileWriter* jsonWriter,
		std::vector<TextureMap*>& textureMaps,
		std::vector<std::string>& textures, 
		int jsonLevel);

	void writeMaterialParameters(Material* cache);

	void writeMaterials(std::vector<Material*> materials);

	void processNode_r(FBXSDK_NAMESPACE::FbxNode* node, int parentIndex);

	//void getNodeLocalMatrixManually(FBXSDK_NAMESPACE::FbxNode* node, int jsonLevel);

    void writeNodeArray();

	struct MeshInfo
	{
		MeshInfo()
		{
			mesh = NULL;
			bboxMin[0] = bboxMin[1] = bboxMin[2] = FLT_MAX;
			bboxMax[0] = bboxMax[1] = bboxMax[2] = FLT_MIN;
			isSkinned = false;
            meshArrayIndex = 0;
            skinIndex = -1;
		}

        int meshArrayIndex;
        int skinIndex;
		FBXSDK_NAMESPACE::FbxMesh* mesh;
		std::string meshName;
		FBXSDK_NAMESPACE::FbxDouble3 bboxMin;
		FBXSDK_NAMESPACE::FbxDouble3 bboxMax;
		bool isSkinned;
		std::vector<int> materials;
	};

	string			mSceneFile;
	Writer<FileWriteStream>* mJSONWriter;
	std::vector<FBXSDK_NAMESPACE::FbxSurfaceMaterial*> mUsedMaterialsInTheScene;
	std::map<int, MeshInfo> mMeshDepot;
	std::map<int, int> mNodeIDtoIndexMap; 
    vector<stNodeInfo> mVecNodeInfo;
	JSONFileWriter* mSceneStructureJSONWriter;
	string			mWorkingDirectory;
	FbxManager*		mFbxManager;
	FbxScene*		mFbxScene;
	BinData*		mBinData;
	std::vector<std::string>* mGeometryFiles;
	int				mVersion;
	float			mScaleFactor;

	//std::vector<FBXSDK_NAMESPACE::FbxUInt64>* mEffectiveBones;

	int				mGeneratedMaterialCount;

	unsigned int	mTotalTriangleCount;
	unsigned int	mTotalPolygonCount;
	unsigned int	mTotalVertexCount;
	unsigned int	mOptimizationFlag;
};

