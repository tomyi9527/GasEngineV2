#include "./SkinWriter.h"
#include "BinData.h"

SkinWriter::SkinWriter()
{

}

SkinWriter::~SkinWriter()
{

}

void SkinWriter::write( 
          rapidjson::Writer<rapidjson::FileWriteStream>* mJSONWriter,
          vector<stSkinInfo>& vSkins,
          std::map<int,int>& mNodeIDtoIndexMap,
          string szBinFile,
          BinData* mBinData)
{
    //for (auto it = mNodeIDtoIndexMap.cbegin(); it != mNodeIDtoIndexMap.cend(); ++it) {
        //dzlog_info("nodeid to index map %d - %d",it->first,it->second);
    //}
    if (vSkins.size() > 0) {
        //write inversedMat
        glTFBin* writer2 = new glTFBin();
        writer2->openExportFile(szBinFile);
        for (auto it = vSkins.begin(); it != vSkins.end(); ++it) {
            Accessor_data d1;
            writer2->writeSkinMatricesBin(it->vBindMat,d1);
            mBinData->add(d1);
            it->iInverseBindMatricesIndex = mBinData->getSize() - 1;
        }
        writer2->closeExportFile();

        //write json
        mJSONWriter->Key("skins");
        mJSONWriter->StartArray();
        for (auto it = vSkins.cbegin(); it != vSkins.cend(); ++it) {
            mJSONWriter->StartObject();
            mJSONWriter->Key("inverseBindMatrices");
            mJSONWriter->Uint(it->iInverseBindMatricesIndex);
            mJSONWriter->Key("joints");
                mJSONWriter->StartArray();
                for (auto boneNodeID = it->vBoneNodeID.cbegin(); boneNodeID != it->vBoneNodeID.cend(); ++boneNodeID) {
                    int nodeIndex = mNodeIDtoIndexMap[*boneNodeID];
                    mJSONWriter->Uint(nodeIndex);
                }
                mJSONWriter->EndArray();
            int firstBoneNodeID = it->vBoneNodeID[0];
            mJSONWriter->Key("skeleton");
            mJSONWriter->Uint(mNodeIDtoIndexMap[firstBoneNodeID]);
            mJSONWriter->EndObject();
        }
        mJSONWriter->EndArray();
    }
}
