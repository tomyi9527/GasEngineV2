#pragma once
#include "CommonStruct.h"
#include "MAX/max_reader.h"
//jpg png jpeg dds tga bmp gif pvr tiff
class TextureMap;

class Material
{
public:
	Material();
	~Material();

	int getColorInfo(
		FBXSDK_NAMESPACE::FbxSurfaceMaterial* material, 
		FBXSDK_NAMESPACE::FbxString type,
		FbxDouble& factor, FbxDouble3& color, 
		std::vector<TextureMap*>& textureMaps,
		std::vector<std::string>& textureFiles);

	int appendTexture(
		std::string& fileName,
		std::vector<std::string>& textures);

    void TakeoutMaxTextureMap(
        const max::Texmaps::MapBasicInfo& texture_map,
        FbxDouble& factor,
        int& MapIndex,
        std::vector<TextureMap*>& textureMaps,
        std::vector<std::string>& textureFiles);

	int appendMaterialMap(
		TextureMap* fileTexture,
		std::vector<TextureMap*>& textureMaps,
		std::vector<std::string>& textureFiles);

	void parseFBXMaterial(
		FBXSDK_NAMESPACE::FbxSurfaceMaterial* material,
		std::vector<TextureMap*>& textureMaps,
		std::vector<std::string>& textureFiles);

	void parsePMXMaterial(
		unsigned int id,
		void* material,
		std::vector<TextureMap*>& textureMaps,
		std::vector<std::string>& textureFiles);

    void parseMaxMaterial(
        void* material,
        std::vector<TextureMap*>& textureMaps,
        std::vector<std::string>& textureFiles);

	void parsePMDMaterial(
		unsigned int id,
		void* material,
		std::vector<TextureMap*>& textureMaps,
		std::vector<std::string>& textureFiles);

	void parseSTLMaterial(
		unsigned int id,
		const FbxVector4& diffuse,
		const FbxVector4& specular,
		const FbxVector4& ambient,
		std::vector<TextureMap*>& textureMaps,
		std::vector<std::string>& textureFiles);

	unsigned int		mUniqueID;
	FbxString			mMaterialName;
	FbxString			mShadeModel;

	//Culling
	unsigned int		mCulling; //0: off  1: cull ccw   2: cull cw, and ccw is front

	//Emissive
	FbxDouble			mEmissiveFactor;
	FbxDouble3			mEmissiveColor;
	int					mEmissiveMapIndexInDepot;

	//Ambient
	FbxDouble			mAmbientFactor;
	FbxDouble3			mAmbientColor;
	int					mAmbientMapIndexInDepot;

	//Diffuse
	FbxDouble			mDiffuseFactor;
	FbxDouble3			mDiffuseColor;
	int					mDiffuseMapIndexInDepot;

	//Specular
	FbxDouble			mSpecularFactor;
	FbxDouble3			mSpecularColor;
	FbxDouble			mGlossinessValue;
	int					mSpecularMapIndexInDepot;

	//Advanced Properties
	//NormalMap
	bool				mNormalEnable;
	FbxDouble3			mNormalColor;
	int					mNormalMapIndexInDepot;

	//Bump
	bool				mBumpEnable;
	FbxDouble			mBumpFactor;
	FbxDouble3			mBumpColor;
	int					mBumpMapIndexInDepot;

	//ReflectionColor
	FbxDouble			mReflectionFactor;
	FbxDouble3			mReflectionColor;
	int					mReflectionMapIndexInDepot;

	//DisplacementColor
	FbxDouble			mDisplacementFactor;
	FbxDouble3			mDisplacementColor;
	int					mDisplacementMapIndexInDepot;

	//Opacity
	FbxDouble			mOpacity;

	//Transparency
	bool				mTransparencyEnable;
	FbxDouble			mTransparencyFactor;
	FbxDouble3			mTransparencyColor;
	int					mTransparencyIndexInDepot;
};
