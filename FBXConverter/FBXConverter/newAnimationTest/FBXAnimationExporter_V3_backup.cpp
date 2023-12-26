#include "stdafx.h"
#include "FBXAnimationExporter.h"
#include "JSONFileWriter.h"

FBXAnimationExporter::FBXAnimationExporter()
	: mSceneStructureJSONWriter(NULL)
{
}


FBXAnimationExporter::~FBXAnimationExporter()
{
}

bool FBXAnimationExporter::init(const string& path, FbxManager* manager, FbxScene* scene, std::map<std::string, unsigned int>& convertedFiles)
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

void FBXAnimationExporter::finl()
{
}

void FBXAnimationExporter::exportAnimationToJSON(FbxScene* scene)
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
		FbxString lOutputString = "Only support 1 animation stack!";
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

void FBXAnimationExporter::processAnimation(FbxAnimStack* pAnimStack, FbxNode* node)
{
	int nbAnimLayers = pAnimStack->GetMemberCount<FbxAnimLayer>();
	FbxString lOutputString;

	lOutputString = "Animation stack contains ";
	lOutputString += nbAnimLayers;
	lOutputString += " Animation Layer(s)\n";
	FBXSDK_printf("%s", (const char*)lOutputString);

	if (nbAnimLayers != 1)
	{
		FbxString lOutputString = "Only support 1 animation layer!";
		FBXSDK_printf("%s", (const char*)lOutputString);
		return;
	}

	//for (int i = 0; i < nbAnimLayers; i++)
	{
		int i = 0;
		FbxAnimLayer* animationLayer = pAnimStack->GetMember<FbxAnimLayer>(i);

		int jsonLevel = 0;
		mSceneStructureJSONWriter->writeObjectInfo("{", jsonLevel);
		mSceneStructureJSONWriter->writeObjectInfo("\"version\":3,", jsonLevel + 1);
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

void FBXAnimationExporter::writeAnimationData(FBXSDK_NAMESPACE::FbxAnimLayer* animationLayer, FBXSDK_NAMESPACE::FbxObject* object, int jsonLevel, bool lastChild)
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

			std::map<FbxLongLong, float> postion_x_component;
			std::map<FbxLongLong, float> postion_y_component;
			std::map<FbxLongLong, float> postion_z_component;
			iteratePositionCurves(node, animationLayer, postion_x_component, postion_y_component, postion_z_component);

			std::map<FbxLongLong, float> rotation_x_component;
			std::map<FbxLongLong, float> rotation_y_component;
			std::map<FbxLongLong, float> rotation_z_component;
			iterateRotationCurves(node, animationLayer, rotation_x_component, rotation_y_component, rotation_z_component);

			std::map<FbxLongLong, float> scaling_x_component;
			std::map<FbxLongLong, float> scaling_y_component;
			std::map<FbxLongLong, float> scaling_z_component;
			iterateScalingCurves(node, animationLayer, scaling_x_component, scaling_y_component, scaling_z_component);

			exportKeys(node, "positionX", postion_x_component, jsonLevel + 2, false);
			exportKeys(node, "positionY", postion_y_component, jsonLevel + 2, false);
			exportKeys(node, "positionZ", postion_z_component, jsonLevel + 2, false);

			exportKeys(node, "rotationX", rotation_x_component, jsonLevel + 2, false);
			exportKeys(node, "rotationY", rotation_y_component, jsonLevel + 2, false);
			exportKeys(node, "rotationZ", rotation_z_component, jsonLevel + 2, false);

			exportKeys(node, "scalingX", scaling_x_component, jsonLevel + 2, false);
			exportKeys(node, "scalingY", scaling_y_component, jsonLevel + 2, false);
			exportKeys(node, "scalingZ", scaling_z_component, jsonLevel + 2, true);
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
					std::map<FbxLongLong, float> timeMorphWeightKeys;

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

void FBXAnimationExporter::gatherAnimatedNode_r(FbxAnimLayer* animationLayer, FbxNode* node)
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

bool FBXAnimationExporter::isObjectAnimated(FbxAnimLayer* animationLayer, FBXSDK_NAMESPACE::FbxObject* object, \
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
		//VisibilityInheritance
		animationCurve = node->Visibility.GetCurve(animationLayer);
		if (animationCurve)
		{
			int visibilityKeyCount = animationCurve->KeyGetCount();
			int jj = 0;
		}
	}
	else if (isMesh)
	{
		FBXSDK_NAMESPACE::FbxMesh* mesh = (FBXSDK_NAMESPACE::FbxMesh*)object;
		int blendShapeDeformerCount = mesh->GetDeformerCount(FbxDeformer::eBlendShape);
		if (blendShapeDeformerCount > 0) //ONLY SUPPORT ONE MORPHER
		{
			int blendShapeIndex = 0;
			FBXSDK_NAMESPACE::FbxBlendShape* blendShape = (FBXSDK_NAMESPACE::FbxBlendShape*)mesh->GetDeformer(blendShapeIndex, FBXSDK_NAMESPACE::FbxDeformer::eBlendShape);

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

void FBXAnimationExporter::iteratePositionCurves(FbxNode* node, FbxAnimLayer* pAnimLayer, 
	std::map<FbxLongLong, float>& positionX, std::map<FbxLongLong, float>& positionY, std::map<FbxLongLong, float>& positionZ)
{
	FbxAnimCurve* lAnimCurve = NULL;
	lAnimCurve = node->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
	if (lAnimCurve)
	{
		gatherAllTimeKeys(lAnimCurve, positionX);
	}

	lAnimCurve = node->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
	if (lAnimCurve)
	{
		gatherAllTimeKeys(lAnimCurve, positionY);
	}

	lAnimCurve = node->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
	if (lAnimCurve)
	{
		gatherAllTimeKeys(lAnimCurve, positionZ);
	}
}

void FBXAnimationExporter::iterateRotationCurves(FbxNode* node, FbxAnimLayer* pAnimLayer,
	std::map<FbxLongLong, float>& rotationX, std::map<FbxLongLong, float>& rotationY, std::map<FbxLongLong, float>& rotationZ)
{
	FbxAnimCurve* lAnimCurve = NULL;
	lAnimCurve = node->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
	if (lAnimCurve)
	{
		gatherAllTimeKeys(lAnimCurve, rotationX);
	}

	lAnimCurve = node->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
	if (lAnimCurve)
	{
		gatherAllTimeKeys(lAnimCurve, rotationY);
	}

	lAnimCurve = node->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
	if (lAnimCurve)
	{
		gatherAllTimeKeys(lAnimCurve, rotationZ);
	}
}

void FBXAnimationExporter::iterateScalingCurves(FbxNode* node, FbxAnimLayer* pAnimLayer,
	std::map<FbxLongLong, float>& scalingX, std::map<FbxLongLong, float>& scalingY, std::map<FbxLongLong, float>& scalingZ)
{
	FbxAnimCurve* lAnimCurve = NULL;
	lAnimCurve = node->LclScaling.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
	if (lAnimCurve)
	{
		gatherAllTimeKeys(lAnimCurve, scalingX);
	}

	lAnimCurve = node->LclScaling.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
	if (lAnimCurve)
	{
		gatherAllTimeKeys(lAnimCurve, scalingY);
	}

	lAnimCurve = node->LclScaling.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
	if (lAnimCurve)
	{
		gatherAllTimeKeys(lAnimCurve, scalingZ);
	}
}

void FBXAnimationExporter::gatherAllTimeKeys(FbxAnimCurve* lAnimCurve, std::map<FbxLongLong, float>& timeKeys)
{
	if (lAnimCurve)
	{
		int lKeyCount = lAnimCurve->KeyGetCount();

		for (int lCount = 0; lCount < lKeyCount; lCount++)
		{
			FbxTime lKeyTime = lAnimCurve->KeyGetTime(lCount);
			float lKeyValue = static_cast<float>(lAnimCurve->KeyGetValue(lCount));
			FbxLongLong msTime = lKeyTime.GetMilliSeconds();
			std::map<FbxLongLong, float>::iterator iter = timeKeys.find(msTime);
			if (iter != timeKeys.end())
			{
				FBXSDK_printf("%s", "Animation data error: it is abnormal to find two key frame with the same timing!");
			}
			else
			{
				timeKeys[msTime] = lKeyValue;
			}
		}
	}
}

void FBXAnimationExporter::exportKeys(FbxNode* node, std::string elementName, std::map<FbxLongLong, float>& keyframes, int jsonLevel, bool isLastElement)
{
	std::string elementAttribute = "\"target\":\"" + elementName + "\",";

	mSceneStructureJSONWriter->writeObjectInfo("{", jsonLevel);
	mSceneStructureJSONWriter->writeObjectInfo(elementAttribute, jsonLevel + 1);
	mSceneStructureJSONWriter->writeObjectInfo("\"type\":\"float\",", jsonLevel + 1);
	mSceneStructureJSONWriter->writeObjectInfo("\"keys\":", jsonLevel + 1);
	mSceneStructureJSONWriter->writeObjectInfo("[", jsonLevel + 1);

	writeKeysInJSON(node, keyframes, jsonLevel + 2);

	mSceneStructureJSONWriter->writeObjectInfo("]", jsonLevel + 1);

	if (isLastElement)
	{
		mSceneStructureJSONWriter->writeObjectInfo("}", jsonLevel);
	}
	else
	{
		mSceneStructureJSONWriter->writeObjectInfo("},", jsonLevel);
	}	
}

//void FBXAnimationExporter::exportPositionKeys(FbxNode* node,
//	std::map<FbxLongLong, float>& positionX, std::map<FbxLongLong, float>& positionY, std::map<FbxLongLong, float>& positionZ, int jsonLevel)
//{
//	//POSITION X
//	mSceneStructureJSONWriter->writeObjectInfo("{", jsonLevel);
//	mSceneStructureJSONWriter->writeObjectInfo("\"target\":\"positionX\",", jsonLevel + 1);
//	mSceneStructureJSONWriter->writeObjectInfo("\"keyType\":\"float\",", jsonLevel + 1);
//	mSceneStructureJSONWriter->writeObjectInfo("\"keys\":", jsonLevel + 1);
//	mSceneStructureJSONWriter->writeObjectInfo("[", jsonLevel + 1);
//
//	writeKeysInJSON(node, positionX, jsonLevel + 2);
//
//	mSceneStructureJSONWriter->writeObjectInfo("]", jsonLevel + 1);
//	mSceneStructureJSONWriter->writeObjectInfo("},", jsonLevel);
//
//	//POSITION Y
//	mSceneStructureJSONWriter->writeObjectInfo("{", jsonLevel);
//	mSceneStructureJSONWriter->writeObjectInfo("\"target\":\"positionY\",", jsonLevel + 1);
//	mSceneStructureJSONWriter->writeObjectInfo("\"keyType\":\"float\",", jsonLevel + 1);
//	mSceneStructureJSONWriter->writeObjectInfo("\"keys\":", jsonLevel + 1);
//	mSceneStructureJSONWriter->writeObjectInfo("[", jsonLevel + 1);
//
//	writeKeysInJSON(node, positionY, jsonLevel + 2);
//
//	mSceneStructureJSONWriter->writeObjectInfo("]", jsonLevel + 1);
//	mSceneStructureJSONWriter->writeObjectInfo("},", jsonLevel);
//
//	//POSITION Z
//	mSceneStructureJSONWriter->writeObjectInfo("{", jsonLevel);
//	mSceneStructureJSONWriter->writeObjectInfo("\"target\":\"positionZ\",", jsonLevel + 1);
//	mSceneStructureJSONWriter->writeObjectInfo("\"keyType\":\"float\",", jsonLevel + 1);
//	mSceneStructureJSONWriter->writeObjectInfo("\"keys\":", jsonLevel + 1);
//	mSceneStructureJSONWriter->writeObjectInfo("[", jsonLevel + 1);
//
//	writeKeysInJSON(node, positionZ, jsonLevel + 2);
//
//	mSceneStructureJSONWriter->writeObjectInfo("]", jsonLevel + 1);
//	mSceneStructureJSONWriter->writeObjectInfo("},", jsonLevel);
//}
//
//void FBXAnimationExporter::exportRotationKeys(FbxNode* node, 
//	std::map<FbxLongLong, float>& rotationX, std::map<FbxLongLong, float>& rotationY, std::map<FbxLongLong, float>& rotationZ, int jsonLevel)
//{
//	mSceneStructureJSONWriter->writeObjectInfo("{", jsonLevel);
//	mSceneStructureJSONWriter->writeObjectInfo("\"target\":\"rotationX\",", jsonLevel + 1);
//	mSceneStructureJSONWriter->writeObjectInfo("\"keyType\":\"float\",", jsonLevel + 1);
//	mSceneStructureJSONWriter->writeObjectInfo("\"keys\":", jsonLevel + 1);
//	mSceneStructureJSONWriter->writeObjectInfo("[", jsonLevel + 1);
//
//	writeKeysInJSON(node, rotationX, jsonLevel + 2);
//
//	mSceneStructureJSONWriter->writeObjectInfo("]", jsonLevel + 1);
//	mSceneStructureJSONWriter->writeObjectInfo("},", jsonLevel);
//
//	mSceneStructureJSONWriter->writeObjectInfo("{", jsonLevel);
//	mSceneStructureJSONWriter->writeObjectInfo("\"target\":\"rotationY\",", jsonLevel + 1);
//	mSceneStructureJSONWriter->writeObjectInfo("\"keyType\":\"float\",", jsonLevel + 1);
//	mSceneStructureJSONWriter->writeObjectInfo("\"keys\":", jsonLevel + 1);
//	mSceneStructureJSONWriter->writeObjectInfo("[", jsonLevel + 1);
//
//	writeKeysInJSON(node, rotationY, jsonLevel + 2);
//
//	mSceneStructureJSONWriter->writeObjectInfo("]", jsonLevel + 1);
//	mSceneStructureJSONWriter->writeObjectInfo("},", jsonLevel);
//}
//
//void FBXAnimationExporter::exportScalingKeys(FbxNode* node,
//	std::map<FbxLongLong, float>& scalingX, std::map<FbxLongLong, float>& scalingY, std::map<FbxLongLong, float>& scalingZ, int jsonLevel)
//{
//	mSceneStructureJSONWriter->writeObjectInfo("{", jsonLevel);
//	mSceneStructureJSONWriter->writeObjectInfo("\"target\":\"scaling\",", jsonLevel + 1);
//	mSceneStructureJSONWriter->writeObjectInfo("\"keyType\":\"float4\",", jsonLevel + 1);
//	mSceneStructureJSONWriter->writeObjectInfo("\"keys\":", jsonLevel + 1);
//	mSceneStructureJSONWriter->writeObjectInfo("[", jsonLevel + 1);
//
//	writeKeysInJSON(node, timeScalingKeys, 2, jsonLevel + 2);
//
//	mSceneStructureJSONWriter->writeObjectInfo("]", jsonLevel + 1);
//
//	mSceneStructureJSONWriter->writeObjectInfo("}", jsonLevel);
//}

void FBXAnimationExporter::writeKeysInJSON(FbxNode* node, std::map<FbxLongLong, float>& keyframes, int jsonLevel)
{
	char buffer[__TEMP_BUFFER_FLOAT__];
	std::map<FbxLongLong, float>::iterator iter = keyframes.begin();
	while (iter != keyframes.end())
	{
		double keyframeTime = (double)(iter->first) / 1000.0;
		float keyframeValue = iter->second;
		memset(buffer, 0, __TEMP_BUFFER_FLOAT__);	

		++iter;
		if (iter != keyframes.end())
		{
			sprintf(buffer, "{\"t\": %f, \"v\": %f},", keyframeTime, keyframeValue);
		}
		else
		{
			sprintf(buffer, "{\"t\": %f, \"v\": %f}", keyframeTime, keyframeValue);
		}

		mSceneStructureJSONWriter->writeObjectInfo(buffer, jsonLevel);
	}
}

FbxAnimCurve* FBXAnimationExporter::iterateMorphWeightCurves(FbxMesh* mesh, int channelIndex, FbxAnimLayer* pAnimLayer, std::map<FbxLongLong, float>& timeKeys)
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

void FBXAnimationExporter::exportMorphWeightKeys(const char* channelName, int channelIndex, FbxAnimCurve* aniCurve, std::map<FbxLongLong, float>& timeMorphWeightKeys, int jsonLevel, bool isLastChannel)
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

void FBXAnimationExporter::writeMorphWeightKeysInJSON(FbxAnimCurve* aniCurve, std::map<FbxLongLong, float>& timeMorphWeightKeys, int jsonLevel)
{
	char buffer[__TEMP_BUFFER_FLOAT__];
	std::map<FbxLongLong, float>::iterator iter = timeMorphWeightKeys.begin();
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