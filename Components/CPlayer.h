/* The MIT License (MIT)
 * Copyright (c) 2012 Vittorio Romeo
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef CPLAYER_H_
#define CPLAYER_H_

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SSVEntitySystem.h>
#include "Global/Config.h"
#include "HexagonGame.h"

using namespace sf;
using namespace sses;

namespace hg
{
	class CPlayer : public Component
	{
		private:
			HexagonGame* hgPtr;
			Vector2f pLeft, pRight, startPos, pos;
			VertexArray vertices{PrimitiveType::Triangles, 3};
			float hue{0};
			float size{getPlayerSize()};
			float angle{0};
			float speed{getPlayerSpeed()};
			float focusSpeed{getPlayerFocusSpeed()};
			bool isDead{false};

			inline void drawPivot();			

		public:		
			CPlayer(HexagonGame*, Vector2f);

			void update(float mFrameTime) override;
			void draw() override;
	};
}
#endif /* CPLAYER_H_ */
