#ifndef QS_FBXImporter_h__
#define QS_FBXImporter_h__
#pragma warning(push, 1)
#pragma warning (disable:4100)
#include <fbxsdk.h>
#include <fbxfilesdk/kfbxio/kfbxiosettings.h>
#pragma warning(pop)

#define WIN32_LEAN_AND_MEAN
#ifndef  _WIN32_WINDOWS
	#define _WIN32_WINDOWS 0x0500	// for IsDebuggerPresent
#endif
#define NOMINMAX
#include <windows.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <stdio.h>
#include <iostream>
#include <assert.h>
#include <map>
#include <unordered_map>
//#include <d3dx9math.h>

#include "Core.h"
#include "CommonRender.h"
#include "QSRenderBaseType.h"
#include "QSMaterial.h"

//using namespace std;
typedef unsigned int uint;
typedef unsigned short ushort;

//---------------------------------------------------------------------
// helper functions
//---------------------------------------------------------------------
inline D3DXVECTOR3 KFbxColorToD3DXVECTOR3( const KFbxColor& vSrcVector )
{
	return D3DXVECTOR3( (float)vSrcVector.mRed, (float)vSrcVector.mGreen, (float)vSrcVector.mBlue );
}

inline D3DXVECTOR3 KFbxVector4ToD3DXVECTOR3( const KFbxVector4& vSrcVector )
{
	return D3DXVECTOR3( (float)vSrcVector.GetAt(0), (float)vSrcVector.GetAt(1), (float)vSrcVector.GetAt(2) );
}

inline D3DXVECTOR2 KFbxVector2ToD3DXVECTOR2(KFbxVector2& vSrcVector)
{
	return D3DXVECTOR2( (float)vSrcVector.GetAt(0), (float)vSrcVector.GetAt(1) );
}

inline D3DXVECTOR2 FBXTexCoord2DirectXTexCoord(const D3DXVECTOR2& vSrcVector)
{
	return D3DXVECTOR2( (float)vSrcVector.x, 1.0f - (float)vSrcVector.y );
}

inline D3DXMATRIX KFbxXMatrixToD3DXMATRIX(KFbxXMatrix& matSrcMatrix)
{
	KFbxVector4 r1 = matSrcMatrix.GetRow(0);
	KFbxVector4 r2 = matSrcMatrix.GetRow(1);
	KFbxVector4 r3 = matSrcMatrix.GetRow(2);
	KFbxVector4 r4 = matSrcMatrix.GetRow(3);

	return D3DXMATRIX( (float)r1.GetAt(0), (float)r1.GetAt(1), (float)r1.GetAt(2), (float)r1.GetAt(3),
		(float)r2.GetAt(0), (float)r2.GetAt(1), (float)r2.GetAt(2), (float)r2.GetAt(3), 
		(float)r3.GetAt(0), (float)r3.GetAt(1), (float)r3.GetAt(2), (float)r3.GetAt(3),
		(float)r4.GetAt(0), (float)r4.GetAt(1), (float)r4.GetAt(2), (float)r4.GetAt(3) );
}

//---------------------------------------------------------------------
struct BLENDWEIGHTS
{
	eastl::vector<int>			blendIndices;
	eastl::vector<float>		blendWeights;
};
struct SkinningData
{
	SkinningData( int controlPointCnt );
	~SkinningData();
	void				AddBoneInvolved( const char* boneName );
	int					GetInvolvedBoneCount() const { return mBoneNames.size(); }
	const char*			GetBoneName( int index );

	void				AddBlendWeight( int controlPointIndex, int boneIndex, float blendWeight );
	void				Normalize();

public:
	eastl::vector<BLENDWEIGHTS> mBlendWeights;
	eastl::vector<D3DXMATRIX>	 mSkinToBoneMats;
private:
	eastl::vector<eastl::string>		 mBoneNames;

};
//---------------------------------------------------------------------
struct QS_VERTEX
{
	QS_VERTEX() : cpIndex(-1), smoothingIndex(0), mFaceParity(false), uvIndex(0), useOriginalTBN(false) {}
	QS_VERTEX( const QS_VERTEX& vertex );
	~QS_VERTEX();
	bool Equal ( QS_VERTEX* rhs ) const;

	D3DXVECTOR3		position;
	eastl::vector<D3DXVECTOR2>		UVs;   // two sets of uv we support
	D3DXVECTOR3		normal;
	D3DXVECTOR3		binormal;
	D3DXVECTOR3		tangent;
	D3DXVECTOR3		vertColor;
	float			tbnDir;		//direction of TBN space
	bool			mFaceParity;
	bool			hasMirror;
	bool			hasRotation;
	bool			useOriginalTBN;
	BLENDWEIGHTS	blendWeights;

	int				cpIndex;

	int				smoothingIndex;

	float			angleOfEdges;
	int				uvIndex;		// when vertex merge start, if uvIndices are different means the two vertex can't merge
};

typedef eastl::vector<QS_VERTEX*>		Vertices;
struct ParamBool
{
	NameHandle	name;
	bool		value;
	ParamBool(NameHandle n,bool v)
		:name(n)
		,value(v){}
};
struct ParamFloat
{
	NameHandle	name;
	f32			value;
	ParamFloat(NameHandle n,f32 v)
		:name(n)
		,value(v){}
};
struct ParamFloat2
{
	NameHandle	name;
	QSVec2f		value;
	ParamFloat2(NameHandle n,const QSVec2f& v)
		:name(n)
		,value(v){}
};
struct ParamFloat3
{
	NameHandle	name;
	QSVec3f		value;
	ParamFloat3(NameHandle n,const QSVec3f& v)
		:name(n)
		,value(v){}
};
struct ParamFloat4
{
	NameHandle	name;
	QSVec4f		value;
	ParamFloat4(NameHandle n,const QSVec4f& v)
		:name(n)
		,value(v){}
};
struct ParamTex
{
	NameHandle	name;
	qsstring	value;
	KFbxFileTexture* tex;
	ParamTex(NameHandle n,const qsstring& v , KFbxFileTexture* t)
		:name(n)
		,value(v)
		,tex(t){}
};
//---------------------------------------------------------------------
// class FBXMaterial
//---------------------------------------------------------------------
struct RENDER_API QS_FBXMaterial
{
	QS_FBXMaterial():mHasAlpha(false), mTwoSided(false){}
	~QS_FBXMaterial();

	enum TEX_CHANNEL
	{
		TC_AMBIENT,
		TC_DIFFUSE,
		TC_SPEC_COLOR,
		TC_BUMP_MAP,
		TC_DISPLACEMENT,
		TC_EMISSIVE_MAP,

		TC_UNKNOWN,
	};

	eastl::string			mName;
	eastl::string			mShaderName;
	typedef eastl::map<TEX_CHANNEL, eastl::string>	TextureMapping;
	TextureMapping			mTextureURLs;
	bool					mHasAlpha;
	bool					mTwoSided;
	bool					mNeedVC;

	qsvector<ParamBool>							mBoolParams;
	qsvector<ParamFloat>  						mFloatParams;
	qsvector<ParamFloat2> 						mFloat2Params;
	qsvector<ParamFloat3> 						mFloat3Params;
	qsvector<ParamFloat4> 						mFloat4Params;
	qsvector<ParamTex>    						mTexParams;

	bool				GetBoolParam(NameHandle name,bool& v);
	bool				GetFloatParam(NameHandle name,f32& v);
	bool				GetFloat2Param(NameHandle name,QSVec2f& v);
	bool				GetFloat3Param(NameHandle name,QSVec3f& v);
	bool				GetFloat4Param(NameHandle name,QSVec4f& v);
	bool				GetTexParam(NameHandle name,eastl::string& v);
	static bool			HasAlpha( int texIndex );
	static TEX_CHANNEL	GetMaterialType( int texIndex );
	static bool			HasTwoSided( int texIndex );

	static const char*	GetTexTypeName( QS_FBXMaterial::TEX_CHANNEL channel );
	
};
//---------------------------------------------------------------------
// class FBXMesh
//---------------------------------------------------------------------
struct RENDER_API QS_FBXMesh
{
	QS_FBXMesh()
		: mIndex( NULL ), mVertexData(NULL), mFbxMat( NULL ), mbSkinned(false),
		mTriangleCnt(0), mCompressingScale(1), mBackupVertexData(NULL)
		, mIsCollisionMesh( false )
		{}

	~QS_FBXMesh();

	QS_FBXMaterial*					mFbxMat;

	eastl::string	mName;
	//index
	uint		mTriangleCnt;
	void*		mIndex;

	float		mCompressingScale;
	//vertex
	bool		mbSkinned;
	bool		mIsCollisionMesh;
	VertexDecl	mVertDecl;
	VertexDecl	mbackupVertDecl;
	Vertices	mVerts;
	int			mVertDataSize;
	void*		mVertexData;
	void*		mBackupVertexData;
	int			mVertStride;
	int			mUVChannelCnt;

	//bones were binded by skinned mesh
	eastl::vector<NameHandle>		mBoneNames;
	eastl::vector<D3DXMATRIX>	mSkinToBoneMats;
	D3DXVECTOR3			mBoundMax;
	D3DXVECTOR3			mBoundMin;
	
	void	ResetVertices();
	void	AssembleVertexData();
	void	Optimize();
	void	ComputeNTB( int cpCount );
	void	SortVertexIndices();
	float	GetVertexDataCompressScale();
};

struct QS_BoundingBox
{
	D3DXVECTOR3		mBoundCenter;
	D3DXVECTOR3		mHalfLength;
};
//---------------------------------------------------------------------
// class FBXNode
//---------------------------------------------------------------------
class RENDER_API QS_FBXNode
{
public:
	QS_FBXNode()
		: mParent ( NULL ){}
	virtual ~QS_FBXNode();
	bool			AddChild( QS_FBXNode* pNode );
	bool			RemoveChild( QS_FBXNode* pNode );
	QS_FBXNode*		GetChild( int idx );
	int				GetChildrenCount() const { return (int)mChildren.size(); }
	void			SetlclTransform( KFbxNode* pNode );
	void			SetBonelclTransform( KFbxNode* pNode );

public:
	eastl::string	mName;
	D3DXVECTOR3		mTranslation;
	D3DXVECTOR3		mRotation[3];
	D3DXVECTOR3		mScaling;

	typedef eastl::vector<QS_BoundingBox> BoundingBoxArray;
	BoundingBoxArray	mBoundingBoxes;
protected:
	QS_FBXNode*		mParent;
	typedef eastl::vector<QS_FBXNode*> FBXNodeArray;
	FBXNodeArray	mChildren;
};
//---------------------------------------------------------------------
// class FBXMeshNode
//---------------------------------------------------------------------
class RENDER_API QS_FBXMeshNode: public QS_FBXNode
{
public:
	~QS_FBXMeshNode();

	void			AddMesh( const eastl::string& matName, QS_FBXMesh* pMesh );
	void			AddMeshLOD( const eastl::string& matName, QS_FBXMesh* pMeshLOD );
	void			AddCollisionMesh( QS_FBXMesh* pMesh );
	int				GetMeshCount() const { return (int)mFbxMeshes.size(); }
	int				GetMeshLODCount( const eastl::string& matName );
	int				GetMaxMeshLODCount();
	QS_FBXMesh*		GetMesh( int index, int lodLv = 0 );
	QS_FBXMesh*		GetMesh( const eastl::string& matName, int lodLv = 0 );


private:
	typedef eastl::vector<QS_FBXMesh*>		FBXMeshLOD;
	//typedef std::unordered_map<eastl::string, FBXMeshLOD>		FBXMeshes;
	typedef eastl::map<eastl::string, FBXMeshLOD>		FBXMeshes;

	FBXMeshes						mFbxMeshes;

public:
	FBXMeshLOD						mCollisionMeshes;
};
//---------------------------------------------------------------------
// class FBXBoneNode
//---------------------------------------------------------------------
class RENDER_API QS_FBXBoneNode : public QS_FBXNode
{

public:
	QS_FBXBoneNode()
		: mBoundCenter( D3DXVECTOR3(0,0,0) ), mBoundRadius(0) {}

	void					SetWorldTransform( KFbxNode* pNode );
	const D3DXMATRIX&		GetWorldTransform() const { return mWorld; }

	int						GetBoneCountRecursive();
public:
	D3DXVECTOR3		mBoundCenter;
	float			mBoundRadius;
private:
	D3DXMATRIX		mWorld;

};
//---------------------------------------------------------------------
// class FBXScene
//---------------------------------------------------------------------
struct RENDER_API QS_FBXScene
{
	QS_FBXScene() : mRootNode( NULL ), mHasSkeleton( false ), mSkinned( false ),
		mRootBone(NULL), mBoneCount(0) {}
	~QS_FBXScene();

	eastl::string	mName;
	bool			mSkinned;
	bool			mHasSkeleton;
	QS_FBXNode*		mRootNode;
	QS_FBXBoneNode*	mRootBone;
	int				mBoneCount;
};

//---------------------------------------------------------------------
// class QSFBXImporter
//---------------------------------------------------------------------
class RENDER_API QSFBXImporter
{
public:
	enum NODE_TYPE
	{
		QSNT_MESH,
		QSNT_BONE,
		QSNT_UNKNOWN,
	};

	enum IMPORT_OPTIONS
	{
		IO_GEOMETRY,
		IO_MESH,
		IO_SKELETON,
		IO_BOTH,
		IO_UNKNOWN,
	};

public:
	QSFBXImporter();
	~QSFBXImporter();

	bool				LoadFbxFromFile( const eastl::string& strFileName );
	QS_FBXScene*		ParseFBX(IMPORT_OPTIONS option, bool enableCompress = true, bool enableVertexColor = false, 
								bool uncompressPos = false, bool keepNormal = true );

	void				Reset();

	bool				HasLOD( KFbxNode* pNode );
	// extra collision mesh data especially for editor
	bool				IsCollisionMesh( KFbxMesh* pMesh );
private:
	QS_FBXNode*			ParseMeshNode( KFbxNode* pNode, bool isLOD = false, QS_FBXMeshNode* pParentLOD = NULL );
	QS_FBXBoneNode*		ParseBoneNode( KFbxNode* pNode );
	
	QS_FBXMesh*			ParseMesh( KFbxMesh* pMesh );
	void				ParseMeshByMultipleMats( KFbxMesh* pMesh, eastl::map<QS_FBXMaterial*, QS_FBXMesh*>& subMeshes );
	void				PostParseMesh( QS_FBXMesh* pMesh, KFbxMesh* pFbxMesh );

	bool				ParseMaterial( KFbxMesh* pMesh, QS_FBXMeshNode* pInNode, bool isLOD = false );
	bool				ParseHLSLMat(KFbxSurfaceMaterial* pMat,QS_FBXMaterial* pMyMat);
	int					 GetMaterialIndexLinkedWithPolygon(KFbxMesh* pFBXMesh, 
							int nLayerIndex, int nPolygonIndex, int nPolygonVertexIndex, int nVertexIndex);

	void				ParseBoneWeights( KFbxMesh* pMesh, SkinningData& skinningData );
	void				ParseBoneWeights( KFbxSkin* pSkin, SkinningData& skinningData );
	
	D3DXVECTOR2 GetUV( const char* uvName, KFbxMesh* pMesh, int polygonIdx, int posInPoly, int vertIdx );
	D3DXVECTOR3			GetVertexColor( KFbxMesh* pMesh, int cpIdx, int vertIdx );
	D3DXVECTOR3			GetTangent( KFbxMesh* pMesh, int vertIdx );
	D3DXVECTOR3			GetBinormal( KFbxMesh* pMesh, int vertIdx );
	int					GetSmoothingIndex( KFbxMesh* pMesh, int polygonIdx );

	int					GetMappingIndex(KFbxLayerElement::EMappingMode MappingMode, 
							int nPolygonIndex, int nPolygonVertexIndex, int nVertexIndex);

	bool				VerifyMaterialNames( const eastl::vector<QS_FBXMaterial*>& mats );
	bool				IsBoundingVolume( KFbxNode* pNode );
	bool				IsBelong(const eastl::string& boundingBoxName, const eastl::string& meshName );
private:
	KFbxSdkManager*					mFbxSDKManager;
	KFbxScene*						mFbxScene;
	bool							mInited;
	bool							mHasLOD;
	
	eastl::vector<QS_FBXMaterial*>			mMaterials;
public:
	eastl::string					mFileName;
	static bool						VERTEX_COMPRESSING_ENABLE;
	static bool						mUncompressPos;
	static bool						mKeepNormal;
	static bool						mEnableVCForce;
	static bool						mAvatarExport;
	static LPDIRECT3DDEVICE9		Device;
};

#endif // QS_FBXImporter_h__