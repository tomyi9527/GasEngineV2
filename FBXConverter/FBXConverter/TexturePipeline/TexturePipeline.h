#pragma once

class TexturePipeline
{
public:
	TexturePipeline();
	~TexturePipeline();

	void* loadTGA(const std::string& filePath, \
		unsigned short& width, unsigned short& height, unsigned char& depth, unsigned char& alpha);

	bool writePNG(const std::string& filePath, unsigned short width, unsigned short height, \
		unsigned char depth, unsigned char alpha,  unsigned char* data);
};

