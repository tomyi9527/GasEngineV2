#include "stdafx.h"
#include "FBXAnimationExporter_V2.h"
#include "JSONFileWriter.h"

FBXAnimationExporter_V2::FBXAnimationExporter_V2()
	: mSceneStructureJSONWriter(NULL)
{
}


FBXAnimationExporter_V2::~FBXAnimationExporter_V2()
{
}

bool FBXAnimationExporter_V2::init(const string& path, FbxManager* manager, FbxScene* scene, std::map<std::string, unsigned int>& convertedFiles)
{
	std::string filePath = path + ".default.animation.json";

	convertedFiles[filePath] = 1;

	mSceneStructureJSONWriter = new JSONFileWriter();
	mSceneStructureJSONWriter->openExportFile(filePath);

	exportAnimationToJSON(scene);

	mSceneStructureJSONWriter->closeExportFile();
	delete mSceneStructureJSONWriter;
	mSceneStructureJSONWriter = NULL;

	return true;
}

void FBXAnimationExporter_V2::finl()
{
}

void FBXAnimationExporter_V2::exportAnimationToJSON(FbxScene* scene)
{
	FbxAxisSystem sceneAxisSystem = scene->GetGlobalSettings().GetAxisSystem();
	FbxSystemUnit sceneSystemUnit = scene->GetGlobalSettings().GetSystemUnit();
	//double scaleFactor = sceneSystemUnit.GetScaleFactor();

	//FbxTime::EMode timeMode = scene->GetGlobalSettings().GetTimeMode();
	//FbxTime::EProtocol timeProtocol = scene->GetGlobalSettings().GetTimeProtocol();
	//double frameRate = FbxTime::GetFrameRate(timeMode);

	int animationStackCount = scene->GetSrcObjectCount<FbxAnimStack>();
	if (animationStackCount > 1)
	{
		FbxString lOutputString = "Only support 1 animation stack!\n";
		FBXSDK_printf("%s", (const char*)lOutputString);
	}
	else if (animationStackCount <= 0)
	{
		return;
	}

	//for (int i = 0; i < animationStackCount; i++)
	{
		FbxAnimStack* lAnimStack = scene->GetSrcObject<FbxAnimStack>(0);
		if (lAnimStack == NULL)
			return;

		//FbxTimeSpan timeSpan = lAnimStack->GetReferenceTimeSpan();
		//double startTime = timeSpan.GetStart().GetSecondDouble();
		//double stopTime = timeSpan.GetStop().GetSecondDouble();

		FbxString lOutputString = "Animation Stack Name: ";
		lOutputString += lAnimStack->GetName();
		lOutputString += "\n\n";
		FBXSDK_printf("%s", (const char*)lOutputString);

		FbxNode* rootNode = scene->GetRootNode();

		processAnimation(lAnimStack, rootNode);
	}
}

void FBXAnimationExporter_V2::processAnimation(FbxAnimStack* pAnimStack, FbxNode* node)
{
	int nbAnimLayers = pAnimStack->GetMemberCount<FbxAnimLayer>();
	FbxString lOutputString;

	lOutputString = "Animation stack contains ";
	lOutputString += nbAnimLayers;
	lOutputString += " Animation Layer(s)\n";
	FBXSDK_printf("%s", (const char*)lOutputString);

	if (nbAnimLayers != 1)
	{
		FbxString lOutputString = "Only support 1 animation layer!\n";
		FBXSDK_printf("%s", (const char*)lOutputString);
	}

	//for (int i = 0; i < nbAnimLayers; i++)
	{
		int i = 0;
		FbxAnimLayer* animationLayer = pAnimStack->GetMember<FbxAnimLayer>(i);

		int jsonLevel = 0;
		mSceneStructureJSONWriter->writeObjectInfo("{", jsonLevel);
		mSceneStructureJSONWriter->writeObjectInfo("\"version\":2,", jsonLevel + 1);
		mSceneStructureJSONWriter->writeObjectInfo("\"animation\":", jsonLevel + 1);
		mSceneStructureJSONWriter->writeObjectInfo("{", jsonLevel + 1);
		mSceneStructureJSONWriter->writeObjectInfo("\"clipName\":\"default\",", jsonLevel + 2);
		mSceneStructureJSONWriter->writeObjectInfo("\"fps\":30,", jsonLevel + 2);
		mSceneStructureJSONWriter->writeObjectInfo("\"nodes\":", jsonLevel + 2);
		mSceneStructureJSONWriter->writeObjectInfo("[", jsonLevel + 2); //all object keyframes		

		mAnimationNode.clear();
		gatherAnimatedNode_r(animationLayer, node);
		int animatedNodeCount = (int)mAnimationNode.size();
		if (animatedNodeCount > 0)
		{
			for (int k = 0; k < animatedNodeCount; ++k)
			{
				FBXSDK_NAMESPACE::FbxObject* animatedObject = mAnimationNode[k];
				bool lastChild = (k == (animatedNodeCount - 1));
				writeAnimationData(animationLayer, animatedObject, jsonLevel + 3, lastChild);
			}
		}

		mSceneStructureJSONWriter->writeObjectInfo("]", jsonLevel + 2); //tracks
		mSceneStructureJSONWriter->writeObjectInfo("}", jsonLevel + 1); //animations consist of multiple clips
		mSceneStructureJSONWriter->writeObjectInfo("}", jsonLevel);
	}
}

void FBXAnimationExporter_V2::writeAnimationData(FBXSDK_NAMESPACE::FbxAnimLayer* animationLayer, FBXSDK_NAMESPACE::FbxObject* object, int jsonLevel, bool lastChild)
{
	char buffer[__TEMP_BUFFER_FLOAT__];

	int translationKeyCount = 0;
	int rotationKeyCount = 0;
	int scalingKeyCount = 0;
	int morphWeightKeyCount = 0;

	bool animated = isObjectAnimated(animationLayer, object, translationKeyCount, rotationKeyCount, scalingKeyCount, morphWeightKeyCount);
	if (animated)
	{
		string nodeName = object->GetName();
		mSceneStructureJSONWriter->writeObjectInfo("{", jsonLevel);

		mSceneStructureJSONWriter->writeObjectInfo("\"name\":\"" + nodeName + "\",", jsonLevel + 1);

		FbxUInt64 id = object->GetUniqueID();
		sprintf(buffer, "\"uniqueID\":%llu,", id);
		mSceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 1);

		mSceneStructureJSONWriter->writeObjectInfo("\"keyframes\":", jsonLevel + 1);
		mSceneStructureJSONWriter->writeObjectInfo("[", jsonLevel + 1);
		
		bool isNode = object->Is<FBXSDK_NAMESPACE::FbxNode>();
		bool isMesh = object->Is<FBXSDK_NAMESPACE::FbxMesh>();
		if (isNode)
		{
			FBXSDK_NAMESPACE::FbxNode* node = (FBXSDK_NAMESPACE::FbxNode*)object;

			std::map<FbxLongLong, int> timePositionKeys;
			iteratePositionCurves(node, animationLayer, timePositionKeys);
			exportPositionKeys(node, timePositionKeys, jsonLevel + 2);

			std::map<FbxLongLong, int> timeRotationKeys;
			iterateRotationCurves(node, animationLayer, timeRotationKeys);
			exportRotationKeys(node, timeRotationKeys, jsonLevel + 2);

			std::map<FbxLongLong, int> timeScalingKeys;
			iterateScalingCurves(node, animationLayer, timeScalingKeys);
			exportScalingKeys(node, timeScalingKeys, jsonLevel + 2);
		}
		else if (isMesh)
		{
			FbxMesh* mesh = (FBXSDK_NAMESPACE::FbxMesh*)object;
			int blendShapeDeformerCount = mesh->GetDeformerCount(FbxDeformer::eBlendShape);
			if (blendShapeDeformerCount > 0) //ONLY SUPPORT ONE MORPHER
			{
				int blendShapeIndex = 0;
				FbxBlendShape* blendShape = (FbxBlendShape*)mesh->GetDeformer(blendShapeIndex, FbxDeformer::eBlendShape);

				int channelCount = blendShape->GetBlendShapeChannelCount();
				for (int channelIndex = 0; channelIndex < channelCount; ++channelIndex)
				{
					std::map<FbxLongLong, int> timeMorphWeightKeys;

					FbxBlendShapeChannel* channel = blendShape->GetBlendShapeChannel(channelIndex);
					const char* channelName = channel->GetName();
					FbxAnimCurve* aniCurve = iterateMorphWeightCurves(mesh, channelIndex, animationLayer, timeMorphWeightKeys);

					bool isLastChannel = (channelIndex == (channelCount - 1));
					exportMorphWeightKeys(channelName, channelIndex, aniCurve, timeMorphWeightKeys, jsonLevel + 2, isLastChannel);
				}
			}
		}

		mSceneStructureJSONWriter->writeObjectInfo("]", jsonLevel + 1);	//end -> keyframes

		if (lastChild)
			mSceneStructureJSONWriter->writeObjectInfo("}", jsonLevel);
		else
			mSceneStructureJSONWriter->writeObjectInfo("},", jsonLevel);
	}
}

void FBXAnimationExporter_V2::gatherAnimatedNode_r(FbxAnimLayer* animationLayer, FbxNode* node)
{
	if (node != NULL)
	{
		int translationKeyCount = 0;
		int rotationKeyCount = 0;
		int scalingKeyCount = 0;
		int morphWeightKeyCount = 0;		

		bool animated = isObjectAnimated(animationLayer, node, translationKeyCount, rotationKeyCount, scalingKeyCount, morphWeightKeyCount);
		if (animated)
		{
			mAnimationNode.push_back(node);
		}

		translationKeyCount = 0;
		rotationKeyCount = 0;
		scalingKeyCount = 0;
		morphWeightKeyCount = 0;
		if (node->GetNodeAttribute())
		{
			FbxNodeAttribute* nodeAttr = node->GetNodeAttribute();
			FbxNodeAttribute::EType nodeAttrType = nodeAttr->GetAttributeType();
			if (nodeAttrType == FbxNodeAttribute::eMesh)
			{
				FbxMesh* mesh = node->GetMesh();
				bool animated = isObjectAnimated(animationLayer, mesh, translationKeyCount, rotationKeyCount, scalingKeyCount, morphWeightKeyCount);
				if (animated)
				{
					mAnimationNode.push_back(mesh);
				}
			}
		}

		int childCount = node->GetChildCount();
		for (int i = 0; i < childCount; ++i)
		{
			FbxNode* child = node->GetChild(i);
			gatherAnimatedNode_r(animationLayer, child);
		}
	}
}

bool FBXAnimationExporter_V2::isObjectAnimated(FbxAnimLayer* animationLayer, FBXSDK_NAMESPACE::FbxObject* object, \
	int& translationKeyCount, int& rotationKeyCount, int& scalingKeyCount, int& morphWeightKeyCount)
{
	bool isNode = object->Is<FBXSDK_NAMESPACE::FbxNode>();
	bool isMesh = object->Is<FBXSDK_NAMESPACE::FbxMesh>();

	FbxAnimCurve* animationCurve = NULL;
	if (isNode)
	{
		FBXSDK_NAMESPACE::FbxNode* node = (FBXSDK_NAMESPACE::FbxNode*)object;		
		translationKeyCount = 0;
		// Translation
		animationCurve = node->LclTranslation.GetCurve(animationLayer, FBXSDK_CURVENODE_COMPONENT_X);
		if (animationCurve)
			translationKeyCount += animationCurve->KeyGetCount();

		animationCurve = node->LclTranslation.GetCurve(animationLayer, FBXSDK_CURVENODE_COMPONENT_Y);
		if (animationCurve)
			translationKeyCount += animationCurve->KeyGetCount();

		animationCurve = node->LclTranslation.GetCurve(animationLayer, FBXSDK_CURVENODE_COMPONENT_Z);
		if (animationCurve)
			translationKeyCount += animationCurve->KeyGetCount();

		// Rotation
		rotationKeyCount = 0;
		animationCurve = node->LclRotation.GetCurve(animationLayer, FBXSDK_CURVENODE_COMPONENT_X);
		if (animationCurve)
			rotationKeyCount += animationCurve->KeyGetCount();

		animationCurve = node->LclRotation.GetCurve(animationLayer, FBXSDK_CURVENODE_COMPONENT_Y);
		if (animationCurve)
			rotationKeyCount += animationCurve->KeyGetCount();

		animationCurve = node->LclRotation.GetCurve(animationLayer, FBXSDK_CURVENODE_COMPONENT_Z);
		if (animationCurve)
			rotationKeyCount += animationCurve->KeyGetCount();

		// Scaling
		scalingKeyCount = 0;
		animationCurve = node->LclScaling.GetCurve(animationLayer, FBXSDK_CURVENODE_COMPONENT_X);
		if (animationCurve)
			scalingKeyCount += animationCurve->KeyGetCount();

		animationCurve = node->LclScaling.GetCurve(animationLayer, FBXSDK_CURVENODE_COMPONENT_Y);
		if (animationCurve)
			scalingKeyCount += animationCurve->KeyGetCount();

		animationCurve = node->LclScaling.GetCurve(animationLayer, FBXSDK_CURVENODE_COMPONENT_Z);
		if (animationCurve)
			scalingKeyCount += animationCurve->KeyGetCount();
	}
	else if (isMesh)
	{
		FbxMesh* mesh = (FBXSDK_NAMESPACE::FbxMesh*)object;
		int blendShapeDeformerCount = mesh->GetDeformerCount(FbxDeformer::eBlendShape);
		if (blendShapeDeformerCount > 0) //ONLY SUPPORT ONE MORPHER
		{
			int blendShapeIndex = 0;
			FbxBlendShape* blendShape = (FbxBlendShape*)mesh->GetDeformer(blendShapeIndex, FbxDeformer::eBlendShape);

			int channelCount = blendShape->GetBlendShapeChannelCount();
			for (int channelIndex = 0; channelIndex < channelCount; ++channelIndex)
			{
				FbxBlendShapeChannel* channel = blendShape->GetBlendShapeChannel(channelIndex);
				if (channel)
				{
					animationCurve = mesh->GetShapeChannel(blendShapeIndex, channelIndex, animationLayer);
					if (animationCurve)
						morphWeightKeyCount += animationCurve->KeyGetCount();
				}
			}
		}
	}

	return ((translationKeyCount + rotationKeyCount + scalingKeyCount + morphWeightKeyCount) != 0);
}

FbxAnimCurve* FBXAnimationExporter_V2::iterateMorphWeightCurves(FbxMesh* mesh, int channelIndex, FbxAnimLayer* pAnimLayer, std::map<FbxLongLong, int>& timeKeys)
{
	int blendShapeDeformerCount = mesh->GetDeformerCount(FbxDeformer::eBlendShape);
	if (blendShapeDeformerCount > 0) //ONLY SUPPORT ONE MORPHER
	{
		int blendShapeIndex = 0;
		FbxBlendShape* blendShape = (FbxBlendShape*)mesh->GetDeformer(blendShapeIndex, FbxDeformer::eBlendShape);

		FbxBlendShapeChannel* channel = blendShape->GetBlendShapeChannel(channelIndex);
		if (channel)
		{
			FbxAnimCurve* lAnimCurve = mesh->GetShapeChannel(blendShapeIndex, channelIndex, pAnimLayer);
			if (lAnimCurve)
			{
				gatherAllTimeKeys(lAnimCurve, timeKeys);
				return lAnimCurve;
			}
		}
	}

	return NULL;
}

void FBXAnimationExporter_V2::iteratePositionCurves(FbxNode* node, FbxAnimLayer* pAnimLayer, std::map<FbxLongLong, int>& timeKeys)
{
	FbxAnimCurve* lAnimCurve = NULL;
	lAnimCurve = node->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
	if (lAnimCurve)
	{
		gatherAllTimeKeys(lAnimCurve, timeKeys);
	}

	lAnimCurve = node->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
	if (lAnimCurve)
	{
		gatherAllTimeKeys(lAnimCurve, timeKeys);
	}

	lAnimCurve = node->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
	if (lAnimCurve)
	{
		gatherAllTimeKeys(lAnimCurve, timeKeys);
	}
}

void FBXAnimationExporter_V2::iterateRotationCurves(FbxNode* node, FbxAnimLayer* pAnimLayer, std::map<FbxLongLong, int>& timeKeys)
{
	FbxAnimCurve* lAnimCurve = NULL;
	lAnimCurve = node->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
	if (lAnimCurve)
	{
		gatherAllTimeKeys(lAnimCurve, timeKeys);
	}

	lAnimCurve = node->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
	if (lAnimCurve)
	{
		gatherAllTimeKeys(lAnimCurve, timeKeys);
	}

	lAnimCurve = node->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
	if (lAnimCurve)
	{
		gatherAllTimeKeys(lAnimCurve, timeKeys);
	}
}

void FBXAnimationExporter_V2::iterateScalingCurves(FbxNode* node, FbxAnimLayer* pAnimLayer, std::map<FbxLongLong, int>& timeKeys)
{
	FbxAnimCurve* lAnimCurve = NULL;
	lAnimCurve = node->LclScaling.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
	if (lAnimCurve)
	{
		gatherAllTimeKeys(lAnimCurve, timeKeys);
	}

	lAnimCurve = node->LclScaling.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
	if (lAnimCurve)
	{
		gatherAllTimeKeys(lAnimCurve, timeKeys);
	}

	lAnimCurve = node->LclScaling.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
	if (lAnimCurve)
	{
		gatherAllTimeKeys(lAnimCurve, timeKeys);
	}
}

void FBXAnimationExporter_V2::gatherAllTimeKeys(FbxAnimCurve* lAnimCurve, std::map<FbxLongLong, int>& timeKeys)
{
	if (lAnimCurve)
	{
		int lKeyCount = lAnimCurve->KeyGetCount();

		for (int lCount = 0; lCount < lKeyCount; lCount++)
		{
			FbxTime lKeyTime = lAnimCurve->KeyGetTime(lCount);
			FbxLongLong msTime = lKeyTime.GetMilliSeconds();
			std::map<FbxLongLong, int>::iterator iter = timeKeys.find(msTime);
			if (iter != timeKeys.end())
			{
				int c = timeKeys[msTime];
				timeKeys[msTime] = c + 1;
			}
			else
			{
				timeKeys[msTime] = 1;
			}
		}
	}
}

void FBXAnimationExporter_V2::exportMorphWeightKeys(const char* channelName, int channelIndex, FbxAnimCurve* aniCurve, std::map<FbxLongLong, int>& timeMorphWeightKeys, int jsonLevel, bool isLastChannel)
{
	char buffer[__TEMP_BUFFER_FLOAT__];
	sprintf(buffer, "\"channelName\":\"%s\",", channelName);

	mSceneStructureJSONWriter->writeObjectInfo("{", jsonLevel);
	mSceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel + 1);
	mSceneStructureJSONWriter->writeObjectInfo("\"target\":\"morphWeight\",", jsonLevel + 1);
	mSceneStructureJSONWriter->writeObjectInfo("\"keyType\":\"float\",", jsonLevel + 1);
	mSceneStructureJSONWriter->writeObjectInfo("\"keys\":", jsonLevel + 1);
	mSceneStructureJSONWriter->writeObjectInfo("[", jsonLevel + 1);

	writeMorphWeightKeysInJSON(aniCurve, timeMorphWeightKeys, jsonLevel + 2);

	mSceneStructureJSONWriter->writeObjectInfo("]", jsonLevel + 1);
	if (isLastChannel)
	{
		mSceneStructureJSONWriter->writeObjectInfo("}", jsonLevel);
	}
	else
	{
		mSceneStructureJSONWriter->writeObjectInfo("},", jsonLevel);
	}
}

void FBXAnimationExporter_V2::writeMorphWeightKeysInJSON(FbxAnimCurve* aniCurve, std::map<FbxLongLong, int>& timeMorphWeightKeys, int jsonLevel)
{
	char buffer[__TEMP_BUFFER_FLOAT__];
	std::map<FbxLongLong, int>::iterator iter = timeMorphWeightKeys.begin();
	while (iter != timeMorphWeightKeys.end())
	{
		double timeValue = (double)(iter->first) / 1000.0;

		string keyPairStr = "{";

		memset(buffer, 0, __TEMP_BUFFER_FLOAT__);
		sprintf(buffer, "%0.3f", timeValue);
		keyPairStr = keyPairStr + "\"time\":" + buffer + ",";

		FbxTime time;
		time.SetMilliSeconds(iter->first);
		float weight = aniCurve->Evaluate(time);
		sprintf(buffer, "%0.3f", weight);

		keyPairStr = keyPairStr + " \"value\":" + buffer;
		keyPairStr += "}";

		++iter;
		if (iter != timeMorphWeightKeys.end())
		{
			keyPairStr += ",";
		}

		mSceneStructureJSONWriter->writeObjectInfo(keyPairStr, jsonLevel);
	}
}

void FBXAnimationExporter_V2::exportPositionKeys(FbxNode* node, std::map<FbxLongLong, int>& timePositionKeys, int jsonLevel)
{
	mSceneStructureJSONWriter->writeObjectInfo("{", jsonLevel);
	mSceneStructureJSONWriter->writeObjectInfo("\"target\":\"position\",", jsonLevel + 1);
	mSceneStructureJSONWriter->writeObjectInfo("\"keyType\":\"float4\",", jsonLevel + 1);
	mSceneStructureJSONWriter->writeObjectInfo("\"keys\":", jsonLevel + 1);
	mSceneStructureJSONWriter->writeObjectInfo("[", jsonLevel + 1);

	writeKeysInJSON(node, timePositionKeys, 0, jsonLevel + 2);

	mSceneStructureJSONWriter->writeObjectInfo("]", jsonLevel + 1);
	mSceneStructureJSONWriter->writeObjectInfo("},", jsonLevel);
}

void FBXAnimationExporter_V2::exportRotationKeys(FbxNode* node, std::map<FbxLongLong, int>& timeRotationKeys, int jsonLevel)
{
	mSceneStructureJSONWriter->writeObjectInfo("{", jsonLevel);
	mSceneStructureJSONWriter->writeObjectInfo("\"target\":\"rotation\",", jsonLevel + 1);
	mSceneStructureJSONWriter->writeObjectInfo("\"keyType\":\"float4\",", jsonLevel + 1);
	mSceneStructureJSONWriter->writeObjectInfo("\"keys\":", jsonLevel + 1);
	mSceneStructureJSONWriter->writeObjectInfo("[", jsonLevel + 1);

	writeKeysInJSON(node, timeRotationKeys, 1, jsonLevel + 2);

	mSceneStructureJSONWriter->writeObjectInfo("]", jsonLevel + 1);
	mSceneStructureJSONWriter->writeObjectInfo("},", jsonLevel);
}

void FBXAnimationExporter_V2::exportScalingKeys(FbxNode* node, std::map<FbxLongLong, int>& timeScalingKeys, int jsonLevel)
{
	mSceneStructureJSONWriter->writeObjectInfo("{", jsonLevel);
	mSceneStructureJSONWriter->writeObjectInfo("\"target\":\"scaling\",", jsonLevel + 1);
	mSceneStructureJSONWriter->writeObjectInfo("\"keyType\":\"float4\",", jsonLevel + 1);
	mSceneStructureJSONWriter->writeObjectInfo("\"keys\":", jsonLevel + 1);
	mSceneStructureJSONWriter->writeObjectInfo("[", jsonLevel + 1);

	writeKeysInJSON(node, timeScalingKeys, 2, jsonLevel + 2);

	mSceneStructureJSONWriter->writeObjectInfo("]", jsonLevel + 1);

	mSceneStructureJSONWriter->writeObjectInfo("}", jsonLevel);
}

void FBXAnimationExporter_V2::writeKeysInJSON(FbxNode* node, std::map<FbxLongLong, int>& timePositionKeys, int type, int jsonLevel)
{
	char buffer[__TEMP_BUFFER_FLOAT__];
	std::map<FbxLongLong, int>::iterator iter = timePositionKeys.begin();
	while (iter != timePositionKeys.end())
	{
		double timeValue = (double)(iter->first) / 1000.0;
		//int count = iter->second;

		string keyPairStr = "{";

		memset(buffer, 0, __TEMP_BUFFER_FLOAT__);
		sprintf(buffer, "%0.3f", timeValue);
		keyPairStr = keyPairStr + "\"time\":" + buffer + ",";

		FbxTime time;
		time.SetMilliSeconds(iter->first);
		FbxAMatrix& localMatrix = node->EvaluateLocalTransform(time);
		FbxVector4 tr = localMatrix.GetT();
		FbxQuaternion rt = localMatrix.GetQ();
		FbxVector4 sc = localMatrix.GetS();

		memset(buffer, 0, __TEMP_BUFFER_FLOAT__);
		switch (type)
		{
		case 0:
			sprintf(buffer, "[%0.3f,%0.3f,%0.3f,%0.3f]", tr[0], tr[1], tr[2], tr[3]);
			break;
		case 1:
			sprintf(buffer, "[%0.6f,%0.6f,%0.6f,%0.6f]", rt[0], rt[1], rt[2], rt[3]);
			break;
		case 2:
			sprintf(buffer, "[%0.3f,%0.3f,%0.3f,%0.3f]", sc[0], sc[1], sc[2], sc[3]);
			break;
		default:
			break;
		}

		keyPairStr = keyPairStr + " \"value\":" + buffer;
		keyPairStr += "}";

		++iter;
		if (iter != timePositionKeys.end())
		{
			keyPairStr += ",";
		}
		mSceneStructureJSONWriter->writeObjectInfo(keyPairStr, jsonLevel);
	}
}