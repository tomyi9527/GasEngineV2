#include "stdafx.h"
#include "FBXAnimationExporter_V4.h"
#include "JSONFileWriter.h"
#include "JsonToBin_V4.h"
#include "Material.h"

FBXAnimationExporter_V4::FBXAnimationExporter_V4()
{
}


FBXAnimationExporter_V4::~FBXAnimationExporter_V4()
{
}

bool FBXAnimationExporter_V4::init(FBXSDK_NAMESPACE::FbxManager* manager, FBXSDK_NAMESPACE::FbxScene* scene)
{
	FBXSDK_NAMESPACE::FbxNode* rootNode = scene->GetRootNode();
	if (rootNode == NULL)
	{
		return false;
	}

	keyFrameCount = 0;

	FBXSDK_NAMESPACE::FbxGlobalSettings& globalSettings = scene->GetGlobalSettings();
	FBXSDK_NAMESPACE::FbxTime::EProtocol timeProtocol = globalSettings.GetTimeProtocol();
	if (timeProtocol != FBXSDK_NAMESPACE::FbxTime::eFrameCount)
	{
		FBXSDK_printf("%s", "Error: Only support eFrameCount time protocol./n");
		return false;
	}

	mTimeMode = globalSettings.GetTimeMode();

	int animationStackCount = scene->GetSrcObjectCount<FBXSDK_NAMESPACE::FbxAnimStack>();
	for (int stackIndex = 0; stackIndex < animationStackCount; stackIndex++)
	{
		FBXSDK_NAMESPACE::FbxAnimStack* animationStack = scene->GetSrcObject<FBXSDK_NAMESPACE::FbxAnimStack>(stackIndex);
		if (animationStack == NULL)
		{
			continue;
		}

		std::string clipName = standardizeFileName(animationStack->GetName());
		int animationLayerCount = animationStack->GetMemberCount<FBXSDK_NAMESPACE::FbxAnimLayer>();
		if (animationLayerCount == 0)
		{
			continue;
		}
		else if (animationLayerCount > 1)
		{
			FBXSDK_printf("Warning: clip \"%s\" contain %d animation layers. Only the first one would be exported!\n", clipName.c_str(), animationLayerCount);
		}

		FBXSDK_printf("********* Start process animation clip: %s *********\n", clipName.c_str());

		unsigned int clipID = (unsigned int)animationStack->GetUniqueID();

		std::vector<animationClipData>* clipData = new std::vector<animationClipData>();

		getAnimationSingleClip(animationStack, rootNode, clipData);

		if (clipData->size() > 0)
		{
			double fps = 0.0;
			if (mTimeMode == FBXSDK_NAMESPACE::FbxTime::eCustom)
			{
				fps = globalSettings.GetCustomFrameRate();
			}
			else
			{
				fps = FBXSDK_NAMESPACE::FbxTime::GetFrameRate(mTimeMode);
			}

			//FBXSDK_NAMESPACE::FbxTimeSpan refSpan = animationStack->GetReferenceTimeSpan();
			//FBXSDK_NAMESPACE::FbxDouble startRefFrame = refSpan.GetStart().GetFrameCmConvertedFilesountPrecise(mTimeMode);
			//FBXSDK_NAMESPACE::FbxDouble endRefFrame = refSpan.GetStop().GetFrameCountPrecise(mTimeMode);
			FBXSDK_NAMESPACE::FbxTimeSpan localSpan = animationStack->GetLocalTimeSpan();
			FBXSDK_NAMESPACE::FbxDouble startFrame = localSpan.GetStart().GetFrameCountPrecise(mTimeMode);
			FBXSDK_NAMESPACE::FbxDouble endFrame = localSpan.GetStop().GetFrameCountPrecise(mTimeMode);

			KeyframeAnimation kfa;
			kfa.clipName = clipName;
			kfa.clipID = clipID;
			kfa.keyframeCount = keyFrameCount;
			kfa.fps = fps;
			kfa.startFrame = startFrame;
			kfa.endFrame = endFrame;
			kfa.nodes = clipData;
			mAnimations.push_back(kfa);
		}

		FBXSDK_printf("********* End process animation clip: %s *********\n", clipName.c_str());
		FBXSDK_printf("\n");
	}

	return true;
}

void FBXAnimationExporter_V4::finl()
{
	for (size_t s = 0; s < mAnimations.size(); ++s)
	{
		KeyframeAnimation& kfa = mAnimations[s];
		if (kfa.nodes != NULL)
		{
			for (int i = 0; i < (int)kfa.nodes->size(); ++i)
			{
				animationClipData& acd = (*kfa.nodes)[i];
				if (acd.trsvAnimation != NULL)
				{
					for (int j = 0; j < (int)ANIMATION_TYPE_COUNT; ++j)
					{
						std::vector<float>* keyframes = (acd.trsvAnimation)[j];
						delete keyframes;
					}

					delete[] acd.trsvAnimation;
				}

				if (acd.morphAnimation != NULL)
				{
					for (int j = 0; j < (int)acd.morphAnimation->size(); ++j)
					{
						morphTargetAnimation* mta = (*acd.morphAnimation)[j];
						delete mta;
					}
					delete acd.morphAnimation;
				}
			}

			delete kfa.nodes;
			kfa.nodes = NULL;
		}
	}
}

void FBXAnimationExporter_V4::getAnimationSingleClip(
	FBXSDK_NAMESPACE::FbxAnimStack* animationStack,
	FBXSDK_NAMESPACE::FbxNode* root,
	std::vector<animationClipData>* container)
{
	FBXSDK_NAMESPACE::FbxAnimLayer* animationLayer = animationStack->GetMember<FBXSDK_NAMESPACE::FbxAnimLayer>(0);
	if (animationLayer != NULL)
	{
		getNodeAnimation_r(animationLayer, root, container);
	}	
}

std::string FBXAnimationExporter_V4::getNodePath(FBXSDK_NAMESPACE::FbxNode* node)
{
	std::string path = "/" + standardizeFileName(node->GetName());
	FBXSDK_NAMESPACE::FbxNode* parent = node->GetParent();
	while (parent != NULL)
	{
		std::string nodeName = standardizeFileName(parent->GetName());
		path = "/" + nodeName + path;
		parent = parent->GetParent();
	}

	return path;
}

void FBXAnimationExporter_V4::getNodeAnimation_r(
	FBXSDK_NAMESPACE::FbxAnimLayer* animationLayer,
	FBXSDK_NAMESPACE::FbxNode* node,
	std::vector<animationClipData>* container)
{
	if (node != NULL)
	{
		std::vector<float>** trsvAnimation = getTRSVAnimationData(animationLayer, node);
		if (trsvAnimation != NULL)
		{
			animationClipData clipData;

			string nodePath = getNodePath(node);
			clipData.nodeName = standardizeFileName(node->GetName());
			clipData.nodeID = 0xffffffff;
			//clipData.nodeID = (unsigned int)node->GetUniqueID();
			clipData.trsvAnimation = trsvAnimation;
			clipData.morphAnimation = NULL;
			clipData.extAnimationData = NULL;

			container->push_back(clipData);
		}

		//Get blend shape animation
		FBXSDK_NAMESPACE::FbxNodeAttribute* nodeAttr = node->GetNodeAttribute();
		if (nodeAttr != NULL)
		{			
			FBXSDK_NAMESPACE::FbxNodeAttribute::EType nodeAttrType = nodeAttr->GetAttributeType();
			if (nodeAttrType == FBXSDK_NAMESPACE::FbxNodeAttribute::eMesh)
			{
				FBXSDK_NAMESPACE::FbxMesh* mesh = node->GetMesh();
				int blendShapeDeformerCount = mesh->GetDeformerCount(FbxDeformer::eBlendShape);
				if (blendShapeDeformerCount >= 1)
				{
					if (blendShapeDeformerCount > 1)
					{
						FBXSDK_printf("Error: only support one blend shape deformer!\n");
					}

					FbxBlendShape* blendShape = (FbxBlendShape*)mesh->GetDeformer(0, FbxDeformer::eBlendShape);
					if (blendShape == NULL)
					{
						FBXSDK_printf("Error: only support blend shape deformer on the top of the stack!\n");
					}
					else
					{
						std::vector<morphTargetAnimation*>* morphAnimation = getMophAnimationData(animationLayer, mesh);
						if (morphAnimation != NULL)
						{
							animationClipData clipData;

							clipData.nodeName = standardizeFileName(mesh->GetName());
							//clipData.nodeID = 0xffffffff;
							clipData.nodeID = mesh->GetUniqueID();
							clipData.trsvAnimation = NULL;
							clipData.morphAnimation = morphAnimation;
							clipData.extAnimationData = NULL;

							container->push_back(clipData);
						}
					}
				}
				//<
			}
		}


		int childCount = node->GetChildCount();
		for (int i = 0; i < childCount; ++i)
		{
			FBXSDK_NAMESPACE::FbxNode* child = node->GetChild(i);
			getNodeAnimation_r(animationLayer, child, container);
		}
	}
}

std::vector<float>** FBXAnimationExporter_V4::getTRSVAnimationData(
	FBXSDK_NAMESPACE::FbxAnimLayer* animationLayer,
	FBXSDK_NAMESPACE::FbxNode* node)
{
	std::vector<float>** trsvAnimation = new std::vector<float>*[ANIMATION_TYPE_COUNT];
	memset(trsvAnimation, 0, sizeof(std::vector<float>*)*ANIMATION_TYPE_COUNT);

	unsigned int animationFlag = 0;
	FBXSDK_NAMESPACE::FbxAnimCurve* animationCurve = NULL;
	// Translation
	animationCurve = node->LclTranslation.GetCurve(animationLayer, FBXSDK_CURVENODE_COMPONENT_X);
	if (animationCurve)
	{
		int keyCount = animationCurve->KeyGetCount();
		if (keyCount > 0)
		{
			animationFlag |= (1 << ANIMATION_POSITION_X);
			trsvAnimation[ANIMATION_POSITION_X] = new std::vector<float>();
			getKeyframes(animationCurve, *trsvAnimation[ANIMATION_POSITION_X]);
		}
	}

	animationCurve = node->LclTranslation.GetCurve(animationLayer, FBXSDK_CURVENODE_COMPONENT_Y);
	if (animationCurve)
	{
		int keyCount = animationCurve->KeyGetCount();
		if (keyCount > 0)
		{
			animationFlag |= (1 << ANIMATION_POSITION_Y);
			trsvAnimation[ANIMATION_POSITION_Y] = new std::vector<float>();
			getKeyframes(animationCurve, *trsvAnimation[ANIMATION_POSITION_Y]);
		}
	}

	animationCurve = node->LclTranslation.GetCurve(animationLayer, FBXSDK_CURVENODE_COMPONENT_Z);
	if (animationCurve)
	{
		int keyCount = animationCurve->KeyGetCount();
		if (keyCount > 0)
		{
			animationFlag |= (1 << ANIMATION_POSITION_Z);
			trsvAnimation[ANIMATION_POSITION_Z] = new std::vector<float>();
			getKeyframes(animationCurve, *trsvAnimation[ANIMATION_POSITION_Z]);
		}
	}

	// Rotation
	animationCurve = node->LclRotation.GetCurve(animationLayer, FBXSDK_CURVENODE_COMPONENT_X);
	if (animationCurve)
	{
		int keyCount = animationCurve->KeyGetCount();
		if (keyCount > 0)
		{
			animationFlag |= (1 << ANIMATION_ROTATION_EX);
			trsvAnimation[ANIMATION_ROTATION_EX] = new std::vector<float>();
			getKeyframes(animationCurve, *trsvAnimation[ANIMATION_ROTATION_EX]);
		}
	}

	animationCurve = node->LclRotation.GetCurve(animationLayer, FBXSDK_CURVENODE_COMPONENT_Y);
	if (animationCurve)
	{
		int keyCount = animationCurve->KeyGetCount();
		if (keyCount > 0)
		{
			animationFlag |= (1 << ANIMATION_ROTATION_EY);
			trsvAnimation[ANIMATION_ROTATION_EY] = new std::vector<float>();
			getKeyframes(animationCurve, *trsvAnimation[ANIMATION_ROTATION_EY]);
		}
	}

	animationCurve = node->LclRotation.GetCurve(animationLayer, FBXSDK_CURVENODE_COMPONENT_Z);
	if (animationCurve)
	{
		int keyCount = animationCurve->KeyGetCount();
		if (keyCount > 0)
		{
			animationFlag |= (1 << ANIMATION_ROTATION_EZ);
			trsvAnimation[ANIMATION_ROTATION_EZ] = new std::vector<float>();
			getKeyframes(animationCurve, *trsvAnimation[ANIMATION_ROTATION_EZ]);
		}
	}

	// Scaling
	animationCurve = node->LclScaling.GetCurve(animationLayer, FBXSDK_CURVENODE_COMPONENT_X);
	if (animationCurve)
	{
		int keyCount = animationCurve->KeyGetCount();
		if (keyCount > 0)
		{
			animationFlag |= (1 << ANIMATION_SCALING_X);
			trsvAnimation[ANIMATION_SCALING_X] = new std::vector<float>();
			getKeyframes(animationCurve, *trsvAnimation[ANIMATION_SCALING_X]);
		}
	}

	animationCurve = node->LclScaling.GetCurve(animationLayer, FBXSDK_CURVENODE_COMPONENT_Y);
	if (animationCurve)
	{
		int keyCount = animationCurve->KeyGetCount();
		if (keyCount > 0)
		{
			animationFlag |= (1 << ANIMATION_SCALING_Y);
			trsvAnimation[ANIMATION_SCALING_Y] = new std::vector<float>();
			getKeyframes(animationCurve, *trsvAnimation[ANIMATION_SCALING_Y]);
		}
	}

	animationCurve = node->LclScaling.GetCurve(animationLayer, FBXSDK_CURVENODE_COMPONENT_Z);
	if (animationCurve)
	{
		int keyCount = animationCurve->KeyGetCount();
		if (keyCount > 0)
		{
			animationFlag |= (1 << ANIMATION_SCALING_Z);
			trsvAnimation[ANIMATION_SCALING_Z] = new std::vector<float>();
			getKeyframes(animationCurve, *trsvAnimation[ANIMATION_SCALING_Z]);
		}
	}

	//VisibilityInheritance
	animationCurve = node->Visibility.GetCurve(animationLayer);
	if (animationCurve)
	{
		int keyCount = animationCurve->KeyGetCount();
		if (keyCount > 0)
		{
			animationFlag |= (1 << ANIMATION_VISIBILITY);
			trsvAnimation[ANIMATION_VISIBILITY] = new std::vector<float>();
			getKeyframes(animationCurve, *trsvAnimation[ANIMATION_VISIBILITY]);
		}
	}

	if (animationFlag > 0)
	{
		return trsvAnimation;
	}
	else
	{
		delete[] trsvAnimation;
		return NULL;
	}
}

std::vector<morphTargetAnimation*>* FBXAnimationExporter_V4::getMophAnimationData(
	FBXSDK_NAMESPACE::FbxAnimLayer* animationLayer,
	FBXSDK_NAMESPACE::FbxMesh* mesh)
{	
	std::vector<morphTargetAnimation*>* morph = NULL;

	FbxBlendShape* blendShape = (FbxBlendShape*)mesh->GetDeformer(0, FbxDeformer::eBlendShape);
	int channelCount = blendShape->GetBlendShapeChannelCount();
	for (int channelIndex = 0; channelIndex < channelCount; ++channelIndex)
	{
		FBXSDK_NAMESPACE::FbxAnimCurve* animationCurve = mesh->GetShapeChannel(0, channelIndex, animationLayer);
		if (animationCurve != NULL)
		{
			unsigned int keyCount = (unsigned int)animationCurve->KeyGetCount();
			if (keyCount > 0)
			{
				if (morph == NULL)
				{
					morph = new std::vector<morphTargetAnimation*>();
				}

				morphTargetAnimation* data = new morphTargetAnimation();
				data->channelIndex = channelIndex;

				FbxBlendShapeChannel* channel = blendShape->GetBlendShapeChannel(channelIndex);
				if (channel != NULL)
				{
					data->channelName = channel->GetName();
				}

				getKeyframes(animationCurve, data->animations);
				morph->push_back(data);
			}			
		}
	}

	return morph;
}

void FBXAnimationExporter_V4::getKeyframes(FBXSDK_NAMESPACE::FbxAnimCurve* animationCurve, std::vector<float>& keyframes)
{
	//FBXSDK_NAMESPACE::FbxProperty prop = animationCurve->FindProperty("KeyTime");
	//FBXSDK_NAMESPACE::FbxString name = prop.GetName();
	//FBXSDK_NAMESPACE::FbxUInt64 id = animationCurve->GetUniqueID();
	unsigned int keyCount = (unsigned int)animationCurve->KeyGetCount();
	keyframes.resize(2 * keyCount);
	keyFrameCount += keyCount;
	for (int keyIndex = 0; keyIndex < (int)keyCount; keyIndex++)
	{		
		//FBXSDK_NAMESPACE::FbxAnimCurveKey key = animationCurve->KeyGet(keyIndex);
		//FBXSDK_NAMESPACE::FbxTime time0 = key.GetTime();
		FBXSDK_NAMESPACE::FbxTime time = animationCurve->KeyGetTime(keyIndex);
		float frame = (float)time.GetFrameCountPrecise(mTimeMode);
		float value = (float)animationCurve->KeyGetValue(keyIndex);		
		keyframes[keyIndex] = (isnan(frame) ? 0.0f : frame);
		keyframes[keyCount + keyIndex] = (isnan(value) ? 0.0f : value);
	}
}