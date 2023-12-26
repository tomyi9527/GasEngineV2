#include "stdafx.h"
#include "Utils.h"
//#include <fcntl.h>
#include "zlib-1.2.8/zlib.h"
#ifdef _MSC_VER
#include <io.h>
#else
#include <sys/io.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

double safeDouble(double n){
  return isnan(n) ? 0.0f : n;
}

FBXSDK_NAMESPACE::FbxQuaternion transEulerToQuaternion(float x,float y,float z) 
{
    //dzlog_info("trans %f %f %f",x,y,z);
    FBXSDK_NAMESPACE::FbxQuaternion q;
    FBXSDK_NAMESPACE::FbxVector4 euler(x,y,z,0);
    q.ComposeSphericalXYZ(euler);
    //dzlog_debug("trans euler [%f,%f,%f] to quaternion [%f,%f,%f,%f]",x,y,z,q[0],q[1],q[2],q[3]);
    return q;
}

void showFBXMatrix(FbxAMatrix m,const char * szMatName) 
{

    //dzlog_info("show mat: %s",szMatName);
    for (int i = 0; i < 4; ++i) {
        //dzlog_info("| %8.3f %8.3f %8.3f %8.3f  |",m.Get(i,0),m.Get(i,1),m.Get(i,2),m.Get(i,3));
    }
}

string standardizeFileName(const string& fileName)
{
	char illegalCharacters[] = { '\\', '/', '*', '|', '?', '<', '>', ':', '"', ' ', '\t', '\n', '\r'};
	string str = fileName;
	if (fileName.length() > 0)
	{
		int count = (int)sizeof(illegalCharacters);
		for (int i = 0; i < count; ++i)
		{
			char c = illegalCharacters[i];
            std::replace(str.begin(), str.end(), c, '_');
		}
	}
	return str;
}


void utilShowBytes(unsigned char *p,int len) 
{  
    for(int i=0;i<len;i++)  
    {  
        printf("%x ",*(p++));  
    }  
    printf("\n");  
}  

string intToString(int v) 
{
    return std::to_string(v);
}

string floatToString(float v) 
{
    return std::to_string(v);
}

void showFloatBufferString(void *buffer,int iFloatCount,int lineCount)
{
    //dzlog_info("====show float count %d=====",iFloatCount);
    string show = "\n";
    float *p = reinterpret_cast<float *>(buffer);
    int lineNumber = 0;
    for (int i = 0; i < iFloatCount; ++i) {
        if (i % lineCount == 0) {
           show += intToString(lineNumber++) + ": ";
        }
        show += floatToString(p[i]) + " ";
        if (i % lineCount == (lineCount - 1)) {
           show += "\n";
        }
    }
    //dzlog_info("%s",show.c_str());
    //dzlog_info("====show float End=====");
}

void showUint16BufferString(void *buffer,uint16_t iShortCount,int lineCount)
{
    //dzlog_info("====show uint16 count %d=====",iShortCount);
    string show = "\n";
    uint16_t *p = reinterpret_cast<uint16_t *>(buffer);
    int lineNumber = 0;
    for (int i = 0; i < iShortCount; ++i) {
        if (i % lineCount == 0) {
           show += intToString(lineNumber++) + ": ";
        }
        show += intToString(p[i]) + " ";
        if (i % lineCount == (lineCount - 1)) {
           show += "\n";
        }
    }
    //dzlog_info("%s",show.c_str());
    //dzlog_info("====show float End=====");
}

//int roundUp(int numToRound, int multiple)
//{
//	if (multiple == 0)
//		return numToRound;
//
//	int remainder = numToRound % multiple;
//	if (remainder == 0)
//		return numToRound;
//
//	return numToRound + multiple - remainder;
//}

void displaceMemcpy(float toBuffer[],const vector<float>& vFromVec,float baseBuffer[],int iFloatCount)
{
    for (int i = 0; i < iFloatCount; ++i) {
        //dzlog_info("%d %f %f",i,vFromVec[i],baseBuffer[i]);
        toBuffer[i] = vFromVec[i] - baseBuffer[i];
    }
}

std::string newUUID()
{
#ifdef _MSC_VER
	UUID uuid;
	UuidCreate(&uuid);

	unsigned char * str;
	UuidToStringA(&uuid, &str);

	std::string s((char*)str);

	RpcStringFreeA(&str);
#else
	uuid_t uuid;
	uuid_generate_random(uuid);
	char s[37];
	uuid_unparse(uuid, s);
#endif
	return s;
}

#ifdef _MSC_VER 
#include <windows.h>
std::wstring& mbs2wcs(unsigned int codePage, const std::string& strMB, std::wstring& tmpWstr)
{
	int len = MultiByteToWideChar(codePage, 0, strMB.c_str(), (int)strMB.size(), 0, 0);
	tmpWstr.resize(len);
	MultiByteToWideChar(codePage, 0, strMB.c_str(), (int)strMB.size(), &tmpWstr[0], len);
	return tmpWstr;
}

std::string& wcs2mbs(unsigned int codePage, const std::wstring& strWCS, std::string& tmpStr)
{
	int len = WideCharToMultiByte(codePage, 0, strWCS.c_str(), (int)strWCS.size(), 0, 0, 0, 0);
	tmpStr.resize(len);
	WideCharToMultiByte(codePage, 0, strWCS.c_str(), (int)strWCS.size(), &tmpStr[0], len, 0, 0);
	return tmpStr;
}

std::wstring& Platform_UTF8_UTF16(const std::string& strMB, std::wstring& tmpWstr)
{
	return mbs2wcs(CP_UTF8, strMB, tmpWstr);
}

std::string& Platform_UTF16_UTF8(std::wstring& strWCS, std::string& tmpStr)
{
	return wcs2mbs(CP_UTF8, strWCS, tmpStr);
}

std::string& Platform_SHIFTJIS_UTF8(const std::string& strShiftJIS, std::string& outStrUTF8)
{
	std::wstring tmpWstr;
	mbs2wcs(932, strShiftJIS, tmpWstr);
	wcs2mbs(CP_UTF8, tmpWstr, outStrUTF8);
	return outStrUTF8;
}

std::wstring& Platform_SHIFTJIS_UTF16(const std::string& strShiftJIS, std::wstring& outUTF16)
{
	mbs2wcs(932, strShiftJIS, outUTF16);
	return outUTF16;
}
#else
#include <iconv.h>
#include <string.h>
#include <errno.h>

std::wstring& mbs2wcs(unsigned int codePage, const std::string& strMB, std::wstring& tmpWstr)
{
	return tmpWstr;
}

std::string& wcs2mbs(unsigned int codePage, const std::wstring& strWCS, std::string& tmpStr)
{
	return tmpStr;
}

std::wstring& Platform_UTF8_UTF16(const std::string& strMB, std::wstring& tmpWstr)
{
	return tmpWstr;
}

std::string& Platform_UTF16_UTF8(std::wstring& strWCS, std::string& strUTF8)
{
	const char* encFrom = "UNICODE//TRANSLIT";
	const char* encTo = "UTF-8";

	iconv_t des = iconv_open(encTo, encFrom);
	if (des != (iconv_t)-1)
	{

		const int OUT_BUFFER_LEN = 2048;
		char outBuffer[OUT_BUFFER_LEN];
		memset(outBuffer, 0, OUT_BUFFER_LEN);
		size_t outBytesLeft = OUT_BUFFER_LEN;

		char* inBuffer = (char*)strWCS.c_str();
		size_t inBytesLeft = strWCS.size() * 2;
		char* outBufferPointer = outBuffer;

		size_t ret = iconv(des, &inBuffer, &inBytesLeft, &outBufferPointer, &outBytesLeft);
		if (ret != -1)
		{
			strUTF8 = outBuffer;
		}
		else
		{
			FBXSDK_printf("%s:%d", "Error: Platform_UTF16_UTF8 Failed 0", errno);
		}

		iconv_close(des);
	}
	else
	{
		FBXSDK_printf("%s:%d", "Error: Platform_UTF16_UTF8 Failed 1", errno);
	}

	return strUTF8;
}

std::string& Platform_SHIFTJIS_UTF8(const std::string& strSHIFTJIS, std::string& strUTF8)
{
	const char* encFrom = "SHIFT-JIS";
	const char* encTo = "UTF-8//IGNORE";

	iconv_t des = iconv_open(encTo, encFrom);
	if (des != (iconv_t)-1)
	{

		const int OUT_BUFFER_LEN = 2048;
		char outBuffer[OUT_BUFFER_LEN];
		memset(outBuffer, 0, OUT_BUFFER_LEN);
		size_t outBytesLeft = OUT_BUFFER_LEN;

		char* inBuffer = (char*)strSHIFTJIS.c_str();
		size_t inBytesLeft = strSHIFTJIS.size();
		char* outBufferPointer = outBuffer;

		size_t ret = iconv(des, &inBuffer, &inBytesLeft, &outBufferPointer, &outBytesLeft);
		if (ret != -1)
		{
			strUTF8 = outBuffer;
		}
		else
		{
			strUTF8 = outBuffer;
			FBXSDK_printf("%s:%d", "Error: Platform_SHIFTJIS_UTF8 Failed 0", errno);
		}

		iconv_close(des);
	}
	else
	{
		FBXSDK_printf("%s:%d", "Error: Platform_SHIFTJIS_UTF8 Failed 1", errno);
	}

	return strUTF8;
}

#endif

void leftToRightVector3(float v[3])
{
	v[2] = -v[2];
}

void leftToRightQuaternion(float q[4])
{
	q[0] = -q[0];
	q[1] = -q[1];
}

void leftToRightEuler(float r[3])
{
	r[0] = -r[0];
	r[1] = -r[1];
}

void leftToRightIndexOrder(int p[3])
{
	int tmp = p[2];
	p[2] = p[0];
	p[0] = tmp;
}

void leftToRightVector3Range(float v1[3], float v2[3])
{
	float tmp = -v2[2];
	v2[2] = -v1[2];
	v1[2] = tmp;
}

void leftToRightEulerRange(float r1[3], float r2[3])
{
	float tmp1 = -r2[0];
	float tmp2 = -r2[1];
	r2[0] = -r1[0];
	r2[1] = -r1[1];
	r1[0] = tmp1;
	r1[1] = tmp2;
}

std::string getFileDrive(const std::string& filePath)
{
	size_t index = filePath.find(':');
	if (index != std::string::npos)
	{
		index = index + 1;
	}

	std::string str;
	if (index != std::string::npos)
	{
		str = filePath.substr(0, index);
	}
	else
	{
		str = "";
	}

	return str;
}

std::string getFileDir(const std::string& filePath)
{
	size_t index3 = filePath.find(':');
	if (index3 != std::string::npos)
	{
		index3 = index3 + 1;
	}
	else
	{
		index3 = 0;
	}

	size_t index = std::string::npos;
	size_t index0 = filePath.rfind('/');
	size_t index1 = filePath.rfind('\\');
	if (index0 != std::string::npos && index1 != std::string::npos)
	{
		index = (index0 > index1 ? index0 : index1);
	}
	else if (index0 != std::string::npos)
	{
		index = index0;
	}
	else if (index1 != std::string::npos)
	{
		index = index1;
	}

	std::string str;
	if (index != std::string::npos && index3 < index)
	{
		str = filePath.substr(index3, index - index3 + 1);
	}
	else
	{
		str = "";
	}

	return str;
}

std::string getFileName(const std::string& filePath)
{
	size_t index = std::string::npos;
	size_t index0 = filePath.rfind('/');
	size_t index1 = filePath.rfind('\\');
	if (index0 != std::string::npos && index1 != std::string::npos)
	{
		index = (index0 > index1 ? index0 : index1) + 1;
	}
	else if (index0 != std::string::npos)
	{
		index = index0 + 1;
	}
	else if (index1 != std::string::npos)
	{
		index = index1 + 1;
	}

	std::string str;
	if (index != std::string::npos)
	{
		str = filePath.substr(index, filePath.length() - index);
	}
	else
	{
		str = filePath;
	}

	return str;
}

_FILE_TYPE getFileType(std::string& filePath)
{
	size_t index = filePath.rfind('.');
	if (index == std::string::npos)
	{
		return FT_UNKNOWN;
	}

	std::string ext = filePath.substr(index + 1);
	if (strcasecmp(ext.c_str(), "png") == 0)
	{
		return FT_PNG;
	}
	else if (strcasecmp(ext.c_str(), "jpg") == 0 || strcasecmp(ext.c_str(), "jpeg") == 0)
	{
		return FT_JPG;
	}
	else
	{
		return FT_UNKNOWN;
	}
}

bool getPNGInfo(std::string& filePath, unsigned int& width, unsigned int& height)
{
	png_structp png_ptr;
	png_infop info_ptr;
	int sig_read = 0;
	int bit_depth, color_type, interlace_type;
	FILE *fp;

	if ((fp = fopen(filePath.c_str(), "rb")) == NULL)
		return (false);

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (png_ptr == NULL)
	{
		fclose(fp);
		return (false);
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		fclose(fp);
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return (false);
	}

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		/* Free all of the memory associated with the png_ptr and info_ptr */
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(fp);
		/* If we get here, we had a problem reading the file */
		return (false);
	}

	png_init_io(png_ptr, fp);

	png_set_sig_bytes(png_ptr, sig_read);

	png_read_info(png_ptr, info_ptr);

	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
		&interlace_type, NULL, NULL);

	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	fclose(fp);

	return true;
}

bool getJPEGInfo(std::string& filePath, unsigned int& width, unsigned int& height)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	bool result = false;

	FILE* infile = fopen(filePath.c_str(), "rb");
	if (infile == NULL)
	{
		FBXSDK_printf("Error: Cannot open %s!\n", filePath.c_str());
		return result;
	}
	jpeg_stdio_src(&cinfo, infile);

	jpeg_read_header(&cinfo, TRUE);

	if (cinfo.num_components == 3 && cinfo.out_color_space == JCS_RGB)
	{
		width = cinfo.image_width;
		height = cinfo.image_height;
		result = true;
	}
	//jpeg_start_decompress(&cinfo);
	//jpeg_finish_decompress(&cinfo);

	jpeg_destroy_decompress(&cinfo);

	fclose(infile);

	return result;
}

#ifdef _MSC_VER 
bool getFilePathElements(const string& path, string& outDrive, string& outDir, string& outFileName, string& outExt)
{
	outDrive = getFileDrive(path);
	outDir = getFileDir(path);

	std::string fileName = getFileName(path);
	size_t s = fileName.rfind(".");
	if (s != std::string::npos)
	{
		outFileName = fileName.substr(0, s);
		outExt = fileName.substr(s, fileName.size() - s);
	}
	else
	{
		outFileName = fileName;
		outExt = "";
	}

	return true;
}

void clearDirectory_r(const std::wstring& directory)
{
	_wfinddata_t fileInfo;
	std::wstring files = directory + L"\\*";
	intptr_t handle = _wfindfirst(files.c_str(), &fileInfo);

	if (handle == -1L)
	{
		return;
	}

	do
	{
		if (fileInfo.attrib & _A_SUBDIR)
		{
			if ((wcscmp(fileInfo.name, L".") != 0) && (wcscmp(fileInfo.name, L"..") != 0))
			{
				std::wstring subDirectory = directory + L"\\" + fileInfo.name;
				clearDirectory_r(subDirectory);

				BOOL result = RemoveDirectoryW(subDirectory.c_str());
				if (result == FALSE)
				{
					DWORD err = GetLastError();
					wprintf(L"Error: Delete directory %ls failed with error code %d.\n", fileInfo.name, err);
				}
			}
		}
		else
		{
			BOOL result = DeleteFileW((directory + L"\\" + fileInfo.name).c_str());
			if (result == FALSE)
			{
				DWORD err = GetLastError();
				wprintf(L"Error: Delete file %ls failed with error code %d.\n", fileInfo.name, err);
			}
		}
	} while (_wfindnext(handle, &fileInfo) == 0);

	_findclose(handle);
}

bool makeDirectory(const std::wstring& directory) //without back slash
{
	if (_waccess(directory.c_str(), 0) == -1)
	{
		if (_wmkdir(directory.c_str()) != 0)
		{
			wprintf(L"Error: Create directory %ls failed.\n", directory.c_str());
			return false;
		}
		else
		{
			return true;
		}
	}
	else
	{
		clearDirectory_r(directory);
		return true;
	}
}

void findFile_r(const std::wstring& directory, const std::wstring& fileName, std::vector<std::wstring>& results)
{
	_wfinddata_t fileInfo;
	std::wstring files = directory + L"\\*";
	intptr_t handle = _wfindfirst(files.c_str(), &fileInfo);

	if (handle == -1L)
	{
		return;
	}

	do
	{
		if (fileInfo.attrib & _A_SUBDIR)
		{
			if ((wcscmp(fileInfo.name, L".") != 0) && (wcscmp(fileInfo.name, L"..") != 0))
			{
				std::wstring subDirectory = directory + L"\\" + fileInfo.name;
				findFile_r(subDirectory, fileName, results);
			}
		}
		else
		{
			if (_wcsnicmp(fileName.c_str(), fileInfo.name, fileName.length()) == 0)
			{
				results.push_back(directory + L"\\" + fileInfo.name);
			}
		}
	} while (_wfindnext(handle, &fileInfo) == 0);

	_findclose(handle);
}

void copyFiles(std::string& sourceDirectory, std::string& outputDirectory, std::vector<std::string>& files)
{
	std::wstring ucsSourceDirectory = UTF8_To_UCS16(sourceDirectory.c_str());
	std::wstring ucsOutputDirectory = UTF8_To_UCS16(outputDirectory.c_str());

	for (size_t s = 0; s < files.size(); ++s)
	{
		std::string texturePath = files[s];
		std::string fileDrive, fileDirectory, fileName, fileExt;
		getFilePathElements(texturePath, fileDrive, fileDirectory, fileName, fileExt);

		std::vector<std::wstring> results;
		std::wstring ucsTargetFile = UTF8_To_UCS16((fileName + fileExt).c_str());
		findFile_r(ucsSourceDirectory, ucsTargetFile, results);
		if (results.size() > 0)
		{
			CopyFileW(results[0].c_str(), (ucsOutputDirectory + ucsTargetFile).c_str(), TRUE);
		}
	}
}

#else
bool getFilePathElements(const string& path, string& outDrive, string& outDir, string& outFileName, string& outExt)
{
	outDrive = "";

	std::string fileName = getFileName(path);
	printf("filename:%s\n", fileName.c_str());
	size_t s = fileName.find(".");
	if (s != std::string::npos)
	{
		outFileName = fileName.substr(0, s);
		outExt = fileName.substr(s, fileName.size() - s);
	}
	else
	{
		outFileName = fileName;
		outExt = "";
	}

	outDir = path.substr(0, path.length() - fileName.length());

	return true;
}

bool makeDirectory(const std::string& directory) //without back slash
{
	if (access(directory.c_str(), F_OK) == -1)
	{
		if (mkdir(directory.c_str(), 0777))
		{
			wprintf(L"Error: Create directory %ls failed.\n", directory.c_str());
			return false;
		}
		else
		{
			return true;
		}
	}
	else
	{
		rmdir(directory.c_str());
		return true;
	}
}

#endif

#ifdef _MSC_VER
FBXSDK_NAMESPACE::FbxString ANSI_To_UTF8(std::string str)
{
	FBXSDK_NAMESPACE::FbxString retStr = "";
	char * newStr = NULL;
	FBXSDK_NAMESPACE::FbxAnsiToUTF8(str.c_str(), newStr);
	if (NULL != newStr)
	{
		retStr = newStr;
		delete[] newStr;
	}

	return retStr;
}

std::string UTF8_To_ANSI(FBXSDK_NAMESPACE::FbxString fbxStr)
{
	string retStr = "";
	char * newStr = NULL;
	FBXSDK_NAMESPACE::FbxUTF8ToAnsi(fbxStr.Buffer(), newStr);
	if (NULL != newStr)
	{
		retStr = newStr;
		delete[] newStr;
	}

	return retStr;
}

std::wstring UTF8_To_UCS16(FBXSDK_NAMESPACE::FbxString fbxStr)
{
	std::wstring retStr = L"";
	wchar_t* newStr = NULL;
	FBXSDK_NAMESPACE::FbxUTF8ToWC(fbxStr.Buffer(), newStr);
	if (NULL != newStr)
	{
		retStr = newStr;
		delete[] newStr;
	}

	return retStr;
}

std::string UCS16_To_UTF8(const std::wstring& unicodeString)
{
	std::string retStr = "";
	char* newStr = NULL;
	FBXSDK_NAMESPACE::FbxWCToUTF8(unicodeString.c_str(), newStr);
	if (NULL != newStr)
	{
		retStr = newStr;
		delete[] newStr;
	}

	return retStr;
}

//void compressMemory(const std::string& path)
//{
//	FILE* file = fopen(path.c_str(), "rb");
//	if (file != NULL)
//	{
//		fseek(file, 0, SEEK_END);
//		size_t size = ftell(file);
//		fseek(file, 0, SEEK_SET);
//
//		void* source = malloc(size);
//		void* destination = malloc(size);
//
//		size_t bytesRead = fread(source, 1, size, file);
//		fclose(file);
//		file = NULL;
//
//		if (bytesRead != size)
//		{
//			return;
//		}
//
//		unsigned long sourceSize = size;
//		unsigned long compressedSize = size;
//		int err = compress((unsigned char*)destination, &compressedSize, (const Bytef*)source, sourceSize);
//		if (err != Z_OK)
//		{
//			FBXSDK_printf("Error: zlib compress error!\n");
//		}
//		else
//		{
//			std::string gzFile = path + ".gz";
//			FILE* file = fopen(gzFile.c_str(), "wb+");
//			if (file != NULL)
//			{
//				fwrite(destination, compressedSize, 1, file);
//				fclose(file);
//			}
//		}
//
//		free(destination);
//		free(source);
//	}
//}

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#include <fcntl.h>
#include <io.h>
#define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#define SET_BINARY_MODE(file)
#endif

#define CHUNK 16384
int def(FILE *source, FILE *dest, int level)
{
	int ret, flush;
	unsigned have;
	z_stream strm;
	unsigned char in[CHUNK];
	unsigned char out[CHUNK];

	/* allocate deflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	//ret = deflateInit2(&strm, level);
	ret = deflateInit2(&strm, -1, 8, 15 + 16, 8, 0);

	if (ret != Z_OK)
		return ret;

	/* compress until end of file */
	do {
		strm.avail_in = (uInt)fread(in, 1, CHUNK, source);
		if (ferror(source)) {
			(void)deflateEnd(&strm);
			return Z_ERRNO;
		}
		flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
		strm.next_in = in;

		/* run deflate() on input until output buffer not full, finish
		compression if all of source has been read in */
		do {
			strm.avail_out = CHUNK;
			strm.next_out = out;
			ret = deflate(&strm, flush);    /* no bad return value */
			assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
			have = CHUNK - strm.avail_out;
			if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
				(void)deflateEnd(&strm);
				return Z_ERRNO;
			}
		} while (strm.avail_out == 0);
		assert(strm.avail_in == 0);     /* all input will be used */

										/* done when last data in file processed */
	} while (flush != Z_FINISH);
	assert(ret == Z_STREAM_END);        /* stream will be complete */

										/* clean up and return */
	(void)deflateEnd(&strm);
	return Z_OK;
}

void zerr(int ret)
{
	FBXSDK_printf("zpipe: ");
	switch (ret) {
	case Z_ERRNO:
		FBXSDK_printf("error reading from or writing to file!\n");
		//if (ferror(stdin))
		//	fputs("error reading stdin\n", stderr);
		//if (ferror(stdout))
		//	fputs("error writing stdout\n", stderr);
		break;
	case Z_STREAM_ERROR:
		FBXSDK_printf("invalid compression level\n");
		break;
	case Z_DATA_ERROR:
		FBXSDK_printf("invalid or incomplete deflate data\n");
		break;
	case Z_MEM_ERROR:
		FBXSDK_printf("out of memory\n");
		break;
	case Z_VERSION_ERROR:
		FBXSDK_printf("zlib version mismatch!\n");
	}
}

std::string compressFile(const std::string& path)
{
#ifdef _MSC_VER
	std::wstring unicodePathIn = UTF8_To_UCS16(path.c_str());
	FILE* fileIn = _wfopen(unicodePathIn.c_str(), L"rb");

	std::string gzFile = path + ".gz";
	std::wstring unicodePathOut = UTF8_To_UCS16(gzFile.c_str());
	FILE* fileOut = _wfopen(unicodePathOut.c_str(), L"wb+");
#else
	FILE* fileIn = fopen(path.c_str(), "rb");
	std::string gzFile = path + ".gz";
	FILE* fileOut = fopen(gzFile.c_str(), "wb+");
#endif

	if (fileIn != NULL && fileOut != NULL)
	{
		SET_BINARY_MODE(stdin);
		SET_BINARY_MODE(stdout);
		int ret = def(fileIn, fileOut, Z_DEFAULT_COMPRESSION);
		if (ret != Z_OK)
		{
			FBXSDK_printf("Error: zlib compress error!\n");
			zerr(ret);
		}

		fclose(fileOut);
		fclose(fileIn);
	}

	return gzFile;
}

#endif

