#pragma once
#include "CommonStruct.h"
#include "PmxReader.h"

class JSONFileWriter;
class SceneNode;
class IKChain;

class PmxConverter
{
public:
	PmxConverter();
	~PmxConverter();

	bool convert(
		const std::string& workingDirectory, 
		const std::string& filePath,
		unsigned int optimizationFlag,
		std::string& background);

	template <typename t> static void getTopologicalIndices(const std::vector<t>& indices, std::vector<unsigned int>& topologyIndices)
	{
		std::map<int, vector<int> > dictionary;

		int polygonCount = (int)indices.size() / 3;
		for (int polygonIndex = 0; polygonIndex < polygonCount; ++polygonIndex)
		{
			int polygonSize = 3;

			for (int vertexIndex = 1; vertexIndex <= polygonSize; ++vertexIndex)
			{
				int controlPointIndex0 = indices[polygonIndex * polygonSize + (vertexIndex - 1) % polygonSize];
				int controlPointIndex1 = indices[polygonIndex * polygonSize + vertexIndex % polygonSize];

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
					FBXSDK_printf("%s", "Find topological indices failed!\n");
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

	void buildSceneStructure(PmxReader* pmx);

	void getMorph(
		std::vector<PmxReader::PmxMorph>& morphs,
		float* originalPositions,
		unsigned int vertexCount,
		FBXSDK_NAMESPACE::FbxDouble3 posBboxMin,
		FBXSDK_NAMESPACE::FbxDouble3 posBboxMax,
		std::vector<MORPH_DATA_V2>* morphTargetsData);

	std::vector<BONE_>* createSkeleton(std::vector<PmxReader::PmxBone>& bones);

	static void convertSceneFile(
		const std::string& workingDirectory,
		PmxReader* pmx,
		std::vector<std::string>& convertedFiles,
		const std::string& background,
		int version);

	static void convertStructureFile(
		const std::string& fullFilePath,
		const std::string& workingDirectory,
		int materialCount,
		SceneNode* sceneRoot,
		FBXSDK_NAMESPACE::FbxDouble3& posBboxMin,
		FBXSDK_NAMESPACE::FbxDouble3& posBboxMax,
		int version);

	static void writeSceneNode_r(
		JSONFileWriter* sceneStructureJSONWriter,
		SceneNode* node, 
		int jsonLevel, 
		bool lastObject);

	static void writeMeshNode(
		JSONFileWriter* sceneStructureJSONWriter,
		int materialCount,
		FBXSDK_NAMESPACE::FbxDouble3& posBboxMin,
		FBXSDK_NAMESPACE::FbxDouble3& posBboxMax,
		int jsonLevel,
		bool lastObject);

	SceneNode* sceneRoot;
	std::vector<SceneNode*> nodes;
	int mVersion;
};

