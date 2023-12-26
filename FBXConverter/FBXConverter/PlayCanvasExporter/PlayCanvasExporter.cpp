#include "PlayCanvasExporter.h"
#include "BinData.h"
#include "../optimization/NeonateVertexCompression_V4.h"
#include "../optimization/PlayCanvasWriter.h"

PlayCanvasExporter::PlayCanvasExporter()
	: mSceneFile("")
	, mJSONWriter(NULL)
	//, mSceneStructureJSONWriter(NULL)
	, mFbxManager(NULL)
	, mFbxScene(NULL)
	, mBinData(NULL)
	, mOptimizationFlag(0)
{
    mTotalTriangleCount = 0;
    mTotalPolygonCount = 0;
    mTotalVertexCount = 0;
    mVecNodeInfo.clear();
}

PlayCanvasExporter::~PlayCanvasExporter()
{
}

void findMeshAttributesAndWriteIndex(
	const string szAttrName, 
	std::map<string, int>& mMeshBuffMap,
	rapidjson::Writer<rapidjson::FileWriteStream>* mJSONWriter,
	const int ibufferViewerIndex) 
{
    auto iter = mMeshBuffMap.find(szAttrName);
    if (iter != mMeshBuffMap.end()) {
          mJSONWriter->Key(szAttrName.c_str());
          mJSONWriter->Uint(ibufferViewerIndex + (*iter).second);
    }
}

//void PlayCanvasExporter::init(	
//	const string& fullFilePath,
//	const string& workingDirectory,
//	FbxManager* manager,
//	FbxScene* scene)
//{
//    mFbxManager = manager;
//    mFbxScene = scene;
//
//    mWorkingDirectory = workingDirectory;
//
//    //dzlog_info("***************** Start Write glTF File *****************");
//    mBinData = new BinData("scene.bin","animation.bin");
//    string szBinFile = mWorkingDirectory + "scene.bin";
//    string szAnimationFile = mWorkingDirectory + "animation.bin";
//    mSceneFile = mWorkingDirectory + "scene.gltf";
//    FILE *fp = fopen(mSceneFile.c_str(),"w");
//
//    char jsonWriteBuffer[65536];
//    rapidjson::FileWriteStream os(fp, jsonWriteBuffer, sizeof(jsonWriteBuffer));
//    mJSONWriter = new rapidjson::Writer<rapidjson::FileWriteStream>(os);
//    mJSONWriter->SetMaxDecimalPlaces(15);
//
//    mJSONWriter->StartObject();
//    
//    //Textures and Materials
//    mUsedMaterialsInTheScene.clear();
//    FBXSDK_NAMESPACE::FbxNode* rootNode = scene->GetRootNode();
//
//    fbxShow::PrintNode(rootNode);
//
//    mGeneratedMaterialCount = 0;
//    std::vector<FBXSDK_NAMESPACE::FbxUInt64> effectiveBones;
//
//    gatherAllUsedMaterils_r(rootNode, effectiveBones);
//
//    vector<string> textureFiles;
//    std::vector<Material*> materials;
//    std::vector<TextureMap*> textureMaps;
//    for (int i = 0; i < (int)mUsedMaterialsInTheScene.size(); ++i)
//    {
//        Material* cache = new Material();
//        cache->parseFBXMaterial(mUsedMaterialsInTheScene[i], textureMaps, textureFiles);
//        materials.push_back(cache);
//    }
//
//    //Write mesh binaries
//    std::vector<std::string> meshFiles;
//    std::vector<FBXSDK_NAMESPACE::FbxMesh*> meshes;
//    std::vector<stSkinInfo> vSkins;
//
//    mJSONWriter->Key("meshes");
//    mJSONWriter->StartArray();
//
//    int ibufferViewerIndex = 0;
//    int iMeshCount = 0;
//
//    std::map<int, MeshInfo>::iterator iter = mMeshDepot.begin();
//    for (; iter != mMeshDepot.end(); ++iter)
//    {
//        FBXSDK_NAMESPACE::FbxMesh* mesh = iter->second.mesh;
//
//        int polygonCount = mesh->GetPolygonCount();
//        if (polygonCount == 0)
//        {
//            break;
//        }
//
//        int nodeCount = mesh->GetNodeCount();
//        if (nodeCount > 1)
//        {
//            FBXSDK_printf("Error: Detected a mesh associates with multiple nodes!\n");
//        }
//
//        int maxMaterialCount = 0;
//        for (int k = 0; k < nodeCount; ++k)
//        {
//            FBXSDK_NAMESPACE::FbxNode* node = mesh->GetNode(k);
//            int materialCount = node->GetMaterialCount();
//            maxMaterialCount = max_0(maxMaterialCount, materialCount);
//        }
//
//        unsigned int triangleCountOfMesh = 0, polygonCountOfMesh = 0, vertexCountOfMesh = 0;
//        vector<Accessor_data> vBinAccessorData;
//        stSkinInfo skinInfo;
//
//        vector<stSubMeshInfo> vSubMeshInfo;
//
//        MeshVertex* compressor = new MeshVertex();
//
//        compressor->compressVertices(
//            vBinAccessorData,
//            skinInfo,
//            vSubMeshInfo,
//            mFbxManager,
//            mOptimizationFlag,
//            mesh,
//            maxMaterialCount,
//            iter->second.bboxMin,
//            iter->second.bboxMax,
//            szBinFile,
//            triangleCountOfMesh,
//            polygonCountOfMesh,
//            vertexCountOfMesh,
//            iter->second.isSkinned);
//
//        if (skinInfo.vBoneNodeID.size() > 0) {
//            vSkins.push_back(skinInfo);
//            iter->second.skinIndex = vSkins.size() - 1;
//        }
//
//        delete compressor;
//
//        std::map<string, int> mMeshBuffMap;
//        std::map<int,stMorphTargetAttr> morphTargetMap;
//        vector<int> vSubMeshIndices;
//        for (auto it = vBinAccessorData.cbegin(); it != vBinAccessorData.cend(); ++it) {
//            auto data = (*it);
//            if (data.iMorphUniqueID > 0) {
//                //morph target iMorphUniqueID 非0
//                stMorphTargetAttr mta;
//                auto find_it = morphTargetMap.find(data.iMorphUniqueID);
//                if (find_it == morphTargetMap.cend()) {
//                    morphTargetMap.insert(std::pair<int,stMorphTargetAttr>(data.iMorphUniqueID,mta));
//                    find_it = morphTargetMap.find(data.iMorphUniqueID);
//                }
//                switch (data.iAttrType) {
//                    case MORPH_TARGET_NORMAL:
//                        find_it->second.iNormalIndex = data.iIsolateIndex;
//                        break;
//                    case MORPH_TARGET_POSITION:
//                        find_it->second.iPostionIndex = data.iIsolateIndex;
//                        break;
//                    case MORPH_TARGET_TANGENT:
//                        find_it->second.iTangentIndex = data.iIsolateIndex;
//                        break;
//                    default:
//                        //dzlog_info("unknwon iAttrType %d",data.iAttrType);
//						break;
//                  }
//                mBinData->add(data);
//            } else {
//                if (data.szTypeName == "indices" && vSubMeshInfo.size() > 0) {
//                    //submesh vertex index
//                    vSubMeshIndices.push_back(data.iIsolateIndex);
//                    mBinData->add(data);
//                } else {
//                    mMeshBuffMap.insert(std::pair<string,int>(data.szTypeName,data.iIsolateIndex));
//                    mBinData->add(data);
//                }
//            }
//        }
//
//        //dzlog_info("Get Mesh with polygon %d triangle %d vertex %d submesh %ld",
//         //   polygonCountOfMesh,triangleCountOfMesh, vertexCountOfMesh, vSubMeshInfo.size());
//
//        FBXSDK_NAMESPACE::FbxNode* parentNode = mesh->GetNode();
//        string szMeshName = parentNode->GetName();
//        unsigned iMeshUID = mesh->GetUniqueID();
//        mJSONWriter->StartObject();
//          szMeshName = intToString(iMeshUID) + "_" + szMeshName;
//          mJSONWriter->Key("name");
//          mJSONWriter->String(szMeshName.c_str());
//          mJSONWriter->Key("primitives");
//          mJSONWriter->StartArray();
//            for (size_t i = 0; i < vSubMeshIndices.size(); ++i) {
//                mJSONWriter->StartObject();
//                  mJSONWriter->Key("attributes");
//                  mJSONWriter->StartObject();
//                    findMeshAttributesAndWriteIndex("POSITION",mMeshBuffMap,mJSONWriter,ibufferViewerIndex);
//                    findMeshAttributesAndWriteIndex("NORMAL",mMeshBuffMap,mJSONWriter,ibufferViewerIndex);
//                    findMeshAttributesAndWriteIndex("TEXCOORD_0",mMeshBuffMap,mJSONWriter,ibufferViewerIndex);
//                    findMeshAttributesAndWriteIndex("TANGENT",mMeshBuffMap,mJSONWriter,ibufferViewerIndex);
//                    findMeshAttributesAndWriteIndex("COLOR_0",mMeshBuffMap,mJSONWriter,ibufferViewerIndex);
//                    findMeshAttributesAndWriteIndex("JOINTS_0",mMeshBuffMap,mJSONWriter,ibufferViewerIndex);
//                    findMeshAttributesAndWriteIndex("WEIGHTS_0",mMeshBuffMap,mJSONWriter,ibufferViewerIndex);
//                  mJSONWriter->EndObject();
//                  mJSONWriter->Key("indices");
//                  mJSONWriter->Uint(ibufferViewerIndex + vSubMeshIndices.at(i));
//                  mJSONWriter->Key("material");
//                  mJSONWriter->Uint(iter->second.materials[i]);
//                  if (morphTargetMap.size() > 0) {
//                      mJSONWriter->Key("targets");
//                      mJSONWriter->StartArray();
//                      for (auto it = morphTargetMap.cbegin(); it != morphTargetMap.cend(); ++it) {
//                          mJSONWriter->StartObject();
//                          if (it->second.iPostionIndex > 0) {
//                              mJSONWriter->Key("POSITION");
//                              mJSONWriter->Uint(it->second.iPostionIndex);
//                          }
//                          if (it->second.iNormalIndex > 0) {
//                              mJSONWriter->Key("NORMAL");
//                              mJSONWriter->Uint(it->second.iNormalIndex);
//                          }
//                          if (it->second.iTangentIndex > 0) {
//                              mJSONWriter->Key("TANGENT");
//                              mJSONWriter->Uint(it->second.iTangentIndex);
//                          }
//                          mJSONWriter->EndObject();
//                      }
//                      mJSONWriter->EndArray();
//                  }
//                  mJSONWriter->Key("mode");
//                  // 4: GLTF2.EMeshPrimitiveMode.TRIANGLES
//                  mJSONWriter->Uint(4);
//                mJSONWriter->EndObject();
//            }
//          mJSONWriter->EndArray();
//        if (morphTargetMap.size() > 0) {
//            mJSONWriter->Key("weights");
//                mJSONWriter->StartArray();
//                for (auto it = morphTargetMap.cbegin(); it != morphTargetMap.cend(); ++it) {
//                    mJSONWriter->Double(it->second.fWeight);
//                }
//                mJSONWriter->EndArray();
//        }
//        mJSONWriter->EndObject();
//
//        //for later node json value write
//        iter->second.meshArrayIndex = iMeshCount++;
//
//        ibufferViewerIndex += vBinAccessorData.size();
//
//        mTotalTriangleCount += triangleCountOfMesh;
//        mTotalPolygonCount += polygonCountOfMesh;
//        mTotalVertexCount += vertexCountOfMesh;
//
//        meshes.push_back(mesh);
//    }
//
//    mJSONWriter->EndArray();
//
//
//	std::string filePath = fullFilePath + ".structure.json";
//	mSceneStructureJSONWriter = new JSONFileWriter(0);
//	mSceneStructureJSONWriter->openExportFile(filePath);
//
//    stNodeInfo rootNodeInfo;
//    rootNodeInfo = processNode_r(rootNode);
//
//    // x 270 y 0 z 0;
//    FbxQuaternion q = transEulerToQuaternion(0,0,0);
//    rootNodeInfo.rotation[0] = q[0];
//    rootNodeInfo.rotation[1] = q[1];
//    rootNodeInfo.rotation[2] = q[2];
//    rootNodeInfo.rotation[3] = q[3];
//    //rootNodeInfo.scale[0] = -1;
//    //rootNodeInfo.scale[1] = -1;
//    //rootNodeInfo.scale[2] = -1;
//    mVecNodeInfo.push_back(rootNodeInfo);
//    mNodeIDtoIndexMap[rootNodeInfo.iNodeID] = mVecNodeInfo.size() - 1;
//    
//    writeSceneFile(materials, textureMaps, textureFiles);
//
//    for (int i = 0; i < (int)materials.size(); ++i)
//    {
//        delete materials[i];
//    }
//
//    SkinWriter::write(mJSONWriter,vSkins,mNodeIDtoIndexMap,szBinFile,mBinData);
//
//    //parse animation
//    //parse animationFiles and write bin file , result move to vAnimationArray 
//    vector<RawAnimation> vAnimationArray;
//    AnimationExport::getSceneAnimation(scene,vAnimationArray);
//    AnimationExport::writeAnimationBin(vAnimationArray,szAnimationFile,mBinData);
//    //parse animation end
//
//    //write animation JSON
//    AnimationsWriter::write(mJSONWriter,vAnimationArray,mNodeIDtoIndexMap);
//
//    mBinData->writeJSON(mJSONWriter);
//
//    mJSONWriter->EndObject();
//
//	mSceneStructureJSONWriter->closeExportFile();
//
//    //dzlog_info("***************** End Of glTF File *****************");
//    fclose(fp);
//}

void PlayCanvasExporter::init(
	const string& fullFilePath,
	const string& workingDirectory,
	FbxManager* manager,
	FbxScene* scene)
{
	mFbxManager = manager;
	mFbxScene = scene;

	mWorkingDirectory = workingDirectory;

	//dzlog_info("***************** Start Write glTF File *****************");
	//mBinData = new BinData("scene.bin", "animation.bin");
	//string szBinFile = mWorkingDirectory + "scene.bin";
	//string szAnimationFile = mWorkingDirectory + "animation.bin";
	mSceneFile = mWorkingDirectory + "model.json";
	FILE* fp = fopen(mSceneFile.c_str(), "w");

	char jsonWriteBuffer[65536];
	rapidjson::FileWriteStream os(fp, jsonWriteBuffer, sizeof(jsonWriteBuffer));
	mJSONWriter = new rapidjson::Writer<rapidjson::FileWriteStream>(os);
	mJSONWriter->SetMaxDecimalPlaces(6);

	mJSONWriter->StartObject();

	//Textures and Materials
	mUsedMaterialsInTheScene.clear();
	FBXSDK_NAMESPACE::FbxNode* rootNode = scene->GetRootNode();

	//fbxShow::PrintNode(rootNode);

	mGeneratedMaterialCount = 0;
	std::vector<FBXSDK_NAMESPACE::FbxUInt64> effectiveBones;

	gatherAllUsedMaterils_r(rootNode, effectiveBones);

	vector<string> textureFiles;
	std::vector<Material*> materials;
	std::vector<TextureMap*> textureMaps;
	for (int i = 0; i < (int)mUsedMaterialsInTheScene.size(); ++i)
	{
		Material* cache = new Material();
		cache->parseFBXMaterial(mUsedMaterialsInTheScene[i], textureMaps, textureFiles);
		materials.push_back(cache);
	}

	//Write nodes
	processNode_r(rootNode, -1);

	mJSONWriter->Key("model");
	mJSONWriter->StartObject();

	mJSONWriter->Key("version");
	mJSONWriter->Int(3);

	writeNodeArray();

	//Write mesh binaries
	std::map<int, MeshInfo>::iterator iter = mMeshDepot.begin();
	for (; iter != mMeshDepot.end(); ++iter)
	{
		FBXSDK_NAMESPACE::FbxMesh* mesh = iter->second.mesh;

		int polygonCount = mesh->GetPolygonCount();
		if (polygonCount == 0)
		{
			break;
		}

		int nodeCount = mesh->GetNodeCount();
		if (nodeCount > 1)
		{
			FBXSDK_printf("Error: Detected a mesh associates with multiple nodes!\n");
		}

		int maxMaterialCount = 0;
		for (int k = 0; k < nodeCount; ++k)
		{
			FBXSDK_NAMESPACE::FbxNode* node = mesh->GetNode(k);
			int materialCount = node->GetMaterialCount();
			maxMaterialCount = max_0(maxMaterialCount, materialCount);
		}

		//******************* Write Mesh File *******************//
		NeonateVertexCompression_V4* compressor = new NeonateVertexCompression_V4();
		unsigned int triangleCountOfMesh = 0, polygonCountOfMesh = 0, vertexCountOfMesh = 0;

		FbxUInt64 id = mesh->GetUniqueID();
		FBXSDK_NAMESPACE::FbxNode* parentNode = mesh->GetNode();
		std::string objectName = parentNode->GetName();

		if (objectName.length() == 0)
		{
			objectName = "unknown";
		}
		objectName = standardizeFileName(objectName);

		PlayCanvasWriter* writer = new PlayCanvasWriter();
		writer->setRapidJsonWriter(mJSONWriter);		

		compressor->compressVertices(
			mOptimizationFlag,
			mesh,
			maxMaterialCount,
			iter->second.bboxMin,
			iter->second.bboxMax,
			triangleCountOfMesh,
			polygonCountOfMesh,
			vertexCountOfMesh,
			iter->second.isSkinned,
			writer);

		delete writer;
		delete compressor;

		mJSONWriter->Key("meshInstances");
		mJSONWriter->StartArray();

		int parentNodeIndex = -1;
		for (size_t s = 0; s < mVecNodeInfo.size(); ++s)
		{
			if (parentNode->GetUniqueID() == mVecNodeInfo[s].iNodeID)
			{
				parentNodeIndex = s;
				break;
			}
		}
		mJSONWriter->StartObject();
		mJSONWriter->Key("node");
		mJSONWriter->Int(parentNodeIndex);

		mJSONWriter->Key("mesh");
		mJSONWriter->Int(0);
		mJSONWriter->EndObject();

		mJSONWriter->EndArray();

		//******************* End Mesh File *******************//

		mTotalTriangleCount += triangleCountOfMesh;
		mTotalPolygonCount += polygonCountOfMesh;
		mTotalVertexCount += vertexCountOfMesh;
	}
	mJSONWriter->EndObject(); //End of Model
	//

	////Write mesh binaries
	//std::vector<std::string> meshFiles;
	//std::vector<FBXSDK_NAMESPACE::FbxMesh*> meshes;
	//std::vector<stSkinInfo> vSkins;

	//mJSONWriter->Key("meshes");
	//mJSONWriter->StartArray();

	//int ibufferViewerIndex = 0;
	//int iMeshCount = 0;

	//std::map<int, MeshInfo>::iterator iter = mMeshDepot.begin();
	//for (; iter != mMeshDepot.end(); ++iter)
	//{
	//	FBXSDK_NAMESPACE::FbxMesh* mesh = iter->second.mesh;

	//	int polygonCount = mesh->GetPolygonCount();
	//	if (polygonCount == 0)
	//	{
	//		break;
	//	}

	//	int nodeCount = mesh->GetNodeCount();
	//	if (nodeCount > 1)
	//	{
	//		FBXSDK_printf("Error: Detected a mesh associates with multiple nodes!\n");
	//	}

	//	int maxMaterialCount = 0;
	//	for (int k = 0; k < nodeCount; ++k)
	//	{
	//		FBXSDK_NAMESPACE::FbxNode* node = mesh->GetNode(k);
	//		int materialCount = node->GetMaterialCount();
	//		maxMaterialCount = max_0(maxMaterialCount, materialCount);
	//	}

	//	unsigned int triangleCountOfMesh = 0, polygonCountOfMesh = 0, vertexCountOfMesh = 0;
	//	vector<Accessor_data> vBinAccessorData;
	//	stSkinInfo skinInfo;

	//	vector<stSubMeshInfo> vSubMeshInfo;

	//	MeshVertex* compressor = new MeshVertex();

	//	compressor->compressVertices(
	//		vBinAccessorData,
	//		skinInfo,
	//		vSubMeshInfo,
	//		mFbxManager,
	//		mOptimizationFlag,
	//		mesh,
	//		maxMaterialCount,
	//		iter->second.bboxMin,
	//		iter->second.bboxMax,
	//		szBinFile,
	//		triangleCountOfMesh,
	//		polygonCountOfMesh,
	//		vertexCountOfMesh,
	//		iter->second.isSkinned);

	//	if (skinInfo.vBoneNodeID.size() > 0) {
	//		vSkins.push_back(skinInfo);
	//		iter->second.skinIndex = vSkins.size() - 1;
	//	}

	//	delete compressor;

	//	std::map<string, int> mMeshBuffMap;
	//	std::map<int, stMorphTargetAttr> morphTargetMap;
	//	vector<int> vSubMeshIndices;
	//	for (auto it = vBinAccessorData.cbegin(); it != vBinAccessorData.cend(); ++it) {
	//		auto data = (*it);
	//		if (data.iMorphUniqueID > 0) {
	//			//morph target iMorphUniqueID 非0
	//			stMorphTargetAttr mta;
	//			auto find_it = morphTargetMap.find(data.iMorphUniqueID);
	//			if (find_it == morphTargetMap.cend()) {
	//				morphTargetMap.insert(std::pair<int, stMorphTargetAttr>(data.iMorphUniqueID, mta));
	//				find_it = morphTargetMap.find(data.iMorphUniqueID);
	//			}
	//			switch (data.iAttrType) {
	//			case MORPH_TARGET_NORMAL:
	//				find_it->second.iNormalIndex = data.iIsolateIndex;
	//				break;
	//			case MORPH_TARGET_POSITION:
	//				find_it->second.iPostionIndex = data.iIsolateIndex;
	//				break;
	//			case MORPH_TARGET_TANGENT:
	//				find_it->second.iTangentIndex = data.iIsolateIndex;
	//				break;
	//			default:
	//				//dzlog_info("unknwon iAttrType %d",data.iAttrType);
	//				break;
	//			}
	//			mBinData->add(data);
	//		}
	//		else {
	//			if (data.szTypeName == "indices" && vSubMeshInfo.size() > 0) {
	//				//submesh vertex index
	//				vSubMeshIndices.push_back(data.iIsolateIndex);
	//				mBinData->add(data);
	//			}
	//			else {
	//				mMeshBuffMap.insert(std::pair<string, int>(data.szTypeName, data.iIsolateIndex));
	//				mBinData->add(data);
	//			}
	//		}
	//	}

	//	//dzlog_info("Get Mesh with polygon %d triangle %d vertex %d submesh %ld",
	//	//   polygonCountOfMesh,triangleCountOfMesh, vertexCountOfMesh, vSubMeshInfo.size());

	//	FBXSDK_NAMESPACE::FbxNode* parentNode = mesh->GetNode();
	//	string szMeshName = parentNode->GetName();
	//	unsigned iMeshUID = mesh->GetUniqueID();
	//	mJSONWriter->StartObject();
	//	szMeshName = intToString(iMeshUID) + "_" + szMeshName;
	//	mJSONWriter->Key("name");
	//	mJSONWriter->String(szMeshName.c_str());
	//	mJSONWriter->Key("primitives");
	//	mJSONWriter->StartArray();
	//	for (size_t i = 0; i < vSubMeshIndices.size(); ++i) {
	//		mJSONWriter->StartObject();
	//		mJSONWriter->Key("attributes");
	//		mJSONWriter->StartObject();
	//		findMeshAttributesAndWriteIndex("POSITION", mMeshBuffMap, mJSONWriter, ibufferViewerIndex);
	//		findMeshAttributesAndWriteIndex("NORMAL", mMeshBuffMap, mJSONWriter, ibufferViewerIndex);
	//		findMeshAttributesAndWriteIndex("TEXCOORD_0", mMeshBuffMap, mJSONWriter, ibufferViewerIndex);
	//		findMeshAttributesAndWriteIndex("TANGENT", mMeshBuffMap, mJSONWriter, ibufferViewerIndex);
	//		findMeshAttributesAndWriteIndex("COLOR_0", mMeshBuffMap, mJSONWriter, ibufferViewerIndex);
	//		findMeshAttributesAndWriteIndex("JOINTS_0", mMeshBuffMap, mJSONWriter, ibufferViewerIndex);
	//		findMeshAttributesAndWriteIndex("WEIGHTS_0", mMeshBuffMap, mJSONWriter, ibufferViewerIndex);
	//		mJSONWriter->EndObject();
	//		mJSONWriter->Key("indices");
	//		mJSONWriter->Uint(ibufferViewerIndex + vSubMeshIndices.at(i));
	//		mJSONWriter->Key("material");
	//		mJSONWriter->Uint(iter->second.materials[i]);
	//		if (morphTargetMap.size() > 0) {
	//			mJSONWriter->Key("targets");
	//			mJSONWriter->StartArray();
	//			for (auto it = morphTargetMap.cbegin(); it != morphTargetMap.cend(); ++it) {
	//				mJSONWriter->StartObject();
	//				if (it->second.iPostionIndex > 0) {
	//					mJSONWriter->Key("POSITION");
	//					mJSONWriter->Uint(it->second.iPostionIndex);
	//				}
	//				if (it->second.iNormalIndex > 0) {
	//					mJSONWriter->Key("NORMAL");
	//					mJSONWriter->Uint(it->second.iNormalIndex);
	//				}
	//				if (it->second.iTangentIndex > 0) {
	//					mJSONWriter->Key("TANGENT");
	//					mJSONWriter->Uint(it->second.iTangentIndex);
	//				}
	//				mJSONWriter->EndObject();
	//			}
	//			mJSONWriter->EndArray();
	//		}
	//		mJSONWriter->Key("mode");
	//		// 4: GLTF2.EMeshPrimitiveMode.TRIANGLES
	//		mJSONWriter->Uint(4);
	//		mJSONWriter->EndObject();
	//	}
	//	mJSONWriter->EndArray();
	//	if (morphTargetMap.size() > 0) {
	//		mJSONWriter->Key("weights");
	//		mJSONWriter->StartArray();
	//		for (auto it = morphTargetMap.cbegin(); it != morphTargetMap.cend(); ++it) {
	//			mJSONWriter->Double(it->second.fWeight);
	//		}
	//		mJSONWriter->EndArray();
	//	}
	//	mJSONWriter->EndObject();

	//	//for later node json value write
	//	iter->second.meshArrayIndex = iMeshCount++;

	//	ibufferViewerIndex += vBinAccessorData.size();

	//	mTotalTriangleCount += triangleCountOfMesh;
	//	mTotalPolygonCount += polygonCountOfMesh;
	//	mTotalVertexCount += vertexCountOfMesh;

	//	meshes.push_back(mesh);
	//}

	//mJSONWriter->EndArray();

	

	//writeSceneFile(materials, textureMaps, textureFiles);

	for (int i = 0; i < (int)materials.size(); ++i)
	{
		delete materials[i];
	}

	//SkinWriter::write(mJSONWriter, vSkins, mNodeIDtoIndexMap, szBinFile, mBinData);

	//vector<RawAnimation> vAnimationArray;
	//AnimationExport::getSceneAnimation(scene, vAnimationArray);
	//AnimationExport::writeAnimationBin(vAnimationArray, szAnimationFile, mBinData);

	//AnimationsWriter::write(mJSONWriter, vAnimationArray, mNodeIDtoIndexMap);

	//mBinData->writeJSON(mJSONWriter);

	mJSONWriter->EndObject();

	fclose(fp);
}

void PlayCanvasExporter::writeNodeArray()
{
    mJSONWriter->Key("nodes");
    mJSONWriter->StartArray();
    for (auto it = mVecNodeInfo.cbegin(); it != mVecNodeInfo.cend(); ++it) 
	{
        stNodeInfo info = *it;

        mJSONWriter->StartObject();

        mJSONWriter->Key("name");
        mJSONWriter->String(info.szName.c_str());

		mJSONWriter->Key("position");
		mJSONWriter->StartArray();
		mJSONWriter->Double(info.translation[0]);
		mJSONWriter->Double(info.translation[1]);
		mJSONWriter->Double(info.translation[2]);
		mJSONWriter->EndArray();

        mJSONWriter->Key("rotation");
        mJSONWriter->StartArray();
        mJSONWriter->Double(info.rotation[0]);
        mJSONWriter->Double(info.rotation[1]);
        mJSONWriter->Double(info.rotation[2]);
        mJSONWriter->EndArray();

        mJSONWriter->Key("scale");
        mJSONWriter->StartArray();
        mJSONWriter->Double(info.scale[0]);
        mJSONWriter->Double(info.scale[1]);
        mJSONWriter->Double(info.scale[2]);
        mJSONWriter->EndArray();

        mJSONWriter->Key("scaleCompensation");
        mJSONWriter->Bool(false);

        mJSONWriter->EndObject();
    }
    mJSONWriter->EndArray();

	mJSONWriter->Key("parents");
	mJSONWriter->StartArray();
	for (auto it = mVecNodeInfo.cbegin(); it != mVecNodeInfo.cend(); ++it)
	{
		stNodeInfo info = *it;
		mJSONWriter->Int(info.parentIndex);
	}
	mJSONWriter->EndArray();
}

void PlayCanvasExporter::writeSceneFile(
	std::vector<Material*>& materials,
	std::vector<TextureMap*>& textureMaps,
	std::vector<std::string>& textureFiles) 
{
    //basic info
    mJSONWriter->Key("asset");
        mJSONWriter->StartObject();
        mJSONWriter->Key("generator");
        mJSONWriter->String("glTFExporter v0.0.3");
        mJSONWriter->Key("version");
        mJSONWriter->String("2.0");
    mJSONWriter->EndObject();

    mJSONWriter->Key("scene");
        mJSONWriter->Uint(0);

    //scene info
    mJSONWriter->Key("scenes");
        mJSONWriter->StartArray();
            mJSONWriter->StartObject();
                mJSONWriter->Key("name");
                mJSONWriter->String("Root Scene");
                mJSONWriter->Key("nodes");
                    mJSONWriter->StartArray();
                    mJSONWriter->Uint(mVecNodeInfo.size() - 1);
                    mJSONWriter->EndArray();
                mJSONWriter->Key("extras");
                mJSONWriter->StartObject();
                    mJSONWriter->Key("totalTriangleCount");
                    mJSONWriter->Uint(mTotalTriangleCount);
                    mJSONWriter->Key("totalVertexCount");
                    mJSONWriter->Uint(mTotalVertexCount);
                    mJSONWriter->Key("totalPolygonCount");
                    mJSONWriter->Uint(mTotalPolygonCount);
                mJSONWriter->EndObject();
            mJSONWriter->EndObject();
        mJSONWriter->EndArray();

    //dzlog_info("texture map count %ld, texture file count %ld",textureMaps.size(),textureFiles.size());
	writeTextures(textureMaps, textureFiles);

    writeMaterials(materials);
}

void PlayCanvasExporter::gatherAllUsedMaterils_r(
	FBXSDK_NAMESPACE::FbxNode* node, 
	std::vector<FBXSDK_NAMESPACE::FbxUInt64>& effectiveBones)
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
					MeshVertex::getEffectiveBones(mesh, effectiveBones);

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
                                    //FBXSDK_printf("Error: The mesh belong to multiple nodes!\n");
                                    iter->second.mesh = mesh;
                                    iter->second.meshName = mesh->GetNode()->GetName();
                                    iter->second.materials.push_back(index0);
                                }
                                else
                                {
                                    mMeshDepot[meshUniqueID] = MeshInfo();
                                    mMeshDepot[meshUniqueID].mesh = mesh;
                                    mMeshDepot[meshUniqueID].meshName = mesh->GetNode()->GetName();
                                    mMeshDepot[meshUniqueID].materials.push_back(index0);
                                }
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

                        mGeneratedMaterialCount++;

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
                            mMeshDepot[meshUniqueID].mesh = mesh;
                            mMeshDepot[meshUniqueID].meshName = mesh->GetNode()->GetName();
                            mMeshDepot[meshUniqueID].materials.push_back(index0);
                        }
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

void PlayCanvasExporter::writeTextures(vector<TextureMap *> textureMaps,vector<string>& textures)
{
    if (textureMaps.size() == 0 || textures.size() == 0) {
        return;
    }

    for (auto it = textureMaps.begin(); it != textureMaps.end(); ++it) {
        //dzlog_info("%ld texture %s",it - textureMaps.begin(),(*it)->fileName.c_str());
    }

    mJSONWriter->Key("textures");
    mJSONWriter->StartArray();
    for (auto it = textureMaps.cbegin(); it != textureMaps.cend(); ++it) {
        mJSONWriter->StartObject();
        mJSONWriter->Key("sampler");
        mJSONWriter->Uint(0);
        mJSONWriter->Key("source");
        mJSONWriter->Uint(it - textureMaps.cbegin());
        mJSONWriter->EndObject();
    }
    mJSONWriter->EndArray();


    mJSONWriter->Key("samplers");
    mJSONWriter->StartArray();
        mJSONWriter->StartObject();
        //mJSONWriter->Key("magFilter");
        //mJSONWriter->Uint(9729);
        //mJSONWriter->Key("minFilter");
        //mJSONWriter->Uint(9986);
        //mJSONWriter->Key("wrapS");
        //mJSONWriter->Uint(10497);
        //mJSONWriter->Key("wrapT");
        //mJSONWriter->Uint(10497);
        mJSONWriter->EndObject();
    mJSONWriter->EndArray();

    mJSONWriter->Key("images");
    mJSONWriter->StartArray();
	for (int i = 0; i < (int)textureMaps.size(); ++i)
	{

		unsigned int width = 0;
		unsigned int height = 0;
        string filename = (*textureMaps[i]).fileName;
		_FILE_TYPE fileType = getFileType(filename);
        string fullTexturePath = mWorkingDirectory + filename;
		switch (fileType)
		{
		case FT_PNG:
			getPNGInfo(fullTexturePath, width, height);
			break;
		case FT_JPG:
			getJPEGInfo(fullTexturePath, width, height);
			break;
		default:
			break;
		}		

		string fileName = getFileName(filename);

        mJSONWriter->StartObject();
        mJSONWriter->Key("uri");
        mJSONWriter->String(fileName.c_str());
        mJSONWriter->Key("extras");
            mJSONWriter->StartObject();
                mJSONWriter->Key("width");
                mJSONWriter->Uint(width);
                mJSONWriter->Key("height");
                mJSONWriter->Uint(height);
            mJSONWriter->EndObject();
        mJSONWriter->EndObject();
	}
    mJSONWriter->EndArray();
}

void PlayCanvasExporter::writeMapParameters(
	JSONFileWriter* jsonWriter,
	TextureMap* map,
	int mapID,
	std::vector<std::string>* textures,
	bool isLastItem,
	int jsonLevel)
{
	char buffer[__TEMP_BUFFER_FLOAT__];

	jsonWriter->writeObjectInfo("{", jsonLevel);

	FBXSDK_NAMESPACE::FbxDouble3 T(0.0, 0.0, 0.0);
	FBXSDK_NAMESPACE::FbxDouble3 R(0.0, 0.0, 0.0);
	FBXSDK_NAMESPACE::FbxDouble3 S(1.0, 1.0, 1.0);
	FBXSDK_NAMESPACE::FbxDouble3 Rp(0.0, 0.0, 0.0);
	FBXSDK_NAMESPACE::FbxDouble3 Sp(0.0, 0.0, 0.0);

	FBXSDK_NAMESPACE::FbxFileTexture::EWrapMode wrapU = FBXSDK_NAMESPACE::FbxFileTexture::eRepeat;
	FBXSDK_NAMESPACE::FbxFileTexture::EWrapMode wrapV = FBXSDK_NAMESPACE::FbxFileTexture::eRepeat;
	FbxBool uvSwapFlag = false;
	std::string fileName = "";

	if (map)
	{
		T = map->T;
		R = map->R;
		S = map->S;
		Rp = map->Rp;
		Sp = map->Sp;

		wrapU = map->wrapU;
		wrapV = map->wrapV;
		uvSwapFlag = map->uvSwapFlag;

		fileName = getFileName(map->fileName);
	}

	sprintf(buffer, "\"id\":%d,", mapID);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);

	sprintf(buffer, "\"texture\":\"%s\",", fileName.c_str());
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);

	sprintf(buffer, "\"wrapModeU\":%d,", wrapU);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);

	sprintf(buffer, "\"wrapModeV\":%d,", wrapV);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);

	sprintf(buffer, "\"minFilter\":%d,", 5);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);

	sprintf(buffer, "\"maxFilter\":%d,", 3);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);

	sprintf(buffer, "\"uvSwap\":%s,", uvSwapFlag ? "true" : "false");
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);

	sprintf(buffer, "\"T\":[%f,%f,%f],", T[0], T[1], T[1]);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);

	sprintf(buffer, "\"R\":[%f,%f,%f],", R[0], R[1], R[1]);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);

	sprintf(buffer, "\"S\":[%f,%f,%f],", S[0], S[1], S[1]);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);

	sprintf(buffer, "\"Rp\":[%f,%f,%f],", Rp[0], Rp[1], Rp[1]);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);

	sprintf(buffer, "\"Sp\":[%f,%f,%f]", Sp[0], Sp[1], Sp[1]);
	jsonWriter->writeObjectInfo(buffer, jsonLevel + 1);

	if (isLastItem)
	{
		jsonWriter->writeObjectInfo("}", jsonLevel);
	}
	else
	{
		jsonWriter->writeObjectInfo("},", jsonLevel);
	}
}

void PlayCanvasExporter::writeMaps(
	JSONFileWriter* jsonWriter,
	std::vector<TextureMap*>& textureMaps, 
	std::vector<std::string>& textures, 
	int jsonLevel)
{
	for (int i = 0; i < (int)textureMaps.size(); ++i)
	{
		TextureMap* map = textureMaps[i];

		bool isLastItem = (i == (int)textureMaps.size() - 1) ? true : false;
		
		writeMapParameters(jsonWriter, map, i, &textures, isLastItem, jsonLevel);
	}
}

void PlayCanvasExporter::writeMaterialParameters(Material* cache)
{

    mJSONWriter->StartObject();
     mJSONWriter->Key("extras");
     mJSONWriter->StartObject();
         mJSONWriter->Key("id");
         mJSONWriter->Uint(cache->mUniqueID);
         mJSONWriter->Key("culling");
         mJSONWriter->Uint(cache->mCulling);
     mJSONWriter->EndObject();

     mJSONWriter->Key("name");
     mJSONWriter->String((const char*)cache->mMaterialName);

     mJSONWriter->Key("pbrMetallicRoughness");
      mJSONWriter->StartObject();
      mJSONWriter->Key("baseColorFactor");
      mJSONWriter->StartArray();
        mJSONWriter->Double(cache->mDiffuseColor[0]);
        mJSONWriter->Double(cache->mDiffuseColor[1]);
        mJSONWriter->Double(cache->mDiffuseColor[2]);
        mJSONWriter->Double(1.0f);
      mJSONWriter->EndArray();
      if (cache->mDiffuseMapIndexInDepot >= 0) {
          mJSONWriter->Key("baseColorTexture");
              mJSONWriter->StartObject();
                  mJSONWriter->Key("index");
                  mJSONWriter->Uint(cache->mDiffuseMapIndexInDepot);
                  mJSONWriter->Key("texCoord");
                  //This integer value is used to construct a string in the format TEXCOORD_<set index> which is a reference to a key in mesh.primitives.attributes (e.g. A value of 0 corresponds to TEXCOORD_0). Mesh must have corresponding texture coordinate attributes for the material to be applicable to it.
                  mJSONWriter->Uint(0);
              mJSONWriter->EndObject();
      }
      //这两个参数如何获取
      mJSONWriter->Key("metallicFactor");
      mJSONWriter->Uint(0);
      mJSONWriter->Key("roughnessFactor");
      mJSONWriter->Double(0.6);
      mJSONWriter->EndObject();

     mJSONWriter->Key("emissiveFactor");
       mJSONWriter->StartArray();
       mJSONWriter->Uint(0);
       mJSONWriter->Uint(0);
       mJSONWriter->Uint(0);
       mJSONWriter->EndArray();
     mJSONWriter->Key("alphaMode");
     // Other Choice OPAQUE MASK 
     // BLEND - Support for this mode varies. There is no perfect and fast solution that works for all cases. Implementations should try to achieve the correct blending output for as many situations as possible. Whether depth value is written or whether to sort is up to the implementation. For example, implementations can discard pixels which have zero or close to zero alpha value to avoid sorting issues.
     mJSONWriter->String("BLEND");
     mJSONWriter->Key("doubleSided");
     mJSONWriter->Bool(false);
    mJSONWriter->EndObject();
}

void PlayCanvasExporter::writeMaterials(std::vector<Material*> materials)
{	
    mJSONWriter->Key("materials");
    mJSONWriter->StartArray();
	for (int i = 0; i < (int)materials.size(); ++i)
	{
		Material* cache = materials[i];
		writeMaterialParameters(cache);
	}
    mJSONWriter->EndArray();
}

void PlayCanvasExporter::processNode_r(FBXSDK_NAMESPACE::FbxNode* node, int parentIndex)
{
	bool layerVis = true;
	FBXSDK_NAMESPACE::FbxDisplayLayer* displayLayer = 
		(FBXSDK_NAMESPACE::FbxDisplayLayer*)
		(
			node->GetDstObject
			(
				FBXSDK_NAMESPACE::FbxCriteria::ObjectType(FBXSDK_NAMESPACE::FbxDisplayLayer::ClassId), 
				0
			)
		);

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


	FbxUInt64 id = node->GetUniqueID();
	stNodeInfo nodeInfo;
	nodeInfo.parentIndex = parentIndex;
    nodeInfo.iNodeID = id;

	string nodeName = standardizeFileName(node->GetName());
    nodeInfo.szName = nodeName;
	
    const FBXSDK_NAMESPACE::FbxAMatrix    localTransform   = node->EvaluateLocalTransform();
    const FBXSDK_NAMESPACE::FbxVector4    localTranslation = localTransform.GetT();
    const FBXSDK_NAMESPACE::FbxVector4    localRotation    = localTransform.GetR();
    const FBXSDK_NAMESPACE::FbxVector4    localScale       = localTransform.GetS();

    nodeInfo.translation[0] = safeDouble(localTranslation[0]);
    nodeInfo.translation[1] = safeDouble(localTranslation[1]);
    nodeInfo.translation[2] = safeDouble(localTranslation[2]);
    nodeInfo.rotation[0] = safeDouble(localRotation[0]);
    nodeInfo.rotation[1] = safeDouble(localRotation[1]);
    nodeInfo.rotation[2] = safeDouble(localRotation[2]);
    nodeInfo.rotation[3] = safeDouble(localRotation[3]);
    nodeInfo.scale[0] = safeDouble(localScale[0]);
    nodeInfo.scale[1] = safeDouble(localScale[1]);
    nodeInfo.scale[2] = safeDouble(localScale[2]);

	//const int materialCount = node->GetMaterialCount();
	//for (int i = 0; i < materialCount; ++i)
	//{
	//	FBXSDK_NAMESPACE::FbxSurfaceMaterial* material = node->GetMaterial(i);
	//	if (material != NULL)
	//	{
	//		void* p = material->GetUserDataPtr();
	//		unsigned long long v = reinterpret_cast<unsigned long long>(p);
	//		unsigned int index = (unsigned int)(v & 0xffffffff);
	//		if (i == materialCount - 1)
	//		{
	//			sprintf(buffer, "%d", (int)index);
	//		}
	//		else
	//		{
	//			sprintf(buffer, "%d,", (int)index);
	//		}
	//	}
	//}
	
	//if (vis && showFlag && layerVis)
	//{
	//	FBXSDK_NAMESPACE::FbxNodeAttribute* nodeAttr = node->GetNodeAttribute();
	//	if (nodeAttr)
	//	{			
	//		FBXSDK_NAMESPACE::FbxNodeAttribute::EType nodeAttrType = nodeAttr->GetAttributeType();
	//		switch (nodeAttrType)
	//		{
	//		case FBXSDK_NAMESPACE::FbxNodeAttribute::eMesh:
	//		{
	//			FBXSDK_NAMESPACE::FbxMesh* mesh = node->GetMesh();
	//			int meshUniqueID = (int)mesh->GetUniqueID();

	//			std::map<int, MeshInfo>::iterator iter = mMeshDepot.find(meshUniqueID);
	//			if (iter != mMeshDepot.end())
	//			{			
 //                   nodeInfo.iMeshIndex = iter->second.meshArrayIndex;
 //                   if (iter->second.isSkinned) {
 //                       nodeInfo.iSkinIndex = iter->second.skinIndex;
 //                   }
	//			}
	//			else
	//			{
	//				FBXSDK_printf("Error: found a dummy mesh.");
	//			}
	//			
	//			break;
	//		}
	//		case FBXSDK_NAMESPACE::FbxNodeAttribute::eSkeleton:
 //           {
 //               //dzlog_info("get bone node %lld",id);
	//			break;
 //           }
	//		case FBXSDK_NAMESPACE::FbxNodeAttribute::eNurbs:
	//		case FBXSDK_NAMESPACE::FbxNodeAttribute::ePatch:
	//		case FBXSDK_NAMESPACE::FbxNodeAttribute::eNurbsSurface:
	//		case FBXSDK_NAMESPACE::FbxNodeAttribute::eShape:
	//		case FBXSDK_NAMESPACE::FbxNodeAttribute::eNurbsCurve:
	//		{
	//			FBXSDK_printf("%s : %d\n", "Error: do not support specified node attribute", nodeAttrType);
	//			break;
	//		}
	//		case FBXSDK_NAMESPACE::FbxNodeAttribute::eLight:
	//		case FBXSDK_NAMESPACE::FbxNodeAttribute::eCamera:
	//		case FBXSDK_NAMESPACE::FbxNodeAttribute::eUnknown:
	//		default:				
	//			break;
	//		}
	//	}
	//}

	mVecNodeInfo.push_back(nodeInfo);
	int nodeIndex = int(mVecNodeInfo.size() - 1);

    int childCount = node->GetChildCount();
    if (childCount > 0) 
	{
        for (int i = 0; i < childCount; ++i)
        {
            FBXSDK_NAMESPACE::FbxNode* child = node->GetChild(i);
			if (child != NULL)
			{
				processNode_r(child, nodeIndex);
			}
        }
    }
}
