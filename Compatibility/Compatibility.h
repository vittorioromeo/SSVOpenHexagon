#ifndef HG_COMPATIBLITY
#define HG_COMPATIBLITY

#include <string>

namespace hg
{
	namespace Compatibility
	{
		std::string get181MD5Hash(const std::string& mString);
		std::string get181UrlEncoded(const std::string& mString);
		std::string get181FileContents(const std::string& mFilePath);
		std::string get181Validator(const std::string& mPackPath, const std::string& mLevelId, const std::string& mJsonRootPath, const std::string& mLuaScriptPath, float mDifficultyMultiplier);
		void convert181to183Hashes(const std::string& mSourceJsonPath, const std::string& mTargetJsonPath);
		void convert182to183Hashes(const std::string& mSourceJsonPath, const std::string& mTargetJsonPath);
	}
}

#endif
