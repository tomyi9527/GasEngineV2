#pragma once
#include "../Streamer.h"

class JSONFileWriter;

class GLTF_V2_Streamer : public Streamer
{
public:
	GLTF_V2_Streamer();
	virtual ~GLTF_V2_Streamer();

	virtual bool save
	(
		const std::string&	directoryPath,
		const std::string&	modelFileName,
		bool				jsonBeautify,
		bool				generateGZFile,
		unsigned int		meshOptimization,
		unsigned int		animationOptimization
	);

private:
	void saveStructure
	(
		const std::string&	path, 
		const std::string&	fileName, 
		bool				beautified
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

	void writeHierarchy_r(JSONFileWriter* writer, int padding);

	void saveOutputLog(const std::string& path);

	int mTotalTriangleCount;
	int mTotalPolygonCount;
	int mTotalVertexCount;

	std::string mModelFileName;
	std::vector<std::string> mOutputFiles;
};