#include "stdafx.h"
#include "Material.h"
#include "TextureMap.h"
#include "MMD/PmxReader.h"
#include "MMD/PmdReader.h"
#include "MAX/max_reader.h"
#include <algorithm>

Material::Material()
{
}


Material::~Material()
{
}

int Material::appendTexture(
	std::string& fileName,
	std::vector<std::string>& textures)
{
	for (int i = 0; i < (int)textures.size(); ++i)
	{
		if (textures[i] == fileName)
		{
			return i;
		}
	}

	textures.push_back(fileName);

	return (int)textures.size() - 1;
}

int Material::appendMaterialMap(
	TextureMap* map, 
	std::vector<TextureMap*>& textureMaps,
	std::vector<std::string>& textureFiles)
{
	int index = appendTexture(map->fileName, textureFiles);
	textureMaps.push_back(map);
	return (int)textureMaps.size() - 1;
}

int Material::getColorInfo(
	FBXSDK_NAMESPACE::FbxSurfaceMaterial* material, 
	FBXSDK_NAMESPACE::FbxString type,
	FbxDouble& factor,
	FbxDouble3& color, 
	std::vector<TextureMap*>& textureMaps,
	std::vector<std::string>& textureFiles)
{
	FbxProperty factorProp = material->FindProperty(type + "Factor");
	if (factorProp.IsValid())
	{
		factor = factorProp.Get<FbxDouble>();
	}

	FbxProperty colorProp = material->FindProperty(type + "Color");
	if (colorProp.IsValid())
	{
		color = colorProp.Get<FBXSDK_NAMESPACE::FbxDouble3>();
		int associatedTextureCount = colorProp.GetSrcObjectCount<FBXSDK_NAMESPACE::FbxFileTexture>();
		if (associatedTextureCount > 0)
		{
			FBXSDK_NAMESPACE::FbxFileTexture* fbxMap = colorProp.GetSrcObject<FBXSDK_NAMESPACE::FbxFileTexture>(0);
			TextureMap* map = new TextureMap();
			map->parseFBXTextureMap(fbxMap);
			int materialMapDepotIndex = appendMaterialMap(map, textureMaps, textureFiles);
			return materialMapDepotIndex;
		}
	}

	return -1;
}

void Material::parseFBXMaterial(
	FBXSDK_NAMESPACE::FbxSurfaceMaterial* material,
	std::vector<TextureMap*>& textureMaps, 
	std::vector<std::string>& textureFiles)
{
	//P : "ShadingModel", "KString", "", "", "Phong"
	//P : "MultiLayer", "bool", "", "", 0
	//P : "EmissiveColor", "Color", "", "A", 0, 0, 0
	//P : "EmissiveFactor", "Number", "", "A", 1
	//P : "AmbientColor", "Color", "", "A", 0.2, 0.2, 0.2
	//P : "AmbientFactor", "Number", "", "A", 1
	//P : "DiffuseColor", "Color", "", "A", 0.8, 0.8, 0.8
	//P : "DiffuseFactor", "Number", "", "A", 1
	//P : "Bump", "Vector3D", "Vector", "", 0, 0, 0
	//P : "NormalMap", "Vector3D", "Vector", "", 0, 0, 0
	//P : "BumpFactor", "double", "Number", "", 1
	//P : "TransparentColor", "Color", "", "A", 0, 0, 0
	//P : "TransparencyFactor", "Number", "", "A", 0
	//P : "DisplacementColor", "ColorRGB", "Color", "", 0, 0, 0
	//P : "DisplacementFactor", "double", "Number", "", 1
	//P : "VectorDisplacementColor", "ColorRGB", "Color", "", 0, 0, 0
	//P : "VectorDisplacementFactor", "double", "Number", "", 1
	//P : "SpecularColor", "Color", "", "A", 0.2, 0.2, 0.2
	//P : "SpecularFactor", "Number", "", "A", 1
	//P : "ShininessExponent", "Number", "", "A", 20
	//P : "ReflectionColor", "Color", "", "A", 0, 0, 0
	//P : "ReflectionFactor", "Number", "", "A", 1

	//MaterialName
	mUniqueID = material->GetUniqueID();
	mMaterialName = standardizeFileName(material->GetName()).c_str();
	if (mMaterialName.IsEmpty())
	{
		mMaterialName = "Unknown_Material";
	}

	//Culling
	void* p = material->GetUserDataPtr();
	unsigned long long v = reinterpret_cast<unsigned long long>(p);
	mCulling = (unsigned int)((v >> 32) & 0x3);

	//ShadeModel
	mShadeModel = "Unknown";
	FbxProperty shadingModelProp = material->FindProperty("ShadingModel");
	if (shadingModelProp.IsValid())
	{
		mShadeModel = shadingModelProp.Get<FbxString>();
	}
	//EmissiveColor
	mEmissiveFactor = 1.0;
	mEmissiveColor = FBXSDK_NAMESPACE::FbxDouble3(0.0, 0.0, 0.0);
	mEmissiveMapIndexInDepot = getColorInfo(material, "Emissive", mEmissiveFactor, mEmissiveColor, textureMaps, textureFiles);

	//AmbientColor
	mAmbientFactor = 1.0;
	mAmbientColor = FBXSDK_NAMESPACE::FbxDouble3(1.0, 1.0, 1.0);
	mAmbientMapIndexInDepot = getColorInfo(material, "Ambient", mAmbientFactor, mAmbientColor, textureMaps, textureFiles);

	//DiffuseColor
	mDiffuseFactor = 1.0;
	mDiffuseColor = FBXSDK_NAMESPACE::FbxDouble3(1.0, 1.0, 1.0);
	mDiffuseMapIndexInDepot = getColorInfo(material, "Diffuse", mDiffuseFactor, mDiffuseColor, textureMaps, textureFiles);

	//SpecularColor
	mSpecularFactor = 1.0;
	mSpecularColor = FBXSDK_NAMESPACE::FbxDouble3(0.0, 0.0, 0.0);
	mSpecularMapIndexInDepot = getColorInfo(material, "Specular", mSpecularFactor, mSpecularColor, textureMaps, textureFiles);

	FbxProperty shininessExponentProp = material->FindProperty("ShininessExponent"); //shininessExponent = pow(2.0, max_glossiness/10.0);
	if (shininessExponentProp.IsValid())
	{
		FbxDouble shininessExponent = shininessExponentProp.Get<FbxDouble>();
		mGlossinessValue = FBXSDK_NAMESPACE::FbxClamp((log(shininessExponent) / log(2)) * 10.0, 0.0, 100.0);
	}

	//Advanced Properties
	//NormalMap
	mNormalEnable = false;
	mNormalMapIndexInDepot = -1;
	mNormalColor = FBXSDK_NAMESPACE::FbxDouble3(0.0, 0.0, 1.0);
	FbxProperty normalMapProp = material->FindProperty("NormalMap");
	if (normalMapProp.IsValid())
	{
		mNormalColor = normalMapProp.Get<FBXSDK_NAMESPACE::FbxDouble3>();
		int associatedTextureCount = normalMapProp.GetSrcObjectCount<FBXSDK_NAMESPACE::FbxFileTexture>();
		if (associatedTextureCount > 0)
		{
			FBXSDK_NAMESPACE::FbxFileTexture* fbxMap = normalMapProp.GetSrcObject<FBXSDK_NAMESPACE::FbxFileTexture>(0);
			TextureMap* map = new TextureMap();
			map->parseFBXTextureMap(fbxMap);
			//mNormalMapIndexInDepot = appendMaterialMap(map, textureMaps, textureFiles);
			mNormalEnable = true;
		}
	}

	//Bump
	mBumpFactor = 1.0;
	FbxProperty bumpFactorProp = material->FindProperty("BumpFactor");
	if (bumpFactorProp.IsValid())
	{
		mBumpFactor = bumpFactorProp.Get<FbxDouble>();
	}

	mBumpEnable = false;
	mBumpMapIndexInDepot = -1;
	mBumpColor = FBXSDK_NAMESPACE::FbxDouble3(0.0, 0.0, 1.0);
	FbxProperty bumpColorProp = material->FindProperty("Bump");
	if (bumpColorProp.IsValid())
	{
		mBumpColor = bumpColorProp.Get<FBXSDK_NAMESPACE::FbxDouble3>();
		int associatedTextureCount = bumpColorProp.GetSrcObjectCount<FBXSDK_NAMESPACE::FbxFileTexture>();
		if (associatedTextureCount > 0)
		{
			FBXSDK_NAMESPACE::FbxFileTexture* fbxMap = bumpColorProp.GetSrcObject<FBXSDK_NAMESPACE::FbxFileTexture>(0);
			TextureMap* map = new TextureMap();
			map->parseFBXTextureMap(fbxMap);
			mBumpMapIndexInDepot = appendMaterialMap(map, textureMaps, textureFiles);
			mBumpEnable = true;
		}
	}

	//ReflectionColor
	mReflectionFactor = 1.0;
	mReflectionColor = FBXSDK_NAMESPACE::FbxDouble3(0.0, 0.0, 0.0);
	mReflectionMapIndexInDepot = getColorInfo(material, "Reflection", mReflectionFactor, mReflectionColor, textureMaps, textureFiles);

	//DisplacementColor
	mDisplacementFactor = 1.0;
	mDisplacementColor = FBXSDK_NAMESPACE::FbxDouble3(0.0, 0.0, 0.0);
	mDisplacementMapIndexInDepot = getColorInfo(material, "Displacement", mDisplacementFactor, mDisplacementColor, textureMaps, textureFiles);

	//Opacity
	mOpacity = 1.0;
	FBXSDK_NAMESPACE::FbxProperty opacityProp = material->FindProperty("Opacity");
	if (opacityProp.IsValid())
	{
		mOpacity = opacityProp.Get<FbxDouble>();
	}

	//Transparency
	mTransparencyEnable = false;
	mTransparencyIndexInDepot = -1;
	mTransparencyFactor = 1.0;
	FbxProperty transFactorProp = material->FindProperty("TransparencyFactor");
	if (transFactorProp.IsValid())
	{
		mTransparencyFactor = transFactorProp.Get<FbxDouble>();		
	}
	mTransparencyColor = FBXSDK_NAMESPACE::FbxDouble3(0.0, 0.0, 0.0);
	FbxProperty transparencyColorProp = material->FindProperty("TransparentColor");
	if (transparencyColorProp.IsValid())
	{
		mTransparencyColor = transparencyColorProp.Get<FBXSDK_NAMESPACE::FbxDouble3>();
		int associatedTextureCount = transparencyColorProp.GetSrcObjectCount<FBXSDK_NAMESPACE::FbxFileTexture>();
		if (associatedTextureCount > 0)
		{
			FBXSDK_NAMESPACE::FbxFileTexture* fbxMap = transparencyColorProp.GetSrcObject<FBXSDK_NAMESPACE::FbxFileTexture>(0);
			TextureMap* map = new TextureMap();
			map->parseFBXTextureMap(fbxMap);
			mTransparencyIndexInDepot = appendMaterialMap(map, textureMaps, textureFiles);
			mTransparencyEnable = true;
			mTransparencyFactor = 1.0;
		}
	}
}

void Material::parsePMXMaterial(
	unsigned int id,
	void* material,
	std::vector<TextureMap*>& textureMaps,
	std::vector<std::string>& textureFiles)
{
	PmxReader::PmxMaterial* mat = static_cast<PmxReader::PmxMaterial*>(material);

	//MaterialName
	mUniqueID = id;

	mMaterialName = standardizeFileName(mat->Name).c_str();
	if (mMaterialName.IsEmpty())
	{
		mMaterialName = "Unknown_Material";
	}

	//Culling
	mCulling = 0; //PmxMaterial::Flags

	//ShadeModel
	mShadeModel = "Toon";
	
	//EmissiveColor
	mEmissiveFactor = 1.0;
	mEmissiveColor = FBXSDK_NAMESPACE::FbxDouble3(0.0, 0.0, 0.0);
	mEmissiveMapIndexInDepot = -1;

	//AmbientColor
	mAmbientFactor = 1.0;
	mAmbientColor = FBXSDK_NAMESPACE::FbxDouble3(1.0, 1.0, 1.0);
	mAmbientMapIndexInDepot = -1;

	//DiffuseColor
	mDiffuseFactor = 1.0;
	mDiffuseColor = FBXSDK_NAMESPACE::FbxDouble3(mat->Diffuse.X, mat->Diffuse.Y, mat->Diffuse.Z);
	{
		TextureMap* map = new TextureMap();

		std::string fullFileName = getFileName(mat->Tex);
		size_t index = fullFileName.rfind('.');
		std::string ext = fullFileName.substr(index + 1);
		if (ext == "png" || ext == "tga" || mat->Diffuse.W < 1.0)
		{
			mTransparencyEnable = true;
		}
		else
		{
			mTransparencyEnable = false;
		}
		
		map->parseTextureMap(mat->Tex);
		mDiffuseMapIndexInDepot = appendMaterialMap(map, textureMaps, textureFiles);
	}

	//SpecularColor
	mSpecularFactor = 1.0;
	mSpecularColor = FBXSDK_NAMESPACE::FbxDouble3(mat->Specular.X, mat->Specular.Y, mat->Specular.Z);
	mSpecularMapIndexInDepot = -1;

	mGlossinessValue = mat->Power;

	//Advanced Properties
	//NormalMap
	mNormalMapIndexInDepot = -1;
	mNormalColor = FBXSDK_NAMESPACE::FbxDouble3(0.0, 0.0, 1.0);	

	//Bump
	mBumpFactor = 1.0;
	mBumpMapIndexInDepot = -1;
	mBumpColor = FBXSDK_NAMESPACE::FbxDouble3(0.0, 0.0, 1.0);
	mBumpMapIndexInDepot = -1;	

	//ReflectionColor
	mReflectionFactor = 1.0;
	mReflectionColor = FBXSDK_NAMESPACE::FbxDouble3(0.0, 0.0, 0.0);
	mReflectionMapIndexInDepot = -1;

	//DisplacementColor
	mDisplacementFactor = 1.0;
	mDisplacementColor = FBXSDK_NAMESPACE::FbxDouble3(0.0, 0.0, 0.0);
	mDisplacementMapIndexInDepot = -1;

	//Opacity
	mOpacity = 1.0;

	//Transparency
	mTransparencyIndexInDepot = -1;
	mTransparencyFactor = mat->Diffuse.W;
	mTransparencyColor = FBXSDK_NAMESPACE::FbxDouble3(0.0, 0.0, 0.0);	
}

inline FbxDouble3 ConvertFromArray(const std::array<float, 3>& v) {
    return FbxDouble3(v[0], v[1], v[2]);
}
inline std::string GetFileNameFromPath(const std::string& path) {
    auto pos = path.find_last_of("/\\");
    if (pos == std::string::npos) {
        return path;
    } else {
        return path.substr(pos + 1);
    }
}
void Material::TakeoutMaxTextureMap(const max::Texmaps::MapBasicInfo& texture_map, 
    FbxDouble& factor, 
    int& MapIndex,
    std::vector<TextureMap*>& textureMaps,
    std::vector<std::string>& textureFiles) {
    MapIndex = -1;
    factor = 1.0;
    if (texture_map.enabled) {
        TextureMap* map = new TextureMap();
        factor = texture_map.amount;
        if (texture_map.target_map_node) {
            std::string filepath = texture_map.target_map_node->GetImageFile();
            map->fileName = GetFileNameFromPath(filepath);
        }
        MapIndex = appendMaterialMap(map, textureMaps, textureFiles);
    }
}
void Material::parseMaxMaterial(void * material, std::vector<TextureMap*>& textureMaps, std::vector<std::string>& textureFiles) {
    max::StandardMaterial* mat = static_cast<max::StandardMaterial*>(material);

    mUniqueID = mat->id;
    mMaterialName = mat->thumbnail.name.c_str();
    if (mMaterialName.IsEmpty()) {
        mMaterialName = "Unknown_Material";
    }

    mCulling = mat->Culling();
    
    if (mat->mat_param) {
        mShadeModel = GetShaderTypeName(mat->mat_param->GetShaderType());
        // can take out:
        max::Texmaps::MapBasicInfo texture_map;

        //AmbientColor
        mAmbientFactor = 1.0;
        mAmbientColor = ConvertFromArray(mat->mat_param->ambient_color);
        texture_map = mat->GetAmbientMap();
        TakeoutMaxTextureMap(texture_map, mAmbientFactor, mAmbientMapIndexInDepot, textureMaps, textureFiles);

        //DiffuseColor
        mDiffuseFactor = 1.0;
        mDiffuseColor = ConvertFromArray(mat->mat_param->diffuse_color);
        texture_map = mat->GetDiffuseMap();
        TakeoutMaxTextureMap(texture_map, mDiffuseFactor, mDiffuseMapIndexInDepot, textureMaps, textureFiles);

        //SpecularColor
        mSpecularFactor = 1.0;
        mSpecularColor = ConvertFromArray(mat->mat_param->specular_color);
        texture_map = mat->GetSpecularMap();
        TakeoutMaxTextureMap(texture_map, mSpecularFactor, mSpecularMapIndexInDepot, textureMaps, textureFiles);

        // mGlossinessValue
        mGlossinessValue = mat->mat_param->glossiness;

        // mOpacity
        mOpacity = mat->opacity;

        //Transparency
        mTransparencyFactor = 1.0;
        mTransparencyColor = ConvertFromArray(mat->mat_param->diffuse_color);
        texture_map = mat->GetTransparencyMap();
        mTransparencyEnable = texture_map.enabled;
        TakeoutMaxTextureMap(texture_map, mTransparencyFactor, mTransparencyIndexInDepot, textureMaps, textureFiles);
    } else {
        // default values
        mShadeModel = "Unknown";
        mAmbientFactor = 1.0;
        mAmbientColor = FBXSDK_NAMESPACE::FbxDouble3(1.0, 1.0, 1.0);
        mAmbientMapIndexInDepot = -1;
        mDiffuseFactor = 1.0;
        mDiffuseColor = FBXSDK_NAMESPACE::FbxDouble3(1.0, 1.0, 1.0);
        mDiffuseMapIndexInDepot = -1;
        mSpecularFactor = 1.0;
        mSpecularColor = FBXSDK_NAMESPACE::FbxDouble3(0.0, 0.0, 0.0);
        mSpecularMapIndexInDepot = -1;
        mGlossinessValue = 0;
        mOpacity = 1.0;
        mTransparencyEnable = false;
        mTransparencyFactor = 1.0;
        mTransparencyColor = FBXSDK_NAMESPACE::FbxDouble3(1.0, 1.0, 1.0);
        mTransparencyIndexInDepot = -1;
    }
    // default:

    //EmissiveColor
    mEmissiveFactor = 1.0;
    mEmissiveColor = FBXSDK_NAMESPACE::FbxDouble3(0.0, 0.0, 0.0);
    mEmissiveMapIndexInDepot = -1;

    //Advanced Properties
    //NormalMap
    mNormalEnable = false;
    mNormalMapIndexInDepot = -1;
    mNormalColor = FBXSDK_NAMESPACE::FbxDouble3(0.0, 0.0, 1.0);

    //Bump
    mBumpFactor = 1.0;
    mBumpEnable = false;
    mBumpMapIndexInDepot = -1;
    mBumpColor = FBXSDK_NAMESPACE::FbxDouble3(0.0, 0.0, 1.0);

    //ReflectionColor
    mReflectionFactor = 1.0;
    mReflectionColor = FBXSDK_NAMESPACE::FbxDouble3(0.0, 0.0, 0.0);
    mReflectionMapIndexInDepot = -1;

    //DisplacementColor
    mDisplacementFactor = 1.0;
    mDisplacementColor = FBXSDK_NAMESPACE::FbxDouble3(0.0, 0.0, 0.0);
    mDisplacementMapIndexInDepot = -1;
}

void Material::parsePMDMaterial(
	unsigned int id,
	void* material,
	std::vector<TextureMap*>& textureMaps,
	std::vector<std::string>& textureFiles)
{
	pmd::PmdMaterial* mat = static_cast<pmd::PmdMaterial*>(material);

	//MaterialName
	mUniqueID = id;

	mMaterialName = "Unknown_Material";

	//Culling
	mCulling = 0; //PmxMaterial::Flags

				  //ShadeModel
	mShadeModel = "Toon";

	//EmissiveColor
	mEmissiveFactor = 1.0;
	mEmissiveColor = FBXSDK_NAMESPACE::FbxDouble3(0.0, 0.0, 0.0);
	mEmissiveMapIndexInDepot = -1;

	//AmbientColor
	mAmbientFactor = 1.0;
	mAmbientColor = FBXSDK_NAMESPACE::FbxDouble3(1.0, 1.0, 1.0);
	mAmbientMapIndexInDepot = -1;

	//DiffuseColor
	mDiffuseFactor = 1.0;
	mDiffuseColor = FBXSDK_NAMESPACE::FbxDouble3(mat->diffuse[0], mat->diffuse[1], mat->diffuse[2]);

	{
		TextureMap* map = new TextureMap();

		std::string fullFileName = getFileName(mat->texture_filename);
		size_t index = fullFileName.rfind('.');
		std::string ext = fullFileName.substr(index + 1);
		if (ext == "png" || ext == "tga" || mat->diffuse[3] < 1.0)
		{
			mTransparencyEnable = true;
		}
		else
		{
			mTransparencyEnable = false;
		}

		map->parseTextureMap(mat->texture_filename);
		mDiffuseMapIndexInDepot = appendMaterialMap(map, textureMaps, textureFiles);
	}

	//SpecularColor
	mSpecularFactor = 1.0;
	mSpecularColor = FBXSDK_NAMESPACE::FbxDouble3(mat->specular[0], mat->specular[1], mat->specular[2]);
	mSpecularMapIndexInDepot = -1;

	mGlossinessValue = mat->power;

	//Advanced Properties
	//NormalMap
	mNormalMapIndexInDepot = -1;
	mNormalColor = FBXSDK_NAMESPACE::FbxDouble3(0.0, 0.0, 1.0);

	//Bump
	mBumpFactor = 1.0;
	mBumpMapIndexInDepot = -1;
	mBumpColor = FBXSDK_NAMESPACE::FbxDouble3(0.0, 0.0, 1.0);
	mBumpMapIndexInDepot = -1;

	//ReflectionColor
	mReflectionFactor = 1.0;
	mReflectionColor = FBXSDK_NAMESPACE::FbxDouble3(0.0, 0.0, 0.0);
	mReflectionMapIndexInDepot = -1;

	//DisplacementColor
	mDisplacementFactor = 1.0;
	mDisplacementColor = FBXSDK_NAMESPACE::FbxDouble3(0.0, 0.0, 0.0);
	mDisplacementMapIndexInDepot = -1;

	//Opacity
	mOpacity = 1.0;

	//Transparency
	mTransparencyIndexInDepot = -1;
	mTransparencyFactor = mat->diffuse[3];
	mTransparencyColor = FBXSDK_NAMESPACE::FbxDouble3(0.0, 0.0, 0.0);
}

void Material::parseSTLMaterial(
	unsigned int id,
	const FbxVector4& diffuse,
	const FbxVector4& specular,
	const FbxVector4& ambient,
	std::vector<TextureMap*>& textureMaps,
	std::vector<std::string>& textureFiles)
{
	//MaterialName
	mUniqueID = id;

	mMaterialName = "Unknown_Material";

	//Culling
	mCulling = 0;

	mShadeModel = "Toon";

	//EmissiveColor
	mEmissiveFactor = 1.0;
	mEmissiveColor = FBXSDK_NAMESPACE::FbxDouble3(0.0, 0.0, 0.0);
	mEmissiveMapIndexInDepot = -1;

	//AmbientColor
	mAmbientFactor = 1.0;
	mAmbientColor = FBXSDK_NAMESPACE::FbxDouble3(ambient[0], ambient[1], ambient[2]);
	mAmbientMapIndexInDepot = -1;

	//DiffuseColor
	mDiffuseFactor = 1.0;
	mDiffuseColor = FBXSDK_NAMESPACE::FbxDouble3(diffuse[0], diffuse[1],diffuse[2]);
	mDiffuseMapIndexInDepot = -1;	

	//SpecularColor
	mSpecularFactor = 1.0;
	mSpecularColor = FBXSDK_NAMESPACE::FbxDouble3(specular[0], specular[1], specular[2]);
	mSpecularMapIndexInDepot = -1;

	mGlossinessValue = 0;

	//Advanced Properties
	//NormalMap
	mNormalMapIndexInDepot = -1;
	mNormalColor = FBXSDK_NAMESPACE::FbxDouble3(0.0, 0.0, 1.0);

	//Bump
	mBumpFactor = 1.0;
	mBumpMapIndexInDepot = -1;
	mBumpColor = FBXSDK_NAMESPACE::FbxDouble3(0.0, 0.0, 1.0);
	mBumpMapIndexInDepot = -1;

	//ReflectionColor
	mReflectionFactor = 1.0;
	mReflectionColor = FBXSDK_NAMESPACE::FbxDouble3(0.0, 0.0, 0.0);
	mReflectionMapIndexInDepot = -1;

	//DisplacementColor
	mDisplacementFactor = 1.0;
	mDisplacementColor = FBXSDK_NAMESPACE::FbxDouble3(0.0, 0.0, 0.0);
	mDisplacementMapIndexInDepot = -1;

	//Opacity
	mOpacity = 1.0;

	//Transparency
	mTransparencyIndexInDepot = -1;
	mTransparencyFactor = diffuse[3];
	mTransparencyColor = FBXSDK_NAMESPACE::FbxDouble3(0.0, 0.0, 0.0);
}