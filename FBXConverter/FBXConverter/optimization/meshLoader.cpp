#include "stdafx.h"
#include "meshLoader.h"

meshLoader::meshLoader()
	: mTriangleCount(0)
	, mVertexCount(0)
	, mTopologicalLineCount(0)
	\
	, mPositions(NULL)
	, mPositionBufferSize(0)
	, mNormals(NULL)
	, mNormalBufferSize(0)
	, mVertexColors(NULL)
	, mVertexColorBufferSize(0)
	, mIndices(NULL)
	, mIndexBufferSize(0)
	, mTopologicalIndices(NULL)
	, mTopologicalIndexBufferSize(0)
	\
	, mMinX(0.0f)
	, mMinY(0.0f)
	, mMinZ(0.0f)
	, mMaxX(0.0f)
	, mMaxY(0.0f)
	, mMaxZ(0.0f)
	, mUniqueID(-1)
	, mParentUniqueID(-1)
{
	mStringTable.clear();
}

meshLoader::~meshLoader()
{

}

bool meshLoader::load(const std::string& filePath)
{
#ifdef _MSC_VER
	std::wstring unicodePath = UTF8_To_UCS16(filePath.c_str());
	FILE* binFile = _wfopen(unicodePath.c_str(), L"rb");
#else
	FILE* binFile = fopen(filePath.c_str(), "rb");
#endif

	clear();

	if (binFile)
	{
		//read header
		BIN_HEADER header;
		fread(&header, 1, sizeof(header), binFile);

		mMinX = header.POS_BBOX_MIN[0];
		mMinY = header.POS_BBOX_MIN[1];
		mMinZ = header.POS_BBOX_MIN[2];
		mMaxX = header.POS_BBOX_MAX[0];
		mMaxY = header.POS_BBOX_MAX[1];
		mMaxZ = header.POS_BBOX_MAX[2];

		mUniqueID = header.UNIQUEID;
		mParentUniqueID = header.PARENTUNIQUEID;

		//read string table
		fseek(binFile, header.STRING_TABLE_OFFSET, SEEK_SET);
		int stringCount = 0;
		int stringSize = 0;
		fread(&stringCount, sizeof(int), 1, binFile);
		for (int i = 0; i < stringCount; ++i)
		{
			fread(&stringSize, sizeof(int), 1, binFile);
			char* p = new char[stringSize + 1];
			fread(p, 1, stringSize, binFile);
			p[stringSize] = 0;
			mStringTable.push_back(std::string(p));
			delete[] p;
		}

		//read section table
		fseek(binFile, header.SECTION_TABLE_OFFSET, SEEK_SET);

		int sectionCount = 0;	
		fread(&sectionCount, sizeof(int), 1, binFile);
		std::vector<SECTION_TABLE_ENTRY> sectionEntries;
		for (int i = 0; i < sectionCount; ++i)
		{
			SECTION_TABLE_ENTRY sectionEntry;			
			fread(&sectionEntry, 1, sizeof(sectionEntry), binFile);

			sectionEntries.push_back(sectionEntry);
		}

		for (int i = 0; i < sectionCount; ++i)
		{
			SECTION_TABLE_ENTRY& sectionEntry = sectionEntries[i];

			if ((SECITON_TYPE)sectionEntry.SECTION_TYPE == POSITION)
			{
				mVertexCount = sectionEntry.ATTRIBUTE_COUNT;

				mPositionBufferSize = sectionEntry.ATTRIBUTE_COUNT*sectionEntry.ELEMENT_COUNT * sizeof(float);
				mPositions = new float[sectionEntry.ATTRIBUTE_COUNT*sectionEntry.ELEMENT_COUNT];
				fseek(binFile, sectionEntry.SECTION_OFFSET + 16, SEEK_SET);
				fread(mPositions, 1, sectionEntry.SECTION_LENGTH, binFile);
			}

			if ((SECITON_TYPE)sectionEntry.SECTION_TYPE == NORMAL0)
			{
				mNormalBufferSize = sectionEntry.ATTRIBUTE_COUNT*sectionEntry.ELEMENT_COUNT * sizeof(float);
				mNormals = new float[sectionEntry.ATTRIBUTE_COUNT*sectionEntry.ELEMENT_COUNT];
				fseek(binFile, sectionEntry.SECTION_OFFSET + 16, SEEK_SET);
				fread(mNormals, 1, sectionEntry.SECTION_LENGTH, binFile);
			}

			if ((SECITON_TYPE)sectionEntry.SECTION_TYPE == INDEX)
			{
				mTriangleCount = sectionEntry.ATTRIBUTE_COUNT;

				mIndexBufferSize = sectionEntry.ATTRIBUTE_COUNT*sectionEntry.ELEMENT_COUNT * sizeof(int);
				mIndices = new int[sectionEntry.ATTRIBUTE_COUNT*sectionEntry.ELEMENT_COUNT];
				fseek(binFile, sectionEntry.SECTION_OFFSET + 16, SEEK_SET);
				fread(mIndices, 1, sectionEntry.SECTION_LENGTH, binFile);
			}

			if ((SECITON_TYPE)sectionEntry.SECTION_TYPE == TOPOLOGY)
			{
				mTopologicalLineCount = sectionEntry.ATTRIBUTE_COUNT;

				mTopologicalIndexBufferSize = sectionEntry.ATTRIBUTE_COUNT*sectionEntry.ELEMENT_COUNT * sizeof(int);
				mTopologicalIndices = new int[sectionEntry.ATTRIBUTE_COUNT*sectionEntry.ELEMENT_COUNT];
				fseek(binFile, sectionEntry.SECTION_OFFSET + 16, SEEK_SET);
				fread(mTopologicalIndices, 1, sectionEntry.SECTION_LENGTH, binFile);
			}
		}

		//clean
		fclose(binFile);

		return true;
	}

	return false;
}

void meshLoader::clear()
{
	mTriangleCount = 0;
	mVertexCount = 0;
	mTopologicalLineCount = 0;

	mMinX = mMinY = mMinZ = mMaxX = mMaxY = mMaxZ = 0.0f;

	if (mPositions)
	{
		delete[] mPositions;
		mPositions = NULL;
		mPositionBufferSize = 0;
	}

	if (mNormals)
	{
		delete[] mNormals;
		mNormals = NULL;
		mNormalBufferSize = 0;
	}

	if (mVertexColors)
	{
		delete[] mVertexColors;
		mVertexColors = NULL;
		mVertexColorBufferSize = 0;
	}

	if (mIndices)
	{
		delete[] mIndices;
		mIndices = NULL;
		mIndexBufferSize = 0;
	}

	if (mTopologicalIndices)
	{
		delete[] mTopologicalIndices;
		mTopologicalIndices = NULL;
		mTopologicalIndexBufferSize = 0;
	}

	mStringTable.clear();
}