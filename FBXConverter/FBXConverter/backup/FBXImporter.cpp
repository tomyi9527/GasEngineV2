//#pragma optimize("",off)
#pragma  warning(disable:4100)
#pragma  warning(disable:4512)
#pragma  warning(disable:4239)

#include "RenderPCH.h"
#include "QS_FBXImporter.h"
#include <fbxfilesdk/fbxfilesdk_def.h>
#include <d3dx9math.h>
#include "QSRenderBaseType.h"

bool NodeCompare(KFbxNode* a, KFbxNode* b)
{
	eastl::string as(a->GetName());
	eastl::string bs(b->GetName());
	return as > bs;
}


LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
	case WM_DESTROY:
		PostQuitMessage( 0 );
		return 0;

	case WM_PAINT:
	//	ValidateRect( hWnd, NULL );
		return 0;
	}

	return DefWindowProc( hWnd, msg, wParam, lParam );
}



void D3DOptimizeTriList( QS_FBXMesh* pMesh, LPDIRECT3DDEVICE9 deviceIn )
{
	HINSTANCE hInst;
	UNREFERENCED_PARAMETER( hInst );
	// Register the window class
	WNDCLASSEX wc =
	{
		sizeof( WNDCLASSEX ), CS_CLASSDC, MsgProc, 0L, 0L,
		GetModuleHandle( NULL ), NULL, NULL, NULL, NULL,
		"FbxImporter", NULL
	};
	RegisterClassEx( &wc );

	// Create the application's window
	HWND hWnd = CreateWindow( "FbxImporter", "FbxImporter",
		WS_OVERLAPPEDWINDOW, 100, 100, 300, 300,
		NULL, NULL, wc.hInstance, NULL );

	LPDIRECT3DDEVICE9 device = deviceIn;
	LPDIRECT3D9 pD3D = NULL;
	if( !device )
	{
		pD3D = Direct3DCreate9( D3D_SDK_VERSION );
		if( pD3D )
		{
			D3DPRESENT_PARAMETERS d3dpp;
			ZeroMemory( &d3dpp, sizeof( d3dpp ) );
			d3dpp.Windowed = TRUE;
			d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
			d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

			if( FAILED( pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
				D3DCREATE_SOFTWARE_VERTEXPROCESSING,
				&d3dpp, &device ) ) )
			{
				printf("D3D device create failed.\n" );
			}

		}

		if ( !device )
			return;

	}
	ID3DXMesh* d3dMesh = NULL;
	ID3DXMesh* optimizedMesh = NULL;
	u32 indexCnt = pMesh->mTriangleCnt * 3;
	DWORD flag = D3DXMESH_MANAGED;
	bool index16 = indexCnt > 65534 ? false : true;
	if( !index16 )
		flag = flag | D3DXMESH_32BIT;
	u32 indexStride = index16 ? 2 : 4;

	qsvector<D3DVERTEXELEMENT9> VertexElements;
	VertexDecl& delc = QSFBXImporter::VERTEX_COMPRESSING_ENABLE ? pMesh->mbackupVertDecl : pMesh->mVertDecl;
	for(u32 i = 0;i < delc.mVertexEleVector.size();i++)
	{
		D3DVERTEXELEMENT9 VertexElement;
		VertexElement.Stream = delc.mVertexEleVector[i].StreamIndex;
		VertexElement.Offset = delc.mVertexEleVector[i].Offset;
		switch(delc.mVertexEleVector[i].Type)
		{
		case VET_Float1:		VertexElement.Type = D3DDECLTYPE_FLOAT1; break;
		case VET_Float2:		VertexElement.Type = D3DDECLTYPE_FLOAT2; break;
		case VET_Float3:		VertexElement.Type = D3DDECLTYPE_FLOAT3; break;
		case VET_Float4:		VertexElement.Type = D3DDECLTYPE_FLOAT4; break;
		case VET_PackedNormal:	VertexElement.Type = D3DDECLTYPE_UBYTE4; break;
		case VET_UByte4:		VertexElement.Type = D3DDECLTYPE_UBYTE4; break;
		case VET_UByte4N:		VertexElement.Type = D3DDECLTYPE_UBYTE4N; break;
		case VET_Color:			VertexElement.Type = D3DDECLTYPE_D3DCOLOR; break;
		case VET_Short2:		VertexElement.Type = D3DDECLTYPE_SHORT2; break;
		case VET_Short2N:		VertexElement.Type = D3DDECLTYPE_SHORT2N; break;
		case VET_Short4:		VertexElement.Type = D3DDECLTYPE_SHORT4; break;				
		case VET_Half2:			VertexElement.Type = D3DDECLTYPE_FLOAT16_2; break;
		case VET_Half4:			VertexElement.Type = D3DDECLTYPE_FLOAT16_4; break;
		default: assert(0&&"unsupported data");
		};
		VertexElement.Method = D3DDECLMETHOD_DEFAULT;
		switch(delc.mVertexEleVector[i].Usage)
		{
		case VEU_Position:			VertexElement.Usage = D3DDECLUSAGE_POSITION; break;
		case VEU_TextureCoordinate:	VertexElement.Usage = D3DDECLUSAGE_TEXCOORD; break;
		case VEU_BlendWeight:		VertexElement.Usage = D3DDECLUSAGE_BLENDWEIGHT; break;
		case VEU_BlendIndices:		VertexElement.Usage = D3DDECLUSAGE_BLENDINDICES; break;
		case VEU_Normal:			VertexElement.Usage = D3DDECLUSAGE_NORMAL; break;
		case VEU_Tangent:			VertexElement.Usage = D3DDECLUSAGE_TANGENT; break;
		case VEU_Binormal:			VertexElement.Usage = D3DDECLUSAGE_BINORMAL; break;
		case VEU_Color:				VertexElement.Usage = D3DDECLUSAGE_COLOR; break;
		};
		VertexElement.UsageIndex = delc.mVertexEleVector[i].UsageIndex;

		VertexElements.push_back(VertexElement);
	}

	D3DVERTEXELEMENT9 EndElement = D3DDECL_END();
	VertexElements.push_back(EndElement);

	HRESULT ret = D3DXCreateMesh( pMesh->mTriangleCnt, pMesh->mVerts.size(),
		flag, &VertexElements[0],
		device, &d3dMesh );
	//vertex data
	void* vData = NULL;
	if( QSFBXImporter::VERTEX_COMPRESSING_ENABLE )
		vData = pMesh->mBackupVertexData;
	else
		vData = pMesh->mVertexData;

	void* d3dVertexBuffer = NULL;
	d3dMesh->LockVertexBuffer(0, &d3dVertexBuffer );
	memcpy( d3dVertexBuffer, vData, pMesh->mVertDataSize );
	d3dMesh->UnlockVertexBuffer();

	//index data
	void* d3dIndexBuffer = NULL;
	d3dMesh->LockIndexBuffer(0, &d3dIndexBuffer );
	if( !index16 )
	{
		memcpy( d3dIndexBuffer, pMesh->mIndex, indexCnt * sizeof(uint) );
	}
	else
	{
		u16* ib = new u16[indexCnt];
		u32* val = (u32*)pMesh->mIndex;
		for( u32 i = 0; i < indexCnt; ++i )
		{
			ib[i] = (u16)val[i];
		}
		memcpy( d3dIndexBuffer, (void*)ib, indexCnt * 2 );
		delete [] ib;
	}

	d3dMesh->UnlockIndexBuffer();

	DWORD* adjacencyInfo = new DWORD[d3dMesh->GetNumFaces() * 3];

	ret = d3dMesh->GenerateAdjacency(0.0f, adjacencyInfo);
	ret = d3dMesh->Optimize(D3DXMESHOPT_VERTEXCACHE|D3DXMESHOPT_IGNOREVERTS,adjacencyInfo,NULL, NULL,NULL, &optimizedMesh );
	delete [] adjacencyInfo;
	if( optimizedMesh )
	{
		u32* ib = (u32*)pMesh->mIndex;

		u32 newIdxNum = optimizedMesh->GetNumFaces()*3;
		assert( newIdxNum == indexCnt );
		optimizedMesh->LockIndexBuffer(0, &d3dIndexBuffer );
		u16* ibSrc = (u16*)d3dIndexBuffer;
		if( index16 )
		{
			for( u32 i = 0; i < newIdxNum; ++i )
			{
				ib[i] = ibSrc[i];
			}
		}
		else
		{
			memcpy( (void*)ib, d3dIndexBuffer, newIdxNum*indexStride);
		}

		optimizedMesh->UnlockIndexBuffer();
		optimizedMesh->Release();
	}

	d3dMesh->Release();
	if( !deviceIn )
	{
		if( device )
			device->Release();
		if( pD3D )
			pD3D->Release();
	}
//	PostQuitMessage(0);
	UnregisterClass( "FbxImporter", wc.hInstance );
}

#ifndef HALF_MAX
#define HALF_MAX 65504
#endif
u16 Float32ToFloat16( float val )
{
	u16 hs, he, hm, ret;
	u32 xs, xe, xm;
	u32 x = *(u32*)&val;
	int hes;
	xs = x & 0x80000000u;  // Pick off sign bit
	xe = x & 0x7F800000u;  // Pick off exponent bits
	xm = x & 0x007FFFFFu;  // Pick off mantissa bits

	hs = (u16) (xs >> 16); // Sign bit
	hes = ((int)(xe >> 23)) - 127 + 15; // Exponent unbias the single, then bias the halfp
	if( hes >= 0x1F )
	{  // Overflow
		ret = (u16) ((xs >> 16) | 0x7C00u); // Signed Inf
	} 
	else if( hes <= 0 ) 
	{  // Underflow
		if( (14 - hes) > 24 )
		{  // Mantissa shifted all the way off & no rounding possibility
			hm = (u16) 0u;  // Set mantissa to zero
		}
		else {
			xm |= 0x00800000u;  // Add the hidden leading bit
			hm = (u16) (xm >> (14 - hes)); // Mantissa
			if( (xm >> (13 - hes)) & 0x00000001u ) // Check for rounding
				hm += (u16) 1u; // Round, might overflow into exp bit, but this is OK
		}
		ret = (hs | hm); // Combine sign bit and mantissa bits, biased exponent is zero
	} 
	else 
	{
		he = (u16) (hes << 10); // Exponent
		hm = (u16) (xm >> 13); // Mantissa
		if( xm & 0x00001000u ) // Check for rounding
			ret = (hs | he | hm) + (u16) 1u; // Round, might overflow to inf, this is OK
		else
			ret = (hs | he | hm);  // No rounding
	}

	return ret;

}

float Float16ToFloat32( u16 val )
{
	u16 h, hs, he, hm;
	u32  xs, xe, xm, ret;
	int xes,e;

	h = val;
	hs = h & 0x8000u;  // Pick off sign bit
	he = h & 0x7C00u;  // Pick off exponent bits
	hm = h & 0x03FFu;  // Pick off mantissa bits
	if( he == 0 ) {  // Denormal will convert to normalized
		e = -1; // The following loop figures out how much extra to adjust the exponent
		do
		{
			e++;
			hm <<= 1;
		} while( (hm & 0x0400u) == 0 ); // Shift until leading bit overflows into exponent bit
		xs = ((u32) hs) << 16; // Sign bit
		xes = ((u32) (he >> 10)) - 15 + 127 - e; // Exponent unbias the halfp, then bias the single
		xe = (u32) (xes << 23); // Exponent
		xm = ((u32) (hm & 0x03FFu)) << 13; // Mantissa
		ret = (xs | xe | xm); // Combine sign bit, exponent bits, and mantissa bits
	} 
	else if( he == 0x7C00u )
	{  // Inf or NaN (all the exponent bits are set)
		if( hm == 0 ) 
		{ // If mantissa is zero ...
			ret = (((u32) hs) << 16) | ((u32) 0x7F800000u); // Signed Inf
		} 
		else
		{
			ret = (u32) 0xFFC00000u; // NaN, only 1st mantissa bit set
		}
	}
	else
	{ // Normalized number
		xs = ((u32) hs) << 16; // Sign bit
		xes = ((int) (he >> 10)) - 15 + 127; // Exponent unbias the halfp, then bias the single
		xe = (u32) (xes << 23); // Exponent
		xm = ((u32) hm) << 13; // Mantissa
		ret = (xs | xe | xm); // Combine sign bit, exponent bits, and mantissa bits
	}

	return *(float*)&ret;
}

BYTE EncodeNormalizeFloatToByte( float val )
{
	return (BYTE)(val * 255);
}

float DecodeByteToNormalizeFloat( BYTE val )
{
	return (float)(val / 255.00f);
}

BYTE EncodeNormalizeFloatToChar( float val )
{
	return (BYTE)((val+1) * 127);
}

float DecodeCharToNormalizeFloat( BYTE val )
{
	return (float)((val/ 127.00f) - 1.0f);
}
void Orthonormlize(const D3DXVECTOR3& t, D3DXVECTOR3& b, D3DXVECTOR3& n)
{
	float p1 = D3DXVec3Dot( &b, &t );
	float p2 = D3DXVec3Dot(&t,&t);
	D3DXVECTOR3 oB = b - (p1/p2)*t;
	D3DXVec3Normalize(&oB, &oB);

	float p3 = D3DXVec3Dot( &n, &t );
	float p4 = D3DXVec3Dot( &n, &oB );
	float p5 = D3DXVec3Dot( &oB, &oB );

	D3DXVECTOR3 oN = n - (p3/p2)*t - (p4/p5)*oB;
	

	D3DXVec3Normalize(&oN, &oN);
	b = oB;
	n = oN;
}


bool QSFBXImporter::VERTEX_COMPRESSING_ENABLE = true;
bool QSFBXImporter::mUncompressPos = false;
bool QSFBXImporter::mKeepNormal = true;
bool QSFBXImporter::mEnableVCForce = false;
bool QSFBXImporter::mAvatarExport = true;
//---------------------------------------------------------------------
void SkinningData::Normalize()
{
	for( uint idx = 0; idx < mBlendWeights.size(); ++idx )
	{
		BLENDWEIGHTS& blendWeights = mBlendWeights[idx];

		float weightSum = 0;
		const int weightCnt = blendWeights.blendWeights.size();
		for( int i = 0; i < weightCnt; ++i )
		{
			weightSum += blendWeights.blendWeights[i];
		}
		if( weightSum == 1.0f )
			return;

		float div = 1 / weightSum;
		for( int i = 0; i < weightCnt; ++i )
		{
			blendWeights.blendWeights[i] *= div;
		}
	}
}
//---------------------------------------------------------------------
void SkinningData::AddBlendWeight( int controlPointIndex, int boneIndex, float blendWeight )
{
	BLENDWEIGHTS& bw = mBlendWeights[controlPointIndex];
	QSASSERT( bw.blendWeights.size() < 5 );
	bw.blendIndices.push_back( boneIndex );
	bw.blendWeights.push_back( blendWeight );
	
}
//---------------------------------------------------------------------
SkinningData::SkinningData( int controlPointCnt )
{
	mBlendWeights.resize( controlPointCnt );
	mBoneNames.clear();
}
//---------------------------------------------------------------------
void SkinningData::AddBoneInvolved( const char* boneName )
{
	bool had = false;
	for( uint i = 0; i < mBoneNames.size(); ++i )
	{
		if( mBoneNames[i] == boneName )
		{
			had = true;
			break;
		}
	}

	if( !had )
		mBoneNames.push_back( boneName );
}
//---------------------------------------------------------------------
const char* SkinningData::GetBoneName( int index )
{
	if( index < (int)mBoneNames.size() )
	{
		return mBoneNames[index].c_str();
	}

	return "";
}
//---------------------------------------------------------------------
SkinningData::~SkinningData()
{
	mBlendWeights.clear();
	mBoneNames.clear();
	mSkinToBoneMats.clear();
}

//---------------------------------------------------------------------
// class QS_VERTEX
//---------------------------------------------------------------------
QS_VERTEX::~QS_VERTEX()
{
	while( !UVs.empty() )
	{
		qsvector<D3DXVECTOR2>::iterator it = UVs.begin();
		UVs.erase( it );
	}
}
//---------------------------------------------------------------------
bool QS_VERTEX::Equal( QS_VERTEX* rhs ) const
{
	if( cpIndex != rhs->cpIndex )
		return false;

	if( smoothingIndex != rhs->smoothingIndex )
		return false;

	if( normal != rhs->normal )
		return false;


	if( UVs.size() != rhs->UVs.size() )
		return false;

	for( uint i = 0; i < UVs.size(); ++i )
	{
		if( UVs[i] != rhs->UVs[i] )
			return false;
	}

	if( hasMirror != rhs->hasMirror )
		return false;

	//if( hasRotation != rhs->hasRotation)
	//	return false;

	if( uvIndex != rhs->uvIndex )
		return false;

	return true;
}
//---------------------------------------------------------------------
QS_VERTEX::QS_VERTEX( const QS_VERTEX& vertex )
{
	position = vertex.position;
	normal = vertex.normal;
	binormal = vertex.binormal;
	tangent = vertex.tangent;
	vertColor = vertex.vertColor;

	for( uint i = 0; i < vertex.UVs.size(); ++i )
		UVs.push_back( vertex.UVs[i] );

	blendWeights = vertex.blendWeights;

	tbnDir = vertex.tbnDir;
	smoothingIndex = vertex.smoothingIndex;
	cpIndex = vertex.cpIndex;
	uvIndex = vertex.uvIndex;
	hasMirror = vertex.hasMirror;
}

void Mat2Quat(const QSMatrix33f& mat, QSQuaternion4f& quat )
{
	float WS = mat(0,0) + mat(1,1) + mat(2,2);
	float XS = mat(0,0) - mat(1,1) - mat(2,2);
	float YS = mat(1,1) - mat(0,0) - mat(2,2);
	float ZS = mat(2,2) - mat(0,0) - mat(1,1);
	int biggestIndex = 0;
	float biggestMinus = WS;
	if( XS > biggestMinus )
	{
		biggestMinus = XS;
		biggestIndex = 1;
	}
	if( YS > biggestMinus )
	{
		biggestMinus = YS;
		biggestIndex = 2;
	}
	if( ZS > biggestMinus )
	{
		biggestMinus = ZS;
		biggestIndex = 3;
	}
	float biggestVal = sqrt(biggestMinus +1.0f)*0.5f;
	float mult = 0.25f / biggestVal;

	switch (biggestIndex)
	{
	case 0:
		quat(3) = biggestVal;
		quat(0) = mat(1,2) - mat(2,1) * mult;
		quat(1) = mat(2,0) - mat(0,2) * mult;
		quat(2) = mat(0,1) - mat(1,0) * mult;
		break;
	case 1:
		quat(0) = biggestVal;
		quat(3) = mat(1,2) - mat(2,1) * mult;
		quat(1) = mat(0,1) - mat(1,0) * mult;
		quat(2) = mat(2,0) - mat(0,2) * mult;
		break;
	case 2:
		quat(1) = biggestVal;
		quat(3) = mat(2,0) - mat(0,2) * mult;
		quat(0) = mat(0,1) - mat(1,0) * mult;
		quat(2) = mat(1,2) - mat(2,1) * mult;
		break;
	case 3:
		quat(2) = biggestVal;
		quat(3) = mat(0,1) - mat(1,0) * mult;
		quat(0) = mat(2,0) - mat(0,2) * mult;
		quat(1) = mat(1,2) - mat(2,1) * mult;
		break;
	}

}
//---------------------------------------------------------------------
// class FBXMesh
//---------------------------------------------------------------------
void QS_FBXMesh::AssembleVertexData()
{
	
	if( mVerts.empty() )
		return;

	mCompressingScale = GetVertexDataCompressScale();
	QS_VERTEX* vert = mVerts[0];

	int offset = 0;
	int backupOffset = 0;
	//pos
	{
		QSVertexElement vertexElement;
		//vertexElement.NumVerticesPerInstance = 1; is this useless?
		vertexElement.Offset = (u8)offset;
		vertexElement.StreamIndex = 0;
		if( QSFBXImporter::VERTEX_COMPRESSING_ENABLE && !QSFBXImporter::mUncompressPos )
		{
			vertexElement.Type = VET_Half4;
			vertexElement.Usage = VEU_Position;
			vertexElement.UsageIndex = 0;
			mVertDecl.mVertexEleVector.push_back( vertexElement );
			offset += sizeof(short) *4;

			//back up
			vertexElement.Offset = (u8)backupOffset;
			vertexElement.Type = VET_Float3;
			mbackupVertDecl.mVertexEleVector.push_back( vertexElement );
			backupOffset += sizeof(float ) *3;
		}
		else
		{
			vertexElement.Type = VET_Float3;
			vertexElement.Usage = VEU_Position;
			vertexElement.UsageIndex = 0;
			mVertDecl.mVertexEleVector.push_back( vertexElement );
			offset += sizeof(float ) *3;

			if( QSFBXImporter::mUncompressPos )
			{
				//back up
				vertexElement.Offset = (u8)backupOffset;
				vertexElement.Type = VET_Float3;
				mbackupVertDecl.mVertexEleVector.push_back( vertexElement );
				backupOffset += sizeof(float ) *3;
			}
		}
	}
	//uv
	{
		for(uint i = 0; i < vert->UVs.size(); ++i )
		{
			QSVertexElement vertexElement;
			if( QSFBXImporter::VERTEX_COMPRESSING_ENABLE )
			{
				vertexElement.Offset = (u8)offset;
				vertexElement.StreamIndex = 0;
				vertexElement.Type = VET_Half2;
				vertexElement.Usage = VEU_TextureCoordinate;
				vertexElement.UsageIndex = (u8)i;
				mVertDecl.mVertexEleVector.push_back( vertexElement );
				offset += sizeof(short ) * 2;
				//back up
				vertexElement.Offset = (u8)backupOffset;
				vertexElement.Type = VET_Float2;
				mbackupVertDecl.mVertexEleVector.push_back( vertexElement );
				backupOffset += sizeof(float ) * 2;
			}
			else
			{
				vertexElement.Offset = (u8)offset;
				vertexElement.StreamIndex = 0;
				vertexElement.Type = VET_Float2;
				vertexElement.Usage = VEU_TextureCoordinate;
				vertexElement.UsageIndex = (u8)i;
				mVertDecl.mVertexEleVector.push_back( vertexElement );
				offset += sizeof(float ) * 2;
			}
		}

		mUVChannelCnt = (int)vert->UVs.size();
	}

	//normal
	{

		QSVertexElement vertexElement;
		if( QSFBXImporter::VERTEX_COMPRESSING_ENABLE )
		{
			if( QSFBXImporter::mKeepNormal )
			{
				vertexElement.Offset = (u8)offset;
				vertexElement.StreamIndex = 0;
				vertexElement.Type = VET_UByte4;
				vertexElement.Usage = VEU_Normal;
				vertexElement.UsageIndex = 0;
				mVertDecl.mVertexEleVector.push_back( vertexElement );
				offset += sizeof( char ) * 4;

				//back up
				vertexElement.Offset = (u8)backupOffset;
				vertexElement.Type = VET_Float3;
				mbackupVertDecl.mVertexEleVector.push_back( vertexElement );
				backupOffset += sizeof(float ) *3;
			}
		}
		else
		{
			vertexElement.Offset = (u8)offset;
			vertexElement.StreamIndex = 0;
			vertexElement.Type = VET_Float3;
			vertexElement.Usage = VEU_Normal;
			vertexElement.UsageIndex = 0;
			mVertDecl.mVertexEleVector.push_back( vertexElement );
			offset += sizeof( float ) * 3;
		}

	}

	// binormal
	{
		QSVertexElement vertexElement;
		if( QSFBXImporter::VERTEX_COMPRESSING_ENABLE )
		{
			vertexElement.Offset = (u8)offset;
			vertexElement.StreamIndex = 0;
			vertexElement.Type = VET_UByte4;
			vertexElement.Usage = VEU_Binormal;
			vertexElement.UsageIndex = 0;
			mVertDecl.mVertexEleVector.push_back( vertexElement );
			offset += sizeof( char ) * 4;
			//back up
			vertexElement.Offset = (u8)backupOffset;
			vertexElement.Type = VET_Float3;
			mbackupVertDecl.mVertexEleVector.push_back( vertexElement );
			backupOffset += sizeof(float ) *3;
		}
		else
		{
			vertexElement.Offset = (u8)offset;
			vertexElement.StreamIndex = 0;
			vertexElement.Type = VET_Float3;
			vertexElement.Usage = VEU_Binormal;
			vertexElement.UsageIndex = 0;
			mVertDecl.mVertexEleVector.push_back( vertexElement );
			offset += sizeof( float ) * 3;
		}

	}

	//tangent
	{
		QSVertexElement vertexElement;
		if( QSFBXImporter::VERTEX_COMPRESSING_ENABLE )
		{
			vertexElement.Offset = (u8)offset;
			vertexElement.StreamIndex = 0;
			vertexElement.Type = VET_UByte4;
			vertexElement.Usage = VEU_Tangent;
			vertexElement.UsageIndex = 0;
			mVertDecl.mVertexEleVector.push_back( vertexElement );
			offset += sizeof( char ) * 4;
			//back up
			vertexElement.Offset = (u8)backupOffset;
			vertexElement.Type = VET_Float3;
			mbackupVertDecl.mVertexEleVector.push_back( vertexElement );
			backupOffset += sizeof(float ) *3;
		}
		else
		{
			vertexElement.Offset = (u8)offset;
			vertexElement.StreamIndex = 0;
			vertexElement.Type = VET_Float3;
			vertexElement.Usage = VEU_Tangent;
			vertexElement.UsageIndex = 0;
			mVertDecl.mVertexEleVector.push_back( vertexElement );
			offset += sizeof( float ) * 3;
		}
	}

	
	if( mbSkinned )
	{
		//BlendIndices
		{
			QSVertexElement vertexElement;
			vertexElement.Offset = (u8)offset;
			vertexElement.StreamIndex = 0;
			vertexElement.Type = VET_UByte4;
			vertexElement.Usage = VEU_BlendIndices;
			vertexElement.UsageIndex = 0;
			mVertDecl.mVertexEleVector.push_back( vertexElement );
			offset += sizeof( BYTE ) *4;

			//back up
			vertexElement.Offset = (u8)backupOffset;
			mbackupVertDecl.mVertexEleVector.push_back( vertexElement );
			backupOffset += sizeof(BYTE ) *4;
		}
		//BlendWeights
		{

			QSVertexElement vertexElement;
			if( QSFBXImporter::VERTEX_COMPRESSING_ENABLE )
			{
				vertexElement.Offset = (u8)offset;
				vertexElement.StreamIndex = 0;
				vertexElement.Type = VET_UByte4;
				vertexElement.Usage = VEU_BlendWeight;
				vertexElement.UsageIndex = 0;
				mVertDecl.mVertexEleVector.push_back( vertexElement );
				offset += sizeof( BYTE ) *4;
				//back up
				vertexElement.Offset = (u8)backupOffset;
				vertexElement.Type = VET_Float3;
				mbackupVertDecl.mVertexEleVector.push_back( vertexElement );
				backupOffset += sizeof(float ) *3;
			}
			else
			{
				vertexElement.Offset = (u8)offset;
				vertexElement.StreamIndex = 0;
				vertexElement.Type = VET_Float3;
				vertexElement.Usage = VEU_BlendWeight;
				vertexElement.UsageIndex = 0;
				mVertDecl.mVertexEleVector.push_back( vertexElement );
				offset += sizeof( float ) *3;

				//back up
				vertexElement.Offset = (u8)backupOffset;
				vertexElement.Type = VET_Float3;
				mbackupVertDecl.mVertexEleVector.push_back( vertexElement );
				backupOffset += sizeof(float ) *3;
			}

		}
	}

	//Vertex color
	if( QSFBXImporter::mEnableVCForce || mFbxMat->mNeedVC )
	{
		if( QSFBXImporter::VERTEX_COMPRESSING_ENABLE )
		{
			QSVertexElement vertexElement;
			vertexElement.Offset = (u8)offset;
			vertexElement.StreamIndex = 0;
			vertexElement.Type = VET_UByte4;
			vertexElement.Usage = VEU_Color;
			vertexElement.UsageIndex = 0;
			mVertDecl.mVertexEleVector.push_back( vertexElement );
			offset += sizeof( BYTE ) * 4;

		}
		else
		{
			QSVertexElement vertexElement;
			vertexElement.Offset = (u8)offset;
			vertexElement.StreamIndex = 0;
			vertexElement.Type = VET_Float4;
			vertexElement.Usage = VEU_Color;
			vertexElement.UsageIndex = 0;
			mVertDecl.mVertexEleVector.push_back( vertexElement );
			offset += sizeof( float ) * 4;

		}
	}


	//assemble vertex data
	mVertStride = offset;
	int dataSize = offset * mVerts.size();
	int backupDataSize = backupOffset * mVerts.size();
	mVertDataSize = dataSize;
	mVertexData = new float[dataSize/sizeof(float)];
	float* vertData = (float*)mVertexData;
	mBackupVertexData = new float[backupDataSize/sizeof(float)];
	float* backupVertData = (float*)mBackupVertexData;
	memset( vertData, 0, dataSize );
	memset( backupVertData, 0, backupDataSize );
	int stride = 0;
	int backupStride = 0;
	int vSize = (int)mVerts.size();
	for( int i = 0; i < vSize; ++i )
	{
		QS_VERTEX* vert = mVerts[i];
		
		if( QSFBXImporter::VERTEX_COMPRESSING_ENABLE && !QSFBXImporter::mUncompressPos )
		{
			//pos
			ushort posC[4];
			float x = vert->position.x;
			float y = vert->position.y;
			float z = vert->position.z;
			if( mCompressingScale > 1.0f )
			{
				x /= mCompressingScale;
				y /= mCompressingScale;
				z /= mCompressingScale;
			}

			posC[0] = Float32ToFloat16( x );
			posC[1] = Float32ToFloat16( y );
			posC[2] = Float32ToFloat16( z );
			posC[3] = Float32ToFloat16( mCompressingScale );	//store scaling value for the geometry bound which was bigger than max value of float16

			memcpy( vertData + stride, &posC[0], sizeof(short)*4 );
			stride += 2;
			//back up
			memcpy( backupVertData + backupStride, vert->position, sizeof(float)*3 );
			backupStride += 3;
		}
		else
		{
			//pos
			memcpy( vertData + stride, vert->position, sizeof(float)*3 );
			stride += 3;

		}

		//uv
		int uvSize = (int)vert->UVs.size();
		for( int j = 0; j < uvSize; ++j )
		{
			if( QSFBXImporter::VERTEX_COMPRESSING_ENABLE )
			{
				ushort uvC[2];
				uvC[0] = Float32ToFloat16( vert->UVs[j].x );
				uvC[1] = Float32ToFloat16( vert->UVs[j].y );
				memcpy( vertData + stride, &uvC[0], sizeof(short)*2 );
				stride += 1;
				//back up
				memcpy( backupVertData + backupStride, vert->UVs[j], sizeof(float)*2 );
				backupStride += 2;
			}
			else
			{
				memcpy( vertData + stride, vert->UVs[j], sizeof(float)*2 );
				stride += 2;
			}
		}

		if( QSFBXImporter::VERTEX_COMPRESSING_ENABLE )
		{
			D3DXVECTOR3& tT = vert->tangent;
			D3DXVECTOR3& tB = vert->binormal;
			D3DXVECTOR3& tN = vert->normal;

			if( QSFBXImporter::mKeepNormal )
			{
				//normal
				BYTE n[4];
				n[0] = EncodeNormalizeFloatToChar( tN.x );
				n[1] = EncodeNormalizeFloatToChar( tN.y );
				n[2] = EncodeNormalizeFloatToChar( tN.z );
				memcpy( vertData + stride, &n[0], sizeof(BYTE)*4 );
				stride += 1;
			}

			//binormal
			BYTE b[4];
			b[0] = EncodeNormalizeFloatToChar( tB.x );
			b[1] = EncodeNormalizeFloatToChar( tB.y );
			b[2] = EncodeNormalizeFloatToChar( tB.z );
			memcpy( vertData + stride, &b[0], sizeof(BYTE)*4 );
			stride += 1;

			
			//tangent
			BYTE t[4];
			t[0] = EncodeNormalizeFloatToChar( tT.x );
			t[1] = EncodeNormalizeFloatToChar( tT.y );
			t[2] = EncodeNormalizeFloatToChar( tT.z );
			t[3] = EncodeNormalizeFloatToChar( vert->tbnDir );//vert->tbnDir == 1 ? 1 : 2;
			memcpy( vertData + stride, &t[0], sizeof(BYTE)*4 );
			stride += 1;

			//back up
			if( QSFBXImporter::mKeepNormal )
			{
				//normal
				memcpy( backupVertData + backupStride, vert->normal, sizeof(float)*3 );
				backupStride += 3;
			}
			//binormal
			memcpy( backupVertData + backupStride, vert->binormal, sizeof(float)*3 );
			backupStride += 3;
			//tangent
			memcpy( backupVertData + backupStride, vert->tangent, sizeof(float)*3 );
			backupStride += 3;
		}
		else
		{
			//normal
			memcpy( vertData + stride, vert->normal, sizeof(float)*3 );
			stride += 3;
			//binormal
			memcpy( vertData + stride, vert->binormal, sizeof(float)*3 );
			stride += 3;
			//tangent
			memcpy( vertData + stride, vert->tangent, sizeof(float)*3 );
			stride += 3;
		}

		if( mbSkinned )
		{
			//BlendIndices
			BYTE bi[4];
			float bw[4];
			memset( bi, 0, sizeof(BYTE)*4 );
			memset( bw, 0, sizeof(float)*4 );
			for( uint i = 0; i < vert->blendWeights.blendWeights.size(); ++i )
			{
				bi[i] = (BYTE)(vert->blendWeights.blendIndices[i]);
				bw[i] = vert->blendWeights.blendWeights[i];
			}
			memcpy( vertData + stride, bi, sizeof(BYTE)*4 );
			stride += 1;
			memcpy( backupVertData + backupStride, bi, sizeof(BYTE)*4 );
			backupStride += 1;

			//BlendWeight
			if( QSFBXImporter::VERTEX_COMPRESSING_ENABLE )
			{
				BYTE bwC[4];
				bwC[0] = EncodeNormalizeFloatToByte( bw[0] );
				bwC[1] = EncodeNormalizeFloatToByte( bw[1] );
				bwC[2] = EncodeNormalizeFloatToByte( bw[2] );
				bwC[3] = EncodeNormalizeFloatToByte( bw[3] );

				memcpy( vertData + stride, bwC, sizeof(BYTE)*4 );
				stride += 1;
				//back up
				memcpy( backupVertData + backupStride, bw, sizeof(float)*3 );
				backupStride += 3;
			}
			else
			{
				memcpy( vertData + stride, bw, sizeof(float)*3 );
				stride += 3;

				memcpy( backupVertData + backupStride, bw, sizeof(float)*3 );
					backupStride += 3;
			}
		}

		//vertex color
		if( QSFBXImporter::mEnableVCForce || mFbxMat->mNeedVC )
		{
			if( QSFBXImporter::VERTEX_COMPRESSING_ENABLE )
			{
				BYTE b[4];
				b[0] = EncodeNormalizeFloatToByte( vert->vertColor.x );
				b[1] = EncodeNormalizeFloatToByte( vert->vertColor.y );
				b[2] = EncodeNormalizeFloatToByte( vert->vertColor.z );
				memcpy( vertData + stride, &b[0], sizeof(BYTE)*4 );
				stride += 1;
			}
			else
			{
				D3DXVECTOR4 color(vert->vertColor, 0);

				memcpy( vertData + stride, color, sizeof(float)*4 );
				stride += 4;
			}
		}

	}

	D3DOptimizeTriList(this, QSFBXImporter::Device );
}
//---------------------------------------------------------------------
QS_FBXMesh::~QS_FBXMesh()
{
	ResetVertices();

//	delete mFbxMat;

	delete [] mVertexData;
	mVertexData = NULL;
	
	delete[]  mBackupVertexData;
	mBackupVertexData = NULL;

	delete [] mIndex;
	mIndex = NULL;
}
//---------------------------------------------------------------------
void QS_FBXMesh::ResetVertices()
{
	while( !mVerts.empty() )
	{
		Vertices::iterator it = mVerts.begin();
		delete *it;
		mVerts.erase( it );
	}
}

//---------------------------------------------------------------------
struct IndexMap
{
	IndexMap( uint vi ) : vertIndex( vi ){}

	uint vertIndex;
	qsvector<uint> indexArr;
};
//---------------------------------------------------------------------
void QS_FBXMesh::Optimize()
{
	printf("Optimize...\n");
	qsvector<IndexMap> vIndexMap;
	Vertices vertArr;
	uint vertCnt = mVerts.size();
	vertArr.reserve( vertCnt );
	for( uint i = 0; i < vertCnt; ++i )
	{
		QS_VERTEX* pVert = mVerts[i];
		bool bHad = false;
		for( uint j = 0; j < vertArr.size(); ++j )
		{
			if( vertArr[j]->Equal( pVert ) )
			{
				bHad = true;
				vIndexMap[j].indexArr.push_back( i );
				break;
			}
		}
		
		if( !bHad )
		{			
			vertArr.push_back( new QS_VERTEX( *pVert ) );

			IndexMap idxMap( vertArr.size()-1 );
			idxMap.indexArr.push_back( i );
			vIndexMap.push_back( idxMap );
		}

	}

	qsvector<uint> indexBuf;
	indexBuf.resize(vertCnt );
	for( uint i = 0; i < vIndexMap.size(); ++i )
	{
		const qsvector<uint>& arr = vIndexMap[i].indexArr;
		for( uint j = 0; j < arr.size(); ++j )
		{
			indexBuf[ arr[j] ] = vIndexMap[i].vertIndex;
		}
	}

	ResetVertices();

	// to assign new vertex info
	for( uint i = 0; i < vertArr.size(); ++i )
	{
		////normalize the tbn at last
		D3DXVec3Normalize(&vertArr[i]->normal, &vertArr[i]->normal );
		D3DXVec3Normalize(&vertArr[i]->tangent, &vertArr[i]->tangent );
		D3DXVec3Normalize(&vertArr[i]->binormal, &vertArr[i]->binormal );

		mVerts.push_back( vertArr[i] );
	}
	vertArr.clear();

	// to assign new index buffer
	uint* idxBuf = (uint*)mIndex;
	uint indexCnt = indexBuf.size();
	for( uint i = 0; i < indexCnt; ++i )
	{
		idxBuf[i] = indexBuf[i];
	}

	// sort index buffer
//	SortVertexIndices();
}
//---------------------------------------------------------------------
bool ComputeParity( const D3DXVECTOR3& t, const D3DXVECTOR3& b, const D3DXVECTOR3& n )
{
	D3DXVECTOR3 factTB;
	D3DXVECTOR3 norm = n;
	D3DXVec3Cross( &factTB, &t, &b );
	D3DXVec3Normalize(&factTB, &factTB);
	D3DXVec3Normalize(&norm, &norm);
	float angle = D3DXVec3Dot(&factTB, &norm );
	return angle > 0;

}
//---------------------------------------------------------------------
qsvector<QS_VERTEX*>::iterator FindVertsWithCPIndex( qsvector<QS_VERTEX*>& vertexArray, int cpIndex )
{
	qsvector<QS_VERTEX*>::iterator it = vertexArray.begin();
	for( ; it != vertexArray.end(); ++it )
	{
		if( (*it)->cpIndex == cpIndex )
			return it;
	}

	return vertexArray.end();
}

void GetVertexGroupCanSmooth(qsvector<qsvector<QS_VERTEX*>>& sameVertexMapsOut, qsvector<QS_VERTEX*>& sameSGVerts )
{
	qsvector<QS_VERTEX*> sameCPVerts;
	
	int curCPIndex = -1;
	for( uint i = 0; i < sameSGVerts.size(); ++i )
	{
		QS_VERTEX* vert = sameSGVerts[i];
		if( sameCPVerts.empty() )
		{
			sameCPVerts.push_back( vert );
			curCPIndex = vert->cpIndex;
		}
		else
		{
			if( curCPIndex == vert->cpIndex )
				sameCPVerts.push_back( vert );

		}
	}
	if( !sameCPVerts.empty() )
		sameVertexMapsOut.push_back( sameCPVerts );

	if( sameSGVerts.empty() )
		return;

	//remove verts with curCPIndex
	qsvector<QS_VERTEX*>::iterator it = FindVertsWithCPIndex(sameSGVerts, curCPIndex );
	while ( it != sameSGVerts.end() )
	{
		sameSGVerts.erase( it );
		it = FindVertsWithCPIndex(sameSGVerts, curCPIndex);
	}

}

void MergeVertices( Vertices& vertexArray )
{
	D3DXVECTOR3 finalNormal(0,0,0);
	D3DXVECTOR3 finalTangent(0,0,0);
	D3DXVECTOR3 finalBinormal(0,0,0);
	qsvector<D3DXVECTOR2> uvs;

	for( uint i = 0; i < vertexArray.size(); ++i )
	{
		QS_VERTEX* vert = vertexArray[i];

		finalNormal += vert->normal * vert->angleOfEdges;
		finalTangent += vert->tangent * vert->angleOfEdges;
		finalBinormal += vert->binormal * vert->angleOfEdges;

		if( qsutils::FindInVector( vert->UVs[0], uvs ) == (u32)-1)
		{
			uvs.push_back( vert->UVs[0] );
		}
	}

	D3DXVec3Normalize(&finalNormal, &finalNormal);
	D3DXVec3Normalize(&finalTangent, &finalTangent);
	D3DXVec3Normalize(&finalBinormal, &finalBinormal);

	Orthonormlize(finalNormal, finalBinormal, finalTangent);
	if( uvs.size() > 1 )
	{
		qsvector<qsvector<QS_VERTEX*>> vertArrMap;
		vertArrMap.resize( uvs.size() );
		for( uint i = 0; i < uvs.size(); ++i )
		{
			for( uint j = 0; j < vertexArray.size(); ++j )
			{
				if( vertexArray[j]->UVs[0] == uvs[i] )
				{
					vertArrMap[i].push_back( vertexArray[j] );
				}
			}
		}

		for( uint i = 0; i < vertArrMap.size(); ++i )
		{
			D3DXVECTOR3 mergeT(0,0,0);
			D3DXVECTOR3 mergeB(0,0,0);
			for( uint j = 0; j < vertArrMap[i].size(); ++j )
			{
				mergeT += vertArrMap[i][j]->tangent * vertArrMap[i][j]->angleOfEdges;
				mergeB += vertArrMap[i][j]->binormal * vertArrMap[i][j]->angleOfEdges;
			}

			D3DXVec3Normalize(&mergeT, &mergeT);
			D3DXVec3Normalize(&mergeB, &mergeB);

			for( uint j = 0; j < vertArrMap[i].size(); ++j )
			{
				vertArrMap[i][j]->normal = finalNormal;
				vertArrMap[i][j]->tangent = mergeT;
				vertArrMap[i][j]->binormal = mergeB;
				vertArrMap[i][j]->uvIndex = i;

				Orthonormlize(vertArrMap[i][j]->normal, vertArrMap[i][j]->binormal, vertArrMap[i][j]->tangent );
			}
		}

		//for( uint uvIdx = 0; uvIdx < vertArrMap.size(); ++uvIdx )
		//{
		//	Vertices& verts = vertArrMap[uvIdx];

		//	//find out witch vertices got opposite TBN to the triangle TBN space
		//	bool vertParity = ComputeParity(finalTangent, finalBinormal, finalNormal);

		//	qsvector<QS_VERTEX*> mirrorVerts, noMirrorVerts;
		//	for( uint i = 0; i < verts.size(); ++i )
		//	{
		//		QS_VERTEX* vert = verts[i];
		//		vert->normal = finalNormal;

		//		// has uv mirror or not
		//		vert->hasMirror = ( vert->mFaceParity != vertParity ) ? true : false;
		//		vert->tbnDir = (vertParity) ? 1.0f : -1.0f;
		//		if( vert->hasMirror )
		//		{
		//			mirrorVerts.push_back( vert );
		//		}
		//		else
		//		{
		//			noMirrorVerts.push_back( vert );
		//		}

		//		if( !vert->hasMirror )
		//		{
		//			vert->tangent = finalTangent;
		//			vert->binormal = finalBinormal;
		//			Orthonormlize(vert->normal, vert->binormal, vert->tangent );
		//		}

		//	}

		//	// Verts have mirror
		//	{
		//		D3DXVECTOR3 mergeT(0,0,0);
		//		D3DXVECTOR3 mergeB(0,0,0);
		//		for( uint i = 0; i < mirrorVerts.size(); ++i )
		//		{
		//			mergeT += mirrorVerts[i]->tangent * mirrorVerts[i]->angleOfEdges;
		//			mergeB += mirrorVerts[i]->binormal * mirrorVerts[i]->angleOfEdges;
		//		}
		//		D3DXVec3Normalize(&mergeT, &mergeT);
		//		D3DXVec3Normalize(&mergeB, &mergeB);
		//		for( uint i = 0; i < mirrorVerts.size(); ++i )
		//		{
		//			mirrorVerts[i]->tangent = mergeT;
		//			mirrorVerts[i]->binormal = mergeB;
		//			mirrorVerts[i]->uvIndex = uvIdx;
		//			Orthonormlize(mirrorVerts[i]->normal, mirrorVerts[i]->binormal, mirrorVerts[i]->tangent );
		//		}
		//	}

		//	//Verts have no mirror
		//	//{
		//	//	D3DXVECTOR3 mergeT(0,0,0);
		//	//	D3DXVECTOR3 mergeB(0,0,0);
		//	//	for( uint i = 0; i < noMirrorVerts.size(); ++i )
		//	//	{
		//	//		mergeT += noMirrorVerts[i]->tangent * noMirrorVerts[i]->angleOfEdges;
		//	//		mergeB += noMirrorVerts[i]->binormal * noMirrorVerts[i]->angleOfEdges;
		//	//	}
		//	//	D3DXVec3Normalize(&mergeT, &mergeT);
		//	//	D3DXVec3Normalize(&mergeB, &mergeB);
		//	//	for( uint i = 0; i < noMirrorVerts.size(); ++i )
		//	//	{
		//	//		noMirrorVerts[i]->tangent = mergeT;
		//	//		noMirrorVerts[i]->binormal = mergeB;
		//	//		noMirrorVerts[i]->uvIndex = uvIdx;
		//	//		Orthonormlize(noMirrorVerts[i]->normal, noMirrorVerts[i]->binormal, noMirrorVerts[i]->tangent );
		//	//	}
		//	//}
		//}
	}
	else
	{
		//find out witch vertices got opposite TBN to the triangle TBN space
		bool vertParity = ComputeParity(finalTangent, finalBinormal, finalNormal);

		qsvector<QS_VERTEX*> mirrorVerts, noMirrorVerts;
		for( uint i = 0; i < vertexArray.size(); ++i )
		{
			QS_VERTEX* vert = vertexArray[i];
			vert->normal = finalNormal;

			// has uv mirror or not
			vert->hasMirror = ( vert->mFaceParity != vertParity ) ? true : false;
			vert->tbnDir = (vertParity) ? 1.0f : -1.0f;
			if( vert->hasMirror )
			{
				mirrorVerts.push_back( vert );
			}
			else
			{
				noMirrorVerts.push_back( vert );
			}

			if( !vert->hasMirror )
			{
				vert->tangent = finalTangent;
				vert->binormal = finalBinormal;
				Orthonormlize(vert->normal, vert->binormal, vert->tangent );
			}

		}

		{
			D3DXVECTOR3 mergeT(0,0,0);
			D3DXVECTOR3 mergeB(0,0,0);
			for( uint i = 0; i < mirrorVerts.size(); ++i )
			{
				mergeT += mirrorVerts[i]->tangent * mirrorVerts[i]->angleOfEdges;
				mergeB += mirrorVerts[i]->binormal * mirrorVerts[i]->angleOfEdges;
			}
			D3DXVec3Normalize(&mergeT, &mergeT);
			D3DXVec3Normalize(&mergeB, &mergeB);
			for( uint i = 0; i < mirrorVerts.size(); ++i )
			{
				mirrorVerts[i]->tangent = mergeT;
				mirrorVerts[i]->binormal = mergeB;
				Orthonormlize(mirrorVerts[i]->normal, mirrorVerts[i]->binormal, mirrorVerts[i]->tangent );
			}
		}

		{
			D3DXVECTOR3 mergeT(0,0,0);
			D3DXVECTOR3 mergeB(0,0,0);
			for( uint i = 0; i < noMirrorVerts.size(); ++i )
			{
				mergeT += noMirrorVerts[i]->tangent * noMirrorVerts[i]->angleOfEdges;
				mergeB += noMirrorVerts[i]->binormal * noMirrorVerts[i]->angleOfEdges;
			}
			D3DXVec3Normalize(&mergeT, &mergeT);
			D3DXVec3Normalize(&mergeB, &mergeB);
			for( uint i = 0; i < noMirrorVerts.size(); ++i )
			{
				noMirrorVerts[i]->tangent = mergeT;
				noMirrorVerts[i]->binormal = mergeB;
				Orthonormlize(noMirrorVerts[i]->normal, noMirrorVerts[i]->binormal, noMirrorVerts[i]->tangent );
			}
		}
	}


}
void SmoothVertex( qsvector<QS_VERTEX*>& vertexArray )
{
	//get TBN of every single vertex
	MergeVertices(vertexArray);
}


void SmoothVertexForOnGroup( QS_FBXMesh* pMesh, int cpCount, qsvector<QS_VERTEX*>& vertexArray)
{
	qsvector<QS_VERTEX*> vertWithSameNorIdx;
	int vertCnt = vertexArray.size();
	for( int cpIdx = 0; cpIdx < cpCount; ++cpIdx )
	{
		vertWithSameNorIdx.clear();

		for( int j = 0; j < vertCnt; ++j )
		{
			QS_VERTEX* cmpVert = pMesh->mVerts[j];
			if( cmpVert->cpIndex == cpIdx )
				vertWithSameNorIdx.push_back( cmpVert );
		}

		SmoothVertex( vertWithSameNorIdx );
	}
}
//---------------------------------------------------------------------
void QS_FBXMesh::ComputeNTB( int cpCount )
{
	printf("Compute NTB space...\n");
	//compute NTB space for faces
	for( uint i = 0; i < mTriangleCnt; ++i )
	{
		QS_VERTEX* pVert1 = mVerts[0 + i*3];
		QS_VERTEX* pVert2 = mVerts[1 + i*3];
		QS_VERTEX* pVert3 = mVerts[2 + i*3];

		const D3DXVECTOR3& pos1 = pVert1->position;
		const D3DXVECTOR3& pos2 = pVert2->position;
		const D3DXVECTOR3& pos3 = pVert3->position;

		const D3DXVECTOR2& uv1 = pVert1->UVs[0];
		const D3DXVECTOR2& uv2 = pVert2->UVs[0];
		const D3DXVECTOR2& uv3 = pVert3->UVs[0];

		D3DXVECTOR3 edgeA = pos2 - pos1;
		D3DXVECTOR3 edgeB = pos3 - pos1;

		D3DXVECTOR3 norm1;
		D3DXVec3Cross(&norm1, &edgeA, &edgeB );
		D3DXVec3Normalize( &norm1, &norm1 );

		float deltaU1 = uv2.x - uv1.x;
		float deltaU2 = uv3.x - uv1.x;
		float deltaV1 = uv2.y - uv1.y;
		float deltaV2 = uv3.y - uv1.y;
		float div = ( deltaU1* deltaV2 - deltaU2*deltaV1 );
		D3DXVECTOR3 tan( 1.0f, 0, 0 );
		D3DXVECTOR3 bin( 0, 1.0f, 0 );
		bool faceParity = true;
		if( div != 0 )
		{
			float areaW = fabsf( div );
			float a = deltaV2 / div;
			float b = -deltaV1 / div;
			float c = -deltaU2 / div;
			float d = deltaU1 / div;
			D3DXVECTOR3 p1( edgeA*a + edgeB*b );
			D3DXVECTOR3 p2( edgeA*c + edgeB*d );
			D3DXVec3Normalize( &p1, &p1);
			D3DXVec3Normalize( &p2, &p2);
			tan = p1 * areaW;
			bin = p2 * areaW;

			D3DXVec3Normalize(&tan, &tan);
			D3DXVec3Normalize(&bin, &bin);
			faceParity = ComputeParity(tan, bin, norm1);
			pVert1->mFaceParity = faceParity;
			pVert2->mFaceParity = faceParity;
			pVert3->mFaceParity = faceParity;

		}

		// if a triangle's UVs are CCW, we must flip the Binormals
		// first get the triangle's UV edge vectors
		D3DXVECTOR3 kP1P2Vector;
		D3DXVECTOR3 kP1P3Vector;
		kP1P2Vector.x = uv2.x - uv1.x;
		kP1P2Vector.y = uv2.y - uv1.y;
		kP1P2Vector.z = 0.0f;
		kP1P3Vector.x = uv3.x - uv1.x;
		kP1P3Vector.y = uv3.y - uv1.y;
		kP1P3Vector.z = 0.0f;

		// take the cross product of the triangle edges
		//D3DXVECTOR3 kCrossResult;
		//D3DXVec3Cross(&kCrossResult, &kP1P2Vector, &kP1P3Vector);

		// Get handedness
		// a positive z component indicates Counter-Clockwise
		// Counter-clockwise triangles need to have their Binormal flipped
		//float fCCW = (kCrossResult.z > 0.0f) ? 1.0f : -1.0f;
		float fCCW = (faceParity) ? 1.0f : -1.0f;

		// vert 0 of triangle
		D3DXVECTOR3 bin1(0,0,0), tan1(0,0,0);
		bin1.x = bin.x;
		bin1.y = bin.y;
		bin1.z = bin.z;
		tan1.x = tan.x;
		tan1.y = tan.y;
		tan1.z = tan.z;

		// vert 1 of triangle
		D3DXVECTOR3 bin2(0,0,0), tan2(0,0,0);
		bin2.x = bin.x;
		bin2.y = bin.y;
		bin2.z = bin.z;
		tan2.x = tan.x;
		tan2.y = tan.y;
		tan2.z = tan.z;

		// vert 2 of triangle
		D3DXVECTOR3 bin3(0,0,0), tan3(0,0,0);
		bin3.x += bin.x;
		bin3.y += bin.y;
		bin3.z += bin.z;
		tan3.x += tan.x;
		tan3.y += tan.y;
		tan3.z += tan.z;

		//Compute angle weight
		float angle1 = D3DXVec3Dot( &edgeA, &edgeB );
		angle1 = angle1 / (D3DXVec3Length(&edgeA)*D3DXVec3Length(&edgeB));
		angle1 = acosf( angle1 );

		edgeA = pos1 - pos2;
		edgeB = pos3 - pos2;
		float angle2 = D3DXVec3Dot( &edgeA, &edgeB );
		angle2 = angle2 / (D3DXVec3Length(&edgeA)*D3DXVec3Length(&edgeB));
		angle2 = acosf( angle2 );

		edgeA = pos1 - pos3;
		edgeB = pos2 - pos3;
		float angle3 = D3DXVec3Dot( &edgeA, &edgeB );
		angle3 = angle3 / (D3DXVec3Length(&edgeA)*D3DXVec3Length(&edgeB));
		angle3 = acosf( angle3 );

		pVert1->binormal = bin1;
		pVert2->binormal = bin2;
		pVert3->binormal = bin3;

		pVert1->tangent = tan1;
		pVert2->tangent = tan2;
		pVert3->tangent = tan3;

		//record TBN space handedness
		pVert1->tbnDir = fCCW;
		pVert2->tbnDir = fCCW;
		pVert3->tbnDir = fCCW;

		pVert1->normal = norm1;
		pVert2->normal = norm1;
		pVert3->normal = norm1;

		pVert1->angleOfEdges = angle1;
		pVert2->angleOfEdges = angle2;
		pVert3->angleOfEdges = angle3;
	}

//	printf("Compute NTB every vertex...\n");
	//compute ntb per vertex
	const uint vertCnt = mVerts.size();

	qsvector<int> smoothgroup;
	for( uint vIdx = 0; vIdx < vertCnt; ++vIdx)
	{
		QS_VERTEX* pVert = mVerts[vIdx];
		if( smoothgroup.empty() )
			smoothgroup.push_back( pVert->smoothingIndex );
		else
		{
			bool had = false;
			for( uint j = 0; j < smoothgroup.size(); ++j )
			{
				if( smoothgroup[j] == pVert->smoothingIndex )
				{
					had = true;
					break;
				}
			}
			if( !had )
				smoothgroup.push_back( pVert->smoothingIndex );
		}
	}

	if( smoothgroup.size() == 1 )
	{
		qsvector<QS_VERTEX*> vertWithSameSmooth;
		vertWithSameSmooth.resize(vertCnt);
		for( uint vIdx = 0; vIdx < vertCnt; ++vIdx)
		{
			QS_VERTEX* pVert = mVerts[vIdx];
			vertWithSameSmooth[vIdx] = pVert;
		}
		SmoothVertexForOnGroup( this, cpCount, vertWithSameSmooth );
	}
	else
	{
		for( uint j = 0; j < smoothgroup.size(); ++ j )
		{
			uint curSmoothIndex = smoothgroup[j];
			qsvector<QS_VERTEX*> vertWithSameSmooth;
			for( uint vIdx = 0; vIdx < vertCnt; ++vIdx)
			{
				QS_VERTEX* pVert = mVerts[vIdx];
				if( (uint)pVert->smoothingIndex == curSmoothIndex )
				{
					vertWithSameSmooth.push_back( pVert );
				}
			}

			qsvector<qsvector<QS_VERTEX*>> vertsMapCanSmooth;
			vertsMapCanSmooth.reserve(120000);
			while( !vertWithSameSmooth.empty() )
			{
				GetVertexGroupCanSmooth(vertsMapCanSmooth, vertWithSameSmooth );
			}

			for( uint i = 0; i < vertsMapCanSmooth.size(); ++i )
			{
				SmoothVertex( vertsMapCanSmooth[i] );
			}
		}
	}
}
//---------------------------------------------------------------------
struct TriIndex
{
	uint a, b, c;

	bool Has( uint val )
	{
		if( a == val || b == val || c == val )
			return true;
	}

	bool HasEdge( const TriIndex& triIndex )
	{
		if( Has(triIndex.a) || Has(triIndex.b) || Has(triIndex.c) )
			return true;

		return false;
	}

};
bool CmpTriIndex( const TriIndex& a, const TriIndex& b )
{
	return QSmin( a.a, QSmin( a.b, a.c ) ) < QSmin( b.a, QSmin( b.b, b.c ) );
}

void QS_FBXMesh::SortVertexIndices()
{
	qsvector<TriIndex> triArr;
	//uint vertCnt = mTriangleCnt*3;
	uint* indexBuf = (uint*)mIndex;
	for( uint i = 0; i < mTriangleCnt; ++i )
	{
		TriIndex tri;
		tri.a = indexBuf[0+i*3];
		tri.b = indexBuf[1+i*3];
		tri.c = indexBuf[2+i*3];

		triArr.push_back( tri );

	}
	qsvector<TriIndex>::iterator itBegin = triArr.begin();
	qsvector<TriIndex>::iterator itEnd = triArr.end();
	std::sort( itBegin, itEnd, CmpTriIndex );
	
	for( uint i = 0; i < triArr.size(); ++i )
	{
		const TriIndex& firstTri = triArr[i];
		indexBuf[0 + i*3] = firstTri.a;
		indexBuf[1 + i*3] = firstTri.b;
		indexBuf[2 + i*3] = firstTri.c;
	}
}
//---------------------------------------------------------------------
float QS_FBXMesh::GetVertexDataCompressScale()
{
	float xLength = mBoundMax.x - mBoundMin.x;
	float yLength = mBoundMax.y - mBoundMin.y;
	float zLength = mBoundMax.z - mBoundMin.z;

	float maxLength = qsutils::Max( xLength, qsutils::Max(yLength, zLength ) );
	if( maxLength >= HALF_MAX )
	{
		int scale = int(maxLength / HALF_MAX + 1.0f);
		return (float)scale;
	}
	else
	{
		return 1.0f;
	}
}

//---------------------------------------------------------------------
// class FBXMaterial
//---------------------------------------------------------------------
QS_FBXMaterial::~QS_FBXMaterial()
{
	//while( !mTextureURLs.empty() )
	//{
	//	TextureMapping::iterator it = mTextureURLs.begin();
	//	mTextureURLs.erase( it );
	//}

	mTextureURLs.clear();
}
//---------------------------------------------------------------------
QS_FBXMaterial::TEX_CHANNEL QS_FBXMaterial::GetMaterialType( int texIndex )
{
	QS_FBXMaterial::TEX_CHANNEL texMaps = QS_FBXMaterial::TC_UNKNOWN;
	switch( texIndex )
	{
	case 0:
		texMaps = QS_FBXMaterial::TC_DIFFUSE;
		break;
	case 2:
		texMaps = QS_FBXMaterial::TC_EMISSIVE_MAP;
		break;
	case 6:
		texMaps = QS_FBXMaterial::TC_SPEC_COLOR;
		break;
	case 7:
		texMaps = QS_FBXMaterial::TC_SPEC_COLOR;
		break;
	case 9:
	case 10:
		texMaps = QS_FBXMaterial::TC_BUMP_MAP;
		break;
	default:		
		break;
	}

	return texMaps;
}
//---------------------------------------------------------------------
const char* QS_FBXMaterial::GetTexTypeName( QS_FBXMaterial::TEX_CHANNEL channel )
{
	switch ( channel )
	{
	case QS_FBXMaterial::TC_DIFFUSE :
		return "BaseSampler";
	case QS_FBXMaterial::TC_SPEC_COLOR:
		return "SpecularMap";
	case QS_FBXMaterial::TC_BUMP_MAP:
		return "NormalSampler";
	case QS_FBXMaterial::TC_EMISSIVE_MAP:
		return "EmissiveMap";
	default:
		return "";
	}
}
//---------------------------------------------------------------------
bool QS_FBXMaterial::HasAlpha( int texIndex )
{
	if( texIndex == 11 || texIndex == 12 )
		return true;

	return false;
}
//---------------------------------------------------------------------
bool QS_FBXMaterial::HasTwoSided( int texIndex )
{
	if( texIndex == 4 || texIndex == 5 )
		return true;

	return false;
}

bool QS_FBXMaterial::GetBoolParam( NameHandle name , bool& v)
{
	for(size_t i = 0 ; i < mBoolParams.size() ; ++i)
	{
		if( mBoolParams[i].name == name )
		{
			v = mBoolParams[i].value;
			return true;
		}
	}
	//QSASSERT(0&&"can't found param");
	return false;
}

bool QS_FBXMaterial::GetFloatParam( NameHandle name , f32& v)
{
	for(size_t i = 0 ; i < mFloatParams.size() ; ++i)
	{
		if( mFloatParams[i].name == name )
		{
			v = mFloatParams[i].value;
			return true;
		}
	}
	//QSASSERT(0&&"can't found param");
	return false;
}

bool QS_FBXMaterial::GetFloat2Param( NameHandle name , QSVec2f& v)
{
	for(size_t i = 0 ; i < mFloat2Params.size() ; ++i)
	{
		if( mFloat2Params[i].name == name )
		{
			v = mFloat2Params[i].value;
			return true;
		}
	}
	//QSASSERT(0&&"can't found param");
	return false;
}

bool QS_FBXMaterial::GetFloat3Param( NameHandle name , QSVec3f& v)
{
	for(size_t i = 0 ; i < mFloat3Params.size() ; ++i)
	{
		if( mFloat3Params[i].name == name )
		{
			v = mFloat3Params[i].value;
			return true;
		}
	}
	//QSASSERT(0&&"can't found param");
	return false;
}

bool QS_FBXMaterial::GetFloat4Param( NameHandle name , QSVec4f& v)
{
	for(size_t i = 0 ; i < mFloat4Params.size() ; ++i)
	{
		if( mFloat4Params[i].name == name )
		{
			v = mFloat4Params[i].value;
			return true;
		}
	}
	//QSASSERT(0&&"can't found param");
	return false;
}

bool QS_FBXMaterial::GetTexParam( NameHandle name , eastl::string& v)
{
	for(size_t i = 0 ; i < mTexParams.size() ; ++i)
	{
		if( mTexParams[i].name == name )
		{
			v = mTexParams[i].value;
			return true;
		}
	}
	//QSASSERT(0&&"can't found param");
	return false;
}

//---------------------------------------------------------------------
// class FBXNode
//---------------------------------------------------------------------
bool QS_FBXNode::AddChild( QS_FBXNode* pNode )
{
	FBXNodeArray::iterator it = mChildren.begin();
	for( ; it != mChildren.end(); ++it )
	{
		if( *it == pNode )
			return false;
	}

	mChildren.push_back( pNode );

	if( pNode->mParent )
	{
		pNode->mParent->RemoveChild( pNode );
	}

	pNode->mParent = this;
	
	return true;
}
//---------------------------------------------------------------------
bool QS_FBXNode::RemoveChild( QS_FBXNode* pNode )
{

	FBXNodeArray::iterator it = mChildren.begin();
	for( ; it != mChildren.end(); ++it )
	{
		if( *it == pNode )
		{
			mChildren.erase( it );
			pNode->mParent = NULL;
			return true;
		}
	}

	return false;
}
//---------------------------------------------------------------------
QS_FBXNode* QS_FBXNode::GetChild( int idx )
{
	if( idx < (int)mChildren.size() )
		return mChildren[idx];

	return NULL;
}
//---------------------------------------------------------------------
QS_FBXNode::~QS_FBXNode()
{
	mParent = NULL;

	while( !mChildren.empty() )
	{
		FBXNodeArray::iterator it = mChildren.begin();
		delete *it;
		mChildren.erase( it );
	}
}
//---------------------------------------------------------------------
void QS_FBXNode::SetlclTransform( KFbxNode* pNode )
{
	KFbxXMatrix& mat = pNode->EvaluateLocalTransform();
	fbxDouble3 trans = pNode->GeometricTranslation.Get();

	mTranslation.x	= (f32)trans[0];
	mTranslation.y	= (f32)trans[1];
	mTranslation.z	= (f32)trans[2];

	mRotation[0]		= KFbxVector4ToD3DXVECTOR3( mat.GetRow(0) );
	mRotation[1]		= KFbxVector4ToD3DXVECTOR3( mat.GetRow(1) );
	mRotation[2]		= KFbxVector4ToD3DXVECTOR3( mat.GetRow(2) );

	fbxDouble3 scl = pNode->LclScaling.Get();
	mScaling.x		= (float)scl[0];
	mScaling.y		= (float)scl[1];
	mScaling.z		= (float)scl[2];
}
//---------------------------------------------------------------------
void QS_FBXNode::SetBonelclTransform( KFbxNode* pNode )
{
	KFbxXMatrix& mat = pNode->EvaluateLocalTransform();

	mTranslation.x	= (f32)mat[3][0];
	mTranslation.y	= (f32)mat[3][1];
	mTranslation.z	= (f32)mat[3][2];

	mRotation[0]		= KFbxVector4ToD3DXVECTOR3( mat.GetRow(0) );
	mRotation[1]		= KFbxVector4ToD3DXVECTOR3( mat.GetRow(1) );
	mRotation[2]		= KFbxVector4ToD3DXVECTOR3( mat.GetRow(2) );

	fbxDouble3 scl = pNode->LclScaling.Get();
	mScaling.x		= (float)scl[0];
	mScaling.y		= (float)scl[1];
	mScaling.z		= (float)scl[2];
}

//---------------------------------------------------------------------
// class FBXMeshNode
//---------------------------------------------------------------------
void QS_FBXMeshNode::AddMesh( const eastl::string& matName, QS_FBXMesh* pMesh )
{
	FBXMeshes::iterator itFind = mFbxMeshes.find(matName);
	if( itFind != mFbxMeshes.end() )
		return;

	FBXMeshLOD lod;
	lod.push_back( pMesh );

	mFbxMeshes[matName] = lod;
}
//---------------------------------------------------------------------
QS_FBXMesh* QS_FBXMeshNode::GetMesh( int index, int lodLv )
{
	FBXMeshes::iterator it = mFbxMeshes.begin();
	for( int i = 0; it != mFbxMeshes.end(); ++it, ++i )
	{
		if( i == index )
		{
			if( lodLv < (int)it->second.size() )
				return it->second[lodLv];
		}
	}

	return NULL;
}
//---------------------------------------------------------------------
QS_FBXMesh* QS_FBXMeshNode::GetMesh( const eastl::string& matName, int lodLv /*= 0 */ )
{
	FBXMeshes::iterator itFind = mFbxMeshes.find(matName);
	if( itFind != mFbxMeshes.end() )
	{
		if( lodLv < (int)itFind->second.size() )
			return itFind->second[lodLv];
	}

	return NULL;
}
//---------------------------------------------------------------------
QS_FBXMeshNode::~QS_FBXMeshNode()
{
	while( !mFbxMeshes.empty() )
	{
		FBXMeshes::iterator it = mFbxMeshes.begin();
		
		FBXMeshLOD& lod = it->second;
		while( !lod.empty() )
		{
			FBXMeshLOD::iterator lodIt = lod.begin();
			delete *lodIt;
			lod.erase( lodIt );
		}

		mFbxMeshes.erase( it );
	}

}
//---------------------------------------------------------------------
int QS_FBXMeshNode::GetMeshLODCount( const eastl::string& matName )
{
	FBXMeshes::iterator itFind = mFbxMeshes.find(matName);
	if( itFind != mFbxMeshes.end() )
		return mFbxMeshes[matName].size();

	return 0;
}
//---------------------------------------------------------------------
void QS_FBXMeshNode::AddMeshLOD( const eastl::string& matName, QS_FBXMesh* pMeshLOD )
{
	FBXMeshes::iterator itFind = mFbxMeshes.find(matName);
	if( itFind == mFbxMeshes.end() )
		return;
	itFind->second.push_back( pMeshLOD );
}
//---------------------------------------------------------------------
int QS_FBXMeshNode::GetMaxMeshLODCount()
{
	int lodCnt = 0;
	FBXMeshes::iterator it = mFbxMeshes.begin();
	for( ; it != mFbxMeshes.end(); ++it )
	{
		if( it->second.size() > (u32)lodCnt )
		{
			lodCnt = (int)it->second.size();
		}
	}

	return lodCnt;
}
//---------------------------------------------------------------------
void QS_FBXMeshNode::AddCollisionMesh( QS_FBXMesh* pMesh )
{
	if( pMesh )
	{
		mCollisionMeshes.push_back( pMesh );
	}
}

//---------------------------------------------------------------------
// class FBXBoneNode
//---------------------------------------------------------------------
void QS_FBXBoneNode::SetWorldTransform( KFbxNode* pNode )
{
	KFbxXMatrix& globalMat = pNode->EvaluateGlobalTransform();

	for( int i = 0; i < 4; ++i )
	{
		mWorld.m[0][i] = (float)globalMat[0][i];
		mWorld.m[1][i] = (float)globalMat[1][i];
		mWorld.m[2][i] = (float)globalMat[2][i];
		mWorld.m[3][i] = (float)globalMat[3][i];

	}
}
//---------------------------------------------------------------------
int QS_FBXBoneNode::GetBoneCountRecursive()
{
	int allGeneration = (int)mChildren.size();
	for( uint i = 0; i < mChildren.size(); ++i )
	{
		QS_FBXBoneNode* pBone = (QS_FBXBoneNode*)mChildren[i];
		allGeneration += pBone->GetBoneCountRecursive();
	}

	return allGeneration;
}
//---------------------------------------------------------------------
// class FBXScene
//---------------------------------------------------------------------
QS_FBXScene::~QS_FBXScene()
{
	delete mRootNode;
	delete mRootBone;
}

//---------------------------------------------------------------------
// class QSFBXImporter
//---------------------------------------------------------------------
QSFBXImporter::QSFBXImporter()
	: mFbxSDKManager(NULL)
	, mInited(false)
	, mFbxScene(NULL)
	, mHasLOD(false)
{
	// Initialize the sdk manager. This object handles all our memory management.
	 mFbxSDKManager = KFbxSdkManager::Create();
	 if( mFbxSDKManager )
	 {
		 mInited = true;
		 //set our own memory alloctor
	//	 mFbxSDKManager->SetMemoryAllocator( KFbxMemoryAllocator)

		 //Creating an I/O Settings Object
		 KFbxIOSettings *ios = KFbxIOSettings::Create(mFbxSDKManager, IOSROOT);
		 mFbxSDKManager->SetIOSettings(ios);
		 //
	 }
}

//---------------------------------------------------------------------
QSFBXImporter::~QSFBXImporter()
{
	if( mInited )
	{
		Reset();

		mFbxSDKManager->Destroy();
		mFbxSDKManager = NULL;
	}

	mMaterials.clear();
}

//---------------------------------------------------------------------
bool QSFBXImporter::LoadFbxFromFile( const eastl::string& strFileName )
{
	KFbxImporter* importer = KFbxImporter::Create( mFbxSDKManager, "" );
	assert( importer );

	bool bResult = false;

	if( importer )
	{
		if(importer->Initialize( strFileName.c_str(), -1, mFbxSDKManager->GetIOSettings() ) )
		{
			mFileName = strFileName;
			mFbxScene = KFbxScene::Create( mFbxSDKManager, strFileName.c_str() );
			assert( mFbxScene );

			bResult = importer->Import( mFbxScene );
		}
		importer->Destroy();
	}

	assert(bResult);
	return bResult;
}
//---------------------------------------------------------------------
QS_FBXScene* QSFBXImporter::ParseFBX(IMPORT_OPTIONS option, bool enableCompress, bool enableVertexColor,
	bool uncompressPos, bool keepNormal )
{
	if( !mFbxScene )
		return false;

	KFbxNode* pRootNode = mFbxScene->GetRootNode();
	assert( pRootNode );
	mEnableVCForce = enableVertexColor;
	mUncompressPos = uncompressPos;
	mKeepNormal = keepNormal;

	QSFBXImporter::VERTEX_COMPRESSING_ENABLE = !uncompressPos;
	//const char* rootName = pRootNode->GetName();
	int nodeCnt = pRootNode->GetChildCount();
	qsvector<KFbxNode*> skeletonNodes;
	qsvector<KFbxNode*> meshNodes;
	qsvector<KFbxNode*> boundingNodes;
	//classify different type of nodes
	for( int i = 0; i < nodeCnt; ++i )
	{
		KFbxNode* pNode = pRootNode->GetChild(i);
		eastl::string name = pNode->GetName();
		const KFbxNodeAttribute* nodeAttribute = pNode->GetNodeAttribute();
		if( nodeAttribute == NULL )
			continue;
		KFbxNodeAttribute::EAttributeType attriType = nodeAttribute->GetAttributeType();
		if( attriType == KFbxNodeAttribute::eMESH )
		{
			if( IsBoundingVolume( pNode ))
			{
				boundingNodes.push_back( pNode );
			}
			else
			{
				meshNodes.push_back( pNode );
			}
		}
		else if( attriType == KFbxNodeAttribute::eSKELETON || name == "Root" || attriType == KFbxNodeAttribute::eNULL )
		{
			skeletonNodes.push_back( pNode );
		}
		else{}
	}

	if( meshNodes.empty() && skeletonNodes.empty() )
		return NULL;

	eastl::sort(meshNodes.begin(), meshNodes.end(), &NodeCompare );
	// Parse node info.
	QS_FBXScene* pScene = new QS_FBXScene;
	pScene->mName = mFileName;


	if( option == IO_SKELETON || option == IO_BOTH )
	{
		for( uint i = 0; i < skeletonNodes.size(); ++i )
		{
			KFbxNode* pBoneNode = skeletonNodes[i];
			QS_FBXBoneNode* pBone = ParseBoneNode( pBoneNode );
			if( pBone )
			{
				pScene->mRootBone = pBone;
				pScene->mBoneCount = pBone->GetBoneCountRecursive()+1;
			}
		}
	}

	// all different meshes are spreed under root node.
	if( option == IO_MESH || option == IO_BOTH || option == IO_GEOMETRY)
	{
		for( uint i = 0; i < meshNodes.size(); ++i )
		{
			KFbxNode* pMeshNode = meshNodes[i];
			QS_FBXNode* pChild = ParseMeshNode( pMeshNode );
			if( pChild )
			{
				if( !pScene->mRootNode )
					pScene->mRootNode = new QS_FBXNode;

				pScene->mRootNode->AddChild( pChild );
			}
		}

		//dealing with inner bounding boxes
		for( uint i = 0; i < boundingNodes.size(); ++i )
		{
			KFbxNode* pNode = boundingNodes[i];
			KFbxMesh* pMesh = pNode->GetMesh();

			if( pMesh )
			{
				if( !pMesh->IsTriangleMesh() )
				{
					KFbxGeometryConverter converter(mFbxSDKManager);
					if( !converter.TriangulateInPlace( pNode ) )
					{
						assert(0);
					}
				}
				pMesh = pNode->GetMesh();
				assert( pMesh );

				int cnt = pMesh->GetControlPointsCount();
				KFbxVector4* pControlPointArr = pMesh->GetControlPoints();
				D3DXVECTOR3 minPt(FLT_MAX, FLT_MAX, FLT_MAX);
				D3DXVECTOR3 maxPt(FLT_MIN, FLT_MIN, FLT_MIN);

				for( uint idx = 0; idx < (uint)cnt; ++idx )
				{
					D3DXVECTOR3 cp = KFbxVector4ToD3DXVECTOR3( pControlPointArr[idx] );
					minPt.x = minPt.x > cp.x ? cp.x : minPt.x;
					minPt.y = minPt.y > cp.y ? cp.y : minPt.y;
					minPt.z = minPt.z > cp.z ? cp.z : minPt.z;

					maxPt.x = maxPt.x < cp.x ? cp.x : maxPt.x;
					maxPt.y = maxPt.y < cp.y ? cp.y : maxPt.y;
					maxPt.z = maxPt.z < cp.z ? cp.z : maxPt.z;
				}
				QS_BoundingBox bb;
				bb.mBoundCenter .x = ( maxPt.x + minPt.x ) *0.5f;
				bb.mBoundCenter .y = ( maxPt.y + minPt.y ) *0.5f;
				bb.mBoundCenter .z = ( maxPt.z + minPt.z ) *0.5f;

				bb.mHalfLength.x = maxPt.x - bb.mBoundCenter.x;
				bb.mHalfLength.y = maxPt.y - bb.mBoundCenter.y;
				bb.mHalfLength.z = maxPt.z - bb.mBoundCenter.z;

				//add offset
				fbxDouble3 trans = pNode->LclTranslation.Get();

				bb.mBoundCenter.x += (f32)trans[0];
				bb.mBoundCenter.y += (f32)trans[1];
				bb.mBoundCenter.z += (f32)trans[2];

				int meshCnt = pScene->mRootNode->GetChildrenCount();
				for( int j = 0; j < meshCnt; ++j )
				{
					QS_FBXNode* pMeshNode = pScene->mRootNode->GetChild(j);
					if( IsBelong(pNode->GetName(), pMeshNode->mName ) )
					{
						pMeshNode->mBoundingBoxes.push_back( bb );
						break;
					}
				}

			}
		}
	}

	return pScene;
}
//---------------------------------------------------------------------
QS_FBXNode* QSFBXImporter::ParseMeshNode( KFbxNode* pNode, bool isLOD, QS_FBXMeshNode* pParentLOD )
{
	QS_FBXMeshNode*	pNewNode = NULL;
	
	//	every node has only one mesh before mesh splited
	int attribCnt = pNode->GetNodeAttributeCount();
	UNREFERENCED_PARAMETER(attribCnt);
	assert( attribCnt <= 1 );
	bool bChildIsLOD = false;
	KFbxNodeAttribute* attrib = pNode->GetNodeAttributeByIndex(0);
	if( attrib->GetAttributeType() == KFbxNodeAttribute::eMESH )
	{
		if( isLOD )
		{
			pNewNode = pParentLOD;
		}
		else
		{
			pNewNode		= new QS_FBXMeshNode;
			pNewNode->mName	= pNode->GetName();
		}

		bChildIsLOD = HasLOD( pNode );

		KFbxMesh* pMesh = pNode->GetMesh();
		// dealing with material first,cause we have to decide if we need split mesh by materials
		if( !ParseMaterial( pMesh, pNewNode, isLOD ) )
		{
			delete pNewNode;
			return NULL;

		}
		//transformation data			
		pNewNode->SetlclTransform( pNode );

	}


	//if a non-root node has any children, it should be it's lod mesh
	for( int i = 0; i < pNode->GetChildCount(); ++i )
	{
		KFbxNode* pChild= pNode->GetChild(i);
		ParseMeshNode( pChild, bChildIsLOD, pNewNode );
		//QSASSERT(myChild);
		//if( myChild )
		//	pNewNode->AddChild( myChild );
	}

	return pNewNode;
}
//---------------------------------------------------------------------
QS_FBXBoneNode* QSFBXImporter::ParseBoneNode( KFbxNode* pNode )
{
	KFbxSkeleton* pSkeleton = pNode->GetSkeleton();
	const char* nodeName = pNode->GetName();
	QS_FBXBoneNode* pBone = NULL;
	if( pSkeleton )
	{
		//KFbxSkeleton::ESkeletonType skelType = pSkeleton->GetSkeletonType();

//		if( skelType == KFbxSkeleton::eLIMB_NODE )
		{
			pBone = new QS_FBXBoneNode;
			pBone->mName = nodeName;
			pBone->SetBonelclTransform( pNode );
			pBone->SetWorldTransform( pNode );
			KFbxNode* mParent = pNode->GetParent();

			if( mParent->GetSkeleton() )
			{
				//const KFbxVector4& parentT = mParent->EvaluateGlobalTransform().GetT();
				KFbxVector4 lclParentT = mParent->EvaluateLocalTransform().GetT();

				//const KFbxVector4& t = pNode->EvaluateGlobalTransform().GetT();
				KFbxVector4 lclT = pNode->EvaluateLocalTransform().GetT();

				D3DXVECTOR3 parentPos = KFbxVector4ToD3DXVECTOR3( lclParentT );
				D3DXVECTOR3 pos = KFbxVector4ToD3DXVECTOR3( lclT );
				D3DXVECTOR3 length = pos - parentPos;
				D3DXVECTOR3 midPos = parentPos + 0.5f*(length);
				pBone->mBoundRadius = D3DXVec3Length( &length )*0.5f;
				pBone->mBoundCenter = midPos;
			}
			//another way to get lcl transform
			//KFbxVector4 trans = mFbxScene->GetEvaluator()->GetNodeLocalTranslation(pNode);
		}
	}
	else
	{
		// bones for weapons and clothes
		pBone = new QS_FBXBoneNode;
		pBone->mName = nodeName;
		pBone->SetBonelclTransform( pNode );
		pBone->SetWorldTransform( pNode );
		//KFbxNode* mParent = pNode->GetParent();
	}
	for( int i = 0; i < pNode->GetChildCount(); ++i )
	{
		QS_FBXNode* pChildBone = ParseBoneNode( pNode->GetChild(i) );

		if( pChildBone )
		{
			if( !pBone )
				pBone = (QS_FBXBoneNode*)pChildBone;
			else
				pBone->AddChild( pChildBone );
		}
	}

	return pBone;

}

//--------------------------------------------------------------------

bool QSFBXImporter::ParseHLSLMat( KFbxSurfaceMaterial* pSurfaceMaterial , QS_FBXMaterial* pMyMat)
{
	const KFbxImplementation* pImplementation = GetImplementation(pSurfaceMaterial,ImplementationHLSL);

	pMyMat->mNeedVC = false;
	if( pImplementation == NULL )
	{
		return true;
	}
	KFbxBindingTable const* lRootTable = pImplementation->GetRootTable();
	fbxString lFileName                = lRootTable->DescAbsoluteURL.Get();
	fbxString lTechniqueName           = lRootTable->DescTAG.Get();

	// Name of the effect file
	const char* data = lFileName.Buffer();
	eastl::string shaderName = data;
	int pos = shaderName.find_last_of("\\");
	if( pos == eastl::string::npos )
		pos = shaderName.find_last_of("/");
	pMyMat->mShaderName = shaderName.substr(pos+1);
	KFbxBindingTable const* pBTable = pImplementation->GetRootTable();
	size_t entryCount = pBTable->GetEntryCount();

	//check if the material must export vertex color
	if( pMyMat->mShaderName == "QSNextGenMultiLayerShader.fx")
		pMyMat->mNeedVC = true;

	for(size_t i = 0 ; i < entryCount ; ++i)
	{
		const KFbxBindingTableEntry& btEntry = pBTable->GetEntry(i);
		const char* pEntrySrcType = btEntry.GetEntryType(true);
		KFbxProperty fbxProperty;

		// Name of Parameter
		data = btEntry.GetDestination();

		if(strcmp(KFbxPropertyEntryView::sEntryType , pEntrySrcType) == 0)
		{
			fbxProperty = pSurfaceMaterial->FindPropertyHierarchical(btEntry.GetSource());
			if(!fbxProperty.IsValid())
			{
				fbxProperty = pSurfaceMaterial->RootProperty.FindHierarchical(btEntry.GetSource());
			}
		}
		else
		{
			if(strcmp(KFbxConstantEntryView::sEntryType , pEntrySrcType) == 0)
			{
				fbxProperty = pImplementation->GetConstants().FindHierarchical(btEntry.GetSource());
			}
		}

		if(fbxProperty.IsValid())
		{
			if(fbxProperty.GetSrcObjectCount(FBX_TYPE(KFbxTexture)) > 0)
			{
				// Texture Parameter
				for(int j = 0 ; j < fbxProperty.GetSrcObjectCount(FBX_TYPE(KFbxFileTexture)) ; ++j)
				{
					KFbxFileTexture* pFileTexture = fbxProperty.GetSrcObject(FBX_TYPE(KFbxFileTexture) , j);
					pMyMat->mTexParams.push_back(ParamTex(data,pFileTexture->GetFileName(),pFileTexture));
				}

// 				for(int j = 0 ; j < fbxProperty.GetSrcObjectCount(FBX_TYPE(KFbxLayeredTexture)) ; ++j)
// 				{
// 					KFbxLayeredTexture* pLayeredTexture = fbxProperty.GetSrcObject(FBX_TYPE(KFbxLayeredTexture) , j);
// 				}
// 
// 				for(int j = 0 ; j < fbxProperty.GetSrcObjectCount(FBX_TYPE(KFbxProceduralTexture)) ; ++j)
// 				{
// 					KFbxProceduralTexture* pProceduralTexture = fbxProperty.GetSrcObject(FBX_TYPE(KFbxProceduralTexture) , j);
// 				}
			}
			else
			{
				// Common Parameter
				KFbxDataType dataType = fbxProperty.GetPropertyDataType();

				// Bool value
				if(DTBool == dataType)
				{
					bool boolValue = KFbxGet<bool>(fbxProperty);
					pMyMat->mBoolParams.push_back(ParamBool(data,boolValue));
				}

				// Integer value
// 				if(DTInteger == dataType || DTEnum == dataType)
// 				{
// 					int intValue = KFbxGet<int>(fbxProperty);
// 				}

				// Float
				if(DTFloat == dataType)
				{
					float floatValue = KFbxGet<float>(fbxProperty);

					pMyMat->mFloatParams.push_back(ParamFloat(data,floatValue));
				}

				// Double
				if(DTDouble == dataType)
				{
					double doubleValue = (float)KFbxGet<double>(fbxProperty);
					pMyMat->mFloatParams.push_back(ParamFloat(data,(float)doubleValue));
				}

				// Double2
				if(DTDouble2 == dataType)
				{
					fbxDouble2 lDouble2 = KFbxGet<fbxDouble2>(fbxProperty);
					pMyMat->mFloat2Params.push_back(ParamFloat2(data,QSVec2f((float)lDouble2[0] , (float)lDouble2[1])));
				}

				// Double3
				if(DTDouble3  == dataType || DTVector3D == dataType || DTColor3 == dataType)
				{
					fbxDouble3 lDouble3 = KFbxGet<fbxDouble3>(fbxProperty);
					pMyMat->mFloat3Params.push_back(ParamFloat3(data,QSVec2f((float)lDouble3[0] , (float)lDouble3[1] , (float)lDouble3[2])));
				}

				// Double4
				if(DTDouble4  == dataType || DTVector4D == dataType || DTColor4 == dataType)
				{
					fbxDouble4 lDouble4 = KFbxGet<fbxDouble4>(fbxProperty);
					pMyMat->mFloat4Params.push_back(ParamFloat4(data,QSVec2f((float)lDouble4[0] , (float)lDouble4[1] , (float)lDouble4[2] , (float)lDouble4[3])));
				}

				// Double4x4
				if(DTDouble44 == dataType)
				{
					fbxDouble44 lDouble44 = KFbxGet<fbxDouble44>(fbxProperty);

					D3DXMATRIX double4x4Value;

					for(int i = 0 ; i < 4 ; ++i)
					{
						for(int j = 0 ; j < 4 ; ++j)
						{
							double4x4Value.m[i][j] = (float)lDouble44[i][j];
						}
					}
				}

				// String
// 				if(DTString == dataType || DTUrl == dataType || DTXRefUrl == dataType)
// 				{
// 					char* pStringBuffer =(KFbxGet<fbxString>(fbxProperty)).Buffer();
// 					
// 				}
			}
		}
	}
	return true;
}

bool MaterialCompare(QS_FBXMaterial* a, QS_FBXMaterial* b)
{
	return a->mName > b->mName;
}

//---------------------------------------------------------------------
bool QSFBXImporter::ParseMaterial( KFbxMesh* pMesh, QS_FBXMeshNode* pInNode, bool isLOD )
{
	assert( pMesh );

	KFbxNode* pNode = pMesh->GetNode();
	int matCnt = pNode->GetMaterialCount();
	if(!matCnt)
	{
		assert( 0 && "No material!");
		return false;
	}

	// to group meshes by material
	for( int i = 0; i < matCnt; ++i )
	{
		//pMat: is the true material
		KFbxSurfaceMaterial* pMat = pNode->GetMaterial(i);
		
		QS_FBXMaterial* pMyMat = new QS_FBXMaterial;
		pMyMat->mName = pMat->GetName();
		ParseHLSLMat(pMat,pMyMat);

		//iterate all kinds of texture channels
		for( int textureIndex = 0;textureIndex< KFbxLayerElement::LAYERELEMENT_TYPE_TEXTURE_COUNT; ++textureIndex )
		{
			KFbxProperty lProperty = pMat->FindProperty(KFbxLayerElement::TEXTURE_CHANNEL_NAMES[textureIndex]);
			if( lProperty.IsValid() )
			{
				int lTextureCount = lProperty.GetSrcObjectCount(KFbxTexture::ClassId);
				//if no texture paramters, give material default texture channels
				if( lTextureCount == 0 )
				{
					if( QS_FBXMaterial::GetMaterialType( textureIndex ) == QS_FBXMaterial::TC_DIFFUSE )
						pMyMat->mTextureURLs.insert( make_pair(QS_FBXMaterial::TC_DIFFUSE, eastl::string("") ) );
					if( QS_FBXMaterial::GetMaterialType( textureIndex ) == QS_FBXMaterial::TC_SPEC_COLOR )
						pMyMat->mTextureURLs.insert( make_pair(QS_FBXMaterial::TC_SPEC_COLOR, eastl::string("") ) );
					if( QS_FBXMaterial::GetMaterialType( textureIndex ) == QS_FBXMaterial::TC_BUMP_MAP )
						pMyMat->mTextureURLs.insert( make_pair(QS_FBXMaterial::TC_BUMP_MAP, eastl::string("") ) );
					if( QS_FBXMaterial::GetMaterialType( textureIndex ) == QS_FBXMaterial::TC_EMISSIVE_MAP )
						pMyMat->mTextureURLs.insert( make_pair(QS_FBXMaterial::TC_EMISSIVE_MAP, eastl::string("") ) );
				}

				for (int j = 0; j < lTextureCount; ++j)
				{
					//Here we have to check if it's layeredtextures, or just textures:
					KFbxLayeredTexture *lLayeredTexture = KFbxCast <KFbxLayeredTexture>(lProperty.GetSrcObject(KFbxLayeredTexture::ClassId, j));
					if (lLayeredTexture)
					{
						int texCnt = lLayeredTexture->GetSrcObjectCount(KFbxTexture::ClassId);
						for( int k = 0; k < texCnt; ++k )
						{
							KFbxTexture* pTex = KFbxCast <KFbxTexture> (lLayeredTexture->GetSrcObject(KFbxTexture::ClassId,k));
							if(pTex)
							{
								KFbxFileTexture *pFileTex = KFbxCast<KFbxFileTexture>(pTex);
								eastl::string fileName = pFileTex->GetFileName();
								size_t pos = fileName.find_last_of("\\");
								if( pos == eastl::string::npos )
									pos = fileName.find_last_of("/");
								
								if( pos == eastl::string::npos)
									continue;

								fileName = fileName.substr( pos+1, eastl::string::npos );
								fileName = "materials\\" + fileName;
								QS_FBXMaterial::TEX_CHANNEL texType = QS_FBXMaterial::GetMaterialType( textureIndex );
								if( texType != QS_FBXMaterial::TC_UNKNOWN)
									pMyMat->mTextureURLs.insert( make_pair(texType, fileName) );

								pMyMat->mHasAlpha = QS_FBXMaterial::HasAlpha( textureIndex );
								pMyMat->mTwoSided = (pMyMat->mTwoSided == false )? QS_FBXMaterial::HasTwoSided( textureIndex ) : pMyMat->mTwoSided;
							}
						}
					}
					else
					{
						KFbxTexture* pTex = KFbxCast <KFbxTexture> (lProperty.GetSrcObject(KFbxTexture::ClassId, j));
						if(pTex)
						{
							KFbxFileTexture *pFileTex = KFbxCast<KFbxFileTexture>(pTex);
							eastl::string fileName = pFileTex->GetFileName();

							size_t pos = fileName.find_last_of("\\");
							if( pos == eastl::string::npos )
								pos = fileName.find_last_of("/");

							if( pos == eastl::string::npos)
								continue;

							fileName = fileName.substr( pos+1, eastl::string::npos );							
							fileName = "materials\\" + fileName;
							QS_FBXMaterial::TEX_CHANNEL texType = QS_FBXMaterial::GetMaterialType( textureIndex );
							if( texType != QS_FBXMaterial::TC_UNKNOWN)
								pMyMat->mTextureURLs[texType] = fileName;//pFileTex->GetFileName();

							pMyMat->mHasAlpha = QS_FBXMaterial::HasAlpha( textureIndex );
							pMyMat->mTwoSided = (pMyMat->mTwoSided == false )? QS_FBXMaterial::HasTwoSided( textureIndex ) : pMyMat->mTwoSided;
						}
					}
				}

			}
		}
		mMaterials.push_back( pMyMat );
	}


	//eastl::sort(mMaterials.begin(),mMaterials.end(), &MaterialCompare );
	if( !VerifyMaterialNames( mMaterials ) )
	{
		return NULL;
	}

	// pre-process mesh
	//if( pMesh->CheckSamePointTwice() )
	{
		pMesh->RemoveBadPolygons();
	}

	KFbxGeometryConverter converter(mFbxSDKManager);
	KFbxNode* pFbxNode = pMesh->GetNode();
	if( !pMesh->IsTriangleMesh() )
		if( !converter.TriangulateInPlace( pFbxNode ) )
			return NULL;
	pMesh = pFbxNode->GetMesh();
	if( !pMesh )
		return NULL;

	// pLayerMat: for getting mappingMode
	//int emCnt = pMesh->GetElementMaterialCount();
	KFbxLayerElementMaterial* pLayerMat = pMesh->GetElementMaterial();
	KFbxLayerElement::EMappingMode mappingMode =  pLayerMat->GetMappingMode();
	if( mappingMode == KFbxLayerElement::eBY_POLYGON &&
		matCnt > 1 )
	{

		eastl::map< QS_FBXMaterial*, QS_FBXMesh* > subMeshes;
		for( uint i = 0;i < mMaterials.size(); ++i )
		{
			subMeshes.insert( eastl::make_pair(mMaterials[i], (QS_FBXMesh*)NULL) );
		}

		ParseMeshByMultipleMats( pMesh, subMeshes );
		//new meshes
		qsvector<QS_FBXMaterial*>::iterator it = mMaterials.begin();
		for( ; it != mMaterials.end(); ++it )
		{
			QS_FBXMaterial* mat = *it;
			QS_FBXMesh* pOutMesh = subMeshes[mat];

			if( pOutMesh )
			{
				if( isLOD )
					pInNode->AddMeshLOD( mat->mName, pOutMesh );
				else
					pInNode->AddMesh( mat->mName, pOutMesh );
			}
		}
		
		subMeshes.clear();
	}
	else if( mappingMode == KFbxLayerElement::eALL_SAME )
	{
		QS_FBXMesh* pOutMesh = ParseMesh( pMesh );

		const char* meshName = pFbxNode->GetName();
		pOutMesh->mName = meshName;

		pOutMesh->mFbxMat = mMaterials[0];

		if( pOutMesh->mIsCollisionMesh )
		{
			pInNode->AddCollisionMesh( pOutMesh );
		}
		else
		{
			if( isLOD )
			{
				pInNode->AddMeshLOD( mMaterials[0]->mName, pOutMesh );
			}
			else
			{
				pInNode->AddMesh( mMaterials[0]->mName, pOutMesh );
			}
		}
	}
	else
	{
		// none sense for material
		assert( 0 );
		MessageBox(0, "Dose this geometry has multiple materials, but not set up for every material?", "Error", MB_OK);
	}

	if( isLOD )
	{
		while( !mMaterials.empty() )
		{
			qsvector<QS_FBXMaterial*>::iterator it = mMaterials.begin();
			delete *it;
			mMaterials.erase( it );
		}
	}
	else
		mMaterials.clear();
	return true;
}
//---------------------------------------------------------------------
D3DXVECTOR2 QSFBXImporter::GetUV( const char* uvName, KFbxMesh* pMesh, int polygonIdx, int posInPoly, int vertIdx )
{
	assert( pMesh );

	const KFbxGeometryElementUV* lUVElement = pMesh->GetElementUV(uvName);

//	KFbxLayer* pLayer = pMesh->GetLayer( 0 );
	if( lUVElement )
	{
		//KFbxLayerElementUV* pUVs = pLayer->GetUVs(KFbxLayerElement::eDIFFUSE_TEXTURES);

		KFbxLayerElement::EMappingMode mappingMode = lUVElement->GetMappingMode();
		KFbxLayerElement::EReferenceMode referenceMode = lUVElement->GetReferenceMode();

		const KFbxLayerElementArrayTemplate<KFbxVector2>& pUVArray = lUVElement->GetDirectArray();
		const KFbxLayerElementArrayTemplate<int>& pUVIndexArray = lUVElement->GetIndexArray();

		switch(mappingMode)
		{
		case KFbxLayerElement::eBY_CONTROL_POINT:
			{
				int nMappingIndex = vertIdx;
				switch(referenceMode)
				{
				case KFbxLayerElement::eDIRECT:
					if( nMappingIndex < pUVArray.GetCount() )
					{
							KFbxVector2 v = pUVArray.GetAt(nMappingIndex);
							return FBXTexCoord2DirectXTexCoord( KFbxVector2ToD3DXVECTOR2( v ) );
					}
					break;
				case KFbxLayerElement::eINDEX_TO_DIRECT:
					if( nMappingIndex < pUVIndexArray.GetCount() )
					{
						int nIndex = pUVIndexArray.GetAt(nMappingIndex);
						if( nIndex < pUVArray.GetCount() )
						{
								KFbxVector2 v = pUVArray.GetAt(nIndex);
								return FBXTexCoord2DirectXTexCoord( KFbxVector2ToD3DXVECTOR2( v ) );
						}
					}
					break;

				default:
					assert(0);
					break;
				};
			}
			break;

		case KFbxLayerElement::eBY_POLYGON_VERTEX:
			{
			//	int nMappingIndex = pMesh->GetTextureUVIndex(polygonIdx, posInPoly, KFbxLayerElement::eDIFFUSE_TEXTURES);
				int nMappingIndex = -1;
				switch(referenceMode)
				{
				case KFbxLayerElement::eDIRECT:
				case KFbxLayerElement::eINDEX_TO_DIRECT: //I have no idea why the index array is not used in this case.
					if( nMappingIndex < pUVArray.GetCount() )
					{

						KFbxVector2 uv;
						pMesh->GetPolygonVertexUV(polygonIdx, posInPoly, uvName, uv);
						//KFbxVector2 uv = pUVArray.GetAt(nMappingIndex);
						return FBXTexCoord2DirectXTexCoord( KFbxVector2ToD3DXVECTOR2( uv ) );
					}

					break;
				default:
					assert(0);
					break;
				};
			}
			break;
		default:
			assert(0);
			break;
		};



	}

	return D3DXVECTOR2();
}

//---------------------------------------------------------------------
D3DXVECTOR3 QSFBXImporter::GetTangent( KFbxMesh* pMesh, int vertIdx )
{
	KFbxLayerElementTangent* pTangent = pMesh->GetElementTangent();
	QSASSERT( pTangent );
	KFbxLayerElement::EMappingMode mapMode = pTangent->GetMappingMode();
	KFbxLayerElement::EReferenceMode refMode = pTangent->GetReferenceMode();

	const KFbxLayerElementArrayTemplate<KFbxVector4>& pTangArr = pTangent->GetDirectArray();
	const KFbxLayerElementArrayTemplate<int>& pTangIdxArr = pTangent->GetIndexArray();
	KFbxVector4 tangent;
	switch( mapMode )
	{
	case KFbxLayerElement::eBY_CONTROL_POINT:
		{
			const int& mappingIndex = vertIdx;
			switch(refMode)
			{
			case KFbxLayerElement::eDIRECT:
				if( mappingIndex < pTangArr.GetCount() )
				{
					tangent = pTangArr.GetAt( mappingIndex );
				}
				break;
			case KFbxLayerElement::eINDEX_TO_DIRECT:
				if( mappingIndex < pTangIdxArr.GetCount() )
				{
					int idx = pTangIdxArr.GetAt( mappingIndex );
					if( idx < pTangArr.GetCount() )
						tangent = pTangArr.GetAt( idx );
				}
				break;
			default: assert(0);
			};
		}
		break;
	case KFbxLayerElement::eBY_POLYGON_VERTEX:
		{
			switch( refMode )
			{
			case KFbxLayerElement::eDIRECT:
				{
					tangent = pTangArr.GetAt( vertIdx );
				}
				break;
			case KFbxLayerElement::eINDEX_TO_DIRECT:
				{
					int idx = pTangIdxArr.GetAt( vertIdx );
					tangent = pTangArr.GetAt( idx );	
				}
				break;
			default: assert(0);
			};
			
		}
		break;
	default:
		assert(0);
		break;
	}

	tangent.Normalize();

	return KFbxVector4ToD3DXVECTOR3(tangent);
}
//---------------------------------------------------------------------
D3DXVECTOR3 QSFBXImporter::GetBinormal( KFbxMesh* pMesh, int vertIdx )
{
	KFbxLayerElementBinormal* pBinormal = pMesh->GetElementBinormal();
	assert( pBinormal );
	KFbxLayerElement::EMappingMode mapMode = pBinormal->GetMappingMode();
	KFbxLayerElement::EReferenceMode refMode = pBinormal->GetReferenceMode();

	const KFbxLayerElementArrayTemplate<KFbxVector4>& pBinoArr = pBinormal->GetDirectArray();
	const KFbxLayerElementArrayTemplate<int>& pBinoIdxArr = pBinormal->GetIndexArray();

	KFbxVector4 binormal;
	switch( mapMode )
	{
	case KFbxLayerElement::eBY_CONTROL_POINT:
		{
			const int& mappingIndex = vertIdx;
			switch(refMode)
			{
			case KFbxLayerElement::eDIRECT:
				if( mappingIndex < pBinoArr.GetCount() )
				{
					binormal = pBinoArr.GetAt( mappingIndex );
				}
				break;
			case KFbxLayerElement::eINDEX_TO_DIRECT:
				if( mappingIndex < pBinoIdxArr.GetCount() )
				{
					int idx = pBinoIdxArr.GetAt( mappingIndex );
					if( idx < pBinoArr.GetCount() )
						binormal = pBinoArr.GetAt( idx );
				}
				break;
			default: assert(0);
			};
		}
		break;
	case KFbxLayerElement::eBY_POLYGON_VERTEX:
		{
			switch( refMode )
			{
			case KFbxLayerElement::eDIRECT:
				{
					binormal = pBinoArr.GetAt( vertIdx );
				}
				break;
			case KFbxLayerElement::eINDEX_TO_DIRECT:
				{
					int idx = pBinoIdxArr.GetAt( vertIdx );
					binormal = pBinoArr.GetAt( idx );	
				}
				break;
			default: assert(0);
			};

		}
		break;
	default:
		assert(0);
		break;
	}

	binormal.Normalize();
	return KFbxVector4ToD3DXVECTOR3(binormal);
}
//---------------------------------------------------------------------
int	QSFBXImporter::GetSmoothingIndex( KFbxMesh* pMesh, int polygonIdx )
{
	if( !pMesh )
		return -1;

	KFbxGeometryElementSmoothing* lSmoothingElement = pMesh->GetElementSmoothing();
	if( !lSmoothingElement )
		return 0;

	int smoothIndex = 0;
	// maya way, soft/hard edges
	if( lSmoothingElement->GetMappingMode() == KFbxGeometryElement::eBY_EDGE )
	{
		assert(0 && "Maya smoothing info type, not support yet.");
		MessageBox(0, "Maya smoothing info type, not support yet.", "Error", MB_OK);
		return 0;
	}
	else if( lSmoothingElement->GetMappingMode() == KFbxGeometryElement::eBY_POLYGON )
	{
		if( lSmoothingElement->GetReferenceMode() == KFbxGeometryElement::eDIRECT )
		{
			smoothIndex = polygonIdx;
		}
		if(lSmoothingElement->GetReferenceMode() == KFbxGeometryElement::eINDEX_TO_DIRECT)
			smoothIndex = lSmoothingElement->GetIndexArray().GetAt(polygonIdx);
	}

	return lSmoothingElement->GetDirectArray().GetAt(smoothIndex);


}
//---------------------------------------------------------------------
QS_FBXMesh* QSFBXImporter::ParseMesh( KFbxMesh* pMesh )
{
	const int vertexCnt = pMesh->GetControlPointsCount();
	if( !vertexCnt )
		return NULL;

	QS_FBXMesh* pQsMesh = new QS_FBXMesh;
	unsigned int allVertexIndex = 0;

	// is this a collision mesh
	pQsMesh->mIsCollisionMesh = IsCollisionMesh( pMesh );
	// process skin stuffs
	SkinningData skinningData( vertexCnt );
	if( pMesh->GetDeformerCount( KFbxDeformer::eSKIN ) )
	{
		pQsMesh->mbSkinned = true;
		ParseBoneWeights( pMesh, skinningData );

		const int boneCnt = skinningData.GetInvolvedBoneCount();
		for( int i = 0; i < boneCnt; ++i )
		{
			pQsMesh->mBoneNames.push_back( skinningData.GetBoneName(i) );
			pQsMesh->mSkinToBoneMats.push_back( skinningData.mSkinToBoneMats[i] );
		}
	}

	KFbxVector4* pControlPointArr = pMesh->GetControlPoints();
	const uint polygonCnt = (uint)pMesh->GetPolygonCount();
	
	pQsMesh->mTriangleCnt = polygonCnt;
	unsigned int totalCnt = polygonCnt*3;
	pQsMesh->mIndex = new uint[totalCnt];

	int vertIdx = 0;
	pQsMesh->mVerts.resize( totalCnt );
	for( uint i = 0; i < polygonCnt; ++i )
	{
		// get smoothing index
		int smoothFlag = GetSmoothingIndex( pMesh, i);
		for( int pi = 0; pi < 3; ++pi )
		{
			int vertexIdx = pMesh->GetPolygonVertex( i, pi );
			if( vertexIdx < 0 || vertexIdx >= vertexCnt )
				continue;

			QS_VERTEX* qsVert = new QS_VERTEX;
			//pos
			qsVert->position = KFbxVector4ToD3DXVECTOR3( pControlPointArr[vertexIdx] );

			//UVs
			KStringList lUVSetNameList;
			pMesh->GetUVSetNames(lUVSetNameList);
			//iterating over all uv sets
			for (int lUVSetIndex = 0; lUVSetIndex < lUVSetNameList.GetCount(); ++lUVSetIndex)
			{
				//get lUVSetIndex-th uv set
				const char* lUVSetName = lUVSetNameList.GetStringAt(lUVSetIndex);

				qsVert->UVs.push_back( GetUV( lUVSetName, pMesh, i, pi, vertexIdx ) );
			}

			qsVert->cpIndex = vertexIdx;
			//vertex color
			//if( QSFBXImporter::mEnableVC )
			
			{
				qsVert->vertColor = GetVertexColor( pMesh, vertexIdx, vertIdx );

				float sum = qsVert->vertColor.x + qsVert->vertColor.y + qsVert->vertColor.z;
				if( mMaterials[0]->mNeedVC )
				{
					if(sum == 0.0f )
					{
						qsVert->vertColor.x = 0.333333f;
						qsVert->vertColor.y = 0.333333f;
						qsVert->vertColor.z = 0.333333f;
					}else
					{
						qsVert->vertColor /= sum;
					}
				}else
				{
					D3DXVec3Normalize( &qsVert->vertColor, &qsVert->vertColor );
				}

				
			}

			//mark vertex for use original tbn
			if( QSFBXImporter::mAvatarExport )
			{
				D3DXVECTOR3 vc = GetVertexColor( pMesh, vertexIdx, vertIdx );
				if( vc.x == vc.y && vc.y == vc.z && vc.z == vc.x && vc.x == 0.0f )
					qsVert->useOriginalTBN = true;
			}

			//smoothing index
			qsVert->smoothingIndex = smoothFlag;
			//skin blend weights
			if( pQsMesh->mbSkinned )
			{
				qsVert->blendWeights = skinningData.mBlendWeights[vertexIdx];
			}

			pQsMesh->mVerts[allVertexIndex] = qsVert;

			//record vertex index
			uint* indexBuf = (uint*)pQsMesh->mIndex;
			indexBuf[allVertexIndex] = allVertexIndex++;

			++vertIdx;
		}
	}

	PostParseMesh( pQsMesh, pMesh );

	return pQsMesh;

}
//--------------------------------------------------------------------------------------
void QSFBXImporter::PostParseMesh( QS_FBXMesh* pQsMesh, KFbxMesh* pFbxMesh )
{
	QSASSERT( pQsMesh );

	const int vertexCnt = pFbxMesh->GetControlPointsCount();
	//we compute our own TBN
	pQsMesh->ComputeNTB(vertexCnt);

	//deal with verts marked by vertex color
	if( QSFBXImporter::mAvatarExport )
	{
		uint totalIndex = 0;
		const uint polygonCnt = pQsMesh->mTriangleCnt;
		for( uint i = 0; i < polygonCnt; ++i )
		{
			for( int pi = 0; pi < 3; ++pi )
			{
				QS_VERTEX* qsVert = pQsMesh->mVerts[totalIndex];
				if( qsVert->useOriginalTBN )
				{
					//TBN, get from fbx
					KFbxVector4 fbxNormal;
					pFbxMesh->GetPolygonVertexNormal( i, pi, fbxNormal );
					fbxNormal.Normalize();
					qsVert->normal = KFbxVector4ToD3DXVECTOR3( fbxNormal );
		//			qsVert->tangent = GetTangent(pFbxMesh, totalIndex);
		//			qsVert->binormal = GetBinormal( pFbxMesh, totalIndex );
				}
				++totalIndex;
			}
		}
	}

	//combine the same vertices
	pQsMesh->Optimize();

	//gathering bounding box info
	D3DXVECTOR3 bBoxMax( FLT_MIN, FLT_MIN, FLT_MIN );
	D3DXVECTOR3 bBoxMin( FLT_MAX, FLT_MAX, FLT_MAX );

	Vertices& verts = pQsMesh->mVerts;
	for( uint i = 0; i < verts.size(); ++i )
	{
		QS_VERTEX* qsVert = verts[i];

		//bounding box
		if( qsVert->position.x > bBoxMax.x )
			bBoxMax.x = qsVert->position.x;
		if( qsVert->position.y > bBoxMax.y )
			bBoxMax.y = qsVert->position.y;
		if( qsVert->position.z > bBoxMax.z )
			bBoxMax.z = qsVert->position.z;

		if( qsVert->position.x < bBoxMin.x )
			bBoxMin.x = qsVert->position.x;
		if( qsVert->position.y < bBoxMin.y )
			bBoxMin.y = qsVert->position.y;
		if( qsVert->position.z < bBoxMin.z )
			bBoxMin.z = qsVert->position.z;

	}
	pQsMesh->mBoundMax = bBoxMax;
	pQsMesh->mBoundMin = bBoxMin;
}
//---------------------------------------------------------------------
void QSFBXImporter::ParseBoneWeights( KFbxMesh* pMesh, SkinningData& skinningData )
{
	const int skinCnt = pMesh->GetDeformerCount(KFbxDeformer::eSKIN);
	for( int i = 0; i < skinCnt; ++i )
	{
		KFbxSkin* pFBXSkin = KFbxCast<KFbxSkin>(pMesh->GetDeformer(i, KFbxDeformer::eSKIN));
		if( !pFBXSkin )
			continue;

		ParseBoneWeights( pFBXSkin, skinningData );
	}
}
//---------------------------------------------------------------------
//static FileWriter sDumpFile("e:\\BoneDebug.txt");
void QSFBXImporter::ParseBoneWeights( KFbxSkin* pSkin, SkinningData& skinningData )
{
	KFbxCluster::ELinkMode linkMode = KFbxCluster::eNORMALIZE;


	const int clusterCount = pSkin->GetClusterCount();
	for( int i = 0; i < clusterCount; ++i )
	{
		KFbxCluster* pCluster = pSkin->GetCluster(i);

		linkMode = pCluster->GetLinkMode();
		KFbxNode* pLinkNode = pCluster->GetLink();

		if( !pLinkNode )
			continue;

		skinningData.AddBoneInvolved( pLinkNode->GetName() );
		
		KFbxXMatrix fbxLinkMat;//, fbxMat;
		pCluster->GetTransformLinkMatrix( fbxLinkMat );
	//	pCluster->GetTransformMatrix( fbxMat );

	//	D3DXMATRIX mat = KFbxXMatrixToD3DXMATRIX( fbxMat );
		D3DXMATRIX matLink = KFbxXMatrixToD3DXMATRIX( fbxLinkMat );
		D3DXMATRIX finalMat, invMat;
		D3DXMatrixInverse( &invMat, NULL, &matLink );
		finalMat = matLink;
		finalMat.m[3][0] = invMat.m[3][0];
		finalMat.m[3][1] = invMat.m[3][1];
		finalMat.m[3][2] = invMat.m[3][2];
		skinningData.mSkinToBoneMats.push_back( finalMat );
		int* cpIndices = pCluster->GetControlPointIndices();
		double* cpWeights = pCluster->GetControlPointWeights();
		const int cpIndexCount = pCluster->GetControlPointIndicesCount();

		for( int j = 0; j < cpIndexCount; ++j )
		{
			// NOTICE: blend indices are not the index of the whole skeleton. just indice the bones which this mesh belonged
			skinningData.AddBlendWeight( cpIndices[j], skinningData.GetInvolvedBoneCount()-1, (float)cpWeights[j]) ;
		}
	
	}
	switch( linkMode )
	{
	case KFbxCluster::eNORMALIZE:
		skinningData.Normalize();
		break;
	case KFbxCluster::eADDITIVE:
	case KFbxCluster::eTOTAL1:
		break;
	}

}
//---------------------------------------------------------------------
void QSFBXImporter::Reset()
{
	if(mFbxScene)
	{
		mFbxScene->Destroy();
		mFbxScene = NULL;
	}
}
//---------------------------------------------------------------------
void QSFBXImporter::ParseMeshByMultipleMats( KFbxMesh* pFbxMesh, eastl::map<QS_FBXMaterial*, QS_FBXMesh*>& subMeshes )
{
	// split mesh for get their names
	KFbxGeometryConverter converter( mFbxSDKManager );
	converter.SplitMeshPerMaterial( pFbxMesh );

	KFbxVector4* pControlPointArr = pFbxMesh->GetControlPoints();
	int cpCnt = pFbxMesh->GetControlPointsCount();
	int polyCnt = pFbxMesh->GetPolygonCount();
	
	// process skin stuffs
	bool bSkin = pFbxMesh->GetDeformerCount( KFbxDeformer::eSKIN ) == 0 ? false : true;
	SkinningData skinningData( cpCnt );
	if( bSkin )
	{
		ParseBoneWeights( pFbxMesh, skinningData );
	}

	unsigned int allVertexIndex = 0;
	for( int polyIdx = 0; polyIdx < polyCnt; ++polyIdx )
	{
		QS_FBXMaterial* pMat = NULL;
		QS_FBXMesh* curMesh = NULL;
		// get smoothing index
		int smoothFlag = GetSmoothingIndex( pFbxMesh, polyIdx);

		for( int pvi = 0; pvi < 3; ++pvi )
		{
			int nVertexIndex = pFbxMesh->GetPolygonVertex(polyIdx, pvi);
			if( nVertexIndex < 0 || nVertexIndex >= cpCnt )
				continue;

			if( pMat == NULL )
			{
				const int matIdx = GetMaterialIndexLinkedWithPolygon( pFbxMesh, 0, polyIdx, 0, nVertexIndex );
				assert( matIdx < (int)mMaterials.size() );

				pMat = mMaterials[matIdx];
			}

			curMesh = subMeshes[pMat];
			//init mesh
			if( !curMesh )
			{
				QS_FBXMesh* pMesh = new QS_FBXMesh;
				uint vertexCnt = polyCnt*3;
				pMesh->mIndex = new uint[vertexCnt];
				
				pMesh->mFbxMat = pMat;
				subMeshes[pMat] = pMesh;
				curMesh = pMesh;
				//char buf[1024];
				//sprintf(buf, "Mesh is : %x , mat name is %s IN. \n" ,  curMesh, pMat->mName.c_str() );
				//OutputDebugStringA( buf );
			}
			
			QS_VERTEX* qsVert = new QS_VERTEX;
			//pos,uv
			qsVert->position = KFbxVector4ToD3DXVECTOR3( pControlPointArr[nVertexIndex] );
			//UVs
			KStringList lUVSetNameList;
			pFbxMesh->GetUVSetNames(lUVSetNameList);
			//iterating over all uv sets
			for (int lUVSetIndex = 0; lUVSetIndex < lUVSetNameList.GetCount(); ++lUVSetIndex)
			{
				//get lUVSetIndex-th uv set
				const char* lUVSetName = lUVSetNameList.GetStringAt(lUVSetIndex);

				qsVert->UVs.push_back( GetUV( lUVSetName, pFbxMesh, polyIdx, pvi, nVertexIndex ) );
			}
			qsVert->cpIndex = nVertexIndex;
			qsVert->smoothingIndex = smoothFlag;

			//vertex color
			if( QSFBXImporter::mEnableVCForce || pMat->mNeedVC )
			{
				qsVert->vertColor = GetVertexColor( pFbxMesh, nVertexIndex, allVertexIndex );
				float sum = qsVert->vertColor.x + qsVert->vertColor.y + qsVert->vertColor.z;
				if( pMat->mNeedVC )
				{
					if(sum == 0.0f )
					{
						qsVert->vertColor.x = 0.333333f;
						qsVert->vertColor.y = 0.333333f;
						qsVert->vertColor.z = 0.333333f;
					}else
					{
						qsVert->vertColor /= sum;
					}
				}
			}

			//mark vertex for use original tbn
			if( QSFBXImporter::mAvatarExport )
			{
				D3DXVECTOR3 vc = GetVertexColor( pFbxMesh, nVertexIndex, allVertexIndex );
				if( vc.x == vc.y && vc.y == vc.z && vc.z == vc.x && vc.x == 0.0f )
					qsVert->useOriginalTBN = true;
			}
			
			//skin blend weights
			if( bSkin )
			{
				qsVert->blendWeights = skinningData.mBlendWeights[nVertexIndex];
			}

			unsigned int* indexBuf = (unsigned int*)curMesh->mIndex;
			indexBuf[allVertexIndex] = allVertexIndex++;

			curMesh->mVerts.push_back( qsVert );

		}
		++curMesh->mTriangleCnt;
	}


	KFbxNode* pNode = pFbxMesh->GetNode();
	assert( pNode );
	//Be sure the material index of mesh are in-order.
	for( uint i = 0;i < mMaterials.size(); ++i )
	{
		QS_FBXMesh* curMesh = subMeshes[ mMaterials[i] ];
	//	QS_FBXMesh* curMesh = it->second;
		if( !curMesh )
			continue;

		if( bSkin )
		{
			curMesh->mbSkinned = true;
			const int boneCnt = skinningData.GetInvolvedBoneCount();
			for( int i = 0; i < boneCnt; ++i )
			{
				curMesh->mBoneNames.push_back( skinningData.GetBoneName(i) );
				curMesh->mSkinToBoneMats.push_back( skinningData.mSkinToBoneMats[i] );
			}
		}
		KFbxMesh* pNewMesh = (KFbxMesh*)pNode->GetNodeAttributeByIndex( i+1 );
		assert( pNewMesh );
		
		curMesh->mName = pNewMesh->GetName();

		PostParseMesh( curMesh, pFbxMesh );
	}
}
//---------------------------------------------------------------------
int QSFBXImporter::GetMaterialIndexLinkedWithPolygon(KFbxMesh* pFBXMesh, int nLayerIndex, int nPolygonIndex, int nPolygonVertexIndex, int nVertexIndex)
{
	if( nLayerIndex < 0 || nLayerIndex > pFBXMesh->GetLayerCount() )
		return -1;

	KFbxNode* pNode = pFBXMesh->GetNode();

	if( !pNode )
		return -1;

	KFbxLayerElementMaterial* pFBXMaterial = pFBXMesh->GetLayer(nLayerIndex)->GetMaterials();

	if( pFBXMaterial )
	{
		int nMappingIndex = GetMappingIndex( pFBXMaterial->GetMappingMode(), nPolygonIndex, 0, nVertexIndex );

		if( nMappingIndex < 0 )
			return -1;

		KFbxLayerElement::EReferenceMode referenceMode = pFBXMaterial->GetReferenceMode();


		if( referenceMode == KFbxLayerElement::eDIRECT )
		{
			if( nMappingIndex < pNode->GetMaterialCount() )
			{
				return nMappingIndex;
			}
		}
		else if( referenceMode == KFbxLayerElement::eINDEX_TO_DIRECT )
		{
			const KFbxLayerElementArrayTemplate<int>& pMaterialIndexArray = pFBXMaterial->GetIndexArray();

			if( nMappingIndex < pMaterialIndexArray.GetCount() )
			{
				int nIndex = pMaterialIndexArray.GetAt(nMappingIndex);
				if( nIndex < pNode->GetMaterialCount() )
				{
					return nIndex;
				}
			}
		}
	}

	return -1;
}
//--------------------------------------------------------------------------------------
int QSFBXImporter::GetMappingIndex(KFbxLayerElement::EMappingMode MappingMode, int nPolygonIndex, int nPolygonVertexIndex, int nVertexIndex)
{
	switch(MappingMode)
	{
	case KFbxLayerElement::eALL_SAME:
		return 0;
		break;

	case KFbxLayerElement::eBY_CONTROL_POINT:
		return nVertexIndex;
		break;

	case KFbxLayerElement::eBY_POLYGON_VERTEX:
		return nPolygonIndex*3 + nPolygonVertexIndex;
		break;

	case KFbxLayerElement::eBY_POLYGON:
		return nPolygonIndex;
		break;

	case KFbxLayerElement::eNONE:
	case KFbxLayerElement::eBY_EDGE:
		break;
	}
	return -1;
}
//--------------------------------------------------------------------------------------
const eastl::string lodStr = "_LOD";
bool QSFBXImporter::HasLOD( KFbxNode* pNode )
{
	int childCnt = pNode->GetChildCount();
	if( childCnt )
	{
		eastl::string name = pNode->GetName();
		u32 pos = name.find( lodStr );
		if( pos != eastl::string::npos )
		{
			name = name.substr(0, pos );
		}

		for( int i = 0; i < childCnt; ++i )
		{
			KFbxNode* pChild = pNode->GetChild(i);
			eastl::string childName = pChild->GetName();
			if( childName.find(name) != eastl::string::npos )
			{
				return true;
			}
		}
	}

	return false;
}
//--------------------------------------------------------------------------------------
bool QSFBXImporter::VerifyMaterialNames( const eastl::vector<QS_FBXMaterial*>& mats )
{
	for( u32 idx = 0; idx < mats.size(); ++idx )
	{
		for( u32 cmpIdx = idx+1; cmpIdx < mats.size(); ++cmpIdx )
		{
			if( cmpIdx < mats.size() )
			{
				if( mats[idx]->mName == mats[cmpIdx]->mName )
				{
					MessageBox(0, "多重材质名相同！", "Error", MB_OK);
					return false;
				}
			}
			else
			{
				return true;
			}
		}
	}


	return true;
}
//--------------------------------------------------------------------------------------
D3DXVECTOR3 QSFBXImporter::GetVertexColor( KFbxMesh* pMesh, int cpIdx, int vertIdx )
{
	KFbxLayer* layer = pMesh->GetLayer( 0 );
	if( layer )
	{
		KFbxLayerElementVertexColor* vertexColorElem = layer->GetVertexColors();
		if( vertexColorElem )
		{
			KFbxLayerElement::EMappingMode mappingMode = vertexColorElem->GetMappingMode();
			KFbxLayerElement::EReferenceMode referenceMode = vertexColorElem->GetReferenceMode();

			const KFbxLayerElementArrayTemplate<KFbxColor>& pVCArray = vertexColorElem->GetDirectArray();
			const KFbxLayerElementArrayTemplate<int>& pVCIndexArray = vertexColorElem->GetIndexArray();
			if( mappingMode == KFbxLayerElement::eBY_CONTROL_POINT )
			{
				if( referenceMode == KFbxLayerElement::eDIRECT )
				{
					if( cpIdx < pVCArray.GetCount() )
					{
						return KFbxColorToD3DXVECTOR3( pVCArray.GetAt(cpIdx) );
					}
				}
				else if( referenceMode == KFbxLayerElement::eINDEX_TO_DIRECT )
				{
					if( cpIdx < pVCIndexArray.GetCount() )
					{
						int nIndex = pVCIndexArray.GetAt(cpIdx);
						if( nIndex < pVCArray.GetCount() )
						{
							return KFbxColorToD3DXVECTOR3( pVCArray.GetAt(nIndex) );
						}
					}
				}
				else
				{
					assert(0);
				}
			}
			else if( mappingMode == KFbxLayerElement::eBY_POLYGON_VERTEX )
			{
				if( referenceMode == KFbxLayerElement::eDIRECT ||
					referenceMode == KFbxLayerElement::eINDEX_TO_DIRECT )
				{
					if( vertIdx < pVCIndexArray.GetCount() )
					{
						int nIndex = pVCIndexArray.GetAt( vertIdx );
						if( nIndex < pVCArray.GetCount() )
						{
							return KFbxColorToD3DXVECTOR3( pVCArray.GetAt(nIndex) );
						}
					}
				}
				else
				{
					assert(0);
				}
			}
			else
			{
				assert(0);
			}
		}
	}

	return D3DXVECTOR3();
}
//--------------------------------------------------------------------------------------
bool QSFBXImporter::IsBoundingVolume( KFbxNode* pNode )
{
	eastl::string name = pNode->GetName();
	if( name.find("_Box") != eastl::string::npos )
	{
		int matCnt = pNode->GetMaterialCount();
		return matCnt == 0 ? true : false;
	}

	return false;
}
//--------------------------------------------------------------------------------------
bool QSFBXImporter::IsBelong( const eastl::string& boundingBoxName, const eastl::string& meshName )
{
	if( boundingBoxName.find( meshName ) != eastl::string::npos )
	{
		return true;
	}

	return false;
}
//--------------------------------------------------------------------------------------
bool QSFBXImporter::IsCollisionMesh( KFbxMesh* pMesh )
{
	const eastl::string name = pMesh->GetNode()->GetName();
	if( name.find( "_C" ) != eastl::string::npos && name.find("_Cloth") == eastl::string::npos 
		&& name.find("_CLOTH") == eastl::string::npos )
	{
		return true;
	}

	return false;
}

//--------------------------------------------------------------------------------------
LPDIRECT3DDEVICE9		QSFBXImporter::Device = NULL;
