#pragma once
#include "../Streamer.h"

class JSONFileWriter;

class GAS_V2_Streamer : public Streamer
{
public:
	GAS_V2_Streamer();
	virtual ~GAS_V2_Streamer();

	virtual bool save
	(
		const std::string& directoryPath,
		const std::string& modelFileName,
		bool jsonBeautify,
		bool generateGZFile,
		unsigned int meshOptimization,
		unsigned int animationOptimization
	);

private:
	void saveStructure(const std::string& path, const std::string& fileName, bool beautified);
	void writeHierarchy_r(JSONFileWriter* writer, Node* node, int padding, bool isLast);

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

	void saveMaterials
	(
		const std::string& directoryPath,
		const std::string& modelFileName,
		bool generateGZFile,
		bool jsonBeautify
	);

	void writeCompoundMaterial
	(
		JSONFileWriter* jsonWriter,
		Material* cache,
		std::vector<TextureMap*>* textureMaps,
		std::vector<std::string>* textureFiles,
		bool isLastItem,
		int jsonLevel
	);

	void writeBlinnPhongMaterial
	(
		JSONFileWriter* jsonWriter,
		Material* cache,
		std::vector<TextureMap*>* textureMaps,
		std::vector<std::string>* textureFiles,
		bool isLastItem,
		int jsonLevel
	);

	void writeDielectricMaterial
	(
		JSONFileWriter* jsonWriter,
		Material* cache,
		std::vector<TextureMap*>* textureMaps,
		std::vector<std::string>* textureFiles,
		bool isLastItem,
		int jsonLevel
	);

	void writeElectricMaterial
	(
		JSONFileWriter* jsonWriter,
		Material* cache,
		std::vector<TextureMap*>* textureMaps,
		std::vector<std::string>* textureFiles,
		bool isLastItem,
		int jsonLevel
	);

	void writeMatCapMaterial
	(
		JSONFileWriter* jsonWriter,
		Material* cache,
		std::vector<TextureMap*>* textureMaps,
		std::vector<std::string>* textureFiles,
		bool isLastItem,
		int jsonLevel
	);

	void writeAlbedoMap
	(
		JSONFileWriter*					jsonWriter,
		Material*						material,
		std::vector<TextureMap*>*		textureMaps,
		std::vector<std::string>*		textureFiles,
		bool							isLastItem,
		int								jsonLevel
	);

	void writeSpecularMap
	(
		JSONFileWriter*					jsonWriter,
		Material*						material,
		std::vector<TextureMap*>*		textureMaps,
		std::vector<std::string>*		textureFiles,
		bool							isLastItem,
		int								jsonLevel
	);

	void writeGlossinessMap
	(
		JSONFileWriter*					jsonWriter,
		Material*						material,
		std::vector<TextureMap*>*		textureMaps,
		std::vector<std::string>*		textureFiles,
		bool							isLastItem,
		int								jsonLevel
	);

	void writeLightMap
	(
		JSONFileWriter*					jsonWriter,
		Material*						material,
		std::vector<TextureMap*>*		textureMaps,
		std::vector<std::string>*		textureFiles,
		bool							isLastItem,
		int								jsonLevel
	);

	void writeNormalMap
	(
		JSONFileWriter*					jsonWriter,
		Material*						material,
		std::vector<TextureMap*>*		textureMaps,
		std::vector<std::string>*		textureFiles,
		bool							isLastItem,
		int								jsonLevel
	);

	void writeTransparencyMap
	(
		JSONFileWriter*					jsonWriter,
		Material*						material,
		std::vector<TextureMap*>*		textureMaps,
		std::vector<std::string>*		textureFiles,
		bool							isLastItem,
		int								jsonLevel
	);

	void writeEmissiveMap
	(
		JSONFileWriter*					jsonWriter,
		Material*						material,
		std::vector<TextureMap*>*		textureMaps,
		std::vector<std::string>*		textureFiles,
		bool							isLastItem,
		int								jsonLevel
	);

	void writeDisplacementMap
	(
		JSONFileWriter*					jsonWriter,
		Material*						material,
		std::vector<TextureMap*>*		textureMaps,
		std::vector<std::string>*		textureFiles,
		bool							isLastItem,
		int								jsonLevel
	);

	void writeAoMap
	(
		JSONFileWriter*					jsonWriter,
		Material*						material,
		std::vector<TextureMap*>*		textureMaps,
		std::vector<std::string>*		textureFiles,
		bool							isLastItem,
		int								jsonLevel
	);

	void writeCavityMap
	(
		JSONFileWriter*					jsonWriter,
		Material*						material,
		std::vector<TextureMap*>*		textureMaps,
		std::vector<std::string>*		textureFiles,
		bool							isLastItem,
		int								jsonLevel
	);

	void writeMetalnessMap
	(
		JSONFileWriter*					jsonWriter,
		Material*						material,
		std::vector<TextureMap*>*		textureMaps,
		std::vector<std::string>*		textureFiles,
		bool							isLastItem,
		int								jsonLevel
	);

	void writeSpecularF0Map
	(
		JSONFileWriter*					jsonWriter,
		Material*						material,
		std::vector<TextureMap*>*		textureMaps,
		std::vector<std::string>*		textureFiles,
		bool							isLastItem,
		int								jsonLevel
	);

	void writeRoughnessMap
	(
		JSONFileWriter*					jsonWriter,
		Material*						material,
		std::vector<TextureMap*>*		textureMaps,
		std::vector<std::string>*		textureFiles,
		bool							isLastItem,
		int								jsonLevel
	);

	void writeMatCapMap
	(
		JSONFileWriter*					jsonWriter,
		Material*						material,
		std::vector<TextureMap*>*		textureMaps,
		std::vector<std::string>*		textureFiles,
		bool							isLastItem,
		int								jsonLevel
	);

	void writeMapParameters
	(
		JSONFileWriter*					jsonWriter,
		TextureMap*						map,
		const std::string&				textureName,
		bool							isLastItem,
		int								jsonLevel
	);

	int mTotalTriangleCount;
	int mTotalPolygonCount;
	int mTotalVertexCount;

	std::string mModelFileName;
	std::vector<std::string> mOutputFiles;
};