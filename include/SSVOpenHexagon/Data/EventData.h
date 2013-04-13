// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_EVENTDATA
#define HG_EVENTDATA

#include <string>
#include <SSVJsonCpp/SSVJsonCpp.h>

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

			void update(float mFrameTime);

			void setHexagonGamePtr(HexagonGame* mHgPtr);

			std::string getId();
			bool getFinished();
	};
}

#endif
