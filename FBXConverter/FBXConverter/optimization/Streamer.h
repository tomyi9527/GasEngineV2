#pragma once

class Node;
class Material;
class TextureMap;
struct KeyframeAnimation;

class Streamer
{
public:
	Streamer();
	virtual ~Streamer();

	virtual bool save
	(
		const std::string& directoryPath,
		const std::string& modelFileName,
		bool jsonBeautify,
		bool generateGZFile,
		unsigned int meshOptimization,
		unsigned int animationOptimization
	);

	virtual void setHierarchy(vector<Node*>* hierarchy)
	{
		mHierarchy = hierarchy;
	}

	virtual void setMaterials(std::vector<Material*>* materials)
	{
		mMaterials = materials;
	}

	virtual void setTextureMaps(std::vector<TextureMap*>* textureMaps)
	{
		mTextureMaps = textureMaps;
	}

	virtual void setTextures(std::vector<std::string>* textures)
	{
		mTextures = textures;
	}

	virtual void setMeshes(std::map<int, MeshInfo>* meshes)
	{
		mMeshes = meshes;
	}

	virtual void setAnimations(std::vector<KeyframeAnimation>* animations)
	{
		mAnimations = animations;
	}

	virtual void setEffectiveBones(std::vector<unsigned int>* effectiveBones)
	{
		mEffectiveBones = effectiveBones;
	}

	virtual std::vector<MESH_DETAIL>& getMeshDetails()
	{
		return mMeshDetails;
	}

	virtual std::vector<ANIMATION_DETAIL>& getAnimationDetails()
	{
		return mAnimationDetails;
	}

	virtual std::vector<std::string>* getTextures() 
	{
		return mTextures;
	}

protected:
	std::vector<Material*>*		mMaterials;
	std::vector<TextureMap*>*	mTextureMaps;
	std::vector<std::string>*	mTextures;
	vector<Node*>*				mHierarchy;
	std::map<int, MeshInfo>*	mMeshes;

	std::vector<KeyframeAnimation>*	mAnimations;
	std::vector<unsigned int>*	mEffectiveBones;

	std::vector<MESH_DETAIL> mMeshDetails;
	std::vector<ANIMATION_DETAIL> mAnimationDetails;
};