#pragma once

#include "../stdafx.h"
#include "../rapidjson/writer.h"
#include "../rapidjson/filewritestream.h"
#include "glSpec.h"

enum PRIMITIVE_ATTR_TYPE
{
   DEFAULT_TYPE          = 0,
   MESH_NORMAL           = 1,
   MESH_POSITION         = 2,
   MESH_TANGENT          = 3,
   MESH_TEXCOORD_0       = 4,
   MESH_INDICES          = 5,
   MORPH_TARGET_NORMAL   = 6,
   MORPH_TARGET_POSITION = 7,
   MORPH_TARGET_TANGENT  = 8,
};

struct stMorphTargetAttr {
    int iNormalIndex = -1;
    int iPostionIndex = -1;
    int iTangentIndex = -1;
    float fWeight = 0.0;
};

struct stNodeInfo {
    int iNodeID = 0;
    int iMeshIndex = -1;
    int iSkinIndex = -1;
	int parentIndex = -1;
    string szName;
    double rotation[4];
    double scale[3];
    double translation[3];	
    vector<int> children;
};

struct Accessor_data {
    unsigned int componentType = 0;
    unsigned int count = 0;
    unsigned int len = 0;
    unsigned int iIsolateIndex = 0;
    unsigned int iBufIndex = 0;
    unsigned int iByteOffset = 0;
    unsigned int iMorphUniqueID = 0;
    unsigned int iAttrType = 0;
    string szTypeName;
    string name;
    string type;
    double PosBboxMin[3];
    double PosBboxMax[3];
    float vMax;
    float vMin;
};

struct stSubMeshInfo {
    unsigned iStartTriangleIndiceIndex = 0;
    unsigned iTriangleIndiceCount = 0;
    unsigned iMaterialIndex = 0;
};

class BinData
{
    public:
		BinData(const std::string filename, const std::string szFilename);
        virtual ~BinData ();

        void add(const Accessor_data& Accessor_data);
        unsigned getSize();
        void writeJSON(rapidjson::Writer<rapidjson::FileWriteStream>* writer);
        bool bHasAnimationBin;
    private:
        string mBinFile;
        string mAnimationBinFile;
        vector<Accessor_data> mData;
};



