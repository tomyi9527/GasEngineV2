#include "stdafx.h"
#include "TextureMap.h"

TextureMap::TextureMap()
{
}

TextureMap::~TextureMap()
{
}

void TextureMap::parseFBXTextureMap(FBXSDK_NAMESPACE::FbxFileTexture* map)
{
	T[0] = T[1] = T[2] = 0.0;
	R[0] = R[1] = R[2] = 0.0;
	S[0] = S[1] = S[2] = 1.0;
	Rp[0] = Rp[1] = Rp[2] = 0.0;
	Sp[0] = Sp[1] = Sp[2] = 0.0;

	wrapU = FBXSDK_NAMESPACE::FbxFileTexture::eRepeat;
	wrapV = FBXSDK_NAMESPACE::FbxFileTexture::eRepeat;
	uvSwapFlag = false;

	fileName = "";

	if(map)
	{
		T = map->FindProperty("Translation").Get<FBXSDK_NAMESPACE::FbxDouble3>();
		R = map->FindProperty("Rotation").Get<FBXSDK_NAMESPACE::FbxDouble3>();
		S = map->FindProperty("Scaling").Get<FBXSDK_NAMESPACE::FbxDouble3>();
		Rp = map->FindProperty("TextureRotationPivot").Get<FBXSDK_NAMESPACE::FbxDouble3>();
		Sp = map->FindProperty("TextureScalingPivot").Get<FBXSDK_NAMESPACE::FbxDouble3>();

		wrapU = map->FindProperty("WrapModeU").Get<FBXSDK_NAMESPACE::FbxFileTexture::EWrapMode>();
		wrapV = map->FindProperty("WrapModeV").Get<FBXSDK_NAMESPACE::FbxFileTexture::EWrapMode>();
		uvSwapFlag = map->FindProperty("UVSwap").Get<FbxBool>();

		std::string texturePath = map->GetFileName();
		fileName = getFileName(texturePath);
	}
}

void TextureMap::parseTextureMap(const std::string& diffuseTexture)
{
	T[0] = T[1] = T[2] = 0.0;
	R[0] = R[1] = R[2] = 0.0;
	S[0] = S[1] = S[2] = 1.0;
	Rp[0] = Rp[1] = Rp[2] = 0.0;
	Sp[0] = Sp[1] = Sp[2] = 0.0;

	wrapU = FBXSDK_NAMESPACE::FbxFileTexture::eRepeat;
	wrapV = FBXSDK_NAMESPACE::FbxFileTexture::eRepeat;
	uvSwapFlag = false;

	fileName = diffuseTexture;
}