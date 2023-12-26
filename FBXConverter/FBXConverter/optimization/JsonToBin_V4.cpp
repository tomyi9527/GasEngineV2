#include "stdafx.h"
#include "JsonToBin_V4.h"
#include "NeonateVertexCompression_V4.h"
#include "cJSON/cJSON.h"

const char* SectionTypeName[] =
{
	"POSITION",
	"NORMAL0",
	"NORMAL1",
	"TANGENT0",
	"TANGENT1",
	"BINORMAL0",
	"BINORMAL1",
	"UV0",
	"UV1",
	"VERTEXCOLOR0",
	"VERTEXCOLOR1",
	"BLENDWEIGHT",
	"BLENDINDEX",
	"INDEX",
	"SUBMESH",
	"BONE",
	"MORPHTARGET",
	"KEYFRAME",
	"TOPOLOGY"
};

const char* UNKNOWN_TYPE = "UNKNOWN";
const char* MESH_TYPE = "MESH";
const char* MORPHTARGET_TYPE = "MORPHTARGET";
const char* ANIMATION_TYPE = "ANIMATION";

const char* FILE_BEGINNING_FLAG = "NEO*";
const char* FILE_END_FLAG = "*OEN";

const unsigned int ENDIANFLAG = 0x12345678;
const unsigned int TARGETFRAMEKEYFLAG = 0x59454B00;  //00"KEY"

const double EPSILON_THRESHOLD = 0.00001;

const char* VERTEX_ELEMENT_NAME[] = {
	"positions",
	"normals",
	"normal1s",
	"tangents",
	"tangent1s",
	"binormals",
	"binormal1s",
	"uvs",
	"uv1s",
	"vertexColor0",
	"vertexColor1",
	"blendWeights",
	"blendIndices"
};

JsonToBin_V4::JsonToBin_V4()
	: mBinFile(NULL)
	, mSections(NULL)
{
}


JsonToBin_V4::~JsonToBin_V4()
{
}

void JsonToBin_V4::openExportFile(const string& filePath)
{
#ifdef _MSC_VER
	std::wstring unicodePath = UTF8_To_UCS16(filePath.c_str());
	mBinFile = _wfopen(unicodePath.c_str(), L"wb+");
#else
	mBinFile = fopen(filePath.c_str(), "wb+");
#endif
}

void JsonToBin_V4::closeExportFile()
{
	//Clear Memory	
	if(mSections != NULL)
	{
		for(int i = 0; i < SECITON_TYPE_COUNT; ++i)
		{
			if(mSections[i].entry)
			{
				delete mSections[i].entry;
			}

			if(mSections[i].data)
			{
				delete[](uint8_t*)(mSections[i].data);
			}
		}
		delete[] mSections;
	}

	if (mBinFile != NULL)
	{
		fclose(mBinFile);
		mBinFile = NULL;
	}
}

void JsonToBin_V4::writeObjectBin
(
	unsigned int parentNodeID,
	unsigned int nodeID,
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
	float PosBboxMin[3],
	float PosBboxMax[3],
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
	FBXSDK_NAMESPACE::FbxDouble3& uv0BboxMin,
	FBXSDK_NAMESPACE::FbxDouble3& uv0BboxMax,
	uint8_t* uv0EncodingBuffer,
	unsigned int uv0EncodingSize,
	unsigned int uv0EncodingFlag,
	\
	FBXSDK_NAMESPACE::FbxDouble3& uv1BboxMin,
	FBXSDK_NAMESPACE::FbxDouble3& uv1BboxMax,
	uint8_t* uv1EncodingBuffer,
	unsigned int uv1EncodingSize,
	unsigned int uv1EncodingFlag,
	\
	uint8_t* bwEncodingBuffer,
	unsigned int bwEncodingSize,
	unsigned int bwEncodingFlag,
	\
	uint8_t* biEncodingBuffer,
	unsigned int biEncodingSize,
	unsigned int biBIEncodingFlag)
{
	//HEADER
	BIN_HEADER header;
	memset(&header, 0, sizeof(header));
	memcpy(header.FILE_FLAG, FILE_BEGINNING_FLAG, strlen(FILE_BEGINNING_FLAG));
	header.ENDIAN_FLAG = ENDIANFLAG;
	header.VERSION = 7;

	//header.SECTION_TABLE_OFFSET = sizeof(BIN_HEADER);

	header.POS_BBOX_MIN[0] = (float)PosBboxMin[0];
	header.POS_BBOX_MIN[1] = (float)PosBboxMin[1];
	header.POS_BBOX_MIN[2] = (float)PosBboxMin[2];

	header.POS_BBOX_MAX[0] = (float)PosBboxMax[0];
	header.POS_BBOX_MAX[1] = (float)PosBboxMax[1];
	header.POS_BBOX_MAX[2] = (float)PosBboxMax[2];

	header.UV0_BBOX_MIN[0] = (float)uv0BboxMin[0];
	header.UV0_BBOX_MIN[1] = (float)uv0BboxMin[1];

	header.UV0_BBOX_MAX[0] = (float)uv0BboxMax[0];
	header.UV0_BBOX_MAX[1] = (float)uv0BboxMax[1];

	header.UV1_BBOX_MIN[0] = (float)uv1BboxMin[0];
	header.UV1_BBOX_MIN[1] = (float)uv1BboxMin[1];

	header.UV1_BBOX_MAX[0] = (float)uv1BboxMax[0];
	header.UV1_BBOX_MAX[1] = (float)uv1BboxMax[1];

	header.PARENTUNIQUEID = parentNodeID;
	header.UNIQUEID = nodeID;
	memcpy(header.OBJECTTYPE, objectType, strlen(objectType));


	///////////////////////// String Table /////////////////////////
	std::vector<std::string> strings;
	strings.push_back(objectName);
	if (bones != NULL)
	{
		unsigned int boneCount = (unsigned int)bones->size();
		for (unsigned int boneIndex = 0; boneIndex < boneCount; ++boneIndex)
		{
			const BONE_& bone = (*bones)[boneIndex];
			strings.push_back(bone.name);
			strings.push_back(bone.name1);
		}
	}
	unsigned int stringTableSize = 0;
	char* stringTable = composeStringTable_V1(strings, stringTableSize);
	////////////////////////////////////////////////////////////////

	header.OBJECT_NAME_INDEX = mStringTable_V1[objectName];
	header.STRING_TABLE_OFFSET = sizeof(BIN_HEADER);
	header.SECTION_TABLE_OFFSET = sizeof(BIN_HEADER) + stringTableSize;

	//SECTION TABLE
	mSections = new SECTION_DATA[SECITON_TYPE_COUNT];
	memset(mSections, 0, sizeof(SECTION_DATA)*SECITON_TYPE_COUNT);

	if (bones != NULL)
	{
		unsigned int boneCount = (unsigned int)bones->size();
		if (boneCount > 0)
		{
			unsigned int boneAttributeCount = 20; //ID + STRING_INDEX_3 + 16MATRIX
			float* boneData = (float*)(new uint8_t[sizeof(float)*boneAttributeCount*boneCount]);
			int index = 0;
			for (unsigned int boneIndex = 0; boneIndex < boneCount; ++boneIndex)
			{
				int boneUniqueID = (int)(*bones)[boneIndex].id;
				int* boneData_casted = reinterpret_cast<int*>(boneData);
				boneData_casted[index++] = boneUniqueID;
				boneData_casted[index++] = mStringTable_V1[(*bones)[boneIndex].name];
				boneData_casted[index++] = mStringTable_V1[(*bones)[boneIndex].name1];
				boneData_casted[index++] = -1;
				FBXSDK_NAMESPACE::FbxAMatrix& w2blMatrix = (*bones)[boneIndex].modelWorldToBoneLocal;
				for (int col = 0; col < 4; ++col)
				{
					for (int row = 0; row < 4; ++row)
					{
						boneData[index++] = (float)w2blMatrix.Get(row, col);
					}
				}
			}

			mSections[BONE].entry = new SECTION_TABLE_ENTRY();
			memset(mSections[BONE].entry, 0, sizeof(SECTION_TABLE_ENTRY));
			mSections[BONE].entry->SECTION_TYPE = (unsigned int)BONE;
			mSections[BONE].entry->DATA_ATTRIBUTE = 0;
			mSections[BONE].entry->SECTION_OFFSET = 0;
			mSections[BONE].entry->SECTION_LENGTH = sizeof(float)*boneAttributeCount*boneCount;
			mSections[BONE].entry->ATTRIBUTE_COUNT = boneCount;
			mSections[BONE].entry->ELEMENT_COUNT = boneAttributeCount;
			mSections[BONE].entry->ELEMENT_TYPE = STRUCT_TYPE;
			mSections[BONE].data = (void*)boneData;
		}
	}

	//Vertex Elements
	unsigned int elementCount = (unsigned int)vertexElement.size();
	for (unsigned int i = 0; i < elementCount; ++i)
	{
		VERTEX_LAYER_TYPE type = vertexElement[i];
		if (type == VL_POSITION)
		{
			mSections[POSITION].entry = new SECTION_TABLE_ENTRY();
			memset(mSections[POSITION].entry, 0, sizeof(SECTION_TABLE_ENTRY));
			mSections[POSITION].entry->SECTION_TYPE = (unsigned int)POSITION;
			mSections[POSITION].entry->DATA_ATTRIBUTE = posistionEncodingFlag;
			mSections[POSITION].entry->SECTION_OFFSET = 0;
			mSections[POSITION].entry->SECTION_LENGTH = posistionEncodingSize;
			mSections[POSITION].entry->ATTRIBUTE_COUNT = vertexCount;
			mSections[POSITION].entry->ELEMENT_COUNT = 3;
			mSections[POSITION].entry->ELEMENT_TYPE = FLOAT_TYPE;
			mSections[POSITION].data = (void*)posistionEncodingBuffer;
		}
		else if (type == VL_NORMAL0)
		{
			mSections[NORMAL0].entry = new SECTION_TABLE_ENTRY();
			memset(mSections[NORMAL0].entry, 0, sizeof(SECTION_TABLE_ENTRY));
			mSections[NORMAL0].entry->SECTION_TYPE = (unsigned int)NORMAL0;
			mSections[NORMAL0].entry->DATA_ATTRIBUTE = normalEncodingFlag;
			mSections[NORMAL0].entry->SECTION_OFFSET = 0;
			mSections[NORMAL0].entry->SECTION_LENGTH = normalEncodingSize;
			mSections[NORMAL0].entry->ATTRIBUTE_COUNT = vertexCount;
			mSections[NORMAL0].entry->ELEMENT_COUNT = 3;
			mSections[NORMAL0].entry->ELEMENT_TYPE = FLOAT_TYPE;
			mSections[NORMAL0].data = (void*)normalEncodingBuffer;
		}
		else if (type == VL_TANGENT0)
		{
			mSections[TANGENT0].entry = new SECTION_TABLE_ENTRY();
			memset(mSections[TANGENT0].entry, 0, sizeof(SECTION_TABLE_ENTRY));
			mSections[TANGENT0].entry->SECTION_TYPE = (unsigned int)TANGENT0;
			mSections[TANGENT0].entry->DATA_ATTRIBUTE = tangentEncodingFlag;
			mSections[TANGENT0].entry->SECTION_OFFSET = 0;
			mSections[TANGENT0].entry->SECTION_LENGTH = tangentEncodingSize;
			mSections[TANGENT0].entry->ATTRIBUTE_COUNT = vertexCount;
			mSections[TANGENT0].entry->ELEMENT_COUNT = 4;
			mSections[TANGENT0].entry->ELEMENT_TYPE = FLOAT_TYPE;
			mSections[TANGENT0].data = (void*)tangentEncodingBuffer;
		}
		else if (type == VL_UV0)
		{
			mSections[UV0].entry = new SECTION_TABLE_ENTRY();
			memset(mSections[UV0].entry, 0, sizeof(SECTION_TABLE_ENTRY));
			mSections[UV0].entry->SECTION_TYPE = (unsigned int)UV0;
			mSections[UV0].entry->DATA_ATTRIBUTE = uv0EncodingFlag;
			mSections[UV0].entry->SECTION_OFFSET = 0;
			mSections[UV0].entry->SECTION_LENGTH = uv0EncodingSize;
			mSections[UV0].entry->ATTRIBUTE_COUNT = vertexCount;
			mSections[UV0].entry->ELEMENT_COUNT = 2;
			mSections[UV0].entry->ELEMENT_TYPE = FLOAT_TYPE;
			mSections[UV0].data = (void*)uv0EncodingBuffer;
		}
		else if (type == VL_UV1)
		{
			mSections[UV1].entry = new SECTION_TABLE_ENTRY();
			memset(mSections[UV1].entry, 0, sizeof(SECTION_TABLE_ENTRY));
			mSections[UV1].entry->SECTION_TYPE = (unsigned int)UV1;
			mSections[UV1].entry->DATA_ATTRIBUTE = uv1EncodingFlag;
			mSections[UV1].entry->SECTION_OFFSET = 0;
			mSections[UV1].entry->SECTION_LENGTH = uv1EncodingSize;
			mSections[UV1].entry->ATTRIBUTE_COUNT = vertexCount;
			mSections[UV1].entry->ELEMENT_COUNT = 2;
			mSections[UV1].entry->ELEMENT_TYPE = FLOAT_TYPE;
			mSections[UV1].data = (void*)uv1EncodingBuffer;
		}
		else if (type == VL_VERTEXCOLOR0)
		{
			mSections[VERTEXCOLOR0].entry = new SECTION_TABLE_ENTRY();
			memset(mSections[VERTEXCOLOR0].entry, 0, sizeof(SECTION_TABLE_ENTRY));
			mSections[VERTEXCOLOR0].entry->SECTION_TYPE = (unsigned int)VERTEXCOLOR0;
			mSections[VERTEXCOLOR0].entry->DATA_ATTRIBUTE = vcEncodingFlag;
			mSections[VERTEXCOLOR0].entry->SECTION_OFFSET = 0;
			mSections[VERTEXCOLOR0].entry->SECTION_LENGTH = vcEncodingSize;
			mSections[VERTEXCOLOR0].entry->ATTRIBUTE_COUNT = vertexCount;
			mSections[VERTEXCOLOR0].entry->ELEMENT_COUNT = 4;
			mSections[VERTEXCOLOR0].entry->ELEMENT_TYPE = UBYTE_TYPE;
			mSections[VERTEXCOLOR0].data = (void*)vcEncodingBuffer;
		}
	}

	//BW BI	
	if (bwEncodingBuffer != NULL && biEncodingBuffer != NULL)
	{
		mSections[BLENDWEIGHT].entry = new SECTION_TABLE_ENTRY();
		memset(mSections[BLENDWEIGHT].entry, 0, sizeof(SECTION_TABLE_ENTRY));
		mSections[BLENDWEIGHT].entry->SECTION_TYPE = (unsigned int)BLENDWEIGHT;
		mSections[BLENDWEIGHT].entry->DATA_ATTRIBUTE = bwEncodingFlag;
		mSections[BLENDWEIGHT].entry->SECTION_OFFSET = 0;
		mSections[BLENDWEIGHT].entry->SECTION_LENGTH = bwEncodingSize;
		mSections[BLENDWEIGHT].entry->ATTRIBUTE_COUNT = vertexCount;
		mSections[BLENDWEIGHT].entry->ELEMENT_COUNT = 4;
		mSections[BLENDWEIGHT].entry->ELEMENT_TYPE = FLOAT_TYPE;
		mSections[BLENDWEIGHT].data = (void*)bwEncodingBuffer;

		mSections[BLENDINDEX].entry = new SECTION_TABLE_ENTRY();
		memset(mSections[BLENDINDEX].entry, 0, sizeof(SECTION_TABLE_ENTRY));
		mSections[BLENDINDEX].entry->SECTION_TYPE = (unsigned int)BLENDINDEX;
		mSections[BLENDINDEX].entry->DATA_ATTRIBUTE = biBIEncodingFlag;
		mSections[BLENDINDEX].entry->SECTION_OFFSET = 0;
		mSections[BLENDINDEX].entry->SECTION_LENGTH = biEncodingSize;
		mSections[BLENDINDEX].entry->ATTRIBUTE_COUNT = vertexCount;
		mSections[BLENDINDEX].entry->ELEMENT_COUNT = 4;
		mSections[BLENDINDEX].entry->ELEMENT_TYPE = FLOAT_TYPE;
		mSections[BLENDINDEX].data = (void*)biEncodingBuffer;
	}

	//Indices
	if (indexEncodingBuffer > 0)
	{
		mSections[INDEX].entry = new SECTION_TABLE_ENTRY();
		memset(mSections[INDEX].entry, 0, sizeof(SECTION_TABLE_ENTRY));
		mSections[INDEX].entry->SECTION_TYPE = (unsigned int)INDEX;
		mSections[INDEX].entry->DATA_ATTRIBUTE = indexEncodingFlag;
		mSections[INDEX].entry->SECTION_OFFSET = 0;
		mSections[INDEX].entry->SECTION_LENGTH = indexEncodingSize;
		mSections[INDEX].entry->ATTRIBUTE_COUNT = triangleCount;
		mSections[INDEX].entry->ELEMENT_COUNT = 3;
		mSections[INDEX].entry->ELEMENT_TYPE = UINT_TYPE;
		mSections[INDEX].data = (void*)indexEncodingBuffer;
	}

	//Topology
	if (topologicalLineCount > 0)
	{
		mSections[TOPOLOGY].entry = new SECTION_TABLE_ENTRY();
		memset(mSections[TOPOLOGY].entry, 0, sizeof(SECTION_TABLE_ENTRY));
		mSections[TOPOLOGY].entry->SECTION_TYPE = (unsigned int)TOPOLOGY;
		mSections[TOPOLOGY].entry->DATA_ATTRIBUTE = topologicalIndexEncodingFlag;
		mSections[TOPOLOGY].entry->SECTION_OFFSET = 0;
		mSections[TOPOLOGY].entry->SECTION_LENGTH = topologicalIndexEncodingSize;
		mSections[TOPOLOGY].entry->ATTRIBUTE_COUNT = topologicalLineCount;
		mSections[TOPOLOGY].entry->ELEMENT_COUNT = 2;
		mSections[TOPOLOGY].entry->ELEMENT_TYPE = UINT_TYPE;
		mSections[TOPOLOGY].data = (void*)topologicalIndexEncodingBuffer;
	}

	//Sub meshes
	if (subMeshCount > 0)
	{
		mSections[SUBMESH].entry = new SECTION_TABLE_ENTRY();
		memset(mSections[SUBMESH].entry, 0, sizeof(SECTION_TABLE_ENTRY));
		mSections[SUBMESH].entry->SECTION_TYPE = (unsigned int)SUBMESH;
		mSections[SUBMESH].entry->DATA_ATTRIBUTE = subMeshEncodingFlag;
		mSections[SUBMESH].entry->SECTION_OFFSET = 0;
		mSections[SUBMESH].entry->SECTION_LENGTH = subMeshEncodingSize;
		mSections[SUBMESH].entry->ATTRIBUTE_COUNT = subMeshCount;
		mSections[SUBMESH].entry->ELEMENT_COUNT = 2;
		mSections[SUBMESH].entry->ELEMENT_TYPE = UINT_TYPE;
		mSections[SUBMESH].data = (void*)subMeshEncodingBuffer;
	}

	//NeonateVertexCompression_V4::computerTangent((float*)mSections[POSITION].data, (float*)mSections[UV0].data, (float*)mSections[NORMAL0].data, vertexCount, 
	//	(unsigned int*)mSections[INDEX].data, (unsigned int*)mSections[SUBMESH].data, (unsigned int)subMeshes.size());

	//Write Bin File
	//FBXSDK_printf("%s", "********** Beginning OF MESH **************\n");
	//FBXSDK_printf("Mesh Name:%s\n", objectName.c_str());

	writeObjectBinFile(&header, mSections, stringTable, stringTableSize);

	//FBXSDK_printf("%s", "********** END OF MESH **************\n\n\n");
}

void JsonToBin_V4::writeObjectBinFile(
	BIN_HEADER* header, 
	SECTION_DATA* sections,
	char* stringTable,
	unsigned int stringTableSize)
{	
	//FBXSDK_printf("Mesh Unique ID:%d\n", header->UNIQUEID);

	unsigned int fileStartOffset = ftell(mBinFile);

	fwrite(header, 1, sizeof(BIN_HEADER), mBinFile);

	if (stringTable != NULL)
	{
		fwrite(stringTable, 1, stringTableSize, mBinFile);
	}

	unsigned int sectionTableOffset = ftell(mBinFile);
	unsigned int sectionTableEntryCount = 0;

	fwrite(&sectionTableEntryCount, 1, sizeof(unsigned int), mBinFile);

	for (int i = 0; i < SECITON_TYPE_COUNT; ++i)
	{
		SECTION_DATA& sec = sections[i];
		if (sec.entry && sec.data)
		{
			fwrite(sec.entry, 1, sizeof(SECTION_TABLE_ENTRY), mBinFile);
			++sectionTableEntryCount;
		}
	}

	char sectionType[16];
	for (int i = 0; i < SECITON_TYPE_COUNT; ++i)
	{
		SECTION_DATA& sec = sections[i];
		if (sec.entry && sec.data)
		{
			sec.entry->SECTION_OFFSET = ftell(mBinFile) - fileStartOffset;

			memset(sectionType, 0, 16);
			memcpy(sectionType, SectionTypeName[i], strlen(SectionTypeName[i]));
			fwrite(sectionType, 1, 16, mBinFile);

			fwrite(sec.data, 1, sec.entry->SECTION_LENGTH, mBinFile);

			unsigned int originalSize = 0;
			if (sec.entry->SECTION_TYPE == VERTEXCOLOR0)
			{
				originalSize = sec.entry->ATTRIBUTE_COUNT*sec.entry->ELEMENT_COUNT;
			}
			else
			{
				originalSize = sec.entry->ATTRIBUTE_COUNT*sec.entry->ELEMENT_COUNT * 4;
			}

			float compressionRatio = (float)sec.entry->SECTION_LENGTH / (float)originalSize;
			//FBXSDK_printf("%s", "*************************************\n");			
			//FBXSDK_printf("Section:%s, Size: %d/%d (%3.1f%%)\n", SectionTypeName[i], sec.entry->SECTION_LENGTH, originalSize, compressionRatio*100.f);

		}
	}
	
	fwrite(FILE_END_FLAG, 1, strlen(FILE_END_FLAG), mBinFile);

	fseek(mBinFile, sectionTableOffset, SEEK_SET);
	fwrite(&sectionTableEntryCount, 1, sizeof(unsigned int), mBinFile);

	for (int i = 0; i < SECITON_TYPE_COUNT; ++i)
	{
		SECTION_DATA& sec = sections[i];
		if (sec.entry && sec.data)
		{
			fwrite(sec.entry, 1, sizeof(SECTION_TABLE_ENTRY), mBinFile);
			++sectionTableEntryCount;
		}
	}

	fseek(mBinFile, 0, SEEK_END);
	unsigned int fileEndOffset = ftell(mBinFile);
	unsigned int fileSize = fileEndOffset - fileStartOffset;
	fseek(mBinFile, fileStartOffset + 4, SEEK_SET);
	fwrite(&fileSize, 1, sizeof(unsigned int), mBinFile);

	fseek(mBinFile, 0, SEEK_END);
}

char* JsonToBin_V4::composeStringTable_V1(std::vector<string>& strings, unsigned int& stringTableSize)
{
	mStringTable_V1.clear();

	std::map<int, std::string> tableReverse;

	int id = 0;
	unsigned int stringCount = (unsigned int)strings.size();
	unsigned int bufferLength = 4;
	for (unsigned int stringIndex = 0; stringIndex < stringCount; ++stringIndex)
	{
		const string& str = strings[stringIndex];

		std::map<std::string, int>::iterator iter = mStringTable_V1.find(str);
		if (iter == mStringTable_V1.end())
		{
			mStringTable_V1[str] = id;
			tableReverse[id] = str;
			bufferLength += (sizeof(int) + (unsigned int)str.size() + 1);
			++id;
		}
	}

	bufferLength = roundUp(bufferLength, 4);
	char* sz = new char[bufferLength];
	memset(sz, 0, bufferLength);
	int index = 0;
	*((unsigned int*)&(sz[index])) = id; //the total string count.
	index += sizeof(unsigned int);
	for (int i = 0; i < id; ++i)
	{
		*((unsigned int*)&(sz[index])) = (unsigned int)tableReverse[i].size();
		index += sizeof(unsigned int);

		memcpy(&(sz[index]), tableReverse[i].c_str(), tableReverse[i].size());
		index += ((int)tableReverse[i].size() + 1);
	}

	stringTableSize = bufferLength;

	return sz;
}

//
void JsonToBin_V4::writeAnimationBin
(
	unsigned int clipID,
	const std::string& clipName,
	float fps,
	float startFrame,
	float endFrame,
	std::vector<animationClipData>* clipData
)
{
	//HEADER
	ANIMATION_BIN_HEADER header;
	memset(&header, 0, sizeof(header));

	memcpy(header.FILE_FLAG, FILE_BEGINNING_FLAG, strlen(FILE_BEGINNING_FLAG));
	//header.ANIMATION_FILE_SIZE
	header.ENDIAN_FLAG = ENDIANFLAG;
	header.VERSION = 3;

	unsigned int sectionTableOffset = sizeof(ANIMATION_BIN_HEADER);

	header.CLIP_UNIQUEID = clipID;

	memcpy(header.OBJECTTYPE, ANIMATION_TYPE, strlen(ANIMATION_TYPE));

	header.FPS = fps;
	header.START_FRAME = startFrame;
	header.END_FRAME = endFrame;

	/////////////////// String table //////////////////////////
	std::vector<string> strings;
	strings.push_back(clipName);
	for (int i = 0; i < (int)clipData->size(); ++i)
	{
		animationClipData& acd = (*clipData)[i];

		strings.push_back(acd.nodeName);

		if (acd.morphAnimation != NULL)
		{
			for (int j = 0; j < (int)acd.morphAnimation->size(); ++j)
			{
				morphTargetAnimation* mta = (*acd.morphAnimation)[j];
				strings.push_back(mta->channelName);
			}
		}

		if (acd.extAnimationData != NULL)
		{
			for (size_t s = 0; s < acd.extAnimationData->size(); ++s)
			{
				strings.push_back((*acd.extAnimationData)[s].propertyName);
			}
		}
	}

	unsigned int stringTableSize = 0;
	char* stringTable = composeStringTable_V1(strings, stringTableSize);
	header.STRING_TABLE_OFFSET = sizeof(ANIMATION_BIN_HEADER);
	header.SECTION_TABLE_OFFSET = sizeof(ANIMATION_BIN_HEADER) + stringTableSize;
	///////////////////////////////////////////////////////////
	header.OBJECT_NAME_INDEX = mStringTable_V1[clipName];

	std::vector<ANIMATION_SECTION_DATA> sections;
	for (int i = 0; i < (int)clipData->size(); ++i)
	{
		animationClipData& acd = (*clipData)[i];

		std::vector<ANIMATION_TARGET_DATA>* kfs = new std::vector<ANIMATION_TARGET_DATA>();

		unsigned int sectionTotalLength = 0;
		unsigned int curveCount = 0;
		if (acd.trsvAnimation != NULL)
		{
			for (int j = 0; j < ANIMATION_TYPE_COUNT; ++j)
			{
				std::vector<float>* keyframes = acd.trsvAnimation[j];
				if (keyframes != NULL && keyframes->size() > 0)
				{
					ANIMATION_TARGET_DATA atd;
					memset(&atd, 0, sizeof(ANIMATION_TARGET_DATA));
					atd.target = (unsigned char)j;
					atd.keyValueType = (unsigned char)KEY_VALUE_FLOAT;
					atd.keyIndexType = (unsigned char)KEY_INDEX_FLOAT;
					atd.keySize = sizeof(float);
					atd.keyCount = (unsigned int)keyframes->size() / 2;
					atd.animationDataSize = (unsigned int)keyframes->size()*sizeof(float);
					atd.propertyStringIndex = 0;
					atd.animationData = &((*keyframes)[0]);

					kfs->push_back(atd);

					sectionTotalLength += (sizeof(ANIMATION_TARGET_DATA) - sizeof(void*) + atd.animationDataSize);
					++curveCount;
				}
			}
		}
		else if (acd.morphAnimation != NULL)
		{
			for (int j = 0; j < (int)acd.morphAnimation->size(); ++j)
			{
				morphTargetAnimation* mta = (*acd.morphAnimation)[j];

				ANIMATION_TARGET_DATA atd;
				memset(&atd, 0, sizeof(ANIMATION_TARGET_DATA));

				atd.target = (unsigned char)ANIMATION_MORPHWEIGHT;
				atd.keyValueType = (unsigned char)KEY_VALUE_FLOAT;
				atd.keyIndexType = (unsigned char)KEY_INDEX_FLOAT;
				atd.keySize = sizeof(float);
				atd.keyCount = (unsigned int)(mta->animations.size()) / 2;
				atd.animationDataSize = (unsigned int)(mta->animations.size())*sizeof(float);
				atd.propertyStringIndex = mStringTable_V1[mta->channelName];
				atd.animationData = &(mta->animations[0]);

				kfs->push_back(atd);

				sectionTotalLength += (sizeof(ANIMATION_TARGET_DATA) - sizeof(void*) + atd.animationDataSize);
				++curveCount;
			}
		}

		//EXTENSION
		if (acd.extAnimationData != NULL)
		{
			for (size_t s = 0; s < acd.extAnimationData->size(); ++s)
			{
				const animationChannelData& extAnimation = (*acd.extAnimationData)[s];

				ANIMATION_TARGET_DATA atd;
				memset(&atd, 0, sizeof(ANIMATION_TARGET_DATA));

				atd.target = (unsigned char)extAnimation.animationTarget;
				atd.keyValueType = (unsigned char)extAnimation.keyframeDataType;
				atd.keyIndexType = (unsigned char)extAnimation.keyIndexType;
				atd.keySize = (unsigned char)extAnimation.keyframeDataSize;
				atd.keyCount = extAnimation.keyframeCount;
				atd.animationDataSize = extAnimation.animationDataSize;
				atd.propertyStringIndex = mStringTable_V1[extAnimation.propertyName];
				atd.animationData = extAnimation.animationData;

				kfs->push_back(atd);

				sectionTotalLength += (sizeof(ANIMATION_TARGET_DATA) - sizeof(void*) + atd.animationDataSize);
				++curveCount;
			}
		}

		ANIMATION_SECTION_DATA asd;
		memset(&asd, 0, sizeof(ANIMATION_SECTION_DATA));

		asd.objectNameIndex = mStringTable_V1[acd.nodeName];
		memset(&asd.objectName, 0xff, sizeof(asd.objectName));

		asd.objectID = (unsigned int)acd.nodeID;

		asd.entry = new SECTION_TABLE_ENTRY();
		memset(asd.entry, 0, sizeof(SECTION_TABLE_ENTRY));

		asd.entry->SECTION_TYPE = (unsigned int)KEYFRAME;
		asd.entry->DATA_ATTRIBUTE = 0;
		asd.entry->SECTION_OFFSET = 0;
		asd.entry->SECTION_LENGTH = sectionTotalLength + sizeof(asd.objectName) + sizeof(asd.objectID);
		asd.entry->ATTRIBUTE_COUNT = curveCount; //Animation Curve Count
		asd.entry->ELEMENT_COUNT = 0;
		asd.entry->ELEMENT_TYPE = STRUCT_TYPE;

		asd.data = kfs;

		sections.push_back(asd);
	}

	//Write binary
	writeAnimationBinFile(header, sections, stringTable, stringTableSize);

	//Clear resources
	for (size_t s = 0; s < sections.size(); ++s)
	{
		const ANIMATION_SECTION_DATA& asd = sections[s];

		delete asd.entry;
		delete asd.data;		
	}
}

void JsonToBin_V4::writeAnimationBinFile(
	const ANIMATION_BIN_HEADER& header,
	const std::vector<ANIMATION_SECTION_DATA>& sections,
	char* stringTable,
	unsigned int stringTableSize)
{
	fwrite(&header, 1, sizeof(ANIMATION_BIN_HEADER), mBinFile);	

	if (stringTable != NULL)
	{
		fwrite(stringTable, 1, stringTableSize, mBinFile);
	}

	unsigned int sectionTableEntryCount = (unsigned int)sections.size();
	fwrite(&sectionTableEntryCount, 1, sizeof(unsigned int), mBinFile);

	unsigned int sectionTableOffset = ftell(mBinFile);

	for (size_t s = 0; s < sections.size(); ++s)
	{
		const ANIMATION_SECTION_DATA& sec = sections[s];
		if (sec.entry == NULL || sec.data == NULL)
		{
			FBXSDK_printf("%s\n", "Error: Animation data error!");
			return;
		}

		fwrite(sec.entry, 1, sizeof(SECTION_TABLE_ENTRY), mBinFile);
	}

	//
	for (size_t s = 0; s < sections.size(); ++s)
	{
		const ANIMATION_SECTION_DATA& sec = sections[s];

		sec.entry->SECTION_OFFSET = ftell(mBinFile);
		fwrite(&sec.objectNameIndex, 1, sizeof(sec.objectNameIndex), mBinFile);
		fwrite(sec.objectName, 1, sizeof(sec.objectName), mBinFile);
		fwrite(&sec.objectID, 1, sizeof(sec.objectID), mBinFile);

		for (size_t s1 = 0; s1 < sec.data->size(); ++s1)
		{
			const ANIMATION_TARGET_DATA& atd = (*sec.data)[s1];

			fwrite(&atd.target, 1, sizeof(atd.target), mBinFile);
			fwrite(&atd.keyValueType, 1, sizeof(atd.keyValueType), mBinFile);
			fwrite(&atd.keyIndexType, 1, sizeof(atd.keyIndexType), mBinFile);
			fwrite(&atd.keySize, 1, sizeof(atd.keySize), mBinFile);
			fwrite(&atd.keyCount, 1, sizeof(atd.keyCount), mBinFile);
			fwrite(&atd.animationDataSize, 1, sizeof(atd.animationDataSize), mBinFile);
			fwrite(&atd.propertyStringIndex, 1, sizeof(atd.propertyStringIndex), mBinFile);
			fwrite(atd.animationData, 1, atd.animationDataSize, mBinFile);
		}
	}
	fwrite(FILE_END_FLAG, 1, strlen(FILE_END_FLAG), mBinFile);

	fseek(mBinFile, sectionTableOffset, SEEK_SET);

	for (size_t s = 0; s < sections.size(); ++s)
	{
		const ANIMATION_SECTION_DATA& sec = sections[s];
		fwrite(sec.entry, 1, sizeof(SECTION_TABLE_ENTRY), mBinFile);
	}

	fseek(mBinFile, 0, SEEK_END);
	unsigned int fileSize = ftell(mBinFile);
	fseek(mBinFile, 4, SEEK_SET);
	fwrite(&fileSize, 1, sizeof(unsigned int), mBinFile);

	fseek(mBinFile, 0, SEEK_END);
	//
}