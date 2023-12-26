#include "stdafx.h"
#include "PlayCanvasWriter.h"

PlayCanvasWriter::PlayCanvasWriter()
	: mJSONWriter(NULL)
{

}

PlayCanvasWriter::~PlayCanvasWriter()
{

}

void PlayCanvasWriter::setRapidJsonWriter(rapidjson::Writer<rapidjson::FileWriteStream>* writer)
{
	mJSONWriter = writer;
}

void PlayCanvasWriter::writeObjectBin(
	FbxUInt64 parentNodeID,
	FbxUInt64 nodeID,
	const char* objectType,
	const std::string& objectName,
	unsigned int vertexCount,
	std::vector<VERTEX_LAYER_TYPE>& vertexElement,
	std::vector<BONE_>* bones,
	\
	unsigned int subMeshCount,
	uint8_t* subMeshEncodingBuffer,
	unsigned int subMeshEncodingSize,
	unsigned int subMeshEncodingFlag,
	\
	unsigned int triangleCount,
	uint8_t* indexEncodingBuffer,
	unsigned int indexEncodingSize,
	unsigned int indexEncodingFlag,
	\
	unsigned int topologicalLineCount,
	uint8_t* topologicalIndexEncodingBuffer,
	unsigned int topologicalIndexEncodingSize,
	unsigned int topologicalIndexEncodingFlag,
	\
	FBXSDK_NAMESPACE::FbxDouble3& PosBboxMin,
	FBXSDK_NAMESPACE::FbxDouble3& PosBboxMax,
	uint8_t* posistionEncodingBuffer,
	unsigned int posistionEncodingSize,
	unsigned int posistionEncodingFlag,
	\
	uint8_t* normalEncodingBuffer,
	unsigned int normalEncodingSize,
	unsigned int normalEncodingFlag,
	\
	uint8_t* tangentEncodingBuffer,
	unsigned int tangentEncodingSize,
	unsigned int tangentEncodingFlag,
	\
	uint8_t* vcEncodingBuffer,
	unsigned int vcEncodingSize,
	unsigned int vcEncodingFlag,
	\
	FBXSDK_NAMESPACE::FbxDouble3& uvBboxMin,
	FBXSDK_NAMESPACE::FbxDouble3& uvBboxMax,
	uint8_t* uvEncodingBuffer,
	unsigned int uvEncodingSize,
	unsigned int uvEncodingFlag,
	\
	uint8_t* bwEncodingBuffer,
	unsigned int bwEncodingSize,
	unsigned int bwEncodingFlag,
	\
	uint8_t* biEncodingBuffer,
	unsigned int biEncodingSize,
	unsigned int biBIEncodingFlag)
{
	if (mJSONWriter != NULL)
	{
		if (bones != NULL)
		{
			mJSONWriter->Key("skins");
			mJSONWriter->StartArray();
			mJSONWriter->StartObject();
			unsigned int boneCount = (unsigned int)bones->size();
			mJSONWriter->Key("inverseBindMatrices");
			mJSONWriter->StartArray();
			for (unsigned int boneIndex = 0; boneIndex < boneCount; ++boneIndex)
			{
				const BONE_& bone = (*bones)[boneIndex];
				mJSONWriter->StartArray();
				for (int row = 0; row < 4; ++row)
				{
					for (int col = 0; col < 4; ++col)
					{
						double v = bone.modelWorldToBoneLocal.Get(row, col);
						mJSONWriter->Double(v);
					}
				}				
				mJSONWriter->EndArray();
			}
			mJSONWriter->EndArray();

			mJSONWriter->Key("boneNames");
			mJSONWriter->StartArray();
			for (unsigned int boneIndex = 0; boneIndex < boneCount; ++boneIndex)
			{
				const BONE_& bone = (*bones)[boneIndex];
				mJSONWriter->String(bone.name.c_str());
			}
			mJSONWriter->EndArray();			
			mJSONWriter->EndObject();
			mJSONWriter->EndArray();
		}

		mJSONWriter->Key("vertices");
		mJSONWriter->StartArray();
		mJSONWriter->StartObject();
		//Vertex Elements
		unsigned int elementCount = (unsigned int)vertexElement.size();
		for (unsigned int i = 0; i < elementCount; ++i)
		{
			VERTEX_LAYER_TYPE type = vertexElement[i];
			if (type == VL_POSITION)
			{
				mJSONWriter->Key("position");
				mJSONWriter->StartObject();
					mJSONWriter->Key("type");
					mJSONWriter->Key("float32");
					mJSONWriter->Key("components");
					mJSONWriter->Int(3);
					mJSONWriter->Key("data");
					mJSONWriter->StartArray();
					for (unsigned int i = 0; i < vertexCount * 3; ++i)
					{
						mJSONWriter->Double(((float*)posistionEncodingBuffer)[i]);
					}					
					mJSONWriter->EndArray();
				mJSONWriter->EndObject();
			}
			else if (type == VL_NORMAL0)
			{
				mJSONWriter->Key("normal");
				mJSONWriter->StartObject();
				mJSONWriter->Key("type");
				mJSONWriter->Key("float32");
				mJSONWriter->Key("components");
				mJSONWriter->Int(3);
				mJSONWriter->Key("data");
				mJSONWriter->StartArray();
				for (unsigned int i = 0; i < vertexCount * 3; ++i)
				{
					mJSONWriter->Double(((float*)normalEncodingBuffer)[i]);
				}
				mJSONWriter->EndArray();
				mJSONWriter->EndObject();
			}
			else if (type == VL_TANGENT0)
			{
				
			}
			else if (type == VL_UV0)
			{
				mJSONWriter->Key("texCoord0");
				mJSONWriter->StartObject();
				mJSONWriter->Key("type");
				mJSONWriter->Key("float32");
				mJSONWriter->Key("components");
				mJSONWriter->Int(2);
				mJSONWriter->Key("data");
				mJSONWriter->StartArray();
				for (unsigned int i = 0; i < vertexCount * 2; ++i)
				{
					mJSONWriter->Double(((float*)uvEncodingBuffer)[i]);
				}
				mJSONWriter->EndArray();
				mJSONWriter->EndObject();
			}
			else if (type == VL_VERTEXCOLOR0)
			{
				
			}
		}

		//BW BI	
		if (bwEncodingBuffer != NULL && biEncodingBuffer != NULL)
		{
			mJSONWriter->Key("blendIndices");
			mJSONWriter->StartObject();
			mJSONWriter->Key("type");
			mJSONWriter->String("uint8");
			mJSONWriter->Key("components");
			mJSONWriter->Int(4);
			mJSONWriter->Key("data");
			mJSONWriter->StartArray();
			for (unsigned int i = 0; i < vertexCount * 4; ++i)
			{
				unsigned int bi = (unsigned int)((float*)biEncodingBuffer)[i];
				mJSONWriter->Uint(bi);
			}
			mJSONWriter->EndArray();
			mJSONWriter->EndObject();

			mJSONWriter->Key("blendWeight");
			mJSONWriter->StartObject();
			mJSONWriter->Key("type");
			mJSONWriter->String("float32");
			mJSONWriter->Key("components");
			mJSONWriter->Int(4);
			mJSONWriter->Key("data");
			mJSONWriter->StartArray();
			for (unsigned int i = 0; i < vertexCount * 4; ++i)
			{
				mJSONWriter->Double(((float*)bwEncodingBuffer)[i]);
			}
			mJSONWriter->EndArray();
			mJSONWriter->EndObject();
		}
		mJSONWriter->EndObject();
		mJSONWriter->EndArray();

		////// sub mesh //////
		//Indices
		if (indexEncodingBuffer > 0 && subMeshCount > 0)
		{
			mJSONWriter->Key("meshes");
			mJSONWriter->StartArray();

			mJSONWriter->StartObject();
				mJSONWriter->Key("aabb");
				mJSONWriter->StartObject();
				mJSONWriter->Key("min");
				mJSONWriter->StartArray();
				mJSONWriter->Double(PosBboxMin[0]);
				mJSONWriter->Double(PosBboxMin[1]);
				mJSONWriter->Double(PosBboxMin[2]);
				mJSONWriter->EndArray();

				mJSONWriter->Key("max");
				mJSONWriter->StartArray();
				mJSONWriter->Double(PosBboxMax[0]);
				mJSONWriter->Double(PosBboxMax[1]);
				mJSONWriter->Double(PosBboxMax[2]);
				mJSONWriter->EndArray();
				mJSONWriter->EndObject();

				mJSONWriter->Key("vertices");
				mJSONWriter->Int(0);

				mJSONWriter->Key("skin");
				mJSONWriter->Int(0);

				mJSONWriter->Key("indices");
				mJSONWriter->StartArray();
				for (unsigned int i = 0; i < triangleCount * 3; ++i)
				{
					unsigned int index = ((unsigned int*)indexEncodingBuffer)[i];
					mJSONWriter->Uint(index);
				}
				mJSONWriter->EndArray();

				mJSONWriter->Key("type");
				mJSONWriter->String("triangles");

				mJSONWriter->Key("base");
				mJSONWriter->Int(0);

				mJSONWriter->Key("count");
				mJSONWriter->Int(triangleCount*3);

			mJSONWriter->EndObject();

			mJSONWriter->EndArray();
		}
	}
}

void PlayCanvasWriter::writeAnimationBin(
	FbxUInt64 clipID,
	const std::string& clipName,
	float fps,
	float startFrame,
	float endFrame,
	std::vector<animationClipData>* clipData)
{

}