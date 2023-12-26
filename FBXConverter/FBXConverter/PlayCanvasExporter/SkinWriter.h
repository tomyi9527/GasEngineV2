#pragma once
#include "../stdafx.h"
#include "../rapidjson/writer.h"
#include "../rapidjson/filewritestream.h"
#include "glTFBin.h"

class BinData;

struct stSkinInfo {
    int iInverseBindMatricesIndex = -1;
    vector<FbxAMatrix> vBindMat;
    string name;
    vector<int> vBoneNodeID;
};

class SkinWriter
{
public:
  SkinWriter ();
  ~SkinWriter ();
  static void write(
          rapidjson::Writer<rapidjson::FileWriteStream>* mJSONWriter,
          vector<stSkinInfo>& vSkins,
          map<int,int>& mNodeIDtoIndexMap,
          string outputDirectory,
          BinData* mBinData);
private:
  /* data */
};
