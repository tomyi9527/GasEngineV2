#include "VmdReader.h"

struct morphTargetAnimation;

class VmdConverter
{
public:
	VmdConverter();
	~VmdConverter();

	bool convert(
		const std::string& workingDirectory,
		const std::string& filePath);

	std::vector<morphTargetAnimation*>* getMophAnimationData(
		std::vector<vmd::VmdMorphFrame>& morphKeyframes,
		int minFrame,
		int maxFrame
	);
};