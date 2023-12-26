#pragma once
#include "CommonStruct.h"
#include <vector>
#include <string>
//#include <memory>
//#include <iostream>
//#include <fstream>
#include "MMD/BinReader.h"

namespace pmd
{
	class PmdHeader
	{
	public:
		std::string name;
		std::string name_english;
		std::string comment;
		std::string comment_english;

		bool Read(BinReader* reader)
		{
			char buffer[256];
			reader->read(buffer, 20);
			name = std::string(buffer);
			reader->read(buffer, 256);
			comment = std::string(buffer);
			return true;
		}

		bool ReadExtension(BinReader* reader)
		{
			char buffer[256];
			reader->read(buffer, 20);
			name_english = std::string(buffer);
			reader->read(buffer, 256);
			comment_english = std::string(buffer);
			return true;
		}
	};

	class PmdVertex
	{
	public:
		float position[3];
		float normal[3];
		float uv[2];
		uint16_t bone_index[2];
		uint8_t bone_weight;
		bool edge_invisible;

		bool Read(BinReader* reader)
		{
			reader->read((char*)position, sizeof(float) * 3);
			reader->read((char*)normal, sizeof(float) * 3);
			reader->read((char*)uv, sizeof(float) * 2);
			reader->read((char*)bone_index, sizeof(uint16_t) * 2);
			reader->read((char*)&bone_weight, sizeof(uint8_t));
			reader->read((char*)&edge_invisible, sizeof(uint8_t));

			return true;
		}
	};

	class PmdMaterial
	{
	public:
		float diffuse[4];
		float power;
		float specular[3];
		float ambient[3];
		uint8_t toon_index;
		uint8_t edge_flag;
		uint32_t index_count;
		std::string texture_filename;
		std::string sphere_filename;

		bool Read(BinReader* reader)
		{
			char buffer[20];
			reader->read((char*)&diffuse, sizeof(float) * 4);
			reader->read((char*)&power, sizeof(float));
			reader->read((char*)&specular, sizeof(float) * 3);
			reader->read((char*)&ambient, sizeof(float) * 3);
			reader->read((char*)&toon_index, sizeof(uint8_t));
			reader->read((char*)&edge_flag, sizeof(uint8_t));
			reader->read((char*)&index_count, sizeof(uint32_t));
			reader->read((char*)&buffer, sizeof(char) * 20);
			char* pstar = strchr(buffer, '*');
			if (NULL == pstar)
			{
				texture_filename = std::string(buffer);
				sphere_filename.clear();
			}
			else
			{
				*pstar = NULL;
				texture_filename = std::string(buffer);
				sphere_filename = std::string(pstar+1);
			}
			return true;
		}
	};

	//enum class BoneType : uint8_t
	//{
	//	Rotation,
	//	RotationAndMove,
	//	IkEffector,
	//	Unknown,
	//	IkEffectable,
	//	RotationEffectable,
	//	IkTarget,
	//	Invisible,
	//	Twist,
	//	RotationMovement
	//};

	class PmdBone
	{
	public:
		std::string name;
		std::string name_english;
		int16_t parent_bone_index;
		int16_t tail_pos_bone_index;
		uint8_t bone_type;
		int16_t ik_parent_bone_index;
		float bone_head_pos[3];

		void Read(BinReader* reader)
		{
			char buffer[20];
			reader->read(buffer, 20);
			name = std::string(buffer);
			reader->read((char*)&parent_bone_index, sizeof(int16_t));
			reader->read((char*)&tail_pos_bone_index, sizeof(int16_t));
			reader->read((char*)&bone_type, sizeof(uint8_t));
			reader->read((char*)&ik_parent_bone_index, sizeof(int16_t));
			reader->read((char*)&bone_head_pos, sizeof(float) * 3);
		}

		void ReadExpantion(BinReader* reader)
		{
			char buffer[20];
			reader->read(buffer, 20);
			name_english = std::string(buffer);
		}
	};

	class PmdIk
	{
	public:
		uint16_t ik_bone_index; //effector
		uint16_t target_bone_index;//target
		uint16_t interations; //iterationCount
		float angle_limit;//max angle
		std::vector<uint16_t> ik_child_bone_index;//IK chain

		void Read(BinReader* reader)
		{
			reader->read((char*)&target_bone_index, sizeof(uint16_t));
			reader->read((char*)&ik_bone_index, sizeof(uint16_t));

			uint8_t ik_chain_length;
			reader->read((char*)&ik_chain_length, sizeof(uint8_t));
			reader->read((char*)&interations, sizeof(uint16_t));
			reader->read((char*)&angle_limit, sizeof(float));
			ik_child_bone_index.resize(ik_chain_length);
			for (int i = 0; i < ik_chain_length; i++)
			{
				reader->read((char*)&ik_child_bone_index[i], sizeof(uint16_t));
			}
		}
	};

	class PmdMorphVertex
	{
	public:
		int vertex_index;
		float position[3];

		void Read(BinReader* reader)
		{
			reader->read((char*)&vertex_index, sizeof(int));
			reader->read((char*)position, sizeof(float) * 3);
		}
	};

	//enum class MorphCategory : uint8_t
	//{
	//	Base,
	//	Eyebrow,
	//	Eye,
	//	Mouth,
	//	Other
	//};

	class PmdMorph
	{
	public:
		std::string name;
		uint8_t type;
		std::vector<PmdMorphVertex> vertices;
		std::string name_english;

		void Read(BinReader* reader)
		{
			char buffer[20];
			reader->read(buffer, 20);
			name = std::string(buffer);
			int vertex_count;
			reader->read((char*)&vertex_count, sizeof(int));
			reader->read((char*)&type, sizeof(uint8_t));
			vertices.resize(vertex_count);
			for (int i = 0; i < vertex_count; i++)
			{
				vertices[i].Read(reader);
			}
		}

		void ReadExpantion(BinReader* reader)
		{
			char buffer[20];
			reader->read(buffer, 20);
			name_english = std::string(buffer);
		}
	};

	class PmdBoneDispName
	{
	public:
		std::string bone_disp_name;
		std::string bone_disp_name_english;

		void Read(BinReader* reader)
		{
			char buffer[50];
			reader->read(buffer, 50);
			bone_disp_name = std::string(buffer);
			bone_disp_name_english.clear();
		}
		void ReadExpantion(BinReader* reader)
		{
			char buffer[50];
			reader->read(buffer, 50);
			bone_disp_name_english = std::string(buffer);
		}
	};

	class PmdBoneDisp
	{
	public:
		uint16_t bone_index;
		uint8_t bone_disp_index;

		void Read(BinReader* reader)
		{
			reader->read((char*)&bone_index, sizeof(uint16_t));
			reader->read((char*)&bone_disp_index, sizeof(uint8_t));
		}
	};

	//enum class RigidBodyShape : uint8_t
	//{
	//	Sphere = 0,
	//	Box = 1,
	//	Capsule = 2
	//};

	//enum class RigidBodyType : uint8_t
	//{
	//	FollowBone = 0,
	//	Physics = 1,
	//	PhysicsAndBone = 2
	//};

	class PmdRigidBody
	{
	public:
		std::string name;
		uint16_t related_bone_index;
		uint8_t group_index;
		uint16_t group_target;
		uint8_t shapeType; //Sphere = 0, Box = 1, Capsule = 2
		float size[3]; //width height depth
		float position[3];
		float rotation[3];
		float weight;
		float position_damping;
		float rotation_damping;
		float restitution;
		float friction;
		//FollowBone = 0(Rigid body sticks to bone)
		//Physics = 1(Rigid body uses gravity)
		//PhysicsAndBone = 2(Rigid body uses gravity pivoted to bone)
		uint8_t rigid_type; 

		void Read(BinReader* reader)
		{
			char buffer[20];
			reader->read(buffer, sizeof(char) * 20);
			name = (std::string(buffer));

			reader->read((char*)&related_bone_index, sizeof(uint16_t));
			reader->read((char*)&group_index, sizeof(uint8_t));
			reader->read((char*)&group_target, sizeof(uint16_t));
			reader->read((char*)&shapeType, sizeof(uint8_t));
			reader->read((char*)size, sizeof(float) * 3);
			reader->read((char*)position, sizeof(float) * 3);
			reader->read((char*)rotation, sizeof(float) * 3);
			reader->read((char*)&weight, sizeof(float));
			reader->read((char*)&position_damping, sizeof(float));
			reader->read((char*)&rotation_damping, sizeof(float));
			reader->read((char*)&restitution, sizeof(float));
			reader->read((char*)&friction, sizeof(float));
			reader->read((char*)&rigid_type, sizeof(char));
		}
	};

	class PmdJoint
	{
	public:
		std::string name;
		uint32_t rigid_body_index_a;
		uint32_t rigid_body_index_b;

		float position[3];
		float rotation[3];
		float position_lower_limit[3];
		float position_upper_limit[3];
		float rotation_lower_limit[3];
		float rotation_upper_limit[3];
		float position_stiffness[3];
		float rotation_stiffness[3];

		void Read(BinReader* reader)
		{
			char buffer[20];
			reader->read(buffer, 20);
			name = std::string(buffer);

			reader->read((char*)&rigid_body_index_a, sizeof(uint32_t));
			reader->read((char*)&rigid_body_index_b, sizeof(uint32_t));
			reader->read((char*)position, sizeof(float) * 3);
			reader->read((char*)rotation, sizeof(float) * 3);
			reader->read((char*)position_lower_limit, sizeof(float) * 3);
			reader->read((char*)position_upper_limit, sizeof(float) * 3);
			reader->read((char*)rotation_lower_limit, sizeof(float) * 3);
			reader->read((char*)rotation_upper_limit, sizeof(float) * 3);
			reader->read((char*)position_stiffness, sizeof(float) * 3);
			reader->read((char*)rotation_stiffness, sizeof(float) * 3);
		}
	};

	class PmdModel
	{
	public:
		float version;
		PmdHeader header;
		std::vector<PmdVertex> vertices;
		std::vector<uint16_t> indices;
		std::vector<PmdMaterial> materials;
		std::vector<PmdBone> bones;
		std::vector<PmdIk> iks;
		std::vector<PmdMorph> morphs;
		std::vector<uint16_t> morph_indices;
		std::vector<PmdBoneDispName> bone_disp_name;
		std::vector<PmdBoneDisp> bone_disp;
		std::vector<std::string> toon_filenames;
		std::vector<PmdRigidBody> rigid_bodies;
		std::vector<PmdJoint> joints;

		static PmdModel* LoadFromFile(const char* filename);
		static PmdModel* LoadFromStream(BinReader* reader);
	};
}