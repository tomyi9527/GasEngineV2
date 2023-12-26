#include "stdafx.h"
#include "VmdReader.h"
#include "Common/Utils.h"

namespace vmd
{
	VmdMotion* VmdMotion::LoadFromFile(const char* filename)
	{
#ifdef _MSC_VER
		std::wstring unicodePath = UTF8_To_UCS16(filename);
		FILE* fp = _wfopen(unicodePath.c_str(), L"rb");
#else
		FILE* fp = fopen(filename, "rb");
#endif
		if (fp)
		{
			fseek(fp, 0, SEEK_END);
			size_t fileSize = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			void* binaryData = malloc(fileSize);
			fread(binaryData, fileSize, 1, fp);
			fclose(fp);

			BinReader* reader = new BinReader(binaryData, fileSize);

			VmdMotion* result = LoadFromStream(reader);

			free(binaryData);
			delete reader;

			return result;
		}
		else
		{
			return NULL;
		}
	}

	VmdMotion* VmdMotion::LoadFromStream(BinReader* reader)
	{
		char buffer[30];
		VmdMotion* result = new VmdMotion();

		// magic and version
		reader->read((char*)buffer, 30);
		if (strncmp(buffer, "Vocaloid Motion Data", 20))
		{
			FBXSDK_printf("invalid vmd file.\n");
			return NULL;
		}

		result->version = std::atoi(buffer + 20);

		// name
		reader->read(buffer, 20);
		result->model_name = std::string(buffer);

		// bone frames
		int bone_frame_num;
		reader->read((char*)&bone_frame_num, sizeof(int));
		result->bone_frames.resize(bone_frame_num);
		for (int i = 0; i < bone_frame_num; i++)
		{
			result->bone_frames[i].Read(reader);
		}

		// morph frames
		int morph_frame_num;
		reader->read((char*)&morph_frame_num, sizeof(int));
		result->morph_frames.resize(morph_frame_num);
		for (int i = 0; i < morph_frame_num; i++)
		{
			result->morph_frames[i].Read(reader);
		}

		// camera frames
		int camera_frame_num;
		reader->read((char*)&camera_frame_num, sizeof(int));
		result->camera_frames.resize(camera_frame_num);
		for (int i = 0; i < camera_frame_num; i++)
		{
			result->camera_frames[i].Read(reader);
		}

		// light frames
		int light_frame_num;
		reader->read((char*)&light_frame_num, sizeof(int));
		result->light_frames.resize(light_frame_num);
		for (int i = 0; i < light_frame_num; i++)
		{
			result->light_frames[i].Read(reader);
		}

		// unknown2
		reader->read(buffer, 4);

		// ik frames
		if (!reader->isEOF())
		{
			int ik_num;
			reader->read((char*)&ik_num, sizeof(int));
			result->ik_frames.resize(ik_num);
			for (int i = 0; i < ik_num; i++)
			{
				result->ik_frames[i].Read(reader);
			}
		}

		if (!reader->isEOF())
		{
			FBXSDK_printf("vmd stream has unknown data.\n");
		}

		return result;
	}
}