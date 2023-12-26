
#include "BinData.h"

BinData::BinData(const std::string filename, const std::string szFilename)
{
  bHasAnimationBin = false;
  mBinFile = filename;
  mAnimationBinFile = szFilename;
  mData.clear();
}

BinData::~BinData()
{

}

void BinData::add(const Accessor_data& Accessor_data)
{
  mData.push_back(Accessor_data);
}

unsigned BinData::getSize()
{
  return mData.size();
}

void BinData::writeJSON(rapidjson::Writer<rapidjson::FileWriteStream>* writer)
{
  writer->Key("accessors");
  writer->StartArray();
  for (auto it = mData.cbegin(); it != mData.cend(); ++it) {
    size_t ind = it - mData.cbegin();
    writer->StartObject();
      writer->Key("bufferView");
      writer->Uint(ind);
      writer->Key("byteOffset");
      writer->Uint(0);
      writer->Key("componentType");
      writer->Uint(it->componentType);
      writer->Key("count");
      writer->Uint(it->count);
      writer->Key("name");
      writer->String(it->name.c_str());
      writer->Key("type");
      writer->String(it->type.c_str());
      if (it->szTypeName == "POSITION") {
          writer->Key("max");
          writer->StartArray();
          writer->Double(it->PosBboxMax[0]);
          writer->Double(it->PosBboxMax[1]);
          writer->Double(it->PosBboxMax[2]);
          writer->EndArray();
          writer->Key("min");
          writer->StartArray();
          writer->Double(it->PosBboxMin[0]);
          writer->Double(it->PosBboxMin[1]);
          writer->Double(it->PosBboxMin[2]);
          writer->EndArray();
      }
      if (it->szTypeName == "ANIMATION_FRAME") {
          writer->Key("max");
          writer->StartArray();
          writer->Double(it->vMax);
          writer->EndArray();
          writer->Key("min");
          writer->StartArray();
          writer->Double(it->vMin);
          writer->EndArray();
      }
    writer->EndObject();
  }
  writer->EndArray();

  map<unsigned,unsigned> bufferSizeMap;
  writer->Key("bufferViews");
  writer->StartArray();
  for (auto it = mData.cbegin(); it != mData.cend(); ++it) {
    unsigned iBufIndex = it->iBufIndex;
    if (!bufferSizeMap[iBufIndex]) {
        bufferSizeMap[iBufIndex] = 0;
    }
    writer->StartObject();
      writer->Key("buffer");
      writer->Uint(iBufIndex);
      writer->Key("byteLength");
      writer->Uint(it->len);
      writer->Key("byteOffset");
      writer->Uint(bufferSizeMap[iBufIndex]);
    writer->EndObject();
    bufferSizeMap[iBufIndex] += it->len;
  }
  writer->EndArray();

  writer->Key("buffers");
  writer->StartArray();
  writer->StartObject();
    writer->Key("name");
    writer->String(mBinFile.c_str());
    writer->Key("byteLength");
    writer->Uint(bufferSizeMap[0]);
    writer->Key("uri");
    writer->String(mBinFile.c_str());
  writer->EndObject();
  if (bHasAnimationBin) {
      writer->StartObject();
        writer->Key("name");
        writer->String(mAnimationBinFile.c_str());
        writer->Key("byteLength");
        writer->Uint(bufferSizeMap[1]);
        writer->Key("uri");
        writer->String(mAnimationBinFile.c_str());
      writer->EndObject();
  }
  writer->EndArray();
}



