#include "./AnimationsExport.h"
#include "./BinData.h"

template<typename _type_>
class FbxLayerElementAccess
{
public:

    FbxLayerElementAccess(const FbxLayerElementTemplate<_type_> *layer, int count) :
        mappingMode(FbxGeometryElement::eNone),
        elements(nullptr),
        indices(nullptr)
    {
        if (count <= 0 || layer == nullptr) {
            return;
        }
        const FbxGeometryElement::EMappingMode newMappingMode = layer->GetMappingMode();
        if (newMappingMode == FbxGeometryElement::eByControlPoint ||
            newMappingMode == FbxGeometryElement::eByPolygonVertex ||
            newMappingMode == FbxGeometryElement::eByPolygon) {
            mappingMode = newMappingMode;
            elements    = &layer->GetDirectArray();
            indices     = (
                layer->GetReferenceMode() == FbxGeometryElement::eIndexToDirect ||
                layer->GetReferenceMode() == FbxGeometryElement::eIndex) ? &layer->GetIndexArray() : nullptr;
        }
    }

    bool LayerPresent() const
    {
        return (mappingMode != FbxGeometryElement::eNone);
    }

    _type_ GetElement(const int polygonIndex, const int polygonVertexIndex, const int controlPointIndex, const _type_ defaultValue) const
    {
        if (mappingMode != FbxGeometryElement::eNone) {
            int index = (mappingMode == FbxGeometryElement::eByControlPoint) ? controlPointIndex :
                        ((mappingMode == FbxGeometryElement::eByPolygonVertex) ? polygonVertexIndex : polygonIndex);
            index = (indices != nullptr) ? (*indices)[index] : index;
            _type_ element = elements->GetAt(index);
            return element;
        }
        return defaultValue;
    }

    _type_ GetElement(
        const int polygonIndex, const int polygonVertexIndex, const int controlPointIndex, const _type_ defaultValue,
        const FbxMatrix &transform, const bool normalize) const
    {
        if (mappingMode != FbxGeometryElement::eNone) {
            _type_ element = transform.MultNormalize(GetElement(polygonIndex, polygonVertexIndex, controlPointIndex, defaultValue));
            if (normalize) {
                element.Normalize();
            }
            return element;
        }
        return defaultValue;
    }

private:
    FbxGeometryElement::EMappingMode           mappingMode;
    const FbxLayerElementArrayTemplate<_type_> *elements;
    const FbxLayerElementArrayTemplate<int>    *indices;
};

/**
 * At the FBX level, each Mesh can have a set of FbxBlendShape deformers; organisational units that contain no data
 * of their own. The actual deformation is determined by one or more FbxBlendShapeChannels, whose influences are all
 * additively applied to the mesh. In a simpler world, each such channel would extend each base vertex with alternate
 * position, and optionally normal and tangent.
 *
 * It's not quite so simple, though. We also have progressive morphing, where one logical morph actually consists of
 * several concrete ones, each applied in sequence. For us, this means each channel contains a sequence of FbxShapes
 * (aka target shape); these are the actual data-holding entities that provide the alternate vertex attributes. As such
 * a channel is given more weight, it moves from one target shape to another.
 *
 * The total number of alternate sets of attributes, then, is the total number of target shapes across all the channels
 * of all the blend shapes of the mesh.
 *
 * Each animation in the scene stack can yield one or zero FbxAnimCurves per channel (not target shape). We evaluate
 * these curves to get the weight of the channel: this weight is further introspected on to figure out which target
 * shapes we're currently interpolation between.
 */
class FbxBlendShapesAccess
{
public:
    /**
     * A target shape is on a 1:1 basis with the eventual glTF morph target, and is the object which contains the
     * actual morphed vertex data.
     */
    struct TargetShape
    {
        explicit TargetShape(const FbxShape *shape, FbxDouble fullWeight) :
            shape(shape),
            fullWeight(fullWeight),
            count(shape->GetControlPointsCount()),
            positions(shape->GetControlPoints()),
            normals(FbxLayerElementAccess<FbxVector4>(shape->GetElementNormal(), shape->GetElementNormalCount())),
            tangents(FbxLayerElementAccess<FbxVector4>(shape->GetElementTangent(), shape->GetElementTangentCount()))
        {}

        const FbxShape                          *shape;
        const FbxDouble                         fullWeight;
        const unsigned int                      count;
        const FbxVector4                        *positions;
        const FbxLayerElementAccess<FbxVector4> normals;
        const FbxLayerElementAccess<FbxVector4> tangents;
    };

    /**
     * A channel collects a sequence (often of length 1) of target shapes.
     */
    struct BlendChannel
    {
        BlendChannel(
            FbxMesh *mesh,
            const unsigned int blendShapeIx,
            const unsigned int channelIx,
            const FbxDouble deformPercent,
            const std::vector<TargetShape> &targetShapes
        ) : mesh(mesh),
            blendShapeIx(blendShapeIx),
            channelIx(channelIx),
            targetShapes(targetShapes),
            deformPercent(deformPercent)
        {}

        FbxAnimCurve *ExtractAnimation(unsigned int animIx) const {
            FbxAnimStack *stack = mesh->GetScene()->GetSrcObject<FbxAnimStack>(animIx);
            FbxAnimLayer *layer = stack->GetMember<FbxAnimLayer>(0);
            return mesh->GetShapeChannel(blendShapeIx, channelIx, layer, true);
        }

        FbxMesh *const mesh;

        const unsigned int blendShapeIx;
        const unsigned int channelIx;
        const std::vector<TargetShape> targetShapes;

        const FbxDouble deformPercent;
    };

    explicit FbxBlendShapesAccess(FbxMesh *mesh) :
        channels(extractChannels(mesh))
    { }

    size_t GetChannelCount() const { return channels.size(); }
    const BlendChannel &GetBlendChannel(size_t channelIx) const {
        return channels.at(channelIx);
    }

    size_t GetTargetShapeCount(size_t channelIx) const { return channels[channelIx].targetShapes.size(); }
    const TargetShape &GetTargetShape(size_t channelIx, size_t targetShapeIx) const {
        return channels.at(channelIx).targetShapes[targetShapeIx];
    }

    FbxAnimCurve * GetAnimation(size_t channelIx, size_t animIx) const {
        return channels.at(channelIx).ExtractAnimation(animIx);
    }

private:
    std::vector<BlendChannel> extractChannels(FbxMesh *mesh) const {
        std::vector<BlendChannel> channels;
        for (int shapeIx = 0; shapeIx < mesh->GetDeformerCount(FbxDeformer::eBlendShape); shapeIx++) {
            auto *fbxBlendShape = static_cast<FbxBlendShape *>(mesh->GetDeformer(shapeIx, FbxDeformer::eBlendShape));

            for (int channelIx = 0; channelIx < fbxBlendShape->GetBlendShapeChannelCount(); ++channelIx) {
                FbxBlendShapeChannel *fbxChannel = fbxBlendShape->GetBlendShapeChannel(channelIx);

                if (fbxChannel->GetTargetShapeCount() > 0) {
                    std::vector<TargetShape> targetShapes;
                    const double *fullWeights = fbxChannel->GetTargetShapeFullWeights();
                    for (int targetIx = 0; targetIx < fbxChannel->GetTargetShapeCount(); targetIx ++) {
                        FbxShape *fbxShape = fbxChannel->GetTargetShape(targetIx);
                        targetShapes.push_back(TargetShape(fbxShape, fullWeights[targetIx]));
                    }
                    channels.push_back(BlendChannel(mesh, shapeIx, channelIx, fbxChannel->DeformPercent * 0.01, targetShapes));
                }
            }
        }
        return channels;
    }

    const std::vector<BlendChannel> channels;
};

static FbxVector4 computeLocalScale(FbxNode *pNode, FbxTime pTime = FBXSDK_TIME_INFINITE)
{
    const FbxVector4 lScale = pNode->EvaluateLocalTransform(pTime).GetS();

    if (pNode->GetParent() == nullptr ||
        pNode->GetTransform().GetInheritType() != FbxTransform::eInheritRrs) {
        return lScale;
    }
    // This is a very partial fix that is only correct for models that use identity scale in their rig's joints.
    // We could write better support that compares local scale to parent's global scale and apply the ratio to
    // our local translation. We'll always want to return scale 1, though -- that's the only way to encode the
    // missing 'S' (parent scale) in the transform chain.
    return FbxVector4(1, 1, 1, 1);
}

/////////////////////////////////////////
AnimationExport::AnimationExport()
{

}

AnimationExport::~AnimationExport()
{

}

void AnimationExport::getSceneAnimation(FbxScene *pScene,vector<RawAnimation>& vAnimation)
{
    FbxTime::EMode eMode = FbxTime::eFrames30;
    const double epsilon = 1e-5f;

    const int animationCount = pScene->GetSrcObjectCount<FbxAnimStack>();

    for (int animIx = 0; animIx < animationCount; animIx++) {
        FbxAnimStack *pAnimStack = pScene->GetSrcObject<FbxAnimStack>(animIx);
        FbxString animStackName = pAnimStack->GetName();

        pScene->SetCurrentAnimationStack(pAnimStack);

        FbxTakeInfo *takeInfo = pScene->GetTakeInfo(animStackName);
        if (takeInfo == nullptr) {
            //dzlog_warn("Warning:: animation '%s' has no Take information. Skipping.", &animStackName[0]);
            // not all animstacks have a take
            continue;
        }

        //dzlog_info("animation %d: %s (%d%%)", animIx, (const char *) animStackName, 0);

        FbxTime start = takeInfo->mLocalTimeSpan.GetStart();
        FbxTime end   = takeInfo->mLocalTimeSpan.GetStop();

        RawAnimation animation;
        animation.name = animStackName;

        FbxLongLong firstFrameIndex = start.GetFrameCount(eMode);
        FbxLongLong lastFrameIndex  = end.GetFrameCount(eMode);
        for (FbxLongLong frameIndex = firstFrameIndex; frameIndex <= lastFrameIndex; frameIndex++) {
            FbxTime pTime;
            // first frame is always at t = 0.0
            pTime.SetFrame(frameIndex - firstFrameIndex, eMode);
            animation.times.emplace_back((float) pTime.GetSecondDouble());
        }

        size_t totalSizeInBytes = 0;

        const int nodeCount = pScene->GetNodeCount();
        for (int nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++) {
            FbxNode *pNode = pScene->GetNode(nodeIndex);
            const FbxAMatrix    baseTransform   = pNode->EvaluateLocalTransform();
            const FbxVector4    baseTranslation = baseTransform.GetT();
            const FbxQuaternion baseRotation    = baseTransform.GetQ();
            const FbxVector4    baseScaling     = computeLocalScale(pNode);
            bool hasTranslation = false;
            bool hasRotation    = false;
            bool hasScale       = false;
            bool hasMorphs      = false;

            RawChannel channel;
            //channel.nodeIndex = raw.GetNodeById(pNode->GetUniqueID());
            channel.iNodeID = pNode->GetUniqueID();

            for (FbxLongLong frameIndex = firstFrameIndex; frameIndex <= lastFrameIndex; frameIndex++) {
                FbxTime pTime;
                pTime.SetFrame(frameIndex, eMode);

                const FbxAMatrix    localTransform   = pNode->EvaluateLocalTransform(pTime);
                const FbxVector4    localTranslation = localTransform.GetT();
                const FbxQuaternion localRotation    = localTransform.GetQ();
                const FbxVector4    localScale       = computeLocalScale(pNode, pTime);

                hasTranslation |= (
                    fabs(localTranslation[0] - baseTranslation[0]) > epsilon ||
                    fabs(localTranslation[1] - baseTranslation[1]) > epsilon ||
                    fabs(localTranslation[2] - baseTranslation[2]) > epsilon);
                hasRotation |= (
                    fabs(localRotation[0] - baseRotation[0]) > epsilon ||
                    fabs(localRotation[1] - baseRotation[1]) > epsilon ||
                    fabs(localRotation[2] - baseRotation[2]) > epsilon ||
                    fabs(localRotation[3] - baseRotation[3]) > epsilon);
                hasScale |= (
                    fabs(localScale[0] - baseScaling[0]) > epsilon ||
                    fabs(localScale[1] - baseScaling[1]) > epsilon ||
                    fabs(localScale[2] - baseScaling[2]) > epsilon);

                channel.translations.push_back(localTranslation);
                channel.rotations.push_back(localRotation);
                channel.scales.push_back(localScale);
            }

            //if (hasRotation) {
                //dzlog_info("node id %d has rotation",channel.iNodeID);
                //for (size_t i = 0; i < channel.rotations.size(); ++i) {
                    //DebugHelper::showQuaterion(channel.rotations[i]);
                //}
            //}

            std::vector<FbxAnimCurve *> shapeAnimCurves;
            FbxNodeAttribute *nodeAttr = pNode->GetNodeAttribute();
            if (nodeAttr != nullptr && nodeAttr->GetAttributeType() == FbxNodeAttribute::EType::eMesh) {
                // it's inelegant to recreate this same access class multiple times, but it's also dirt cheap...
                FbxBlendShapesAccess blendShapes(static_cast<FbxMesh *>(nodeAttr));

                for (FbxLongLong frameIndex = firstFrameIndex; frameIndex <= lastFrameIndex; frameIndex++) {
                    FbxTime pTime;
                    pTime.SetFrame(frameIndex, eMode);

                    for (size_t channelIx = 0; channelIx < blendShapes.GetChannelCount(); channelIx++) {
                        FbxAnimCurve *curve = blendShapes.GetAnimation(channelIx, animIx);
                        float influence = (curve != nullptr) ? curve->Evaluate(pTime) : 0; // 0-100

                        int targetCount = static_cast<int>(blendShapes.GetTargetShapeCount(channelIx));

                        // the target shape 'fullWeight' values are a strictly ascending list of floats (between
                        // 0 and 100), forming a sequence of intervals -- this convenience function figures out if
                        // 'p' lays between some certain target fullWeights, and if so where (from 0 to 1).
                        auto findInInterval = [&](const double p, const int n) {
                            if (n >= targetCount) {
                                // p is certainly completely left of this interval
                                return NAN;
                            }
                            double leftWeight = 0;
                            if (n >= 0) {
                                leftWeight = blendShapes.GetTargetShape(channelIx, n).fullWeight;
                                if (p < leftWeight) {
                                    return NAN;
                                }
                                // the first interval implicitly includes all lesser influence values
                            }
                            double rightWeight = blendShapes.GetTargetShape(channelIx, n+1).fullWeight;
                            if (p > rightWeight && n+1 < targetCount-1) {
                                return NAN;
                                // the last interval implicitly includes all greater influence values
                            }
                            // transform p linearly such that [leftWeight, rightWeight] => [0, 1]
                            return static_cast<float>((p - leftWeight) / (rightWeight - leftWeight));
                        };

                        for (int targetIx = 0; targetIx < targetCount; targetIx++) {
                            if (curve) {
                                float result = findInInterval(influence, targetIx-1);
                                if (!isnan(result)) {
                                    // we're transitioning into targetIx
                                    channel.weights.push_back(result);
                                    hasMorphs = true;
                                    continue;
                                }
                                if (targetIx != targetCount-1) {
                                    result = findInInterval(influence, targetIx);
                                    if (!isnan(result)) {
                                        // we're transitioning AWAY from targetIx
                                        channel.weights.push_back(1.0f - result);
                                        hasMorphs = true;
                                        continue;
                                    }
                                }
                            }

                            // this is here because we have to fill in a weight for every channelIx/targetIx permutation,
                            // regardless of whether or not they participate in this animation.
                            channel.weights.push_back(0.0f);
                        }
                    }
                }
            }

            if (hasTranslation || hasRotation || hasScale || hasMorphs) {
                if (!hasTranslation) {
                    channel.translations.clear();
                }
                if (!hasRotation) {
                    channel.rotations.clear();
                }
                if (!hasScale) {
                    channel.scales.clear();
                }
                if (!hasMorphs) {
                    channel.weights.clear();
                }

                animation.channels.emplace_back(channel);

                totalSizeInBytes += channel.translations.size() * sizeof(channel.translations[0]) +
                                    channel.rotations.size() * sizeof(channel.rotations[0]) +
                                    channel.scales.size() * sizeof(channel.scales[0]) +
                                    channel.weights.size() * sizeof(channel.weights[0]);
            }

            //dzlog_info("animation %d: %s (%d%%)", animIx, (const char *) animStackName, nodeIndex * 100 / nodeCount);
        }

        if (!animation.channels.empty()) {
            vAnimation.push_back(animation);

            //dzlog_info("animation %d: %s (%d channels, %3.1f KB)", animIx, (const char *) animStackName,
            //    (int) animation.channels.size(), (float) totalSizeInBytes * 1e-3f);
        }
    }
}

void AnimationExport::writeAnimationBin(vector<RawAnimation>& vAnimation,const string szAnimationFile, BinData* mBinData)
{
    if (vAnimation.size() == 0) {
        return;
    }

    //dzlog_info("write animation bin");

    mBinData->bHasAnimationBin = true;

    FILE *pAnimBinFile = fopen(szAnimationFile.c_str(),"wb");

    for (auto it = vAnimation.begin(); it != vAnimation.end(); ++it) {
        RawAnimation animData = *it;

        if (it->channels.empty()) {
            continue;
        }

        int iFrameTimeCount = animData.times.size();
        int iFrameBufferSize = iFrameTimeCount * sizeof(float);

        fwrite(animData.times.data(),1,iFrameBufferSize,pAnimBinFile);

        Accessor_data animAccessorsData;
        animAccessorsData.componentType = COMPONENT_TYPE_FLOAT; 
        animAccessorsData.count = iFrameTimeCount;
        animAccessorsData.type = "SCALAR";
        animAccessorsData.szTypeName = "ANIMATION_FRAME";
        animAccessorsData.iBufIndex = 1;
        animAccessorsData.len = iFrameBufferSize;
        animAccessorsData.vMax = animData.times.back();
        animAccessorsData.vMin = animData.times.front();
        animAccessorsData.name = "anim_" + animData.name  +  "_frame";
        mBinData->add(animAccessorsData);

        it->iTimeAccessorIndex = mBinData->getSize() - 1;

        for (auto channelIT = animData.channels.cbegin(); channelIT != animData.channels.cend(); ++channelIT) {
            RawChannel channelInfo = *channelIT;

            if (channelInfo.translations.size() > 0) {
                vector<float> vOutBuff;
                vOutBuff.resize(iFrameTimeCount * 3);
                int iValueBufferSize = iFrameTimeCount * 3 * sizeof(float);

                for (int i = 0; i < iFrameTimeCount; ++i) {
                    vOutBuff[i * 3 + 0] = channelInfo.translations.at(i)[0];
                    vOutBuff[i * 3 + 1] = channelInfo.translations.at(i)[1];
                    vOutBuff[i * 3 + 2] = channelInfo.translations.at(i)[2];
                }

                fwrite(vOutBuff.data(),1,iValueBufferSize,pAnimBinFile);

                Accessor_data animValueData;
                animValueData.componentType = COMPONENT_TYPE_FLOAT; 
                animValueData.count = iFrameTimeCount;
                animValueData.type = "VEC3";
                animValueData.szTypeName = "ANIMATION_TRANSLATION";
                animValueData.iBufIndex = 1;
                animValueData.len = iValueBufferSize;
                animValueData.name = "node_" + intToString(channelInfo.iNodeID) + "_anim_translation_values";
                mBinData->add(animValueData);

                gltfChannelInfo outChannel;
                outChannel.iNodeID = channelInfo.iNodeID;
                outChannel.szChannelName = "translation";
                outChannel.iAccessorIndex = mBinData->getSize() - 1;
                it->outChannels.push_back(outChannel);
            }

            if (channelInfo.rotations.size() > 0) {
                vector<float> vOutBuff;
                vOutBuff.resize(iFrameTimeCount * 4);
                int iValueBufferSize = iFrameTimeCount * 4 * sizeof(float);

                for (int i = 0; i < iFrameTimeCount; ++i) {
                    vOutBuff[i * 4 + 0] = channelInfo.rotations.at(i)[0];
                    vOutBuff[i * 4 + 1] = channelInfo.rotations.at(i)[1];
                    vOutBuff[i * 4 + 2] = channelInfo.rotations.at(i)[2];
                    vOutBuff[i * 4 + 3] = channelInfo.rotations.at(i)[3];
                }

                fwrite(vOutBuff.data(),1,iValueBufferSize,pAnimBinFile);

                Accessor_data animValueData;
                animValueData.componentType = COMPONENT_TYPE_FLOAT; 
                animValueData.count = iFrameTimeCount;
                animValueData.type = "VEC4";
                animValueData.szTypeName = "ANIMATION_ROATATION";
                animValueData.iBufIndex = 1;
                animValueData.len = iValueBufferSize;
                animValueData.name = "node_" + intToString(channelInfo.iNodeID) + "_anim_rotation_values";
                mBinData->add(animValueData);

                gltfChannelInfo outChannel;
                outChannel.iNodeID = channelInfo.iNodeID;
                outChannel.szChannelName = "rotation";
                outChannel.iAccessorIndex = mBinData->getSize() - 1;
                it->outChannels.push_back(outChannel);
            }

            if (channelInfo.scales.size() > 0) {
                vector<float> vOutBuff;
                vOutBuff.resize(iFrameTimeCount * 3);
                int iValueBufferSize = iFrameTimeCount * 3 * sizeof(float);

                for (int i = 0; i < iFrameTimeCount; ++i) {
                    vOutBuff[i * 3 + 0] = channelInfo.scales.at(i)[0];
                    vOutBuff[i * 3 + 1] = channelInfo.scales.at(i)[1];
                    vOutBuff[i * 3 + 2] = channelInfo.scales.at(i)[2];
                }

                fwrite(vOutBuff.data(),1,iValueBufferSize,pAnimBinFile);

                Accessor_data animValueData;
                animValueData.componentType = COMPONENT_TYPE_FLOAT; 
                animValueData.count = iFrameTimeCount;
                animValueData.type = "VEC3";
                animValueData.szTypeName = "ANIMATION_SCALES";
                animValueData.iBufIndex = 1;
                animValueData.len = iValueBufferSize;
                animValueData.name = "node_" + intToString(channelInfo.iNodeID) + "_anim_scales_values";
                mBinData->add(animValueData);

                gltfChannelInfo outChannel;
                outChannel.iNodeID = channelInfo.iNodeID;
                outChannel.szChannelName = "scale";
                outChannel.iAccessorIndex = mBinData->getSize() - 1;
                it->outChannels.push_back(outChannel);
            }

            if (channelInfo.weights.size() > 0) {
                int iWeightValueCount = channelInfo.weights.size();
                int iWeightBufferSize = iWeightValueCount * sizeof(float);

                fwrite(channelInfo.weights.data(),1,iWeightBufferSize,pAnimBinFile);

                Accessor_data animValueData;
                animValueData.componentType = COMPONENT_TYPE_FLOAT; 
                animValueData.count = iWeightValueCount;
                animValueData.type = "SCALAR";
                animValueData.szTypeName = "ANIMATION_WEIGHT";
                animValueData.iBufIndex = 1;
                animValueData.len = iWeightBufferSize;
                animValueData.name = "node_" + intToString(channelInfo.iNodeID) + "_anim_weight_values";
                mBinData->add(animValueData);

                gltfChannelInfo outChannel;
                outChannel.iNodeID = channelInfo.iNodeID;
                outChannel.szChannelName = "weights";
                outChannel.iAccessorIndex = mBinData->getSize() - 1;
                it->outChannels.push_back(outChannel);
            }
        }
    }

    if (pAnimBinFile != NULL) {
        fclose(pAnimBinFile);
    }
}

