#include "stdafx.h"
#include "ModelDetails.h"

ModelDetails::ModelDetails()
	: mTotalTriangleCount(0)
	, mTotalPolygonCount(0)
	, mTotalVertexCount(0)
	, mTotalBlendShapeCount(0)
	, mTotalKeyFrameCount(0)
	, mFPS(0.0)
	, mAnimationDuration(0.0)
	, mMeshDetails(NULL)
	, mAnimationDetails(NULL)
	, mTextures(NULL)
{

}

ModelDetails::~ModelDetails()
{

}

void ModelDetails::save(const std::string& directoryPath, const std::string& modelFileName)
{
	for (size_t i = 0;i < (*mAnimationDetails).size();++i) 
	{
		ANIMATION_DETAIL ad = (*mAnimationDetails)[i];
		if (ad.keyframeCount != 0) mTotalKeyFrameCount += ad.keyframeCount;
		if (ad.fps != 0)
		{
			mFPS = ad.fps;
			mAnimationDuration = ad.animationDuration;
		}
	}

	bool isShowVertexFormat = true;

	for (size_t i = 0;i < (*mMeshDetails).size();++i)
	{
	
		MESH_DETAIL md = (*mMeshDetails)[i];

		if (md.polygonCount != 0) 
		{
			mTotalPolygonCount += md.polygonCount; 
		}
		if (md.triangleCount != 0) 
		{
			mTotalTriangleCount += md.triangleCount; 
		}
		if (md.vertexCount != 0)
		{
			mTotalVertexCount += md.vertexCount;
		}
		if (!md.bones.empty())
		{
			std::map<int, std::string>::iterator iterb = md.bones.begin();
			for (; iterb != md.bones.end(); ++iterb)
			{
				mTotalBones.insert(std::pair<int, std::string>(iterb->first, iterb->second));
			}
			std::map<int, int>::iterator iterbh = md.boneHierarchy.begin();
			for (; iterbh != md.boneHierarchy.end(); ++iterbh)
			{
				mBoneHierarchy.insert(std::pair<int, int>(iterbh->first, iterbh->second));
			}
		}
		if (md.blendshapeCount != 0)
		{
			mTotalBlendShapeCount += md.blendshapeCount;
		}

		if (!md.vertexFormats.empty())
		{
			for (size_t j = 0;j < md.vertexFormats.size();++j)
			{
				mTotalVertexFormats.insert((md.vertexFormats)[j]);
			}

			//if (i != 0) 
			//{
			//	if (mTotalVertexFormats.size() != md.vertexFormats.size())
			//	{
			//		isShowVertexFormat = false;
			//	}
			//}
		}
	}

	std::string filePath = directoryPath + modelFileName + ".output.json";

	writeOutput(filePath, isShowVertexFormat);
}

bool ModelDetails::writeOutput(const std::string& filePath, bool isShowVertexFormat)
{
	JSONFileWriter* jsonWriter = new JSONFileWriter(false);
	jsonWriter->openExportFile(filePath);

	char buffer[__TEMP_BUFFER_FLOAT__];
	int jsonLevel = 0;
	jsonWriter->writeObjectInfo("{", jsonLevel);
	jsonWriter->writeObjectInfo("\"version\": \"gas2\",", jsonLevel + 1);

	memset(buffer, 0, __TEMP_BUFFER_FLOAT__);
	sprintf(buffer, "\"totalPolygonCount\": %d,", mTotalPolygonCount);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);

	sprintf(buffer, "\"totalTriangleCount\": %d,", mTotalTriangleCount);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);

	sprintf(buffer, "\"totalVertexCount\": %d,", mTotalVertexCount);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);

	std::vector<std::string> elementStringTable =
	{
		"\"POSITION\"", "\"NORMAL0\"", "\"NORMAL1\"", "\"TANGENT0\"", "\"TANGENT1\"", \
		"\"BINORMAL0\"", "\"BINORMAL1\"", "\"UV0\"", "\"UV1\"", "\"VERTEXCOLOR0\"", "\"VERTEXCOLOR1\"",
		"\"BLENDWEIGHT\"", "\"BLENDINDEX\"", "\"INDEX\"", "\"TOPOLOGICALINDEX\""
	};

	jsonWriter->writeObjectInfo("\"totalVertexFormats\":", jsonLevel + 1);
	jsonWriter->writeObjectInfo("[", jsonLevel + 1);
	std::set<int>::iterator itervf = mTotalVertexFormats.begin();
	//if (isShowVertexFormat)
	//{
		for (int j = 0; itervf != mTotalVertexFormats.end(); ++itervf, ++j)
		{
			int index = *itervf;

			if (j < mTotalVertexFormats.size() - 1)
			{
				jsonWriter->writeObjectInfo(elementStringTable[index] + ",", jsonLevel + 2);
			}
			else
			{
				jsonWriter->writeObjectInfo(elementStringTable[index], jsonLevel + 2);
			}
		}
	//}
	jsonWriter->writeObjectInfo("],", jsonLevel + 1);

	//<
	sprintf(buffer, "\"totalBlendShapeCount\": %d,", mTotalBlendShapeCount);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);

	sprintf(buffer, "\"keyframeCount\": %d,", mTotalKeyFrameCount);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);

	sprintf(buffer, "\"fps\": %d,", (int)mFPS);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);

	sprintf(buffer, "\"animationDuration\": %d,", (int)mAnimationDuration);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);

	// textures
	jsonWriter->writeObjectInfo("\"textures\":", jsonLevel + 1);
	jsonWriter->writeObjectInfo("[", jsonLevel + 1);
	for (int j = 0; j < mTextures->size(); j++)
	{
		if (j < mTextures->size() - 1)
		{
			jsonWriter->writeObjectInfo("\"" + (*mTextures)[j] + "\",", jsonLevel + 2);
		}
		else
		{
			jsonWriter->writeObjectInfo("\"" + (*mTextures)[j] + "\"", jsonLevel + 2);
		}
	}
	jsonWriter->writeObjectInfo("],", jsonLevel + 1);

	// bones
	jsonWriter->writeObjectInfo("\"boneHierarchy\":", jsonLevel + 1);
	jsonWriter->writeObjectInfo("[", jsonLevel + 1);
	std::map<int, std::string>::iterator itb = mTotalBones.begin();
	for (int j = 0; itb != mTotalBones.end(); ++itb, ++j)
	{
		jsonWriter->writeObjectInfo("{", jsonLevel + 2);

		sprintf(buffer, "\"id\": %d,", itb->first);
		jsonWriter->writeObjectInfo(buffer, jsonLevel + 3);

		sprintf(buffer, "\"name\": \"%s\",", itb->second.c_str());
		jsonWriter->writeObjectInfo(buffer, jsonLevel + 3);

		if (mTotalBones.find(mBoneHierarchy[itb->first]) == mTotalBones.end())
		{
			jsonWriter->writeObjectInfo("\"parent\": -1", jsonLevel + 3);
		}
		else
		{
			sprintf(buffer, "\"parent\": %d", mBoneHierarchy[itb->first]);
			jsonWriter->writeObjectInfo(buffer, jsonLevel + 3);
		}

		if (j < mTotalBones.size() - 1)
		{
			jsonWriter->writeObjectInfo("},", jsonLevel + 2);
		}
		else
		{
			jsonWriter->writeObjectInfo("}", jsonLevel + 2);
		}
	}
	jsonWriter->writeObjectInfo("],", jsonLevel + 1);

	// meshInfo
	jsonWriter->writeObjectInfo("\"meshInfo\":", jsonLevel + 1);
	jsonWriter->writeObjectInfo("[", jsonLevel + 1);
	for (size_t j = 0; j < (*mMeshDetails).size(); ++j)
	{
		jsonWriter->writeObjectInfo("{", jsonLevel + 2);

		MESH_DETAIL md = (*mMeshDetails)[j];

		sprintf(buffer, "\"id\": %d,", md.meshID);
		jsonWriter->writeObjectInfo(buffer, jsonLevel + 3);

		sprintf(buffer, "\"name\": \"%s\",", md.meshName.c_str());
		jsonWriter->writeObjectInfo(buffer, jsonLevel + 3);

		sprintf(buffer, "\"polygonCount\": %d,", md.polygonCount);
		jsonWriter->writeObjectInfo(buffer, jsonLevel + 3);

		sprintf(buffer, "\"triangleCount\": %d,", md.triangleCount);
		jsonWriter->writeObjectInfo(buffer, jsonLevel + 3);

		sprintf(buffer, "\"vertexCount\": %d,", md.vertexCount);
		jsonWriter->writeObjectInfo(buffer, jsonLevel + 3);

		sprintf(buffer, "\"blendshapeCount\": %d,", md.blendshapeCount);
		jsonWriter->writeObjectInfo(buffer, jsonLevel + 3);

		jsonWriter->writeObjectInfo("\"boneList\":", jsonLevel + 3);
		jsonWriter->writeObjectInfo("[", jsonLevel + 3);
		if (!md.bones.empty())
		{
			std::map<int, std::string>::iterator iterb = md.bones.begin();
			for (int j = 0;iterb != md.bones.end();++iterb, ++j)
			{
				if (j < md.bones.size() - 1)
				{
					sprintf(buffer, "%d,", iterb->first);
				}
				else
				{
					sprintf(buffer, "%d", iterb->first);
				}
				jsonWriter->writeObjectInfo(buffer, jsonLevel + 4);
			}
		}
		jsonWriter->writeObjectInfo("],", jsonLevel + 3);

		jsonWriter->writeObjectInfo("\"vertexFormats\":", jsonLevel + 3);
		jsonWriter->writeObjectInfo("[", jsonLevel + 3);
		if (!md.vertexFormats.empty())
		{
			std::set<int>::iterator itervf = mTotalVertexFormats.begin();
			for (size_t j = 0;j < md.vertexFormats.size();++j)
			{
				int index = md.vertexFormats[j];
				if (j < md.vertexFormats.size() - 1)
				{
					jsonWriter->writeObjectInfo(elementStringTable[index] + ",", jsonLevel + 4);
				}
				else
				{
					jsonWriter->writeObjectInfo(elementStringTable[index], jsonLevel + 4);
				}
			}
		}
		jsonWriter->writeObjectInfo("]", jsonLevel + 3);

		if (j < (*mMeshDetails).size() - 1)
		{
			jsonWriter->writeObjectInfo("},", jsonLevel + 2);
		}
		else
		{
			jsonWriter->writeObjectInfo("}", jsonLevel + 2);
		}
	}
	jsonWriter->writeObjectInfo("]", jsonLevel + 1);

	// end
	jsonWriter->writeObjectInfo("}", jsonLevel);

 	jsonWriter->closeExportFile();

	return true;
}