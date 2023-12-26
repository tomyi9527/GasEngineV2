#include "stdafx.h"
#include "PmdReader.h"
#include "Common/Utils.h"

namespace pmd
{
	PmdModel* PmdModel::LoadFromFile(const char* filename)
	{
#ifdef _MSC_VER
		std::wstring unicodePath = UTF8_To_UCS16(filename);
		FILE* fp = _wfopen(unicodePath.c_str(), L"rb");
#else
		FILE* fp = fopen(filename, "rb");
#endif
		if(fp)
		{
			fseek(fp, 0, SEEK_END);
			size_t fileSize = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			void* binaryData = malloc(fileSize);
			fread(binaryData, fileSize, 1, fp);
			fclose(fp);

			BinReader* reader = new BinReader(binaryData, fileSize);

			PmdModel* result = LoadFromStream(reader);

			free(binaryData);
			delete reader;

			return result;
		}
		else
		{
			return NULL;
		}		
	}

	PmdModel* PmdModel::LoadFromStream(BinReader* reader)
	{
		//std::ifstream* stream = NULL;

		PmdModel* result = new PmdModel();
		char buffer[100];

		// magic
		char magic[3];
		reader->read(magic, 3);
		if (magic[0] != 'P' || magic[1] != 'm' || magic[2] != 'd')
		{
			FBXSDK_printf("invalid file!\n");
			return NULL;
		}

		// version
		reader->read((char*)&(result->version), sizeof(float));
		if (result->version != 1.0f)
		{
			FBXSDK_printf("invalid version!\n");
			return NULL;
		}

		// header
		result->header.Read(reader);

		// vertices
		uint32_t vertex_num;
		reader->read((char*)&vertex_num, sizeof(uint32_t));
		result->vertices.resize(vertex_num);
		for (uint32_t i = 0; i < vertex_num; i++)
		{
			result->vertices[i].Read(reader);
		}

		// indices
		uint32_t index_num;
		reader->read((char*)&index_num, sizeof(uint32_t));
		result->indices.resize(index_num);
		for (uint32_t i = 0; i < index_num; i++)
		{
			reader->read((char*)&result->indices[i], sizeof(uint16_t));
		}

		// materials
		uint32_t material_num;
		reader->read((char*)&material_num, sizeof(uint32_t));
		result->materials.resize(material_num);
		for (uint32_t i = 0; i < material_num; i++)
		{
			result->materials[i].Read(reader);
		}

		// bones
		uint16_t bone_num;
		reader->read((char*)&bone_num, sizeof(uint16_t));
		result->bones.resize(bone_num);
		for (uint32_t i = 0; i < bone_num; i++)
		{
			result->bones[i].Read(reader);
		}

		// iks
		uint16_t ik_num;
		reader->read((char*)&ik_num, sizeof(uint16_t));
		result->iks.resize(ik_num);
		for (uint32_t i = 0; i < ik_num; i++)
		{
			result->iks[i].Read(reader);
		}

		// morphs
		uint16_t morph_num;
		reader->read((char*)&morph_num, sizeof(uint16_t));
		result->morphs.resize(morph_num);
		for (uint32_t i = 0; i < morph_num; i++)
		{
			result->morphs[i].Read(reader);
		}

		// morph frames
		uint8_t morph_frame_num;
		reader->read((char*)&morph_frame_num, sizeof(uint8_t));
		result->morph_indices.resize(morph_frame_num);
		for (uint32_t i = 0; i < morph_frame_num; i++)
		{
			reader->read((char*)&result->morph_indices[i], sizeof(uint16_t));
		}

		// bone names
		uint8_t bone_disp_num;
		reader->read((char*)&bone_disp_num, sizeof(uint8_t));
		result->bone_disp_name.resize(bone_disp_num);
		for (uint32_t i = 0; i < bone_disp_num; i++)
		{
			result->bone_disp_name[i].Read(reader);
		}

		// bone frame
		uint32_t bone_frame_num;
		reader->read((char*)&bone_frame_num, sizeof(uint32_t));
		result->bone_disp.resize(bone_frame_num);
		for (uint32_t i = 0; i < bone_frame_num; i++)
		{
			result->bone_disp[i].Read(reader);
		}

		// english name
		bool english;
		reader->read((char*)&english, sizeof(char));
		if (english)
		{
			result->header.ReadExtension(reader);

			for (uint32_t i = 0; i < bone_num; i++)
			{
				result->bones[i].ReadExpantion(reader);
			}

			for (uint32_t i = 0; i < morph_num; i++)
			{
				if (result->morphs[i].type == 0/*pmd::MorphCategory::Base*/)
				{
					continue;
				}
				result->morphs[i].ReadExpantion(reader);
			}

			for (uint32_t i = 0; i < result->bone_disp_name.size(); i++)
			{
				result->bone_disp_name[i].ReadExpantion(reader);
			}
		}

		// toon textures
		if (reader->isEOF())
		{
			result->toon_filenames.clear();
		}
		else {
			result->toon_filenames.resize(10);
			for (uint32_t i = 0; i < 10; i++)
			{
				reader->read(buffer, 100);
				result->toon_filenames[i] = std::string(buffer);
			}
		}

		// physics
		if (reader->isEOF())
		{
			result->rigid_bodies.clear();
			result->joints.clear();
		}
		else
		{
			uint32_t rigid_body_num;
			reader->read((char*)&rigid_body_num, sizeof(uint32_t));
			result->rigid_bodies.resize(rigid_body_num);
			for (uint32_t i = 0; i < rigid_body_num; i++)
			{
				result->rigid_bodies[i].Read(reader);
			}
			uint32_t joint_num;
			reader->read((char*)&joint_num, sizeof(uint32_t));
			result->joints.resize(joint_num);
			for (uint32_t i = 0; i < joint_num; i++)
			{
				result->joints[i].Read(reader);
			}
		}

		if (!reader->isEOF())
		{
			FBXSDK_printf("There is unknown data!\n");
		}

		return result;
	}
}