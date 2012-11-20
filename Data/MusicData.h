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

#ifndef MUSICDATA_H_
#define MUSICDATA_H_

#include <SFML/Audio.hpp>
#include <string>
#include <vector>

using namespace std;
using namespace sf;

namespace hg
{
	class MusicData
	{
		private:
			vector<int> segments;
			string id 			{""};
			string fileName 	{""};
			string name			{""};
			string album		{""};
			string author		{""};

			Music* musicPtr		{new Music};
			bool firstPlay		{true};
			int getRandomSegment();

		public:
			MusicData() = default;
			MusicData(string mId, string mFileName, string mName, string mAlbum, string mAuthor);

			void addSegment(int mSeconds);			
			void playRandomSegment(Music*& mMusicPtr);
			void playSegment(Music*& mMusicPtr, int mSegmentIndex);
			void playSeconds(Music*& mMusicPtr, int mSeconds);

			string getId();
			string getFileName();
			string getName();
			string getAlbum();
			string getAuthor();

			void setFirstPlay(bool mFirstPlay);
			bool getFirstPlay();
	};
}

#endif // MUSICDATA_H_
