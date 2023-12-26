#include "stdafx.h"
#include "CommonStruct.h"
#include "VmdConverter.h"
#include "Common/Utils.h"
#include "JsonToBin_V4.h"
#include "Material.h"
#include "FBXSceneStructureExporter_V4.h"
#include "JSONFileWriter.h"

VmdConverter::VmdConverter()
{
}


VmdConverter::~VmdConverter()
{
}

bool less_second(const vmd::VmdBoneFrame& bf1, const vmd::VmdBoneFrame& bf2) 
{
	return bf1.frame < bf2.frame;
}

bool less_second0(const vmd::VmdMorphFrame& bf1, const vmd::VmdMorphFrame& bf2)
{
	return bf1.frame < bf2.frame;
}

bool VmdConverter::convert(const std::string& workingDirectory, const std::string& filePath)
{
	vmd::VmdMotion* motion = vmd::VmdMotion::LoadFromFile(filePath.c_str());

	std::map<std::string, std::vector<vmd::VmdBoneFrame>*> dict;

	for (size_t i = 0; i < motion->bone_frames.size(); ++i)
	{
		vmd::VmdBoneFrame& bf = motion->bone_frames[i];
	
		string utf8String;
		Platform_SHIFTJIS_UTF8(bf.name, utf8String);

		std::map<std::string, std::vector<vmd::VmdBoneFrame>*>::iterator iter = dict.find(utf8String);
		if (iter != dict.end())
		{
			dict[utf8String]->push_back(bf);
		}
		else
		{
			dict[utf8String] = new std::vector<vmd::VmdBoneFrame>();
			dict[utf8String]->push_back(bf);
		}
	}

	std::vector<animationClipData>* animation = new std::vector<animationClipData>();

	int minFrame = INT_MAX;
	int maxFrame = INT_MIN;

	std::map<std::string, std::vector<vmd::VmdBoneFrame>*>::iterator iter = dict.begin();
	for (; iter != dict.end(); ++iter)
	{
		std::sort(iter->second->begin(), iter->second->end(), less_second);

		animationClipData clipData;

		clipData.nodeName = standardizeFileName(iter->first);
		clipData.nodeID = 0xffffffff;
		clipData.trsvAnimation = NULL;
		clipData.morphAnimation = NULL;
		clipData.extAnimationData = NULL;

		std::vector<vmd::VmdBoneFrame>& frames = *(iter->second);
		if (frames.size() > 0)
		{
			clipData.extAnimationData = new std::vector<animationChannelData>();
			clipData.extAnimationData->resize(4);

			(*clipData.extAnimationData)[0].propertyName = "transform.position.x";
			(*clipData.extAnimationData)[0].animationTarget = ANIMATION_POSITION_X;
			(*clipData.extAnimationData)[0].keyIndexType = KEY_INDEX_INT;
			(*clipData.extAnimationData)[0].keyframeDataType = KEY_VALUE_FLOAT_BEZIER_MMD;
			(*clipData.extAnimationData)[0].keyframeDataSize = sizeof(float) + 4;
			(*clipData.extAnimationData)[0].keyframeCount = (unsigned int)frames.size();

			(*clipData.extAnimationData)[1].propertyName = "transform.position.y";
			(*clipData.extAnimationData)[1].animationTarget = ANIMATION_POSITION_Y;
			(*clipData.extAnimationData)[1].keyIndexType = KEY_INDEX_INT;
			(*clipData.extAnimationData)[1].keyframeDataType = KEY_VALUE_FLOAT_BEZIER_MMD;
			(*clipData.extAnimationData)[1].keyframeDataSize = sizeof(float) + 4;
			(*clipData.extAnimationData)[1].keyframeCount = (unsigned int)frames.size();

			(*clipData.extAnimationData)[2].propertyName = "transform.position.z";
			(*clipData.extAnimationData)[2].animationTarget = ANIMATION_POSITION_Z;
			(*clipData.extAnimationData)[2].keyIndexType = KEY_INDEX_INT;
			(*clipData.extAnimationData)[2].keyframeDataType = KEY_VALUE_FLOAT_BEZIER_MMD;
			(*clipData.extAnimationData)[2].keyframeDataSize = sizeof(float) + 4;
			(*clipData.extAnimationData)[2].keyframeCount = (unsigned int)frames.size();
			
			(*clipData.extAnimationData)[3].propertyName = "transform.rotation.quaternion";
			(*clipData.extAnimationData)[3].animationTarget = ANIMATION_ROTATION_QUATERNION;
			(*clipData.extAnimationData)[3].keyIndexType = KEY_INDEX_INT;
			(*clipData.extAnimationData)[3].keyframeDataType = KEY_VALUE_QUATERNION_BEZIER_MMD;
			(*clipData.extAnimationData)[3].keyframeDataSize = sizeof(float)*4 + 4;
			(*clipData.extAnimationData)[3].keyframeCount = (unsigned int)frames.size();

			unsigned int positionBufferSize = (unsigned int)((16 + frames.size()*sizeof(int)) + (16 + frames.size()*(sizeof(float) + 4)));
			unsigned int headerReserved = 4;

			// PositionX
			(*clipData.extAnimationData)[0].animationDataSize = positionBufferSize;
			void* positionXBuffer = malloc(positionBufferSize);
			(*clipData.extAnimationData)[0].animationData = positionXBuffer;
			*((unsigned int*)positionXBuffer + 0) = (unsigned int)sizeof(int);
			*((unsigned int*)positionXBuffer + 1) = (unsigned int)frames.size();
			*((unsigned int*)positionXBuffer + 2) = 0;
			*((unsigned int*)positionXBuffer + 3) = 0;
			*((unsigned int*)positionXBuffer + headerReserved + frames.size() + 0) = (unsigned int)sizeof(float) + 4;
			*((unsigned int*)positionXBuffer + headerReserved + frames.size() + 1) = (unsigned int)frames.size();
			*((unsigned int*)positionXBuffer + headerReserved + frames.size() + 2) = 0;
			*((unsigned int*)positionXBuffer + headerReserved + frames.size() + 3) = 0;

			// PositionY
			(*clipData.extAnimationData)[1].animationDataSize = positionBufferSize;
			void* positionYBuffer = malloc(positionBufferSize);
			(*clipData.extAnimationData)[1].animationData = positionYBuffer;
			*((unsigned int*)positionYBuffer + 0) = (unsigned int)sizeof(int);
			*((unsigned int*)positionYBuffer + 1) = (unsigned int)frames.size();
			*((unsigned int*)positionYBuffer + 2) = 0;
			*((unsigned int*)positionYBuffer + 3) = 0;
			*((unsigned int*)positionYBuffer + headerReserved + frames.size() + 0) = (unsigned int)sizeof(float) + 4;
			*((unsigned int*)positionYBuffer + headerReserved + frames.size() + 1) = (unsigned int)frames.size();
			*((unsigned int*)positionYBuffer + headerReserved + frames.size() + 2) = 0;
			*((unsigned int*)positionYBuffer + headerReserved + frames.size() + 3) = 0;

			// PositionZ
			(*clipData.extAnimationData)[2].animationDataSize = positionBufferSize;
			void* positionZBuffer = malloc(positionBufferSize);
			(*clipData.extAnimationData)[2].animationData = positionZBuffer;
			*((unsigned int*)positionZBuffer + 0) = (unsigned int)sizeof(int);
			*((unsigned int*)positionZBuffer + 1) = (unsigned int)frames.size();
			*((unsigned int*)positionZBuffer + 2) = 0;
			*((unsigned int*)positionZBuffer + 3) = 0;
			*((unsigned int*)positionZBuffer + headerReserved + frames.size() + 0) = (unsigned int)sizeof(float) + 4;
			*((unsigned int*)positionZBuffer + headerReserved + frames.size() + 1) = (unsigned int)frames.size();
			*((unsigned int*)positionZBuffer + headerReserved + frames.size() + 2) = 0;
			*((unsigned int*)positionZBuffer + headerReserved + frames.size() + 3) = 0;
			
			//Rotation
			unsigned int rotationBufferSize = (unsigned int)(16 + frames.size()*sizeof(int)) + (unsigned int)(16 + frames.size()*(sizeof(float)*4 + 4));
			(*clipData.extAnimationData)[3].animationDataSize = rotationBufferSize;
			void* rotationQBuffer = malloc(rotationBufferSize);
			(*clipData.extAnimationData)[3].animationData = rotationQBuffer;
			*((unsigned int*)rotationQBuffer + 0) = (unsigned int)sizeof(int);
			*((unsigned int*)rotationQBuffer + 1) = (unsigned int)frames.size();
			*((unsigned int*)rotationQBuffer + 2) = 0;
			*((unsigned int*)rotationQBuffer + 3) = 0;
			*((unsigned int*)rotationQBuffer + headerReserved + frames.size() + 0) = (unsigned int)sizeof(float) * 4 + 4;
			*((unsigned int*)rotationQBuffer + headerReserved + frames.size() + 1) = (unsigned int)frames.size();
			*((unsigned int*)rotationQBuffer + headerReserved + frames.size() + 2) = 0;
			*((unsigned int*)rotationQBuffer + headerReserved + frames.size() + 3) = 0;

			unsigned int offset = (unsigned int)((headerReserved + frames.size() + headerReserved) * sizeof(unsigned int));
			for (size_t s = 0; s < frames.size(); ++s)
			{
				const vmd::VmdBoneFrame& bf = frames[s];

				*((unsigned int*)positionXBuffer + headerReserved + s) = bf.frame;
				*((unsigned int*)positionYBuffer + headerReserved + s) = bf.frame;
				*((unsigned int*)positionZBuffer + headerReserved + s) = bf.frame;
				*((unsigned int*)rotationQBuffer + headerReserved + s) = bf.frame;

				if (bf.frame < minFrame)
					minFrame = bf.frame;

				if (bf.frame > maxFrame)
					maxFrame = bf.frame;

				unsigned int offset0 = (unsigned int)(offset + s*(sizeof(float) + 4));
				*((float*)((char*)positionXBuffer + offset0)) = bf.position[0];
				*((char*)positionXBuffer + offset0 + sizeof(float) + 0) = bf.interpolation[0];
				*((char*)positionXBuffer + offset0 + sizeof(float) + 1) = bf.interpolation[4];
				*((char*)positionXBuffer + offset0 + sizeof(float) + 2) = bf.interpolation[8];
				*((char*)positionXBuffer + offset0 + sizeof(float) + 3) = bf.interpolation[12];

				*((float*)((char*)positionYBuffer + offset0)) = bf.position[1];
				*((char*)positionYBuffer + offset0 + sizeof(float) + 0) = bf.interpolation[1];
				*((char*)positionYBuffer + offset0 + sizeof(float) + 1) = bf.interpolation[5];
				*((char*)positionYBuffer + offset0 + sizeof(float) + 2) = bf.interpolation[9];
				*((char*)positionYBuffer + offset0 + sizeof(float) + 3) = bf.interpolation[13];

				*((float*)((char*)positionZBuffer + offset0)) = -bf.position[2];
				*((char*)positionZBuffer + offset0 + sizeof(float) + 0) = bf.interpolation[2];
				*((char*)positionZBuffer + offset0 + sizeof(float) + 1) = bf.interpolation[6];
				*((char*)positionZBuffer + offset0 + sizeof(float) + 2) = bf.interpolation[10];
				*((char*)positionZBuffer + offset0 + sizeof(float) + 3) = bf.interpolation[14];

				unsigned int offset1 = (unsigned int)(offset + s*(sizeof(float)*4 + 4));
				*((float*)((char*)rotationQBuffer + offset1 + 0)) = -bf.rotation[0];
				*((float*)((char*)rotationQBuffer + offset1 + 4)) = -bf.rotation[1];
				*((float*)((char*)rotationQBuffer + offset1 + 8)) = bf.rotation[2];
				*((float*)((char*)rotationQBuffer + offset1 + 12)) = bf.rotation[3];
				*((char*)rotationQBuffer + offset1 + sizeof(float)*4 + 0) = bf.interpolation[3];
				*((char*)rotationQBuffer + offset1 + sizeof(float)*4 + 1) = bf.interpolation[7];
				*((char*)rotationQBuffer + offset1 + sizeof(float)*4 + 2) = bf.interpolation[11];
				*((char*)rotationQBuffer + offset1 + sizeof(float)*4 + 3) = bf.interpolation[15];
			}

			animation->push_back(clipData);
		}
	}

	std::vector<morphTargetAnimation*>* morphAnimation = getMophAnimationData(motion->morph_frames, minFrame, maxFrame);
	if (morphAnimation != NULL)
	{
		animationClipData clipData;

		clipData.nodeName = "MMD_Mesh";
		clipData.nodeID = 2;
		clipData.trsvAnimation = NULL;
		clipData.morphAnimation = morphAnimation;
		clipData.extAnimationData = NULL;

		animation->push_back(clipData);
	}

	std::string clipName;
	unsigned int clipID = 0;
	Platform_SHIFTJIS_UTF8(motion->model_name, clipName);
	char buffer[__TEMP_BUFFER_FLOAT__];
	sprintf(buffer, "%s.%d.animation.bin", clipName.c_str(), clipID);
	std::string clipFileName = buffer;
	clipFileName = standardizeFileName(clipFileName);
	std::string animationFullFileName = workingDirectory + clipFileName;

	JsonToBin_V4* writer = new JsonToBin_V4();
	writer->openExportFile(animationFullFileName);

	float fps = 30.0f;
	std::string clipNameUTF8;
	Platform_SHIFTJIS_UTF8(motion->model_name, clipNameUTF8);
	writer->writeAnimationBin(clipID, clipNameUTF8, fps, (float)minFrame, (float)maxFrame, animation);

	writer->closeExportFile();

	std::vector<std::string> convertedFiles;
	convertedFiles.push_back(clipFileName);

	std::string convertedFilePath = filePath + ".convertedFiles";
	convertedFiles.push_back(convertedFilePath);

	writeConvertedFile(convertedFilePath, convertedFiles, 0, 0, 0);

	return true;
}

std::vector<morphTargetAnimation*>* VmdConverter::getMophAnimationData(
	std::vector<vmd::VmdMorphFrame>& morphKeyframes,
	int minFrame,
	int maxFrame
)
{
	std::vector<morphTargetAnimation*>* morph = NULL;

	std::map<std::string, std::vector<vmd::VmdMorphFrame>*> dict;
	for (size_t s = 0; s < morphKeyframes.size(); ++s)
	{
		vmd::VmdMorphFrame& frame = morphKeyframes[s];

		string utf8String;
		Platform_SHIFTJIS_UTF8(frame.morph_name, utf8String);

		std::map<std::string, std::vector<vmd::VmdMorphFrame>*>::iterator iter = dict.find(utf8String);
		if (iter != dict.end())
		{
			dict[utf8String]->push_back(frame);
		}
		else
		{
			dict[utf8String] = new std::vector<vmd::VmdMorphFrame>();
			dict[utf8String]->push_back(frame);
		}
	}

	std::map<std::string, std::vector<vmd::VmdMorphFrame>*>::iterator iter = dict.begin();
	unsigned int channelIndex = 0;
	for (; iter != dict.end(); ++iter)
	{
		std::sort(iter->second->begin(), iter->second->end(), less_second0);

		std::vector<vmd::VmdMorphFrame>* frames = iter->second;
		if (frames->size() > 0)
		{
			vmd::VmdMorphFrame& firstFrame = (*frames)[0];
			vmd::VmdMorphFrame& endFrame = (*frames)[frames->size() - 1];

			if ((int)firstFrame.frame < minFrame)
				minFrame = firstFrame.frame;

			if ((int)endFrame.frame > maxFrame)
				maxFrame = endFrame.frame;

			if (morph == NULL)
			{
				morph = new std::vector<morphTargetAnimation*>();
			}

			morphTargetAnimation* data = new morphTargetAnimation();
			data->channelIndex = channelIndex;
			data->channelName = iter->first;
			data->animations.resize(frames->size() * 2);
			for (size_t s = 0; s < frames->size(); ++s)
			{
				data->animations[s] = (float)(*frames)[s].frame;
				data->animations[frames->size() + s] = (*frames)[s].weight*100.0f;
			}

			morph->push_back(data);

			++channelIndex;
		}
	}

	return morph;
}