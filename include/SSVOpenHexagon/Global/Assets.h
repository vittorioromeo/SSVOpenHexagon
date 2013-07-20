// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_ASSETS
#define HG_ASSETS

#include <unordered_map>
#include <map>
#include <string>
#include <memory>
#include <SSVStart/SSVStart.h>
#include <SFML/Audio.hpp>
#include "SSVOpenHexagon/Data/LevelData.h"
#include "SSVOpenHexagon/Data/MusicData.h"
#include "SSVOpenHexagon/Data/PackData.h"
#include "SSVOpenHexagon/Data/ProfileData.h"
#include "SSVOpenHexagon/Data/StyleData.h"
#include "SSVOpenHexagon/Global/Typedefs.h"

namespace hg
{
	class HGAssets
	{
		private:
			ssvs::AssetManager assetManager;
			ssvs::SoundPlayer soundPlayer;
			ssvs::MusicPlayer musicPlayer;

			std::unordered_map<std::string, Uptr<LevelData>> levelDatas;
			std::unordered_map<std::string, std::vector<std::string>> levelDataIdsByPack;

			std::unordered_map<std::string, Uptr<PackData>> packDatas;
			std::vector<std::string> packIds, packPaths;

			std::map<std::string, MusicData> musicDataMap;
			std::map<std::string, StyleData> styleDataMap;
			std::map<std::string, ProfileData> profileDataMap;
			ProfileData* currentProfilePtr{nullptr};

		public:
			HGAssets();

			inline ssvs::AssetManager& operator()() { return assetManager; }

			inline const std::unordered_map<std::string, Uptr<LevelData>>& getLevelDatas()			{ return levelDatas; }
			inline const LevelData& getLevelData(const std::string& mId)							{ return *levelDatas.at(mId); }
			inline const std::vector<std::string>& getLevelIdsByPack(const std::string& mPackPath)	{ return levelDataIdsByPack.at(mPackPath); }

			inline const PackData& getPackData(const std::string& mId)	{ return *packDatas.at(mId); }
			inline const std::vector<std::string>& getPackPaths()		{ return packPaths; }
			inline const std::vector<std::string>& getPackIds()			{ return packIds; }





			void loadAssets();

			void loadMusic(const std::string& mPath);
			void loadMusicData(const std::string& mPath);
			void loadStyleData(const std::string& mPath);
			void loadLevelData(const std::string& mPath);
			void loadCustomSounds(const std::string& mPackName, const std::string& mPath);
			void loadProfiles();

			void saveCurrentProfile();

			MusicData getMusicData(const std::string& mId);
			StyleData getStyleData(const std::string& mId);


			float getScore(const std::string& mId);
			void setScore(const std::string& mId, float mScore);

			void setCurrentProfile(const std::string& mName);
			ProfileData& getCurrentProfile();
			std::string getCurrentProfileFilePath();
			void createProfile(const std::string& mName);
			int getProfilesSize();
			std::vector<std::string> getProfileNames();
			std::string getFirstProfileName();







			void refreshVolumes();
			void stopMusics();
			void stopSounds();
			void playSound(const std::string& mId, ssvs::SoundPlayer::Mode mMode = ssvs::SoundPlayer::Mode::Override);
			void playMusic(const std::string& mId, sf::Time mPlayingOffset = sf::seconds(0));
			inline ssvs::SoundPlayer& getSoundPlayer() { return soundPlayer; }
			inline ssvs::MusicPlayer& getMusicPlayer() { return musicPlayer; }
	};



}

#endif
