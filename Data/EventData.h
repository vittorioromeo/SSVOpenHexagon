#ifndef PATTERNDATA_H
#define PATTERNDATA_H

#include <vector>
#include <functional>
#include <string>
#include <map>
#include <json/json.h>
#include <SSVStart.h>

using namespace std;

namespace hg
{
	class HexagonGame;

	class EventData
	{
		private:
			Json::Value root;
			HexagonGame* hgPtr{nullptr};
			float currentTime{0.0f};
			bool finished{false};
			
		public:
			EventData(Json::Value mRoot);

			string getId();
			bool getFinished();

			void setHexagonGamePtr(HexagonGame* mHgPtr);

			void update(float mFrameTime);
	};
}

#endif // PATTERNDATA_H
