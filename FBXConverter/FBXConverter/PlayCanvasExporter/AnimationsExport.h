#pragma once
#include "../stdafx.h"
#include "../Common/DebugHelper.h"
#include "../Common/Utils.h"

class BinData;

struct RawChannel
{
    int                nodeIndex;
    int                iNodeID;
    std::vector<FbxVector4> translations;
    std::vector<FbxQuaternion> rotations;
    std::vector<FbxVector4> scales;
    std::vector<float> weights;
};

struct gltfChannelInfo
{
    int            iNodeID;
    int            iAccessorIndex;
    string szChannelName;
};

struct RawAnimation
{
    std::string             name;
    std::vector<float>      times;
    unsigned               iTimeAccessorIndex;
    std::vector<RawChannel> channels;
    vector<gltfChannelInfo> outChannels;
};

class AnimationExport
{
public:
  AnimationExport ();
  ~AnimationExport ();
  static void getSceneAnimation(FbxScene *pScene,vector<RawAnimation>& vAnimation);
  static void writeAnimationBin(vector<RawAnimation>& vAnimation,const string szAnimationFile,BinData* mBinData);
private:
  /* data */
};
