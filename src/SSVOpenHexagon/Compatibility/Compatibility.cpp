#include <fstream>
#include <unordered_set>
#include <SSVUtils/SSVUtils.h>
#include <SSVUtilsJson/SSVUtilsJson.h>
#include <SSVStart/SSVStart.h>
#include "SSVOpenHexagon/Compatibility/Compatibility.h"
#include "SSVOpenHexagon/Utils/Utils.h"
#include "SSVOpenHexagon/Global/Assets.h"
#include "SSVOpenHexagon/Online/Online.h"
#include "SSVOpenHexagon/Online/Definitions.h"

using namespace std;
using namespace ssvs;
using namespace ssvs::Utils;
using namespace hg::Utils;
using namespace ssvu;
using namespace ssvu::Encryption;
using namespace ssvuj;

namespace hg
{
	namespace Compatibility
	{
		string get19MD5Hash(const string& mString) 	{ return encrypt<Type::MD5>(mString); }
		string get19UrlEncoded(const string& mString)	{ string result{""}; for(auto c : mString) if(isalnum(c)) result += c; return result; }
		string get19ControlStripped(const string& mString)	{ string result{""}; for(auto c : mString) if(!iscntrl(c)) result += c; return result; }
		string get19FileContents(const string& mPath)
		{
			FILE* fptr{fopen(mPath.c_str(), "rb")};
			fseek(fptr, 0, SEEK_END);
			size_t fsize(ftell(fptr));
			fseek(fptr, 0, SEEK_SET);
			string content; content.resize(fsize);
			if(fread(const_cast<char*>(content.c_str()), 1, fsize, fptr) != fsize) log("Error: " + mPath, "File loading");
			fclose(fptr); return content;
		}
		string get19Validator(const string& mPackPath, const string& mLevelId, const string& mLevelRootPath, const string& mStyleRootPath, const string& mLuaScriptPath, float mDifficultyMultiplier)
		{
			string luaScriptContents{get19FileContents(mLuaScriptPath)};
			unordered_set<string> luaScriptNames;
			recursiveFillIncludedLuaFileNames(luaScriptNames, mPackPath, luaScriptContents);

			string toEncrypt{""};
			toEncrypt.append(mLevelId);
			toEncrypt.append(toStr(mDifficultyMultiplier));
			toEncrypt.append(get19FileContents(mLevelRootPath));
			toEncrypt.append(get19FileContents(mStyleRootPath));
			toEncrypt.append(luaScriptContents);

			for(const auto& luaScriptName : luaScriptNames)
			{
				string path{mPackPath + "/Scripts/" + luaScriptName};
				string contents{get19FileContents(path)};
				toEncrypt.append(contents);
			}

			toEncrypt = get19ControlStripped(toEncrypt);

			string result{get19UrlEncoded(mLevelId) + get19MD5Hash(toEncrypt + HG_SKEY1 + HG_SKEY2 + HG_SKEY3)};
			return result;
		}
		void merge19Scores(const string& mSourceJsonPathA, const string& mSourceJsonPathB, const string& mTargetJsonPath)
		{
			string aStr{get19FileContents(mSourceJsonPathA)};
			string bStr{get19FileContents(mSourceJsonPathB)};
			Json::Value a{getRootFromString(aStr)};
			Json::Value b{getRootFromString(bStr)};

			Json::Value result{a};

			for(auto itr = b.begin(); itr != b.end(); ++itr)
			{
				string validator = itr.key().asString();
				log("Dealing with validator <" + validator + ">", "Merge");

				if(result.isMember(validator))
				{
					log("Result has alerady validator <" + validator + ">", "Merge");
					Json::Value& lvl = result[validator];

					for(auto nsPairItr = (*itr).begin(); nsPairItr != (*itr).end(); ++nsPairItr)
					{
						Json::Value& nsPair = (*nsPairItr);

						bool found{false};
						for(auto lvlPairItr = lvl.begin(); lvlPairItr != lvl.end(); ++lvlPairItr)
						{
							Json::Value& lvlPair = (*lvlPairItr);
							if(lvlPair["n"] == nsPair["n"] && nsPair["s"].asFloat() > lvlPair["s"].asFloat())
							{
								lvlPair["s"] = nsPair["s"].asFloat();
								found = true;
								break;
							}
						}

						if(!found) lvl.append(nsPair);
					}
				}
				else log("Result has not validator <" + validator + ">, creating", "Merge");
			}

			string resString{""};
			Json::FastWriter fw;
			resString = fw.write(result);
			ofstream o; o.open(mTargetJsonPath); o << resString; o.flush(); o.close();
		}
		void separate19Scores(const string& mSourceJsonPath, const string& mTargetJsonPath)
		{
			string scores{get19FileContents(mSourceJsonPath)};
			Json::Value oldRoot{getRootFromString(scores)};
			vector<string> oldValidators;
			vector<pair<string, float>> newValidators;

			for(auto& levelData : getAllLevelData())
				for(float difficultyMult : levelData.getDifficultyMultipliers())
				{
					log(""); log("");

					log("computing old validator for " + levelData.getId() + ", difficulty multiplier " + toStr(difficultyMult) +  "...");
					string oldValidator{get19Validator(levelData.getPackPath(), levelData.getId(), levelData.getLevelRootPath(), levelData.getStyleRootPath(), levelData.getLuaScriptPath(), difficultyMult)};
					log("\"" + oldValidator + "\"");
					oldValidators.push_back(oldValidator);
					log("");

					log("computing new validator for " + levelData.getId() + ", difficulty multiplier " + toStr(difficultyMult) +  "...");
					string newValidator{Online::getValidator(levelData.getPackPath(), levelData.getId(), levelData.getLevelRootPath(), levelData.getStyleRootPath(), levelData.getLuaScriptPath())};
					float newDifficulty{difficultyMult};
					log("\"" + newValidator + "\"" + "difficulty: " + toStr(difficultyMult));
					newValidators.push_back({newValidator, newDifficulty});
					log("");
				}

			log(""); log("");

			Json::Value result;

			for(unsigned int i{0}; i < oldValidators.size(); ++i)
			{
				string& newValidatorString(newValidators[i].first);
				float& newValidatorDifficulty(newValidators[i].second);

				//log("");
				//log("getting currentScores: " + toStr(oldValidators[i]));
				Json::Value oldScores = oldRoot[oldValidators[i]];
				//log(toStr(currentScores));
				//log("");


				for(unsigned int j{0}; j < oldScores.size(); ++j)
				{
					Json::Value oldScore = oldScores[j];

					//log("");
					//log("setting " + newValidatorString + " to object value");
					//result[newValidatorString] = Json::Value(Json::ValueType::objectValue);

					//log("");
					//log("setting " + newValidatorString + "[" + toStr(newValidatorDifficulty) + "][" + toStr(j) + "] to " + toStr(currentScore));
					//log("");
					//log(toStr(j));
					//log(toStr(oldScore));
					result[newValidatorString][toStr(newValidatorDifficulty)].append(oldScore);
				}
			}

			//log(toStr(result));
			log("DONE");


			Json::FastWriter jsonWriter;
			string out = jsonWriter.write(result);

			ofstream o; o.open(mTargetJsonPath);
			o << out;
			o.flush(); o.close();

			/*
			for(Json::ValueIterator itr = result.begin(); itr != result.end(); itr++)
			{
				Json::Value val = *itr;
				string validator = itr.key().asString();
				log("validator: " + toStr(validator));

				for(Json::ValueIterator itrD = val.begin(); itrD != val.end(); itrD++)
				{
					Json::Value valD = *itrD;
					float difficulty = ::atof(itrD.key().asString().c_str());
					log("-- difficulty: " + toStr(difficulty));

					for(Json::ValueIterator itrAV = valD.begin(); itrAV != valD.end(); itrAV++)
					{
						Json::Value valAV = *itrAV;
						string name = valAV["n"].asString();
						float score = valAV["s"].asFloat();

						if(score < 25.0f) continue;

						log("-- -- name: " + toStr(name));
						log("-- -- score: " + toStr(score));


						ssvs::Utils::waitFor(Online::startSendScore(toLower(name), validator, difficulty, score));
					}
				}
			}
			*/
		}
	}
}
