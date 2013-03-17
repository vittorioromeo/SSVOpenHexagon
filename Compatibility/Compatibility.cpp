#include <fstream>
#include <unordered_set>
#include <SSVStart.h>
#include "Compatibility/Compatibility.h"
#include "Utils/Utils.h"
#include "Utils/MD5.h"
#include "Global/Assets.h"
#include "Online/Online.h"
#include "Online/Definitions.h"

using namespace std;
using namespace ssvs;
using namespace ssvs::Utils;
using namespace hg::Utils;
using namespace ssvs::UtilsJson;

namespace hg
{
	namespace Compatibility
	{
		const string serverKey182{"3g2n9br8bjuwe1"};

		string get181MD5Hash(const string& mString) { MD5 key{mString}; return key.GetHash(); }
		string get182MD5Hash(const string& mString) { MD5 key{mString}; return key.GetHash(); }
		string get181UrlEncoded(const string& mString) { string result{""}; for(unsigned int i{0}; i < mString.size(); ++i) if(isalnum(mString[i])) result += mString[i]; return result; }
		string get182UrlEncoded(const string& mString) { string result{""}; for(unsigned int i{0}; i < mString.size(); ++i) if(isalnum(mString[i])) result += mString[i]; return result; }
		string get182ControlStripped(const string& mString) { string result{""}; for(unsigned int i{0}; i < mString.size(); ++i) if(!iscntrl(mString[i])) result += mString[i]; return result; }

		string get181FileContents(const string& mFilePath) { ifstream ifs(mFilePath); string content{(istreambuf_iterator<char>(ifs)), (istreambuf_iterator<char>())}; return content; }
		string get182FileContents(const string& mFilePath)
		{
			FILE *fptr=fopen(mFilePath.c_str(),"rb");
			fseek(fptr, 0, SEEK_END);
			size_t fsize = ftell(fptr);
			fseek(fptr, 0, SEEK_SET);
			string content; content.resize(fsize);
			if(fread((char*)content.c_str(),1,fsize,fptr) != fsize) log(mFilePath,"FileLoadWarning");
			fclose(fptr); return content;
		}
		string get184FileContents(const string& mPath)
		{
			FILE* fptr{fopen(mPath.c_str(), "rb")};
			fseek(fptr, 0, SEEK_END);
			size_t fsize(ftell(fptr));
			fseek(fptr, 0, SEEK_SET);
			string content; content.resize(fsize);
			if(fread(const_cast<char*>(content.c_str()), 1, fsize, fptr) != fsize) log("Error: " + mPath, "File loading");
			fclose(fptr); return content;
		}

		string get181Validator(const string& mPackPath, const string& mLevelId, const string& mJsonRootPath, const string& mLuaScriptPath, float mDifficultyMultiplier)
		{
			string luaScriptContents{get181FileContents(mLuaScriptPath)}, result{""};
			unordered_set<string> luaScriptNames;
			recursiveFillIncludedLuaFileNames(luaScriptNames, mPackPath, luaScriptContents);

			result.append(get181UrlEncoded(mLevelId));
			result.append(get181MD5Hash(get181FileContents(mJsonRootPath) + serverKey182));
			result.append(get181MD5Hash(luaScriptContents + serverKey182));

			for(auto& luaScriptName : luaScriptNames)
			{
				string path{mPackPath + "/Scripts/" + luaScriptName}, contents{get181FileContents(path)}, hash{get181MD5Hash(contents + serverKey182)}, compressedHash{""};
				for(unsigned int i{0}; i < hash.length(); ++i) if(i % 3 == 0) compressedHash.append(toStr(hash[i]));
				result.append(compressedHash);
			}

			result.append(get181UrlEncoded(toStr(mDifficultyMultiplier))); return result;
		}

		string get182Validator(const string& mPackPath, const string& mLevelId, const string& mLevelRootPath, const string& mStyleRootPath, const string& mLuaScriptPath, float mDifficultyMultiplier)
		{
			string luaScriptContents{get182FileContents(mLuaScriptPath)}, toEncrypt{""};
			unordered_set<string> luaScriptNames;
			recursiveFillIncludedLuaFileNames(luaScriptNames, mPackPath, luaScriptContents);

			toEncrypt.append(mLevelId);
			toEncrypt.append(toStr(mDifficultyMultiplier));
			toEncrypt.append(get182FileContents(mLevelRootPath));
			toEncrypt.append(get182FileContents(mStyleRootPath));
			toEncrypt.append(luaScriptContents);

			for(auto& luaScriptName : luaScriptNames) toEncrypt.append(get182FileContents(mPackPath + "/Scripts/" + luaScriptName));

			toEncrypt = get182ControlStripped(toEncrypt);
			return get182UrlEncoded(mLevelId) + get182MD5Hash(toEncrypt + serverKey182);
		}

		void convert181to183Hashes(const string& mSourceJsonPath, const string& mTargetJsonPath)
		{
			string scores{get184FileContents(mSourceJsonPath)};
			vector<string> oldValidators, newValidators;

			for(auto& levelData : getAllLevelData())
				for(float difficultyMult : levelData.getDifficultyMultipliers())
				{
					log(""); log("");

					log("computing old validator for " + levelData.getId() + ", difficulty multiplier " + toStr(difficultyMult) +  "...");
					string oldValidator{get181Validator(levelData.getPackPath(), levelData.getId(), levelData.getLevelRootPath(), levelData.getLuaScriptPath(), difficultyMult)};
					log("\"" + oldValidator + "\"");
					oldValidators.push_back("\"" + oldValidator + "\"");
					log("");

					log("computing new validator for " + levelData.getId() + ", difficulty multiplier " + toStr(difficultyMult) +  "...");
					string newValidator{Online::getValidator(levelData.getPackPath(), levelData.getId(), levelData.getLevelRootPath(), levelData.getStyleRootPath(), levelData.getLuaScriptPath(), difficultyMult)};
					log("\"" + newValidator + "\"");
					newValidators.push_back("\"" + newValidator + "\"");
					log("");
				}

			log(""); log("");

			for(unsigned int i{0}; i < oldValidators.size(); ++i)
			{
				scores = replaceAll(scores, oldValidators[i], newValidators[i]);
				log("replacing"); log(oldValidators[i]); log("with"); log(newValidators[i]); log("");
			}

			ofstream o; o.open(mTargetJsonPath); o << scores; o.flush(); o.close();
		}
		void convert182to183Hashes(const string& mSourceJsonPath, const string& mTargetJsonPath)
		{
			string scores{get184FileContents(mSourceJsonPath)};
			vector<string> oldValidators, newValidators;

			for(auto& levelData : getAllLevelData())
				for(float difficultyMult : levelData.getDifficultyMultipliers())
				{
					log(""); log("");

					log("computing old validator for " + levelData.getId() + ", difficulty multiplier " + toStr(difficultyMult) +  "...");
					string oldValidator{get182Validator(levelData.getPackPath(), levelData.getId(), levelData.getLevelRootPath(), levelData.getStyleRootPath(), levelData.getLuaScriptPath(), difficultyMult)};
					log("\"" + oldValidator + "\"");
					oldValidators.push_back("\"" + oldValidator + "\"");
					log("");

					log("computing new validator for " + levelData.getId() + ", difficulty multiplier " + toStr(difficultyMult) +  "...");
					string newValidator{Online::getValidator(levelData.getPackPath(), levelData.getId(), levelData.getLevelRootPath(), levelData.getStyleRootPath(), levelData.getLuaScriptPath(), difficultyMult)};
					log("\"" + newValidator + "\"");
					newValidators.push_back("\"" + newValidator + "\"");
					log("");
				}

			log(""); log("");

			for(unsigned int i{0}; i < oldValidators.size(); ++i)
			{
				scores = replaceAll(scores, oldValidators[i], newValidators[i]);
				log("replacing"); log(oldValidators[i]); log("with"); log(newValidators[i]); log("");
			}

			ofstream o; o.open(mTargetJsonPath); o << scores; o.flush(); o.close();
		}
	}
}
