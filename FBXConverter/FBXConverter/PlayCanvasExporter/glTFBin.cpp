#include "glTFBin.h"


glTFBin::glTFBin()
: mBinFile(NULL)
, mAnimationBinFile(NULL)
{
	mData.clear();
}


glTFBin::~glTFBin()
{
}

void glTFBin::openExportFile(const string& filePath)
{
    //must delete old file first!!!!
	mBinFile = fopen(filePath.c_str(), "ab");
    mBinFileName = filePath;
}

void glTFBin::closeExportFile()
{
	if (mBinFile != NULL)
	{
		fclose(mBinFile);
		mBinFile = NULL;
	}
}

void glTFBin::openAnimationFile(const string& filePath)
{
	//must delete old file first!!!!
	mAnimationBinFile = fopen(filePath.c_str(), "ab");
    mAnimationBinFileName = filePath;
}

void glTFBin::closeAnimationFile()
{
	if (mAnimationBinFile != NULL)
	{
		fclose(mAnimationBinFile);
		mAnimationBinFile = NULL;
	}
}

// write 16 float for one mat and merge all vBindMat for skin
void glTFBin::writeSkinMatricesBin(const vector<FbxAMatrix>& vBindMat,Accessor_data & accessor)
{
    int iFloatCount = vBindMat.size() * 16;
    float* pBuf = new float[iFloatCount];
    int iBufSize = iFloatCount * sizeof(float);
    for (auto it = vBindMat.cbegin(); it != vBindMat.cend(); ++it) {
        int iMatIndex = it - vBindMat.cbegin();
        for (int i = 0; i < 4; ++i) 
        {
            string str = "";
            for (int j = 0; j < 4; ++j) 
            {
                float v = (*it).Get(i,j);
                str += floatToString(v) + "  ";
                pBuf[iMatIndex * 16 + i * 4 + j] = v;
            }
        }
    }
    fwrite(pBuf,1,iBufSize,mBinFile);
    delete[] pBuf;

    accessor.type = "MAT4";
    accessor.szTypeName = "SkinMatrices";
    accessor.componentType = COMPONENT_TYPE_FLOAT;
    accessor.count = vBindMat.size();
    accessor.name = "skin_link_mat";
    accessor.len = iBufSize;
}

Accessor_data writeVertexAttribute(string szType,string szTypeName,int iVertextCount,string szName,int iBufSize, uint8_t *pBuf,FILE* mBinFile)
{
    fwrite(pBuf,1,iBufSize,mBinFile);
    Accessor_data d1;
    d1.type = szType;
    d1.szTypeName = szTypeName;
    d1.componentType = COMPONENT_TYPE_FLOAT;
    d1.count = iVertextCount;
    d1.name = szName;
    d1.len = iBufSize;
    //dzlog_debug("get %s LAYEY size %d",szTypeName.c_str(),iBufSize);
    return d1;
}

void checkMorphData(Accessor_data &data,const char * objectType,int nodeID,int iAttrType)
{
    if (string("MORPHTARGET").compare(objectType) == 0) {
        data.iMorphUniqueID = nodeID;
        data.iAttrType = iAttrType;
    }
}

void glTFBin::writeObjectBin(
    vector<Accessor_data>& vBinAccessorData,
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

//    //dzlog_info("parentNodeID %lld nodeID %lld object type: %s objectName: %s subMeshCount : %d", \
//	//parentNodeID,nodeID, objectType,objectName.c_str(),subMeshCount);
//
//    string meshName = "mesh_" + std::to_string(nodeID);
//
//	unsigned int elementCount = (unsigned int)vertexElement.size();
//	for (unsigned int i = 0; i < elementCount; ++i)
//    {
//		VERTEX_LAYER_TYPE type = vertexElement[i];
//        switch (type) {
//            case VL_POSITION:
//                {
//                    string szName =  meshName + "_positions";
//                    Accessor_data d1 = writeVertexAttribute("VEC3",
//                                         "POSITION",
//                                         vertexCount,
//                                         szName,
//                                         posistionEncodingSize,
//                                         posistionEncodingBuffer,
//                                         mBinFile);
//                    checkMorphData(d1,objectType,nodeID,MORPH_TARGET_POSITION);
//                    //showFloatBufferString(posistionEncodingBuffer,posistionEncodingSize / 4);
//                    d1.iIsolateIndex = vBinAccessorData.size();
//                    d1.PosBboxMax[0] = PosBboxMax[0];
//                    d1.PosBboxMax[1] = PosBboxMax[1];
//                    d1.PosBboxMax[2] = PosBboxMax[2];
//                    d1.PosBboxMin[0] = PosBboxMin[0];
//                    d1.PosBboxMin[1] = PosBboxMin[1];
//                    d1.PosBboxMin[2] = PosBboxMin[2];
//                    vBinAccessorData.push_back(d1);
//                }
//                break;
//            case VL_NORMAL0:
//                {
//                    string szName =  meshName + "_normal";
//                    Accessor_data d1 = writeVertexAttribute("VEC3",
//                                         "NORMAL",
//                                         vertexCount,
//                                         szName,
//                                         normalEncodingSize,
//                                         normalEncodingBuffer,
//                                         mBinFile);
//                    checkMorphData(d1,objectType,nodeID,MORPH_TARGET_NORMAL);
//                    d1.iIsolateIndex = vBinAccessorData.size();
//                    vBinAccessorData.push_back(d1);
//                }
//                break;
//            case VL_UV0:
//                {
//                    string szName =  meshName + "_UV0";
//                    Accessor_data d1 = writeVertexAttribute("VEC2",  
//                                         "TEXCOORD_0",
//                                         vertexCount,
//                                         szName,
//                                         uvEncodingSize,
//                                         uvEncodingBuffer,
//                                         mBinFile);
//                    d1.iIsolateIndex = vBinAccessorData.size();
//                    vBinAccessorData.push_back(d1);
//                }
//                break;
//            case VL_TANGENT0:
//                {
//                    string szName =  meshName + "_tangent";
//                    Accessor_data d1 = writeVertexAttribute("VEC4",  // gltf spec need VEC4 , tom output buffer VEC2 float to vertexCount
//                                         "TANGENT",
//                                         vertexCount,
//                                         szName,
//                                         tangentEncodingSize,
//                                         tangentEncodingBuffer,
//                                         mBinFile);
//                    checkMorphData(d1,objectType,nodeID,MORPH_TARGET_TANGENT);
//                    d1.iIsolateIndex = vBinAccessorData.size();
//                    vBinAccessorData.push_back(d1);
//                }
//                break;
//            case VL_VERTEXCOLOR0:
//                {
//                    string szName =  meshName + "_vertexcolor";
//                    Accessor_data d1 = writeVertexAttribute("VEC3",
//                                         "COLOR_0",
//                                         vertexCount,
//                                         szName,
//                                         vcEncodingSize,
//                                         vcEncodingBuffer,
//                                         mBinFile);
//                    d1.iIsolateIndex = vBinAccessorData.size();
//                    vBinAccessorData.push_back(d1);
//                }
//                break;
//            default:
//                //dzlog_warn("get UNKNOWN_TYPE VERTEX LAYEY!!!");
//				break;
//        }
//    }
//
//    if (biEncodingSize > 0 && bwEncodingSize > 0) 
//	{
//        string szName =  meshName + "_joints";
//        Accessor_data d1 = writeVertexAttribute("VEC4",
//                             "JOINTS_0",
//                             vertexCount,
//                             szName,
//                             biEncodingSize,
//                             biEncodingBuffer,
//                             mBinFile);
//        //showUint16BufferString(biEncodingBuffer,vertexCount * 4,4);
//        d1.componentType = COMPONENT_TYPE_UNSIGNED_SHORT;
//        d1.iIsolateIndex = vBinAccessorData.size();
//        vBinAccessorData.push_back(d1);
//
//        szName =  meshName + "_weight";
//        d1 = writeVertexAttribute("VEC4",
//                             "WEIGHTS_0",
//                             vertexCount,
//                             szName,
//                             bwEncodingSize,
//                             bwEncodingBuffer,
//                             mBinFile);
//        d1.iIsolateIndex = vBinAccessorData.size();
//        //showFloatBufferString(bwEncodingBuffer,vertexCount * 4,4);
//        vBinAccessorData.push_back(d1);
//    }
////struct BONE_
////{
//	//FbxUInt64 id;
//	//string name;
//	//string name1;
//	//FBXSDK_NAMESPACE::FbxAMatrix modelWorldToBoneLocal;
////};
//
//	//Indices
//	if (indexEncodingBuffer != NULL)
//	{
//        //dzlog_info("indexEncodingBuffer size %d",indexEncodingSize);
//
//        fwrite(indexEncodingBuffer,1,indexEncodingSize,mBinFile);
//
//        unsigned* pUint32 = (unsigned*)subMeshEncodingBuffer;
//        if (subMeshCount > 1) {
//            for (size_t i = 0; i < subMeshCount; ++i) {
//                unsigned offset = *(pUint32 + i * 2);
//                unsigned subTriangleCount = *(pUint32 + i * 2 + 1);
//                //dzlog_info("offset %u count %u",offset,subTriangleCount);
//                Accessor_data d_ind;
//                d_ind.type = "SCALAR";
//                d_ind.szTypeName = "indices";
//                d_ind.componentType = COMPONENT_TYPE_UNSIGNED_INT;
//                d_ind.count = subTriangleCount * 3;
//                d_ind.name = meshName + "_indices_" + "sub_" + intToString(i) ;
//                d_ind.len = d_ind.count * sizeof(unsigned);
//                d_ind.iIsolateIndex = vBinAccessorData.size();
//                vBinAccessorData.push_back(d_ind);
//            }
//        } else {
//            Accessor_data d_ind;
//            d_ind.type = "SCALAR";
//            d_ind.szTypeName = "indices";
//            d_ind.componentType = COMPONENT_TYPE_UNSIGNED_INT;
//            d_ind.count = triangleCount * 3;
//            d_ind.name = meshName + "_indices";
//            d_ind.len = indexEncodingSize;
//            d_ind.iIsolateIndex = vBinAccessorData.size();
//            vBinAccessorData.push_back(d_ind);
//        }
//	}
}

void pushFrames(vector<float> &frames,const vector<float>& arr)
{
    for (unsigned i = 0; i < (unsigned)arr.size() / 2; ++i) {
        frames.push_back(arr[i]);
    }
}

float getFrameValInterpolation(vector<float> d, const float fFrameVal)
{
    float fRetVal = 0.0;
    int iSize = d.size() / 2;
    for (int i = 0; i < iSize - 1; ++i) {
        float x0 = d[i];
        float y0 = d[i + iSize];

        float x1 = d[i + 1];
        float y1 = d[i + iSize + 1];
        if (fFrameVal <= x0) {
            fRetVal = y0;
            break;
        }
        if (x0 < fFrameVal && x1 >= fFrameVal) {
            fRetVal = y0 + (y1 - y0) * (fFrameVal - x0) / ( x1 - x0);
            break;
        }
        if (x1 < fFrameVal) {
            if (i == (iSize - 2)) { //the last values
              fRetVal = y1;
            } else {
               continue;
            }
        }
    }
    return fRetVal;
}

int mergeAnimationVectors(const vector<vector<float>> vFrames,
                           vector<float>& outArr,
                          float fMaxTime)
{

    vector<float> frames;
    for (auto it = vFrames.cbegin(); it != vFrames.cend(); ++it) {
        pushFrames(frames,*it);
    }

    if (frames.size() == 0) {
        return 0;
    }

    //ONLY KEYFRAME
    //std::sort(frames.begin(),frames.end());
    //frames.erase(std::unique(frames.begin(),frames.end()),frames.end());
    //
    //for (auto it = frames.cbegin(); it != frames.cend(); ++it) {
        //dzlog_debug("frame %f",*it);
        //if (*it < 0) {
            //// drop frame time below than zero
            //continue;
        //}
        //vector<float> vec3;
        //outArr.push_back(*it);
        //for (auto vit = vFrames.cbegin(); vit != vFrames.cend(); ++vit) {
            //float val = getFrameValInterpolation(*vit,*it);
            //outArr.push_back(val); 
        //}
    //}
    //

    for (float t = 0; t <= fMaxTime; t++) {
        //dzlog_debug("frame %f",t);
        vector<float> vec3;
        outArr.push_back(t);
        for (auto vit = vFrames.cbegin(); vit != vFrames.cend(); ++vit) {
            float val = getFrameValInterpolation(*vit,t);
            outArr.push_back(val); 
        }
    }

    return frames.size();
}

// for morph animation
void writeAnimationMorphWeight(unsigned iNodeID,
                        float fps,
                        int iMorphTargetCount,
                        vector<float>& weightKeyFrames,
                        vector<glTFAnimationChannelData>& channelArray,
                        FILE *mAnimationBinFile)
{
    size_t iBufSize = weightKeyFrames.size();
    size_t frameStripeSize = iMorphTargetCount + 1;
    size_t frameCount = iBufSize / frameStripeSize;
    float *buf = new float[iBufSize * sizeof(float)];  
    memset((void *)buf,0,sizeof(buf));
    for (size_t i = 0; i < frameCount; ++i) {
        buf[i] = weightKeyFrames[i * frameStripeSize] / fps;  //fix speed turn frame to second
        for (int j = 0; j < iMorphTargetCount; ++j) {
            buf[frameCount + i * iMorphTargetCount + j] = weightKeyFrames[i * frameStripeSize + 1 + j] / 100; //fix weight 100 => float 0-1
        }
    }

    glTFAnimationChannelData channelData;
    channelData.iTargetNode = iNodeID;
    channelData.szPathName = "weights";
    channelData.iFrameCount = frameCount;
    channelData.iValueCount = frameCount * iMorphTargetCount;
    channelData.fVMin = buf[0];
    channelData.fVMax = buf[frameCount - 1];
    channelData.szDataType = "SCALAR";
    channelData.iBinFrameSize = sizeof(float) * frameCount;
    channelData.iBinValueSize = sizeof(float) * frameCount * iMorphTargetCount;
    channelArray.push_back(channelData);

    fwrite(buf,sizeof(float),iBufSize,mAnimationBinFile);
}

// for postion and sacal
void writeAnimationPath(string szPathname,
                        unsigned iNodeID,
                        float fps,
                        vector<float>& x_keyFrames,
                        vector<float>& y_keyFrames,
                        vector<float>& z_keyFrames,
                        vector<glTFAnimationChannelData>& channelArray,
                        FILE *mAnimationBinFile,
                        float fMaxFrame)
{
    vector<float> outArr;
    vector<vector<float>> vSourceFrames = {x_keyFrames,y_keyFrames,z_keyFrames};
    mergeAnimationVectors(vSourceFrames,outArr,fMaxFrame);

    if (outArr.size() == 0) {
        return;
    }

    //showFloatBufferString(&outArr[0],outArr.size(),4);
    
    size_t iBufSize = outArr.size();
    size_t frameCount = iBufSize / 4;
    float *buf = new float[iBufSize * sizeof(float)];  // 1 for frame 3 for postion
    for (size_t i = 0; i < frameCount; ++i) {
        buf[i]                       = outArr[i * 4 + 0] / fps;  //fix speed turn frame to second
        buf[frameCount + i * 3]      = outArr[i * 4 + 1];
        buf[frameCount + i * 3 + 1]  = outArr[i * 4 + 2];
        buf[frameCount + i * 3 + 2]  = outArr[i * 4 + 3];
    }

    //showFloatBufferString(buf,iBufSize,3);

    glTFAnimationChannelData channelData;
    channelData.iTargetNode = iNodeID;
    channelData.szPathName = szPathname;
    channelData.iFrameCount = frameCount;
    channelData.iValueCount = frameCount;
    channelData.fVMin = buf[0];
    channelData.fVMax = buf[frameCount - 1];
    channelData.szDataType = "VEC3";
    channelData.iBinFrameSize = sizeof(float) * frameCount;
    channelData.iBinValueSize = sizeof(float) * frameCount * 3;
    channelArray.push_back(channelData);

    fwrite(buf,sizeof(float),iBufSize,mAnimationBinFile);

    delete[] buf;
}

// for rotation
void writeAnimationRotation(string szPathname,
                        unsigned iNodeID,
                        float fps,
                        vector<float>& x_keyFrames,
                        vector<float>& y_keyFrames,
                        vector<float>& z_keyFrames,
                        vector<glTFAnimationChannelData>& channelArray,
                        FILE *mAnimationBinFile,
                        float fMaxFrame)
{
    vector<float> outArr;
    vector<vector<float>> vSourceFrames = {x_keyFrames,y_keyFrames,z_keyFrames};

    mergeAnimationVectors(vSourceFrames,outArr,fMaxFrame);

    if (outArr.size() == 0) {
        return;
    }
    
    size_t frameCount = outArr.size() / 4; //origin euler size 1 for frame and 3 for euler
    size_t iBufSize = frameCount * 5; // 1 for frame and 4 for values
    float *buf = new float[iBufSize * sizeof(float)]; 
    for (size_t i = 0; i < frameCount; ++i) {
        FbxQuaternion q = transEulerToQuaternion(outArr[i * 4 + 1],outArr[i * 4 + 2],outArr[i * 4 + 3]);
        buf[i]                       = outArr[i * 4 + 0] / fps;  //fix speed turn frame to second
        buf[frameCount + i * 4 + 0]  = q[0];
        buf[frameCount + i * 4 + 1]  = q[1];
        buf[frameCount + i * 4 + 2]  = q[2];
        buf[frameCount + i * 4 + 3]  = q[3];
    }

    glTFAnimationChannelData channelData;
    channelData.iTargetNode = iNodeID;
    channelData.szPathName = szPathname;
    channelData.iFrameCount = frameCount;
    channelData.iValueCount = frameCount;
    channelData.fVMin = buf[0];
    channelData.fVMax = buf[frameCount - 1];
    channelData.szDataType = "VEC4";
    channelData.iBinFrameSize = sizeof(float) * frameCount;
    channelData.iBinValueSize = sizeof(float) * frameCount * 4;
    channelArray.push_back(channelData);

    fwrite(buf,sizeof(float),iBufSize,mAnimationBinFile);

    delete[] buf;
}

vector<float>* getTRSAnimation(vector<float>* trsvAnimation[], int curveType)
{
    vector<float>* keyFrames = trsvAnimation[curveType];
    if (keyFrames == NULL) {
        return new vector<float>;
    } else {
        return keyFrames;
    }
}

void glTFBin::writeAnimationBin(
    vector<glTFAnimationChannelData>& channelArray,
	unsigned int clipID,
	const std::string& clipName,
	float fps,
	float startFrame,
	float endFrame,
	std::vector<animationClipData>* clipData)
{
    //dzlog_info("animations name %s clip size %ld",clipName.c_str(),clipData->size());
	for (int i = 0; i < (int)clipData->size(); ++i)
	{
		animationClipData& acd = (*clipData)[i];
		if (acd.trsvAnimation != NULL)
		{
            float fMaxFrame = 0.0;
            // loop get
			for (int j = 0; j < ANIMATION_TYPE_COUNT; ++j)
			{
				std::vector<float>* keyframes = acd.trsvAnimation[j];
				if (keyframes != NULL && keyframes->size() > 0)
				{
                    int lastFrameIndex = keyframes->size() / 2 - 1;
                    float curveMaxFrame = keyframes->at(lastFrameIndex);
                    //dzlog_info("before %d - %f - %f",lastFrameIndex, curveMaxFrame,fMaxFrame);
                    if (curveMaxFrame > fMaxFrame) {
                        fMaxFrame = curveMaxFrame;
                    }
                    //dzlog_info("after %d - %f - %f",lastFrameIndex, curveMaxFrame,fMaxFrame);
                    //dzlog_info("type %d size %ld",j,keyframes->size());
				}
			}

            if (fMaxFrame <= 0.0) {
                //dzlog_info("max zero frame on chip all channel, skip chip");
                continue;
            } else {
                //dzlog_debug("max frame %f",fMaxFrame);
            }

            //dzlog_info("node %d animation",acd.nodeID);

            // translation
            vector<float>* pos_x_keyFrames = getTRSAnimation(acd.trsvAnimation, ANIMATION_POSITION_X);
            vector<float>* pos_y_keyFrames = getTRSAnimation(acd.trsvAnimation, ANIMATION_POSITION_Y);
            vector<float>* pos_z_keyFrames = getTRSAnimation(acd.trsvAnimation, ANIMATION_POSITION_Z);
            writeAnimationPath("translation",acd.nodeID,fps,*pos_x_keyFrames,*pos_y_keyFrames,*pos_z_keyFrames,channelArray,mAnimationBinFile,fMaxFrame);

            // scale
            vector<float>* scale_x_keyFrames = getTRSAnimation(acd.trsvAnimation, ANIMATION_SCALING_X);
            vector<float>* scale_y_keyFrames = getTRSAnimation(acd.trsvAnimation, ANIMATION_SCALING_Y);
            vector<float>* scale_z_keyFrames = getTRSAnimation(acd.trsvAnimation, ANIMATION_SCALING_Z);
            writeAnimationPath("scale",acd.nodeID,fps,*scale_x_keyFrames,*scale_y_keyFrames,*scale_z_keyFrames,channelArray,mAnimationBinFile,fMaxFrame);

            // rotation
            vector<float>* rotation_x_keyFrames = getTRSAnimation(acd.trsvAnimation, ANIMATION_ROTATION_EX);
            vector<float>* rotation_y_keyFrames = getTRSAnimation(acd.trsvAnimation, ANIMATION_ROTATION_EY);
            vector<float>* rotation_z_keyFrames = getTRSAnimation(acd.trsvAnimation, ANIMATION_ROTATION_EZ);
            writeAnimationRotation("rotation",acd.nodeID,fps,*rotation_x_keyFrames,*rotation_y_keyFrames,*rotation_z_keyFrames,channelArray,mAnimationBinFile,fMaxFrame);
		}

        if (acd.morphAnimation != NULL)
        {
            vector<vector<float>> morphData;
            float fMaxFrame = FLT_MIN;
			for (int j = 0; j < (int)acd.morphAnimation->size(); ++j)
			{
				morphTargetAnimation* mta = (*acd.morphAnimation)[j];

                int keyCount = (unsigned)(mta->animations.size()) / 2;
				int dataSize = (unsigned int)(mta->animations.size())*sizeof(float);
                string szChannelName = mta->channelName;

                //TODO test it!!
                float channelMaxFrame = mta->animations.at(keyCount - 1);
                if (channelMaxFrame > fMaxFrame) {
                    fMaxFrame = channelMaxFrame;
                }
                //dzlog_info("morph frames count %d dataSize %d szChannelName %s",keyCount,dataSize,szChannelName.c_str());
                //for (auto it = mta->animations.cbegin(); it != mta->animations.cend(); ++it) {
                    //dzlog_info("%f",*it);
                //}
                morphData.push_back(mta->animations);
			}
            vector<float> outArr;
            mergeAnimationVectors(morphData,outArr,fMaxFrame);
            writeAnimationMorphWeight(acd.nodeID,fps,morphData.size(),outArr,channelArray,mAnimationBinFile);
        }
	}
}

