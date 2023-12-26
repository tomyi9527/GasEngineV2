#pragma once
#include "CommonStruct.h"

class meshLoader
{
public:
	meshLoader();
	~meshLoader();

	bool load(const std::string& filePath);
	void clear();

	int mTriangleCount;
	int mVertexCount;
	int mTopologicalLineCount;

	float* mPositions;
	unsigned int mPositionBufferSize;

	float* mNormals;
	unsigned int mNormalBufferSize;

	unsigned int* mVertexColors;
	unsigned int mVertexColorBufferSize;

	int* mIndices;
	unsigned int mIndexBufferSize;

	int* mTopologicalIndices;
	unsigned int mTopologicalIndexBufferSize;

	float mMinX, mMinY, mMinZ, mMaxX, mMaxY, mMaxZ;

	int mUniqueID, mParentUniqueID;

	std::vector<std::string> mStringTable;
};