#ifndef HG_COMPATIBLITY
#define HG_COMPATIBLITY

#include <string>

namespace hg
{
	namespace Compatiblity
	{
		std::string get181MD5Hash(const std::string& mString);
		std::string get181UrlEncoded(const std::string& mString);
		std::string get181FileContents(const std::string& mFilePath);
		std::string get181Validator(const std::string& mPackPath, const std::string& mLevelId, const std::string& mJsonRootPath, const std::string& mLuaScriptPath, float mDifficultyMultiplier);
		void convertHashes(const std::string& mSourceJsonPath, const std::string& mTargetJsonPath);
	}
}

#endif 
