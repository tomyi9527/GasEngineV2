// FBXConverter.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "FBXSceneStructureExporter_V4.h"
#include "MAXSceneStructureExporter_V1.h"
#include "FBXAnimationExporter_V4.h"
#include "Common/Common.h"
#include "MAX/max_reader.h"
#include "NeonateVertexCompression_V4.h"
#include "MMD/PmxConverter.h"
#include "MMD/PmdConverter.h"
#include "MMD/VmdConverter.h"
#include "STL/STLConverter.h"
#include "Common/Utils.h"

#include "GAS_V1/GAS_V1_Streamer.h"
#include "GAS_V2/GAS_V2_Streamer.h"
#include "GLTF_V2/GLTF_V2_Streamer.h"

#include "ModelDetails.h"

#ifdef _MSC_VER
#include "getopt_win.h"
#else
#include <getopt.h>
#endif

#ifdef _MSC_VER 
int wmain(int argc, wchar_t* argv[])
{
	setlocale(LC_CTYPE, "");

	char** argv_utf8 = new char*[argc];
	std::vector<std::string> tempArray;
	for (int i = 0; i < argc; ++i)
	{
		std::wstring ws = argv[i];
		tempArray.push_back(UCS16_To_UTF8(ws));
	}

	for (int i = 0; i < argc; ++i)
	{
		const char* p = tempArray[i].c_str();
		argv_utf8[i] = const_cast<char*>(p);
	}
#else
int main(int argc, char* argv[])
{
	char** argv_utf8 = argv;
#endif
	std::string		fullFilePath			= "";	
	bool			generateGZFile			= false;
	unsigned int	meshOptimization		= 0xffffffff;
	unsigned int	animationOptimization	= 0x0;
	bool			jsonOptimization		= false;
	std::string		outputFileFormat		= "gas2";
	std::string		specifiedOutputDirectory = "";
	bool			mergeMeshes				= false;
	bool			autoScale				= false;
	bool			exportAnimation			= true;
	bool			exportGeometry			= true;
	bool            outputModelDetails		= true;

	int				ret						= 0;
	int				argumentCount			= 0;
	while((ret = getopt(argc, argv_utf8, "f:t:u:cbhagso:m")) != -1)
	{
		argumentCount++;

		switch (ret)
		{		
		case 'a': //only output animation
		{
			exportGeometry = false;
			exportAnimation = true;
			break;
		}
		case 'g': //only output geometry
		{
			exportGeometry = true;
			exportAnimation = false;
			break;
		}
		case 't': //output format
		{
			outputFileFormat = optarg;
			break;
		}
		case 'u': //output directory with back slash
		{
			specifiedOutputDirectory = optarg;
			break;
		}
		case 'm':
		{
			mergeMeshes = true;
			break;
		}
		case 's':
		{
			autoScale = true;
			break;
		}
		case 'f':
		{
			fullFilePath = optarg;
			break;
		}		
		case 'c':
		{
			generateGZFile = true;
			break;
		}
		case 'b':
		{
			jsonOptimization = true;
			break;
		}
		case 'd':
		{
			outputModelDetails = true;
			break;
		}
		case '?':
		{
			FBXSDK_printf("Error: do not support argument: %s\n", argv_utf8[optind]);
			break;
		}
		case 'o':
		{
			meshOptimization = strtoul(optarg, 0, 16);
			break;
		}
		case 'h':
		{
			FBXSDK_printf("%s", "\n");
			FBXSDK_printf("%s\n", "FBXConverter 2.0 by Tomyi");
			FBXSDK_printf("%s", "\n");
			FBXSDK_printf("%s\n", "Command line usage:");
			FBXSDK_printf("%s", "\n");
			FBXSDK_printf("%s\n", "Example:\nFBXConverter -c -b -o0xffff -f\"C:\\\\Test.fbx\"");
			FBXSDK_printf("%s", "\n");
			FBXSDK_printf("%s\n", "-h: show help content.");
			FBXSDK_printf("%s\n", "-c: enable GZIP compression.");
			FBXSDK_printf("%s\n", "-b: enable JSON optimization.");
			FBXSDK_printf("%s\n", "-a: ouput only animation.");
			FBXSDK_printf("%s\n", "-g: output only geometry.");
			FBXSDK_printf("%s\n", "-f arg: specify the full path of input file.");
			FBXSDK_printf("%s\n", "-m arg: merge meshes. It has not implemented yet.");			
			FBXSDK_printf("%s\n", "-u arg: specify output directory.");
			FBXSDK_printf("%s\n", "-s: scale scene unit to centimeter.");
			FBXSDK_printf("%s\n", "-t arg: specify output format. Value can be gas1, gas2 or gltf2.");
			FBXSDK_printf("%s\n", "-o arg: specify the mesh optimization flag. The usage of arg is described below.");
			FBXSDK_printf("%s\n", "   0xffff: full optimization.");
			FBXSDK_printf("%s\n", "   0x0: disable optimization.");
			FBXSDK_printf("%s\n", "   0x1: only optimize position.");
			FBXSDK_printf("%s\n", "   0x2: only optimize normal.");
			FBXSDK_printf("%s\n", "   0x4: only optimize tangent.");
			FBXSDK_printf("%s\n", "   0x80: only optimize uv 0.");
			FBXSDK_printf("%s\n", "   0x200: only optimize vertex color 0.");
			FBXSDK_printf("%s\n", "   0x800: only optimize blend weight and blend index.");
			FBXSDK_printf("%s\n", "   0x2000: only optimize index.");
			FBXSDK_printf("%s\n", "   0x4000: only optimize topological index.");
			FBXSDK_printf("%s", "\n");
			FBXSDK_printf("%s\n", "   FYI: It is legal to generate a new optimization arg by");
			FBXSDK_printf("%s\n", "   combining those ones described before with OR operation.");
			return 0;
		}		
		default:
			break;
		}
	}

#ifdef _MSC_VER //For drag and drop
	if (argc == 2 && argumentCount == 0)
	{
		fullFilePath = argv_utf8[1];
	}
#endif

	if (fullFilePath.length() < 5)
	{
		FBXSDK_printf("%s\n", "Error: invalid input file name!");
		return 0;
	}

	std::string fbxFileDrive, fbxFileDirectory, fbxFileName, fbxFileExt;
	getFilePathElements(fullFilePath, fbxFileDrive, fbxFileDirectory, fbxFileName, fbxFileExt);

	std::string sourceDirectory = fbxFileDrive + fbxFileDirectory;

	std::string outputDirectory = "";
	if (specifiedOutputDirectory != "")
	{
		outputDirectory = specifiedOutputDirectory;
	}
	else
	{
		#ifdef _MSC_VER
		outputDirectory = sourceDirectory + outputFileFormat + "\\";
		#else
		outputDirectory = sourceDirectory + outputFileFormat + "/";
		#endif
	}

#ifdef _MSC_VER
	std::wstring ucsOutputDirectory = UTF8_To_UCS16(outputDirectory.c_str());
	bool result = makeDirectory(ucsOutputDirectory);
	#else
	bool result = makeDirectory(outputDirectory.c_str());
#endif
	
	std::string inputFileFormat = fbxFileExt.substr(1);

	std::string background = "10_Small_Waterfall_In_Park";
	if (strncasecmp(inputFileFormat.c_str(), "pmx", 3) == 0)
	{
		PmxConverter* converter = new PmxConverter();
		converter->convert(outputDirectory, fullFilePath, meshOptimization, background);
		delete converter;
	}
	else if (strncasecmp(inputFileFormat.c_str(), "pmd", 3) == 0)
	{
		PmdConverter* converter = new PmdConverter();
		converter->convert(outputDirectory, fullFilePath, meshOptimization, background);
		delete converter;
	}
	else if (strncasecmp(inputFileFormat.c_str(), "vmd", 3) == 0)
	{
		VmdConverter* converter = new VmdConverter();
		converter->convert(outputDirectory, fullFilePath);
		delete converter;
	}
	else if (strncasecmp(inputFileFormat.c_str(), "stl", 3) == 0)
	{
		STLConverter* converter = new STLConverter();
		converter->convert(outputDirectory, fullFilePath, meshOptimization, background);
		delete converter;
	}
    else if (strncasecmp(inputFileFormat.c_str(), "max", 3) == 0) 
    {
        if (autoScale) {
            FBXSDK_printf("WARNING: -s is not supported by max_converter yet.\n");
        }
        if (exportAnimation) {
            FBXSDK_printf("WARNING: animation exporting is not supported by max_converter yet.\n");
        }

        // 此处的 fbx manager / scene 仅用于生成 mesh 数据
        FBXSDK_NAMESPACE::FbxManager* fbxManager = NULL;
        FBXSDK_NAMESPACE::FbxScene* fbxScene = NULL;
        InitializeSdkObjects(fbxManager, fbxScene);
        FBXSDK_printf("Loading: %s\n", fullFilePath.c_str());

        auto max_objects = LoadFromMax(fullFilePath);
        std::shared_ptr<max::SceneGraphNode> max_root_node;
        if (max_objects)
            max_root_node = max_objects->GetSceneRoot();

        if (max_root_node) {
            NeonateVertexCompression_V4::initNormalCompressor();
#if 0
            FbxSystemUnit SceneSystemUnit = fbxScene->GetGlobalSettings().GetSystemUnit();
            if (SceneSystemUnit.GetScaleFactor() != 1.0 && autoScale) {
                FbxSystemUnit::cm.ConvertScene(fbxScene);
            }
            float scaleFactor = 1.0;
#endif

            Streamer* streamer = NULL;
            if (outputFileFormat == "gas1") {
                streamer = new GAS_V1_Streamer();
            } else if (outputFileFormat == "gas2") {
                streamer = new GAS_V2_Streamer();
            } else if (outputFileFormat == "gltf2") {
                streamer = new GLTF_V2_Streamer();
            }

            // TODO(beanpliu): 实现 MAXAnimationExporter_V1
            // MAXAnimationExporter_V1* animationExporter = NULL;
            MAXSceneStructureExporter_V1* maxSceneStructureExporter = NULL;
#if 0
            if (exportAnimation) {
                animationExporter = new MAXAnimationExporter_V1();
                animationExporter->init(fbxManager, fbxScene);
                std::vector<KeyframeAnimation>& animations = animationExporter->getAnimations();
                streamer->setAnimations(&animations);
            }
#else
            std::vector<KeyframeAnimation>* p_animations = new std::vector<KeyframeAnimation>();
            streamer->setAnimations(p_animations);
#endif

            if (exportGeometry) {
                // Be careful!! Do not add or remove object during process, because object GetUniqueID is highly depend on 
                // the object order in the container.
                maxSceneStructureExporter = new MAXSceneStructureExporter_V1();
                maxSceneStructureExporter->initFBXHelper(fbxManager, fbxScene);
                maxSceneStructureExporter->init(max_root_node);

                //std::string standardizedPath = AssistantFunctions::replace_all_distinct(fullFilePath, "\\", "\\\\");
                std::vector<Node*>&			nodes = maxSceneStructureExporter->getNodes();
                std::map<int, MeshInfo>&	meshes = maxSceneStructureExporter->getMeshes();
                std::vector<Material*>&		materials = maxSceneStructureExporter->getMaterials();
                std::vector<TextureMap*>&	textureMaps = maxSceneStructureExporter->getTextureMaps();
                std::vector<std::string>&	textures = maxSceneStructureExporter->getTextures();
                std::vector<unsigned int>&	effectiveBones = maxSceneStructureExporter->getEffectiveBones();

                streamer->setEffectiveBones(&effectiveBones);
                streamer->setHierarchy(&nodes);
                streamer->setMeshes(&meshes);
                streamer->setMaterials(&materials);
                streamer->setTextureMaps(&textureMaps);
                streamer->setTextures(&textures);
            }

            streamer->save(outputDirectory, fbxFileName + fbxFileExt, jsonOptimization, generateGZFile, meshOptimization, 0x0);

            ModelDetails* modelDetails = NULL;
            if (outputModelDetails) {
                modelDetails = new ModelDetails();
                std::vector<MESH_DETAIL>& meshDetails = streamer->getMeshDetails();
                std::vector<ANIMATION_DETAIL>& animationDetails = streamer->getAnimationDetails();
                std::vector<std::string>* textures = streamer->getTextures();
                modelDetails->setMeshDetails(&meshDetails);
                modelDetails->setAnimationDetails(&animationDetails);
                modelDetails->setTextures(textures);
                modelDetails->save(outputDirectory, fbxFileName + fbxFileExt);
            }

            delete streamer;
            delete modelDetails;
            streamer = NULL;
            modelDetails = NULL;

            if (maxSceneStructureExporter != NULL) {
                maxSceneStructureExporter->finl();
                delete maxSceneStructureExporter;
            }
#if 0
            if (animationExporter != NULL) {
                animationExporter->finl();
                delete animationExporter;
            }
#else
            delete p_animations;
#endif
        }
        if (fbxScene != NULL) {
            fbxScene->Destroy(true);
        }

        DestroySdkObjects(fbxManager, true);
    }
	else
	{
		FBXSDK_NAMESPACE::FbxManager* fbxManager = NULL;
		FBXSDK_NAMESPACE::FbxScene* fbxScene = NULL;
		InitializeSdkObjects(fbxManager, fbxScene);
		FBXSDK_printf("Loading: %s\n", fullFilePath.c_str());
		bool loadingResult = LoadScene(fbxManager, fbxScene, fullFilePath.c_str());
		if (loadingResult)
		{
			NeonateVertexCompression_V4::initNormalCompressor();

			FBXSDK_NAMESPACE::FbxAxisSystem sceneAxisSystem = fbxScene->GetGlobalSettings().GetAxisSystem();

			FBXSDK_NAMESPACE::FbxAxisSystem motionBuilderAxisSystem
			(
				FBXSDK_NAMESPACE::FbxAxisSystem::eYAxis,
				FBXSDK_NAMESPACE::FbxAxisSystem::eParityOdd,
				FBXSDK_NAMESPACE::FbxAxisSystem::eRightHanded
			);

			if (sceneAxisSystem != motionBuilderAxisSystem)
			{
				motionBuilderAxisSystem.ConvertScene(fbxScene);
			}

			FbxSystemUnit SceneSystemUnit = fbxScene->GetGlobalSettings().GetSystemUnit();
			if (SceneSystemUnit.GetScaleFactor() != 1.0 && autoScale)
			{
				FbxSystemUnit::cm.ConvertScene(fbxScene);
			}

			float scaleFactor = 1.0;

			Streamer* streamer = NULL;
			if(outputFileFormat == "gas1")
			{
				streamer = new GAS_V1_Streamer();
			}
			else if (outputFileFormat == "gas2")
			{
				streamer = new GAS_V2_Streamer();
			}
			else if (outputFileFormat == "gltf2")
			{
				streamer = new GLTF_V2_Streamer();
			}

			//< For Debug
			int poseCount = fbxScene->GetPoseCount();

			for (int i = 0; i < poseCount; i++)
			{
				FbxPose* pose = fbxScene->GetPose(i);

				const char* name = pose->GetName();
				int kk = 0;
			}
			//<

			FBXAnimationExporter_V4* animationExporter = NULL;
			FBXSceneStructureExporter_V4* fbxSceneStructureExporter = NULL;
			if(exportAnimation)
			{
				animationExporter = new FBXAnimationExporter_V4();
				animationExporter->init(fbxManager, fbxScene);
				std::vector<KeyframeAnimation>& animations = animationExporter->getAnimations();
				streamer->setAnimations(&animations);
			}

			if(exportGeometry)
			{
				// Be careful!! Do not add or remove object during process, because object GetUniqueID is highly depend on 
				// the object order in the container.
				fbxSceneStructureExporter = new FBXSceneStructureExporter_V4();
				fbxSceneStructureExporter->init(fbxManager, fbxScene);

				//std::string standardizedPath = AssistantFunctions::replace_all_distinct(fullFilePath, "\\", "\\\\");
				std::vector<Node*>&			nodes = fbxSceneStructureExporter->getNodes();
				std::map<int, MeshInfo>&	meshes = fbxSceneStructureExporter->getMeshes();
				std::vector<Material*>&		materials = fbxSceneStructureExporter->getMaterials();
				std::vector<TextureMap*>&	textureMaps = fbxSceneStructureExporter->getTextureMaps();
				std::vector<std::string>&	textures = fbxSceneStructureExporter->getTextures();
				std::vector<unsigned int>&	effectiveBones = fbxSceneStructureExporter->getEffectiveBones();
				
				streamer->setEffectiveBones(&effectiveBones);
				streamer->setHierarchy(&nodes);
				streamer->setMeshes(&meshes);
				streamer->setMaterials(&materials);
				streamer->setTextureMaps(&textureMaps);
				streamer->setTextures(&textures);				
			}

			streamer->save(outputDirectory, fbxFileName + fbxFileExt, jsonOptimization, generateGZFile, meshOptimization, 0x0);

			ModelDetails* modelDetails = NULL;
			if (outputModelDetails)
			{
				modelDetails = new ModelDetails();
				std::vector<MESH_DETAIL>& meshDetails = streamer->getMeshDetails();
				std::vector<ANIMATION_DETAIL>& animationDetails = streamer->getAnimationDetails();
				std::vector<std::string>* textures = streamer->getTextures();
				modelDetails->setMeshDetails(&meshDetails);
				modelDetails->setAnimationDetails(&animationDetails);
				modelDetails->setTextures(textures);
				modelDetails->save(outputDirectory, fbxFileName + fbxFileExt);
			}


			delete streamer;
			delete modelDetails;
			streamer = NULL;
			modelDetails = NULL;
		

			if (fbxSceneStructureExporter != NULL)
			{
				fbxSceneStructureExporter->finl();
				delete fbxSceneStructureExporter;
			}

			if (animationExporter != NULL)
			{
				animationExporter->finl();
				delete animationExporter;
			}
		}

        if (fbxScene != NULL) {
            fbxScene->Destroy(true);
        }

        DestroySdkObjects(fbxManager, true);
	}
	return 0;
}

