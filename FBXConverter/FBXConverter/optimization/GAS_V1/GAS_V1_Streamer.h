#pragma once
#include "../Streamer.h"

class JSONFileWriter;

class GAS_V1_Streamer : public Streamer
{
public:
	GAS_V1_Streamer();
	virtual ~GAS_V1_Streamer();

	virtual bool save
	(
		const std::string& directoryPath,
		const std::string& modelFileName,
		bool JsonBeautify, 
		bool generateGZFile, 
		unsigned int meshOptimization,
		unsigned int animationOptimization
	);

protected:
	void saveScene(const std::string& path, bool beautified);
	void saveStructure(const std::string& path, const std::string& fileName, bool beautified);
	void writeHierarchy_r(JSONFileWriter* writer, Node* node, int padding, bool isLast);

	void writePostProcessEffectConfig(JSONFileWriter* jsonWriter, int jsonLevel);
	void writeLights(JSONFileWriter* jsonWriter, int jsonLevel);
	void writeMaterialParameters
	(
		JSONFileWriter* jsonWriter,
		bool separateFile,
		Material* cache,
		std::vector<TextureMap*>* textureMaps,
		std::vector<std::string>* textureFiles,
		bool isLastItem,
		int jsonLevel
	);

	void writeMaterials(JSONFileWriter* jsonWriter, std::vector<Material*>& materials, int jsonLevel);
	void writeTextures(JSONFileWriter* jsonWriter, std::vector<std::string>& textures, int jsonLevel);
	void writeMapParameters
	(
		JSONFileWriter* jsonWriter,
		TextureMap* map,
		int mapID,
		std::vector<std::string>* textures,
		bool isLastItem,
		int jsonLevel
	);
	void writeMaps
	(
		JSONFileWriter* jsonWriter,
		std::vector<TextureMap*>& textureMaps,
		std::vector<std::string>& textures,
		int jsonLevel
	);

	void saveMeshes
	(
		const std::string& directoryPath,
		const std::string& modelFileName,
		bool generateGZFile,
		unsigned int meshOptimization
	);

	void saveAnimations
	(
		const std::string& directoryPath,
		const std::string& modelFileName,
		bool generateGZFile,
		unsigned int animationOptimization
	);

	void saveOutputLog(const std::string& path);

private:
	int mTotalTriangleCount;
	int mTotalPolygonCount;
	int mTotalVertexCount;

	std::vector<std::string> mOutputFiles;
};