#pragma once
#include <vector>
#include <map>
#include <string>
#include "fbxsdk.h"

extern const char* SectionTypeName[];
extern const char* UNKNOWN_TYPE;
extern const char* MESH_TYPE;
extern const char* MORPHTARGET_TYPE;
extern const char* ANIMATION_TYPE;
extern const char* FILE_BEGINNING_FLAG;
extern const char* FILE_END_FLAG;
extern const unsigned int ENDIANFLAG;
extern const unsigned int TARGETFRAMEKEYFLAG;
extern const double EPSILON_THRESHOLD;
extern const char* VERTEX_ELEMENT_NAME[];

enum SECITON_TYPE
{
	POSITION = 0,
	NORMAL0 = 1,
	NORMAL1 = 2,
	TANGENT0 = 3,
	TANGENT1 = 4,
	BINORMAL0 = 5,
	BINORMAL1 = 6,
	UV0 = 7,
	UV1 = 8,
	VERTEXCOLOR0 = 9,
	VERTEXCOLOR1 = 10,
	BLENDWEIGHT = 11,
	BLENDINDEX = 12,	
	INDEX = 13,
	SUBMESH = 14,
	BONE = 15,
	MORPHTARGET = 16,
	KEYFRAME = 17,
	TOPOLOGY = 18,
	SECITON_TYPE_COUNT = 19,
	SECITON_TYPE_UNKNOWN = 255
};

enum DATA_ELEMENT_TYPE
{
	FLOAT_TYPE = 0,
	UINT_TYPE = 1,	
	STRUCT_TYPE = 2,
	UBYTE_TYPE = 3
};

enum ANIMATION_TARGET
{
	ANIMATION_POSITION_X = 0,
	ANIMATION_POSITION_Y = 1,
	ANIMATION_POSITION_Z = 2,
	ANIMATION_ROTATION_EX = 3,
	ANIMATION_ROTATION_EY = 4,
	ANIMATION_ROTATION_EZ = 5,
	ANIMATION_SCALING_X = 6,
	ANIMATION_SCALING_Y = 7,
	ANIMATION_SCALING_Z = 8,
	ANIMATION_MORPHWEIGHT = 9,
	ANIMATION_VISIBILITY = 10,
	ANIMATION_ROTATION_QUATERNION = 11,
	ANIMATION_CUSTOMIZED_PROPERTY = 12,
	ANIMATION_TYPE_COUNT = 13
};

enum KEY_INDEX_TYPE
{
	KEY_INDEX_FLOAT = 0,
	KEY_INDEX_INT = 1,
	KEY_INDEX_UINT = 2,
	KEY_INDEX_SHORT = 3,
	KEY_INDEX_USHORT = 4,
	KEY_INDEX_BYTE = 5,
	KEY_INDEX_UBYTE = 6,
	KEY_INDEX_VARIABLE = 7
};

enum KEYFRAME_DATA_TYPE
{
	KEY_VALUE_FLOAT = 1,
	KEY_VALUE_FLOAT_BEZIER_MMD = 2,
	KEY_VALUE_QUATERNION_LINEAR = 9,
	KEY_VALUE_QUATERNION_BEZIER_MMD = 10,
	KEY_VALUE_INT = 20,
	KEY_VALUE_UINT = 30,
	KEY_VALUE_VARIABLE = 40
};

enum VERTEX_LAYER_TYPE
{
	VL_POSITION = 0,
	VL_NORMAL0 = 1,
	VL_NORMAL1 = 2,
	VL_TANGENT0 = 3,
	VL_TANGENT1 = 4,
	VL_BINORMAL0 = 5,
	VL_BINORMAL1 = 6,
	VL_UV0 = 7,
	VL_UV1 = 8,
	VL_VERTEXCOLOR0 = 9,
	VL_VERTEXCOLOR1 = 10,
	VL_BLENDWEIGHT = 11,
	VL_BLENDINDEX = 12,
	VL_INDEX = 13,
	VL_TOPOLOGICALINDEX = 14
};

/*
GEOMETRY_BIN_HEADER

OBJECT NAME

SECTION_TABLE_ENTRY_COUNT

SECTION_TABLE_ENTRY ...

SECTION_TYPE
SECTION_DATA ...

FILE_END_FLAG
*/
//Be careful! NOT COMPILE THIS PROGRAM WITH X64 CONFIGURATION!
struct BIN_HEADER
{
	char				FILE_FLAG[4];
	unsigned int		OBJECT_FILE_SIZE;
	unsigned int		ENDIAN_FLAG;
	unsigned int		VERSION;
	unsigned int		SECTION_TABLE_OFFSET;
	int					PARENTUNIQUEID;
	int					UNIQUEID;			//Offset 24bytes	
	char				OBJECTTYPE[12];		//CAMERA  MESH	LIGHT
	unsigned int		OBJECT_NAME_INDEX;
	float				POS_BBOX_MIN[3];
	float				POS_BBOX_MAX[3];
	float				UV0_BBOX_MIN[2];
	float				UV0_BBOX_MAX[2];
	unsigned int		STRING_TABLE_OFFSET;
	float				UV1_BBOX_MIN[2];
	float				UV1_BBOX_MAX[2];
	char				MD5[12];
};

struct SECTION_TABLE_ENTRY
{
	unsigned int		SECTION_TYPE;		//POSITION NORMAL0 NORMAL1 UV0 UV1 VERTEXCOLOR SUBMESH..
	unsigned int		DATA_ATTRIBUTE;		//
	unsigned int		SECTION_OFFSET;
	unsigned int		SECTION_LENGTH;
	unsigned int		ATTRIBUTE_COUNT;
	unsigned int		ELEMENT_COUNT;
	unsigned int		ELEMENT_TYPE;
};

struct SECTION_DATA
{
	SECTION_TABLE_ENTRY*	entry;
	void*					data;
};

struct ANIMATION_BIN_HEADER
{
	char				FILE_FLAG[4];
	unsigned int		ANIMATION_FILE_SIZE;
	unsigned int		ENDIAN_FLAG;
	unsigned int		VERSION;
	unsigned int		SECTION_TABLE_OFFSET;
	int					CLIP_UNIQUEID;
	char				OBJECTTYPE[12];		//CAMERA  MESH	LIGHT
	unsigned int		OBJECT_NAME_INDEX;
	float				FPS;
	float				START_FRAME;
	float				END_FRAME;
	char				UNUSED[32];
	unsigned int		STRING_TABLE_OFFSET;
	char				MD5[12];
};

struct ANIMATION_TARGET_DATA
{
	unsigned char		target;
	unsigned char		keyValueType;
	unsigned char		keyIndexType; //For compression use
	unsigned char		keySize;
	unsigned int		keyCount;
	unsigned int		animationDataSize; //For compression use size = key frame index size + key frame data size
	unsigned int		propertyStringIndex;
	void*				animationData;
};

struct ANIMATION_SECTION_DATA
{
	SECTION_TABLE_ENTRY*	entry;
	unsigned int			objectNameIndex;
	char					objectName[12];
	unsigned int			objectID;
	std::vector<ANIMATION_TARGET_DATA>* data;
};

struct SUBMESH_
{
	SUBMESH_()
		: IndexOffset(0), TriangleCount(0) {}

	int IndexOffset;
	int TriangleCount;
};

struct SUBMESH_V1
{
	SUBMESH_V1() {}

	std::vector<int> polygonIDs;
};

struct NORMAL_
{
	NORMAL_(double x_, double y_, double z_)
		: x(x_), y(y_), z(z_) {}

	bool operator==(const NORMAL_& p)
	{
		double dx = abs(x - p.x);
		double dy = abs(y - p.y);
		double dz = abs(z - p.z);

		if (dx < EPSILON_THRESHOLD && dy < EPSILON_THRESHOLD && dz < EPSILON_THRESHOLD)
			return true;
		else
			return false;
	}
	double x, y, z;
};

struct TANGENT_
{
	TANGENT_(double x_, double y_, double z_, double w_)
		: x(x_), y(y_), z(z_), w(w_) {}

	bool operator==(const TANGENT_& p)
	{
		double dx = abs(x - p.x);
		double dy = abs(y - p.y);
		double dz = abs(z - p.z);
		double dw = abs(w - p.w);

		if (dx < EPSILON_THRESHOLD && dy < EPSILON_THRESHOLD && dz < EPSILON_THRESHOLD && dw < EPSILON_THRESHOLD)
			return true;
		else
			return false;
	}
	double x, y, z, w;
};

struct COLOR_
{
	COLOR_(double r_, double g_, double b_, double a_)
		: r(r_), g(g_), b(b_), a(a_) {}
	double r, g, b, a;
};

struct BW_
{
	BW_(double w0, double w1, double w2, double w3)
	{
		data[0] = w0; data[1] = w1; data[2] = w2; data[3] = w3;
	}
	double data[4];
};

struct BI_
{
	BI_(double i0, double i1, double i2, double i3)
	{
		data[0] = i0; data[1] = i1; data[2] = i2; data[3] = i3;
	}
	double data[4];
};

struct POSITION_
{
	POSITION_(double x_, double y_, double z_)
		: x(x_), y(y_), z(z_) {}

	bool operator==(const POSITION_& p)
	{
		double dx = abs(x - p.x);
		double dy = abs(y - p.y);
		double dz = abs(z - p.z);

		if (dx < EPSILON_THRESHOLD && dy < EPSILON_THRESHOLD && dz < EPSILON_THRESHOLD)
			return true;
		else
			return false;
	}

	double x, y, z;
};

struct UV_
{
	UV_(double x_, double y_)
		: x(x_), y(y_) {}

	bool operator==(const UV_& p)
	{
		double dx = abs(x - p.x);
		double dy = abs(y - p.y);

		if (dx < EPSILON_THRESHOLD && dy < EPSILON_THRESHOLD)
			return true;
		else
			return false;
	}

	double x, y;
};

struct TRIANGLE_
{
	TRIANGLE_(int i0, int i1, int i2)
	{
		index[0] = i0; index[1] = i1; index[2] = i2;
	}
	int index[3];
};

struct BONE_
{
	unsigned int id;
	unsigned int parentID;
	std::string name;
	std::string name1;
	FBXSDK_NAMESPACE::FbxAMatrix modelWorldToBoneLocal;
};

#define MAX_INFLUENCE_BONE_COUNT 4
struct SKINNING_DATA_
{
	int count;
	double indices[MAX_INFLUENCE_BONE_COUNT];
	double weights[MAX_INFLUENCE_BONE_COUNT];
};

struct MORPH_DATA_V2
{
	std::string morphTargetName;
	unsigned int uniqueID;
	int vertexCount;
	std::vector<float> positions;
	std::vector<float> normals;
	std::vector<float> tangents;
	FBXSDK_NAMESPACE::FbxDouble3 bboxMin;
	FBXSDK_NAMESPACE::FbxDouble3 bboxMax;	
};

struct REF_RECORD
{
	REF_RECORD() :INDEX(-1), WHERE(-1), NEXT(NULL)
	{

	}
	int INDEX;
	int WHERE;
	REF_RECORD* NEXT;
};

enum _FILE_TYPE
{
	FT_PNG,
	FT_JPG,
	FT_GIF,
	FT_BMP,
	FT_TGA,
	FT_TIFF,
	FT_DDS,
	FT_PVR,
	FT_UNKNOWN
};

struct morphTargetAnimation
{
	unsigned int channelIndex;
	std::string channelName;
	std::vector<float> animations;
};

struct animationChannelData
{
	std::string propertyName;
	ANIMATION_TARGET animationTarget;
	KEY_INDEX_TYPE keyIndexType;
	KEYFRAME_DATA_TYPE keyframeDataType;
	unsigned int keyframeDataSize;
	unsigned int keyframeCount;
	void* animationData;
	unsigned int animationDataSize;
};

struct animationClipData
{
	std::string nodeName;
	unsigned int nodeID;
	std::vector<float>** trsvAnimation;
	std::vector<morphTargetAnimation*>* morphAnimation;
	//Extension
	std::vector<animationChannelData>* extAnimationData;
};

struct KeyframeAnimation
{
	unsigned int		clipID;
	std::string			clipName;
	unsigned int		keyframeCount;
	float				fps;
	float				startFrame;
	float				endFrame;
	std::vector<animationClipData>* nodes;
};

class MeshInfo
{
public:
	MeshInfo()
	{
		uniqueID = -1;
		mesh = NULL;
		bboxMin[0] = bboxMin[1] = bboxMin[2] = FLT_MAX;
		bboxMax[0] = bboxMax[1] = bboxMax[2] = FLT_MIN;
		isSkinned = false;
		triangleCount = 0;
		polygonCount = 0;
		vertexCount = 0;
	}

	int uniqueID;
	FBXSDK_NAMESPACE::FbxMesh* mesh;
	std::string meshName;
	FBXSDK_NAMESPACE::FbxDouble3 bboxMin;
	FBXSDK_NAMESPACE::FbxDouble3 bboxMax;
	bool isSkinned;
	std::vector<int> materials;
	int triangleCount;
	int polygonCount;
	int vertexCount;
};

class Node
{
public:
	unsigned int uniuqeID;	
	std::string guid;
	std::string name;
	std::string skeletonName;

	FBXSDK_NAMESPACE::FbxVector4 translation;
	FBXSDK_NAMESPACE::FbxVector4 rotation;
	FBXSDK_NAMESPACE::FbxVector4 scaling;
	std::vector<int> materials;

    // MB
	FBXSDK_NAMESPACE::FbxVector4 scalingPivot;
	FBXSDK_NAMESPACE::FbxVector4 scalingOffset;
	FBXSDK_NAMESPACE::FbxVector4 rotationPivot;
	FBXSDK_NAMESPACE::FbxVector4 rotationOffset;
	FBXSDK_NAMESPACE::FbxVector4 preRotation;
	FBXSDK_NAMESPACE::FbxVector4 postRotation;
	std::string rotationOrder;
	std::string inheritType;
	FBXSDK_NAMESPACE::FbxDouble visibility;
	FBXSDK_NAMESPACE::FbxBool visibilityInheritance;

    // MAX
    FBXSDK_NAMESPACE::FbxVector4 scaling_axis;
    FBXSDK_NAMESPACE::FbxVector4 offset_rotation;
    FBXSDK_NAMESPACE::FbxVector4 offset_translation;
    FBXSDK_NAMESPACE::FbxVector4 offset_scaling;
    FBXSDK_NAMESPACE::FbxVector4 offset_scaling_axis;
    bool is_max = false;

	std::vector<int> children;
	int parent;

	MeshInfo* meshInfo;

	int _index_;
};

struct MESH_DETAIL
{
	MESH_DETAIL()
		: meshID(-1)
		, triangleCount(0)
		, polygonCount(0)
		, vertexCount(0)
		, blendshapeCount(0)
	{

	}

	int meshID;
	std::string meshName;
	int triangleCount;
	int polygonCount;
	int vertexCount;	
	int blendshapeCount;

	std::map<int, std::string> bones;
	std::map<int, int> boneHierarchy;
	std::vector<VERTEX_LAYER_TYPE> vertexFormats;
};

struct ANIMATION_DETAIL
{
	ANIMATION_DETAIL(int _keyframeCount, float _fps, float _animationDuration)
		: keyframeCount(_keyframeCount)
		, fps(_fps)
		, animationDuration(_animationDuration)
	{

	}
	int keyframeCount;
	float fps;
	float animationDuration;
};