#pragma once
#include "CommonStruct.h"

class TextureMap
{
public:
	TextureMap();
	~TextureMap();

	void parseFBXTextureMap(FBXSDK_NAMESPACE::FbxFileTexture* map);

	void parseTextureMap(const std::string& diffuseTexture);

	FBXSDK_NAMESPACE::FbxDouble3				T;
	FBXSDK_NAMESPACE::FbxDouble3				R;
	FBXSDK_NAMESPACE::FbxDouble3				S;
	FBXSDK_NAMESPACE::FbxDouble3				Rp;
	FBXSDK_NAMESPACE::FbxDouble3				Sp;
	FBXSDK_NAMESPACE::FbxFileTexture::EWrapMode wrapU;
	FBXSDK_NAMESPACE::FbxFileTexture::EWrapMode wrapV;
	FBXSDK_NAMESPACE::FbxBool					uvSwapFlag;
	std::string									fileName;
};

