#ifndef HG_EVENTDATA
#define HG_EVENTDATA

#include <string>
#include <json/json.h>

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

			std::string getId();
			bool getFinished();

			void setHexagonGamePtr(HexagonGame* mHgPtr);

			void update(float mFrameTime);
	};
}

#endif
