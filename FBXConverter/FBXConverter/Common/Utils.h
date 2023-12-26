#pragma once
#include "CommonStruct.h"

double safeDouble(double n);
string standardizeFileName(const string& fileName);
void utilShowBytes(unsigned char *p,int len);
string floatToString(float v);
string intToString(int v);
FBXSDK_NAMESPACE::FbxQuaternion transEulerToQuaternion(float x,float y,float z);
void showFloatBufferString(void *buffer,int iFloatCount,int lineCount = 3);
void showUint16BufferString(void *buffer,uint16_t iShortCount,int lineCount = 4);
//int roundUp(int numToRound, int multiple);
void displaceMemcpy(float toBuffer[],const vector<float>& vFromVec,float baseBuffer[],int iFloatCount);
void showFBXMatrix(FbxAMatrix m,const char * szMatName);
std::string newUUID();

std::wstring& mbs2wcs(unsigned int codePage, const std::string& strMB, std::wstring& tmpWstr);
std::string& wcs2mbs(unsigned int codePage, const std::wstring& strWCS, std::string& tmpStr);
std::wstring& Platform_UTF8_UTF16(const std::string& strMB, std::wstring& tmpWstr);
std::string& Platform_UTF16_UTF8(std::wstring& strWCS, std::string& tmpStr);
std::string& Platform_SHIFTJIS_UTF8(const std::string& strSHIFTJIS, std::string& strUTF8);
std::wstring& Platform_SHIFTJIS_UTF16(const std::string& strShiftJIS, std::wstring& outUTF16);

void leftToRightVector3(float v[3]);
void leftToRightQuaternion(float q[4]);
void leftToRightEuler(float r[3]);
void leftToRightIndexOrder(int p[3]);
void leftToRightVector3Range(float v1[3], float v2[3]);
void leftToRightEulerRange(float r1[3], float r2[3]);

std::string getFileDrive(const std::string& filePath);
std::string getFileDir(const std::string& filePath);
std::string getFileName(const std::string& filePath);
_FILE_TYPE getFileType(std::string& filePath);
bool getPNGInfo(std::string& filePath, unsigned int& width, unsigned int& height);
bool getJPEGInfo(std::string& filePath, unsigned int& width, unsigned int& height);

bool getFilePathElements(const string& path, string& outDrive, string& outDir, string& outFileName, string& outExt);
void copyFiles(std::string& sourceDirectory, std::string& outputDirectory, std::vector<std::string>& files);
bool makeDirectory(const std::wstring& directory);
bool makeDirectory(const std::string &directory);

std::string UTF8_To_ANSI(FbxString fbxStr);
FBXSDK_NAMESPACE::FbxString ANSI_To_UTF8(std::string str);
std::wstring UTF8_To_UCS16(FbxString fbxStr);
std::string UCS16_To_UTF8(const std::wstring& unicodeString);

std::string compressFile(const std::string& path);