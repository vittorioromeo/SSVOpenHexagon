#include <fstream>
#include <unordered_set>
#include <SSVStart.h>
#include "Compatibility.h"
#include "Utils/Utils.h"
#include "Utils/MD5.h"
#include "Global/Assets.h"
#include "Online/Online.h"

using namespace std;
using namespace ssvs;
using namespace ssvs::Utils;
using namespace hg::Utils;
using namespace hg::UtilsJson;

namespace hg
{
	namespace Compatiblity
	{
		const string serverKey182{"3g2n9br8bjuwe1"};

		string get181MD5Hash(const string& mString) { MD5 key{mString}; return key.GetHash(); }
		string get181UrlEncoded(const string& mString)
		{
			string result{""};
			for(unsigned int i{0}; i < mString.size(); ++i) if(isalnum(mString[i])) result += mString[i];
			return result;
		}
		string get181FileContents(const string& mFilePath)
		{
			ifstream ifs(mFilePath);
			string content{(istreambuf_iterator<char>(ifs)), (istreambuf_iterator<char>())};
			return content;
		}
		string get181Validator(const string& mPackPath, const string& mLevelId, const string& mJsonRootPath, const string& mLuaScriptPath, float mDifficultyMultiplier)
		{
			string luaScriptContents{get181FileContents(mLuaScriptPath)};

			unordered_set<string> luaScriptNames;
			recursiveFillIncludedLuaFileNames(luaScriptNames, mPackPath, luaScriptContents);

			string result{""};
			result.append(get181UrlEncoded(mLevelId));
			result.append(get181MD5Hash(get181FileContents(mJsonRootPath) + serverKey182));
			result.append(get181MD5Hash(luaScriptContents + serverKey182));

			for(auto& luaScriptName : luaScriptNames)
			{
				string path{mPackPath + "/Scripts/" + luaScriptName};
				string contents{get181FileContents(path)};
				string hash{get181MD5Hash(contents + serverKey182)};
				string compressedHash{""};

				for(unsigned int i{0}; i < hash.length(); ++i) if(i % 3 == 0) compressedHash.append(toStr(hash[i]));

				result.append(compressedHash);
			}

			result.append(get181UrlEncoded(toStr(mDifficultyMultiplier)));
			return result;
		}
		void convertHashes(const string& mSourceJsonPath, const string& mTargetJsonPath)
		{
			string scores{getFileContents(mSourceJsonPath)};
			vector<string> oldValidators, newValidators;

			for(auto& levelData : getAllLevelData())
				for(float difficultyMult : levelData.getDifficultyMultipliers())
				{
					log("");
					log("");

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

			log("");
			log("");

			for(unsigned int i{0}; i < oldValidators.size(); ++i)
			{
				scores = replaceAll(scores, oldValidators[i], newValidators[i]);
				log("replacing");
				log(oldValidators[i]);
				log("with");
				log(newValidators[i]);
				log("");
			}

			ofstream o; o.open(mTargetJsonPath); o << scores; o.flush(); o.close();
		}
	}
}
