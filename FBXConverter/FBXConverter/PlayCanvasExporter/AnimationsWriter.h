#pragma once
#include "../stdafx.h"
#include "../rapidjson/writer.h"
#include "../rapidjson/filewritestream.h"
#include "./AnimationsExport.h"

struct glTFAnimationChannelData {
    unsigned int iTargetNode = 0;
    unsigned int iFrameCount = 0;
    unsigned int iValueCount = 0;
    unsigned int iAccessorIndex = 0;
    unsigned int iBinFrameSize = 0;
    unsigned int iBinValueSize = 0;
    string szPathName;
    string szDataType;
    float fVMax;
    float fVMin;
};

class AnimationsWriter
{
public:
  AnimationsWriter ();
  ~AnimationsWriter ();
  static void write(
          rapidjson::Writer<rapidjson::FileWriteStream>* mJSONWriter,
          vector<RawAnimation>& vAnimationChannelArray,
          std::map<int,int>& mNodeIDtoIndexMap);
private:
  /* data */
};
