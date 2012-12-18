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

#ifndef ASSETS_H_
#define ASSETS_H_

#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <SFML/Audio.hpp>
#include <json/json.h>
#include <json/reader.h>
#include "Data/LevelData.h"
#include "Data/MusicData.h"
#include "Data/PackData.h"
#include "Data/ProfileData.h"
#include "Data/StyleData.h"
#include "Utils.h"
#include "HexagonGame.h"

namespace hg
{	
	void loadAssets();

	void loadFonts();
	void loadSounds();
	void loadMusic(std::string mPath);
	void loadMusicData(std::string mPath);
	void loadStyleData(std::string mPath);
	void loadLevelData(std::string mPath);
	void loadProfiles();
	void loadEvents(std::string mPath);

	void saveCurrentProfile();

	void stopAllMusic();
	void stopAllSounds();
	void playSound(std::string mId);

	sf::Font& getFont(std::string mId);
	sf::Sound* getSoundPtr(std::string mId);
	sf::Music* getMusicPtr(std::string mId);
	MusicData getMusicData(std::string mId);
	StyleData getStyleData(std::string mId);
	LevelData getLevelData(std::string mId);
	PackData getPackData(std::string mId);

	std::vector<LevelData> getAllLevelData();
	std::vector<std::string> getAllMenuLevelDataIds();
	std::vector<std::string> getMenuLevelDataIdsByPack(std::string mPackPath);
	std::vector<std::string> getPackPaths();

	float getScore(std::string mId);
	void setScore(std::string mId, float mScore);

	void setCurrentProfile(std::string mName);
	ProfileData& getCurrentProfile();
	std::string getCurrentProfileFilePath();
	void createProfile(std::string mName);
	int getProfilesSize();
	std::vector<std::string> getProfileNames();
	std::string getFirstProfileName();

	EventData* getEventData(std::string mId, HexagonGame* mHgPtr);
}

#endif /* ASSETS_H_ */
