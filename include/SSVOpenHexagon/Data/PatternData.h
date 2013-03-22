#ifndef PATTERNDATA_H
#define PATTERNDATA_H

#include <vector>
#include <functional>
#include <string>
#include <map>
#include <jsoncpp/json.h>
#include <SSVStart.h>

using namespace std;

namespace hg
{
	class HexagonGame;

	class ScriptData
	{
		private:
			HexagonGame* hgPtr;
			Json::Value root;
			float currentTime{0};
			
		public:
			ScriptData(Json::Value mRoot);

			string getId();

			void setHexagonGamePtr(HexagonGame* mHgPtr);

			void update(float mFrameTime);
	};
}

#endif // PATTERNDATA_H
