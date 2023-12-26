// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

//#define FBXSDK_DEFINE_NAMESPACE 0
//#define FBXSDK_NAMESPACE_USING 0

//#include "targetver.h"

//#pragma comment(lib, "rpcrt4.lib")  // UuidCreate - Minimum supported OS Win 2000
//#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
//#include <tchar.h>
#include <fbxsdk.h>

#include <vector>
#include <map>
#include <list>
#include <set>
#include <string>
#include <assert.h>

#define _USE_MATH_DEFINES

#include <math.h>
#include <algorithm> 

#ifdef _MSC_VER 
	#define strncasecmp _strnicmp
	#define strcasecmp _stricmp
#else
	#define _LINUX

typedef signed char        int8_t;
typedef short              int16_t;
typedef int                int32_t;
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;

#endif

extern "C"
{
#include "jpeg-6b/jpeglib.h"
#include "lpng1627/png.h"
}

extern "C"
{
#ifdef _MSC_VER
	#include <Rpc.h>
#else
	#include <uuid/uuid.h>
#endif
}

#ifndef max_0
#define max_0(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min_0
#define min_0(a,b)            (((a) < (b)) ? (a) : (b))
#endif


using std::string;
using std::vector;
using std::map;

#include "AssistantFunctions.h"

#include "high_res_timer.h"

#include "Common/Utils.h"

#define __TEMP_BUFFER_FLOAT__ 1024

#define MAX_PATH 260

extern bool getFilePathElements(const string& path, string& outDrive, string& outDir, string& outFileName, string& outExt);
extern void writeConvertedFile(std::string& fileName, std::vector<std::string>& convertedFiles, 
	unsigned int triangleCount, unsigned int polygonCount, unsigned int vertexCount);
extern int roundUp(int numToRound, int multiple);