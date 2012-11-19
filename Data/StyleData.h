#ifndef STYLEDATA_H_
#define STYLEDATA_H_

#include <SFML/Graphics.hpp>

using namespace sf;
using namespace std;

namespace hg
{
	class StyleData
	{
		private:
			string id;
			float hueMin;
			float hueMax;
			bool huePingPong;
			float hueIncrement;
			bool huePulse;
			bool mainDynamic;
			float mainDynamicDarkness;
			Color mainStatic;
			bool aDynamic;
			float aDynamicDarkness;
			Color aStatic;
			bool bDynamic;
			bool bDynamicOffset;
			float bDynamicDarkness;
			Color bStatic;

			float currentHue;
			float currentSwapTime;
			Color currentMain;
			Color currentA;
			Color currentB;

			float pulseFactor{1.00f};
			float pulseFactorMin{0.77f};
			float pulseFactorMax{1.10f};
			float pulseFactorIncrement{0.018f};

		public:
			StyleData() = default;
			StyleData(string mId, float mHueMin, float mHueMax, bool mHuePingPong, float mHueIncrement, bool mHuePulse, bool mMainDynamic,
						float mMainDynamicDarkness, Color mMainStatic, bool mADynamic, float mADynamicDarkness, Color mAStatic,
						bool mBDynamic, bool mBDynamicOffset, float mBDynamicDarkness, Color mBStatic);

			void update(float mFrameTime);

			string getId();
			float getHueMin();
			float getHueMax();
			bool getHuePingPong();
			float getHueIncrement();
			bool getHuePulse();
			bool getMainDynamic();
			float getMainDynamicDarkness();
			Color getMainStatic();
			bool getADynamic();
			float getADynamicDarkness();
			Color getAStatic();
			bool getBDynamic();
			bool getBDynamicOffset();
			float getBDynamicDarkness();
			Color getBStatic();

			float getCurrentHue();
			float getCurrentSwapTime();
			Color getCurrentMain();
			Color getCurrentA();
			Color getCurrentB();
	};
}

#endif // STYLEDATA_H_
