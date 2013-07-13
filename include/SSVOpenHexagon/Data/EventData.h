// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_EVENTDATA
#define HG_EVENTDATA

#include <string>
#include <SSVUtilsJson/SSVUtilsJson.h>

namespace hg
{
	class HexagonGame;

	class EventData
	{
		private:
			ssvuj::Value root;
			HexagonGame* hgPtr{nullptr};
			float currentTime{0.0f};
			bool finished{false};

		public:
			EventData(const ssvuj::Value& mRoot);

			void update(float mFrameTime);

			inline void setHexagonGamePtr(HexagonGame* mHgPtr) { hgPtr = mHgPtr; }

			inline std::string getId() const { return ssvuj::as<std::string>(root, "id"); }
			inline bool getFinished() const { return finished; }
	};
}

#endif
