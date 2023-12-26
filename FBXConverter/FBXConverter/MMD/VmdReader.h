#pragma once

#include <vector>
#include <string>
//#include <memory>
//#include <iostream>
//#include <fstream>
//#include <ostream>
#include "MMD/BinReader.h"

namespace vmd
{
	class VmdBoneFrame
	{
	public:
		std::string name;
		int frame;
		float position[3];
		float rotation[4];
		unsigned char interpolation[64];

		void Read(BinReader* reader)
		{
			char buffer[15];
			reader->read((char*)buffer, sizeof(char)*15);
			name = std::string(buffer);
			reader->read((char*)&frame, sizeof(int));
			reader->read((char*)position, sizeof(float)*3);
			reader->read((char*)rotation, sizeof(float)*4);
			reader->read((char*)interpolation, sizeof(char) * 4 * 4 * 4);
		}

		//void Write(std::ostream* stream)
		//{
		//	stream->write((char*)name.c_str(), sizeof(char) * 15);
		//	stream->write((char*)&frame, sizeof(int));
		//	stream->write((char*)position, sizeof(float) * 3);
		//	stream->write((char*)rotation, sizeof(float) * 4);
		//	stream->write((char*)interpolation, sizeof(char) * 4 * 4 * 4);
		//}
	};

	class VmdMorphFrame
	{
	public:
		std::string morph_name;
		uint32_t frame;
		float weight;		

		void Read(BinReader* reader)
		{
			char buffer[15];
			reader->read((char*) &buffer, sizeof(char) * 15);
			morph_name = std::string(buffer);
			reader->read((char*) &frame, sizeof(int));
			reader->read((char*) &weight, sizeof(float));
		}

		//void Write(std::ostream* stream)
		//{
		//	stream->write((char*)morph_name.c_str(), sizeof(char) * 15);
		//	stream->write((char*)&frame, sizeof(int));
		//	stream->write((char*)&weight, sizeof(float));
		//}
	};

	class VmdCameraFrame
	{
	public:
		int frame;
		float distance;
		float position[3];
		float rotation[3];
		unsigned char interpolation[24];
		float angle;
		char perspective[1];

		void Read(BinReader* reader)
		{
			reader->read((char*)&frame, sizeof(int));
			reader->read((char*)&distance, sizeof(float));
			reader->read((char*)position, sizeof(float) * 3);
			reader->read((char*)rotation, sizeof(float) * 3);
			reader->read((char*)interpolation, sizeof(char) * 24);
			reader->read((char*)&angle, sizeof(float));
			reader->read((char*)perspective, sizeof(char));
		}

		//void Write(std::ostream *stream)
		//{
		//	stream->write((char*)&frame, sizeof(int));
		//	stream->write((char*)&distance, sizeof(float));
		//	stream->write((char*)position, sizeof(float) * 3);
		//	stream->write((char*)rotation, sizeof(float) * 3);
		//	stream->write((char*)interpolation, sizeof(char) * 24);
		//	stream->write((char*)&angle, sizeof(float));
		//	stream->write((char*)perspective, sizeof(char) * 1);
		//}
	};

	class VmdLightFrame
	{
	public:
		int frame;
		float color[3];
		float position[3];

		void Read(BinReader* reader)
		{
			reader->read((char*)&frame, sizeof(int));
			reader->read((char*)color, sizeof(float) * 3);
			reader->read((char*)position, sizeof(float) * 3);
		}

		//void Write(std::ostream* stream)
		//{
		//	stream->write((char*)&frame, sizeof(int));
		//	stream->write((char*)color, sizeof(float) * 3);
		//	stream->write((char*)position, sizeof(float) * 3);
		//}
	};

	class VmdIkEnable
	{
	public:
		std::string ik_name;
		bool enable;
	};

	class VmdIkFrame
	{
	public:
		int frame;
		bool display;
		std::vector<VmdIkEnable> ik_enable;

		void Read(BinReader* reader)
		{
			char buffer[20];
			reader->read((char*)&frame, sizeof(int));
			reader->read((char*)&display, sizeof(uint8_t));
			int ik_count;
			reader->read((char*)&ik_count, sizeof(int));
			ik_enable.resize(ik_count);
			for (int i = 0; i < ik_count; i++)
			{
				reader->read(buffer, 20);
				ik_enable[i].ik_name = std::string(buffer);
				reader->read((char*) &ik_enable[i].enable, sizeof(uint8_t));
			}
		}

		//void Write(std::ostream *stream)
		//{
		//	stream->write((char*)&frame, sizeof(int));
		//	stream->write((char*)&display, sizeof(uint8_t));
		//	int ik_count = static_cast<int>(ik_enable.size());
		//	stream->write((char*)&ik_count, sizeof(int));
		//	for (int i = 0; i < ik_count; i++)
		//	{
		//		const VmdIkEnable& ik_enable = this->ik_enable.at(i);
		//		stream->write(ik_enable.ik_name.c_str(), 20);
		//		stream->write((char*)&ik_enable.enable, sizeof(uint8_t));
		//	}
		//}
	};

	class VmdMotion
	{
	public:
		std::string model_name;
		int version;
		std::vector<VmdBoneFrame> bone_frames;
		std::vector<VmdMorphFrame> morph_frames;
		std::vector<VmdCameraFrame> camera_frames;
		std::vector<VmdLightFrame> light_frames;
		std::vector<VmdIkFrame> ik_frames;

		static VmdMotion* LoadFromFile(const char* filename);
		static VmdMotion* LoadFromStream(BinReader* reader);

		//static VmdMotion* LoadFromFile(char const *filename)
		//{
		//	std::ifstream stream(filename, std::ios::binary);
		//	VmdMotion* result = LoadFromStream(&stream);
		//	stream.close();
		//	return result;
		//}

		//static VmdMotion* LoadFromStream(std::ifstream *stream)
		//{
		//	char buffer[30];
		//	VmdMotion* result = new VmdMotion();

		//	// magic and version
		//	stream->read((char*) buffer, 30);
		//	if (strncmp(buffer, "Vocaloid Motion Data", 20))
		//	{
		//		std::cerr << "invalid vmd file." << std::endl;
		//		return NULL;
		//	}
		//	result->version = std::atoi(buffer + 20);

		//	// name
		//	stream->read(buffer, 20);
		//	result->model_name = std::string(buffer);

		//	// bone frames
		//	int bone_frame_num;
		//	stream->read((char*) &bone_frame_num, sizeof(int));
		//	result->bone_frames.resize(bone_frame_num);
		//	for (int i = 0; i < bone_frame_num; i++)
		//	{
		//		result->bone_frames[i].Read(stream);
		//	}

		//	// morph frames
		//	int morph_frame_num;
		//	stream->read((char*)&morph_frame_num, sizeof(int));
		//	result->morph_frames.resize(morph_frame_num);
		//	for (int i = 0; i < morph_frame_num; i++)
		//	{
		//		result->morph_frames[i].Read(stream);
		//	}

		//	// camera frames
		//	int camera_frame_num;
		//	stream->read((char*) &camera_frame_num, sizeof(int));
		//	result->camera_frames.resize(camera_frame_num);
		//	for (int i = 0; i < camera_frame_num; i++)
		//	{
		//		result->camera_frames[i].Read(stream);
		//	}

		//	// light frames
		//	int light_frame_num;
		//	stream->read((char*) &light_frame_num, sizeof(int));
		//	result->light_frames.resize(light_frame_num);
		//	for (int i = 0; i < light_frame_num; i++)
		//	{
		//		result->light_frames[i].Read(stream);
		//	}

		//	// unknown2
		//	stream->read(buffer, 4);

		//	// ik frames
		//	if (stream->peek() != std::ios::traits_type::eof())
		//	{
		//		int ik_num;
		//		stream->read((char*) &ik_num, sizeof(int));
		//		result->ik_frames.resize(ik_num);
		//		for (int i = 0; i < ik_num; i++)
		//		{
		//			result->ik_frames[i].Read(stream);
		//		}
		//	}

		//	if (stream->peek() != std::ios::traits_type::eof())
		//	{
		//		std::cerr << "vmd stream has unknown data." << std::endl;
		//	}

		//	return result;
		//}

		//bool SaveToFile(const std::string& filename)
		//{
		//	std::ofstream stream(filename.c_str(), std::ios::binary);
		//	bool result = SaveToStream(&stream);
		//	stream.close();
		//	return result;
		//}

		//bool SaveToStream(std::ofstream *stream)
		//{
		//	std::string magic = "Vocaloid Motion Data 0002\0";
		//	magic.resize(30);

		//	// magic and version
		//	stream->write(magic.c_str(), 30);

		//	// name
		//	stream->write(model_name.c_str(), 20);

		//	// bone frames
		//	const int bone_frame_num = static_cast<int>(bone_frames.size());
		//	stream->write(reinterpret_cast<const char*>(&bone_frame_num), sizeof(int));
		//	for (int i = 0; i < bone_frame_num; i++)
		//	{
		//		bone_frames[i].Write(stream);
		//	}

		//	// morph frames
		//	const int morph_frame_num = static_cast<int>(morph_frames.size());
		//	stream->write(reinterpret_cast<const char*>(&morph_frame_num), sizeof(int));
		//	for (int i = 0; i < morph_frame_num; i++)
		//	{
		//		morph_frames[i].Write(stream);
		//	}

		//	// camera frames
		//	const int camera_frame_num = static_cast<int>(camera_frames.size());
		//	stream->write(reinterpret_cast<const char*>(&camera_frame_num), sizeof(int));
		//	for (int i = 0; i < camera_frame_num; i++)
		//	{
		//		camera_frames[i].Write(stream);
		//	}

		//	// light frames
		//	const int light_frame_num = static_cast<int>(light_frames.size());
		//	stream->write(reinterpret_cast<const char*>(&light_frame_num), sizeof(int));
		//	for (int i = 0; i < light_frame_num; i++)
		//	{
		//		light_frames[i].Write(stream);
		//	}

		//	// self shadow datas
		//	const int self_shadow_num = 0;
		//	stream->write(reinterpret_cast<const char*>(&self_shadow_num), sizeof(int));

		//	// ik frames
		//	const int ik_num = static_cast<int>(ik_frames.size());
		//	stream->write(reinterpret_cast<const char*>(&ik_num), sizeof(int));
		//	for (int i = 0; i < ik_num; i++)
		//	{
		//		ik_frames[i].Write(stream);
		//	}

		//	return true;
		//}
	};
}