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
#include "SSVOpenHexagon/Online/Online.h"

namespace hg
{
	class HGAssets
	{
		private:
			bool playingLocally{true};

			bool levelsOnly{false};

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
			HGAssets(bool mLevelsOnly = false);

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
			void loadLocalProfiles();

			void saveCurrentLocalProfile();

			MusicData getMusicData(const std::string& mId);
			StyleData getStyleData(const std::string& mId);


			float getLocalScore(const std::string& mId);
			void setLocalScore(const std::string& mId, float mScore);
			void setCurrentLocalProfile(const std::string& mName);
			ProfileData& getCurrentLocalProfile();
			const ProfileData& getCurrentLocalProfile() const;
			std::string getCurrentLocalProfileFilePath();
			void createLocalProfile(const std::string& mName);
			int getLocalProfilesSize();
			std::vector<std::string> getLocalProfileNames();
			std::string getFirstLocalProfileName();

			inline std::string pGetName() const
			{
				if(!playingLocally) { if(!Online::isLoggedIn()) throw; return Online::getCurrentUsername(); }
				return getCurrentLocalProfile().getName();
			}
			inline const std::vector<std::string>& pGetTrackedNames() const
			{
				static std::vector<std::string> temp{};
				if(!playingLocally) return temp;
				return getCurrentLocalProfile().getTrackedNames();
			}
			inline void pClearTrackedNames()
			{
				if(!playingLocally) return;
				getCurrentLocalProfile().clearTrackedNames();
			}
			inline void pAddTrackedName(const std::string& mName)
			{
				if(!playingLocally) return;
				getCurrentLocalProfile().addTrackedName(mName);
			}
			inline void pSaveCurrent()
			{
				if(!playingLocally) return;
				saveCurrentLocalProfile();
			}
			inline void pSetCurrent(const std::string& mName)
			{
				if(!playingLocally) throw;
				setCurrentLocalProfile(mName);
			}
			inline void pCreate(const std::string& mName)
			{
				if(!playingLocally) throw;
				createLocalProfile(mName);
			}
			inline bool pIsPlayingLocally() const { return playingLocally; }
			inline void pSetPlayingLocally(bool mPlayingLocally) { playingLocally = mPlayingLocally; }



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
