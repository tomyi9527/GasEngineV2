#include "AnimationsWriter.h" 

AnimationsWriter::AnimationsWriter()
{

}
AnimationsWriter::~AnimationsWriter()
{

}

void AnimationsWriter::write(
        rapidjson::Writer<rapidjson::FileWriteStream>* mJSONWriter,
        vector<RawAnimation>& vAnimation,
        std::map<int,int>& mNodeIDtoIndexMap) {

    if (vAnimation.size() == 0) {
        return;
    }

    mJSONWriter->Key("animations");
    mJSONWriter->StartArray();
        for (auto vt = vAnimation.cbegin(); vt != vAnimation.cend(); ++vt) {
            RawAnimation animData = *vt;
            //dzlog_info("animation %s channel %d",animData.name.c_str(),animData.outChannels.size());
            mJSONWriter->StartObject();
                mJSONWriter->Key("name");
                mJSONWriter->String(animData.name.c_str());
                mJSONWriter->Key("samplers");
                    mJSONWriter->StartArray();
                        for (auto it = animData.outChannels.cbegin(); it != animData.outChannels.cend(); ++it) {
                            mJSONWriter->StartObject();
                            mJSONWriter->Key("input");
                            mJSONWriter->Uint(animData.iTimeAccessorIndex);
                            mJSONWriter->Key("interpolation");
                            mJSONWriter->String("LINEAR");
                            mJSONWriter->Key("output");
                            mJSONWriter->Uint(it->iAccessorIndex);
                            mJSONWriter->EndObject();
                        }
                    mJSONWriter->EndArray();
                mJSONWriter->Key("channels");
                    mJSONWriter->StartArray();
                        for (auto it = animData.outChannels.cbegin(); it != animData.outChannels.cend(); ++it) {
                            unsigned nodeIndex = mNodeIDtoIndexMap[it->iNodeID];
                            mJSONWriter->StartObject();
                            mJSONWriter->Key("sampler");
                            mJSONWriter->Uint(it - animData.outChannels.cbegin());
                            mJSONWriter->Key("target");
                                mJSONWriter->StartObject();
                                    mJSONWriter->Key("node");
                                    mJSONWriter->Uint(nodeIndex);
                                    mJSONWriter->Key("path");
                                    mJSONWriter->String(it->szChannelName.c_str());
                                mJSONWriter->EndObject();
                            mJSONWriter->EndObject();
                        }
                    mJSONWriter->EndArray();
            mJSONWriter->EndObject();
        }
    mJSONWriter->EndArray();
}
