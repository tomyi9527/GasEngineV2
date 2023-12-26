#include "stdafx.h"
#include "FBXSceneStructureExporter_V4.h"
#include "Common/Common.h"
#include "AssistantFunctions.h"
#include "JSONFileWriter.h"
#include "Material.h"
#include "NeonateVertexCompression_V4.h"
#include "TextureMap.h"
#include "meshLoader.h"
#include "JsonToBin_V4.h"

FBXSceneStructureExporter_V4::FBXSceneStructureExporter_V4()
	: mFbxManager(NULL)
	, mFbxScene(NULL)
	, mGeneratedMaterialCount(0)	
{
}


FBXSceneStructureExporter_V4::~FBXSceneStructureExporter_V4()
{
}

bool FBXSceneStructureExporter_V4::init(FbxManager* manager, FbxScene* scene)
{
	mFbxManager = manager;
	mFbxScene = scene;
	
	//Textures and Materials
	mUsedMaterialsInTheScene.clear();
	FBXSDK_NAMESPACE::FbxNode* rootNode = scene->GetRootNode();

	mGeneratedMaterialCount = 0;
	gatherAllUsedMaterils_r(rootNode, mEffectiveBones);

	for (int i = 0; i < (int)mUsedMaterialsInTheScene.size(); ++i)
	{
		Material* material = new Material();
		material->parseFBXMaterial(mUsedMaterialsInTheScene[i], mTextureMaps, mTextureFiles);
		mMaterials.push_back(material);
	}

	collectHierarchicalInfo_r(rootNode, NULL);

	for (size_t s = 0; s < mNodes.size(); ++s)
	{
		Node* node = mNodes[s];
		if (node->parent >= 0)
		{
			mNodes[node->parent]->children.push_back((int)s);
		}
	}	

	return true;
}

void FBXSceneStructureExporter_V4::finl()
{
	for (int i = 0; i < (int)mNodes.size(); ++i)
	{
		delete mNodes[i];
	}
	mNodes.clear();

	for (int i = 0; i < (int)mTextureMaps.size(); ++i)
	{
		delete mTextureMaps[i];
	}
	mTextureMaps.clear();

	for (int i = 0; i < (int)mMaterials.size(); ++i)
	{
		delete mMaterials[i];
	}
	mMaterials.clear();

	mEffectiveBones.clear();
}

void FBXSceneStructureExporter_V4::collectHierarchicalInfo_r(FBXSDK_NAMESPACE::FbxNode* node, Node* parentNode)
{
	if (node == NULL)
		return;

	bool layerVis = true;
	FBXSDK_NAMESPACE::FbxDisplayLayer* displayLayer =
		(FBXSDK_NAMESPACE::FbxDisplayLayer*)(node->GetDstObject(FBXSDK_NAMESPACE::FbxCriteria::ObjectType(FBXSDK_NAMESPACE::FbxDisplayLayer::ClassId), 0));
	if (displayLayer != NULL && !displayLayer->Show)
	{
		layerVis = false;
	}

	bool vis = node->GetVisibility();

	FBXSDK_NAMESPACE::FbxBool showFlag = true;
	FBXSDK_NAMESPACE::FbxProperty showProp = node->FindProperty("Show"); //For 3ds max hide option
	if (showProp.IsValid())
	{
		showFlag = (FBXSDK_NAMESPACE::FbxBool)showProp.Get<FbxBool>();
	}

	Node* myNode = new Node();
	mNodes.push_back(myNode);
	myNode->_index_ = (int)mNodes.size() - 1;
	if (parentNode != NULL)
	{
		myNode->parent = parentNode->_index_;
	}
	else
	{
		myNode->parent = -1;
	}

	FbxUInt64 id = node->GetUniqueID();
	myNode->uniuqeID = id; //<

	std::string guidString = newUUID();	
	myNode->guid = guidString; //<

	string nodeName = standardizeFileName(node->GetName());
	myNode->name = nodeName;

	FbxVector4 tr = node->FindProperty("Lcl Translation").Get<FBXSDK_NAMESPACE::FbxVector4>();
	FbxVector4 rt = node->FindProperty("Lcl Rotation").Get<FBXSDK_NAMESPACE::FbxVector4>();
	FbxVector4 sc = node->FindProperty("Lcl Scaling").Get<FBXSDK_NAMESPACE::FbxVector4>();
	myNode->translation = tr; //<
	myNode->rotation = rt; //<
	myNode->scaling = sc;  //<

	FBXSDK_NAMESPACE::FbxVector4 PropSp = node->GetScalingPivot(FBXSDK_NAMESPACE::FbxNode::eSourcePivot);
	FBXSDK_NAMESPACE::FbxVector4 PropSoff = node->GetScalingOffset(FBXSDK_NAMESPACE::FbxNode::eSourcePivot);

	FBXSDK_NAMESPACE::FbxVector4 PropRp = node->GetRotationPivot(FBXSDK_NAMESPACE::FbxNode::eSourcePivot);
	FBXSDK_NAMESPACE::FbxVector4 PropRpost = node->GetPostRotation(FBXSDK_NAMESPACE::FbxNode::eSourcePivot);
	FBXSDK_NAMESPACE::FbxVector4 PropRpre = node->GetPreRotation(FBXSDK_NAMESPACE::FbxNode::eSourcePivot);
	FBXSDK_NAMESPACE::FbxVector4 PropRoff = node->GetRotationOffset(FBXSDK_NAMESPACE::FbxNode::eSourcePivot);

	FBXSDK_NAMESPACE::FbxTransform::EInheritType PropInherit = node->InheritType.Get();
	FBXSDK_NAMESPACE::FbxInt PropRotOrder = node->RotationOrder.Get();
	FBXSDK_NAMESPACE::FbxDouble PropVisibility = node->Visibility.Get();
	FBXSDK_NAMESPACE::FbxBool PropVisibilityInheritance = node->VisibilityInheritance.Get();

	FBXSDK_NAMESPACE::FbxVector4 Trans = node->FindProperty("Lcl Translation").Get<FBXSDK_NAMESPACE::FbxVector4>();
	FBXSDK_NAMESPACE::FbxVector4 Rot = node->FindProperty("Lcl Rotation").Get<FBXSDK_NAMESPACE::FbxVector4>();
	FBXSDK_NAMESPACE::FbxVector4 Scl = node->FindProperty("Lcl Scaling").Get<FBXSDK_NAMESPACE::FbxVector4>();

	myNode->scalingPivot = PropSp;
	myNode->scalingOffset = PropSoff;
	myNode->rotationPivot = PropRp;
	myNode->rotationOffset = PropRoff;
	myNode->preRotation = PropRpre;
	myNode->postRotation = PropRpost;

	const char* rotationOrderTypeStr[] = { "XYZ", "XZY", "YZX", "YXZ", "ZXY", "ZYX", "SphericXYZ" };
	myNode->rotationOrder = rotationOrderTypeStr[PropRotOrder]; //<

	const char* inheritanceTypeStr[] = { "Local(RrSs)", "Parent(RSrs)", "Scale Compensate(Rrs)" };
	if (PropInherit != 1)
	{
		FBXSDK_printf("Error: Now the converter only support %s scaling inheritance, but node: \"%s\" is %s\n", inheritanceTypeStr[1], node->GetName(), inheritanceTypeStr[PropInherit]);
	}	
	myNode->inheritType = inheritanceTypeStr[PropInherit];
	myNode->visibility = PropVisibility; //<
	myNode->visibilityInheritance = PropVisibilityInheritance;

	const int materialCount = node->GetMaterialCount();
	for (int i = 0; i < materialCount; ++i)
	{
		FBXSDK_NAMESPACE::FbxSurfaceMaterial* material = node->GetMaterial(i);
		if (material != NULL)
		{
			void* p = material->GetUserDataPtr();
			unsigned long long v = reinterpret_cast<unsigned long long>(p);
			unsigned int index = (unsigned int)(v & 0xffffffff);
			myNode->materials.push_back(index);
		}
	}

	////<For Debug
	//int nodeAttrCount = node->GetNodeAttributeCount();
	//if (nodeAttrCount > 1)
	//{
	//	int kk = 0;
	//}
	////<

	myNode->meshInfo = NULL;

	myNode->skeletonName = "eNone";
	
	FBXSDK_NAMESPACE::FbxNodeAttribute* nodeAttr = node->GetNodeAttribute();
	if (nodeAttr)
	{
		FBXSDK_NAMESPACE::FbxNodeAttribute::EType nodeAttrType = nodeAttr->GetAttributeType();
		switch (nodeAttrType)
		{
			case FBXSDK_NAMESPACE::FbxNodeAttribute::eMesh:
			{
				if (vis && showFlag && layerVis)
				{
					FBXSDK_NAMESPACE::FbxMesh* mesh = node->GetMesh();
					int meshUniqueID = (int)mesh->GetUniqueID();

					std::map<int, MeshInfo>::iterator iter = mMeshDepot.find(meshUniqueID);
					if (iter != mMeshDepot.end())
					{
						myNode->meshInfo = &(iter->second);
					}
					else
					{
						FBXSDK_printf("Error: found a dummy mesh.");
					}
				}

				break;
			}
			case FBXSDK_NAMESPACE::FbxNodeAttribute::eSkeleton:
			{
				FbxSkeleton* skeleton = (FbxSkeleton*)nodeAttr;
				FbxSkeleton::EType skeletonType = skeleton->GetSkeletonType();
				if (skeletonType == FbxSkeleton::eRoot)
				{
					myNode->skeletonName = "eRoot";
				}
				else if (skeletonType == FbxSkeleton::eLimb)
				{
					myNode->skeletonName = "eLimb";
				}
				else if (skeletonType == FbxSkeleton::eLimbNode)
				{
					myNode->skeletonName = "eLimbNode";
				}
				else if (skeletonType == FbxSkeleton::eEffector)
				{
					myNode->skeletonName = "eEffector";
				}
				break;
			}
			case FBXSDK_NAMESPACE::FbxNodeAttribute::eNurbs:
			case FBXSDK_NAMESPACE::FbxNodeAttribute::ePatch:
			case FBXSDK_NAMESPACE::FbxNodeAttribute::eNurbsSurface:
			case FBXSDK_NAMESPACE::FbxNodeAttribute::eShape:
			case FBXSDK_NAMESPACE::FbxNodeAttribute::eNurbsCurve:
			{
				FBXSDK_printf("%s : %d\n", "Error: do not support specified node attribute", nodeAttrType);
				break;
			}
			case FBXSDK_NAMESPACE::FbxNodeAttribute::eLight:
			case FBXSDK_NAMESPACE::FbxNodeAttribute::eCamera:
			case FBXSDK_NAMESPACE::FbxNodeAttribute::eUnknown:
			default:
				break;
		}
	}
	
	int childCount = node->GetChildCount();
	for (int i = 0; i < childCount; ++i)
	{
		FBXSDK_NAMESPACE::FbxNode* child = node->GetChild(i);
		collectHierarchicalInfo_r(child, myNode);
	}	
}

//void FBXSceneStructureExporter_V4::simpleMergeMeshes(
//	std::string workingDirectory,
//	std::vector<std::string>& meshFiles,
//	std::vector<FBXSDK_NAMESPACE::FbxMesh*>& meshes,
//	std::vector<Material*>& materials)
//{
//	std::vector<meshLoader*> loaders;
//	for (size_t s = 0; s < meshFiles.size(); ++s)
//	{
//		meshLoader* loader = new meshLoader();
//
//		loader->load(workingDirectory + meshFiles[s]);
//
//		FBXSDK_NAMESPACE::FbxMesh* mesh = meshes[s];
//		int meshUniqueID = (int)mesh->GetUniqueID();
//		const MeshInfo& info = mMeshDepot[meshUniqueID];
//		Material* material = NULL;
//		if (info.materials.size() > 0)
//		{
//			material = materials[info.materials[0]];
//			if (info.materials.size() > 1)
//			{
//				FBXSDK_printf("Error: Converter can not combine meshes with multiple material correctly!\n");
//			}
//		}
//
//		loader->mVertexColors = new unsigned int[loader->mVertexCount];
//		loader->mVertexColorBufferSize = loader->mVertexCount * sizeof(unsigned int);
//		unsigned int r = (unsigned int)(material->mDiffuseColor[0] * 255.0 + 0.5);
//		unsigned int g = (unsigned int)(material->mDiffuseColor[1] * 255.0 + 0.5);
//		unsigned int b = (unsigned int)(material->mDiffuseColor[2] * 255.0 + 0.5);
//		unsigned int a = (unsigned int)(material->mOpacity * 255.0 + 0.5);
//		unsigned int color = (r << 0) | (g << 8) | (b << 16) | (a << 24);
//		for (int k = 0; k < loader->mVertexCount; ++k)
//		{
//			loader->mVertexColors[k] = color;
//		}
//
//		material->mDiffuseColor[0] = material->mDiffuseColor[1] = material->mDiffuseColor[2] = 1.0;
//		material->mTransparencyEnable = true;
//		material->mTransparencyFactor = 1.0;
//		material->mTransparencyColor[0] = material->mTransparencyColor[1] = material->mTransparencyColor[2] = 1.0;
//		material->mTransparencyIndexInDepot = -1;
//		material->mCulling = (unsigned int)1.0;
//
//		loaders.push_back(loader);
//	}
//
//	int mergedParentUniqueID = -1;
//	int mergedUniqueID = -1;
//	FBXSDK_NAMESPACE::FbxDouble3 bboxMin(FLT_MAX, FLT_MAX, FLT_MAX);
//	FBXSDK_NAMESPACE::FbxDouble3 bboxMax(FLT_MIN, FLT_MIN, FLT_MIN);
//
//	std::string mergedMeshName = NeonateVertexCompression_V4::mergeMeshes(
//		mWorkingDirectory,
//		loaders,
//		mergedParentUniqueID,
//		mergedUniqueID,
//		bboxMin,
//		bboxMax);
//
//	mMeshDepot.clear();
//
//	mMeshDepot[mergedUniqueID] = MeshInfo();
//	mMeshDepot[mergedUniqueID].mesh = NULL;
//	mMeshDepot[mergedUniqueID].isSkinned = false;
//	mMeshDepot[mergedUniqueID].meshName = mergedMeshName;
//	mMeshDepot[mergedUniqueID].bboxMin = bboxMin;
//	mMeshDepot[mergedUniqueID].bboxMax = bboxMax;
//	mMeshDepot[mergedUniqueID].materials.push_back(0);
//
//	for (size_t k = 0; k < mGeometryFiles->size(); ++k)
//	{
//		std::string fullPath = mWorkingDirectory + (*mGeometryFiles)[k];
//		remove(fullPath.c_str());
//	}
//	mGeometryFiles->clear();
//	mGeometryFiles->push_back(mergedMeshName);
//
//	for (size_t s = 0; s < loaders.size(); ++s)
//	{
//		meshLoader* loader = loaders[s];
//		loader->clear();
//		delete loader;
//	}
//
//	loaders.clear();
//}

void FBXSceneStructureExporter_V4::gatherAllUsedMaterils_r(
	FBXSDK_NAMESPACE::FbxNode* node, 
	std::vector<unsigned int>& effectiveBones)
{
	bool vis = node->GetVisibility();
	if (vis)
	{
		FBXSDK_NAMESPACE::FbxNodeAttribute* nodeAttr = node->GetNodeAttribute();
		if (nodeAttr != NULL)
		{
			FBXSDK_NAMESPACE::FbxNodeAttribute::EType nodeAttrType = nodeAttr->GetAttributeType();
			if (nodeAttrType == FBXSDK_NAMESPACE::FbxNodeAttribute::eMesh)
			{
				FBXSDK_NAMESPACE::FbxMesh* mesh = node->GetMesh();
				int polygonCount = mesh->GetPolygonCount();
				if (polygonCount > 0)
				{
					NeonateVertexCompression_V4::getEffectiveBones(mesh, effectiveBones);

					FBXSDK_NAMESPACE::FbxNode::ECullingType cullingType = FBXSDK_NAMESPACE::FbxNode::eCullingOff;
					FBXSDK_NAMESPACE::FbxProperty cullProp = node->FindProperty("CullingMode");
					if (cullProp.IsValid())
					{
						cullingType = (FBXSDK_NAMESPACE::FbxNode::ECullingType)cullProp.Get<FbxInt>();
					}					
					//0:eCullingOff	Renders both the inner and outer polygons for the selected model.
					//1:eCullingOnCCW (Counter-Clockwise)	Renders only the polygons that compose the outside of the model.
					//2:eCullingOnCW (Clockwise)	Renders only the polygons that compose the inside of the selected model.

					const int materialCount = node->GetMaterialCount();
					if (materialCount > 0)
					{
						for (int i = 0; i < materialCount; ++i)
						{
							FBXSDK_NAMESPACE::FbxSurfaceMaterial* material = node->GetMaterial(i);
							if (material != NULL)
							{
								bool flag = false;
								for (int j = 0; j < (int)mUsedMaterialsInTheScene.size(); ++j)
								{
									FBXSDK_NAMESPACE::FbxUInt64 id0 = mUsedMaterialsInTheScene[j]->GetUniqueID();
									FBXSDK_NAMESPACE::FbxUInt64 id1 = material->GetUniqueID();
									if (id0 == id1)
									{
										flag = true;
										break;
									}
								}

								if (!flag)
								{
									mUsedMaterialsInTheScene.push_back(material);
									unsigned long long index0 = (unsigned long long)(mUsedMaterialsInTheScene.size()) - 1;
									unsigned long long index = index0 | ((unsigned long long)cullingType << 32);
									material->SetUserDataPtr(reinterpret_cast<void*>(index));
								}

								void* p = material->GetUserDataPtr();
								unsigned long long v = reinterpret_cast<unsigned long long>(p);
								unsigned int index0 = (unsigned int)(v & 0xffffffff);

								int meshUniqueID = (int)mesh->GetUniqueID();
								std::map<int, MeshInfo>::iterator iter = mMeshDepot.find(meshUniqueID);
								if (iter != mMeshDepot.end())
								{
									FBXSDK_printf("Error: The mesh belong to multiple nodes!\n");
									iter->second.mesh = mesh;
									iter->second.meshName = mesh->GetNode()->GetName();
									iter->second.materials.push_back(index0);
								}
								else
								{
									mMeshDepot[meshUniqueID] = MeshInfo();
									mMeshDepot[meshUniqueID].uniqueID = meshUniqueID;
									mMeshDepot[meshUniqueID].mesh = mesh;
									mMeshDepot[meshUniqueID].meshName = mesh->GetNode()->GetName();
									mMeshDepot[meshUniqueID].materials.push_back(index0);
								}
								//<
							}
						}
					}
					else
					{
						FBXSDK_printf("Error: Detected a mesh without material! Generated a default one for it!\n");
						char buffer[__TEMP_BUFFER_FLOAT__];
						sprintf(buffer, "Unknown_%d", mGeneratedMaterialCount);
						FBXSDK_NAMESPACE::FbxSurfacePhong* material = FBXSDK_NAMESPACE::FbxSurfacePhong::Create(mFbxScene, buffer);
						node->AddMaterial(material);

						++mGeneratedMaterialCount;

						bool flag = false;
						for (int j = 0; j < (int)mUsedMaterialsInTheScene.size(); ++j)
						{
							FBXSDK_NAMESPACE::FbxUInt64 id0 = mUsedMaterialsInTheScene[j]->GetUniqueID();
							FBXSDK_NAMESPACE::FbxUInt64 id1 = material->GetUniqueID();
							if (id0 == id1)
							{
								flag = true;
								break;
							}
						}

						if (!flag)
						{
							mUsedMaterialsInTheScene.push_back(material);
							unsigned long long index0 = (unsigned long long)(mUsedMaterialsInTheScene.size()) - 1;
							unsigned long long index = index0 | ((unsigned long long)cullingType << 32);
							material->SetUserDataPtr(reinterpret_cast<void*>(index));
						}

						void* p = material->GetUserDataPtr();
						unsigned long long v = reinterpret_cast<unsigned long long>(p);
						unsigned int index0 = (unsigned int)(v & 0xffffffff);

						int meshUniqueID = (int)mesh->GetUniqueID();
						std::map<int, MeshInfo>::iterator iter = mMeshDepot.find(meshUniqueID);
						if (iter != mMeshDepot.end())
						{
							iter->second.mesh = mesh;
							iter->second.meshName = mesh->GetNode()->GetName();
							iter->second.materials.push_back(index0);
						}
						else
						{
							mMeshDepot[meshUniqueID] = MeshInfo();
							mMeshDepot[meshUniqueID].uniqueID = meshUniqueID;
							mMeshDepot[meshUniqueID].mesh = mesh;
							mMeshDepot[meshUniqueID].meshName = mesh->GetNode()->GetName();
							mMeshDepot[meshUniqueID].materials.push_back(index0);
						}
						//<
					}
				}//if (polygonCount > 0)
			}//if (nodeAttrType == FbxNodeAttribute::eMesh)
		}
	}

	int childCount = node->GetChildCount();
	for (int i = 0; i < childCount; ++i)
	{
		FBXSDK_NAMESPACE::FbxNode* child = node->GetChild(i);
		gatherAllUsedMaterils_r(child, effectiveBones);
	}
}