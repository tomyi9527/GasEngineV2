#include "stdafx.h"
#include "Streamer.h"

Streamer::Streamer()
	: mMaterials(NULL)
	, mTextureMaps(NULL)
	, mTextures(NULL)
	, mHierarchy(NULL)
	, mMeshes(NULL)
	, mAnimations(NULL)
	, mEffectiveBones(NULL)
{

}

Streamer::~Streamer()
{
}

bool Streamer::save
(
	const std::string& directoryPath,
	const std::string& modelFileName,
	bool jsonBeautify,
	bool generateGZFile,
	unsigned int meshOptimization,
	unsigned int animationOptimization
)
{
	return false;
}