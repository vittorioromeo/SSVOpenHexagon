#ifndef HG_COMPATIBLITY
#define HG_COMPATIBLITY

#include <string>

namespace hg
{
	namespace Compatibility
	{
		void merge19Scores(const std::string& mSourceJsonPathA, const std::string& mSourceJsonPathB, const std::string& mTargetJsonPath);
		void separate19Scores(const std::string& mSourceJsonPath, const std::string& mTargetJsonPath);
	}
}

#endif
