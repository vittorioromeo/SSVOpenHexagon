// Copyright (c) 2013-2014 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef SSVS_OH_JSON_UTILSJSON
#define SSVS_OH_JSON_UTILSJSON

#include <future>
#include <SFML/System.hpp>
#include <SSVUtils/Core/Core.hpp>
#include "SSVStart/Tileset/Tileset.hpp"
#include "SSVStart/Animation/Animation.hpp"
#include "SSVStart/Input/Combo.hpp"
#include "SSVStart/Input/Trigger.hpp"
#include "SSVStart/BitmapText/BitmapFont.hpp"
#include "SSVStart/Utils/Input.hpp"
#include "SSVStart/Global/Typedefs.hpp"
#include "SSVStart/Assets/AssetManager.hpp"

namespace ssvs
{
	class AssetManager;

	inline auto getAnimationFromJson(const Tileset& mTileset, const ssvuj::Obj& mObj)
	{
		Animation::Type type{Animation::Type::Loop};

		std::string jsonType{ssvuj::getExtr<std::string>(mObj, "type", "")};
		if(jsonType == "once") type = Animation::Type::Once;
		else if(jsonType == "loop") type = Animation::Type::Loop;
		else if(jsonType == "pingpong") type = Animation::Type::PingPong;

		Animation result{type};

		for(const auto& f : ssvuj::getObj(mObj, "frames"))
		{
			const auto& index(mTileset.getIdx(ssvuj::getExtr<std::string>(f, 0)));
			result.addStep({index, ssvuj::getExtr<float>(f, 1)});
		}

		result.setSpeed(ssvuj::getExtr<float>(mObj, "speed", 1.f));
		return result;
	}
	inline void loadAssetsFromJson(AssetManager& mAssetManager, const Path& mRootPath, const ssvuj::Obj& mObj)
	{
		using namespace std;
		using namespace ssvuj;

		for(const auto& f : getExtr<vector<string>>(mObj, "fonts"))				mAssetManager.load<sf::Font>(f, mRootPath + f);
		for(const auto& f : getExtr<vector<string>>(mObj, "images"))			mAssetManager.load<sf::Image>(f, mRootPath + f);
		for(const auto& f : getExtr<vector<string>>(mObj, "textures"))			mAssetManager.load<sf::Texture>(f, mRootPath + f);
		for(const auto& f : getExtr<vector<string>>(mObj, "soundBuffers"))		mAssetManager.load<sf::SoundBuffer>(f, mRootPath + f);
		for(const auto& f : getExtr<vector<string>>(mObj, "musics"))			mAssetManager.load<sf::Music>(f, mRootPath + f);
		for(const auto& f : getExtr<vector<string>>(mObj, "shadersVertex"))		mAssetManager.load<sf::Shader>(f, mRootPath + f, sf::Shader::Type::Vertex, Internal::ShaderFromPath{});
		for(const auto& f : getExtr<vector<string>>(mObj, "shadersFragment"))	mAssetManager.load<sf::Shader>(f, mRootPath + f, sf::Shader::Type::Fragment, Internal::ShaderFromPath{});

		const auto& bfs(getObj(mObj, "bitmapFonts"));
			for(auto itr(begin(bfs)); itr != end(bfs); ++itr)
				mAssetManager.load<BitmapFont>(ssvuj::getKey(itr), mAssetManager.get<sf::Texture>(getExtr<string>(*itr, 0)), getExtr<BitmapFontData>(getFromFile(mRootPath + getExtr<string>(*itr, 1))));

		const auto& tilesets(getObj(mObj, "tilesets"));
			for(auto itr(begin(tilesets)); itr != end(tilesets); ++itr)
				mAssetManager.load<Tileset>(ssvuj::getKey(itr), ssvuj::getExtr<ssvs::Tileset>(ssvuj::getFromFile("Data/" + getExtr<string>(*itr))));
	}
}

namespace ssvuj
{
	template<typename T> SSVUJ_CNV_SIMPLE(ssvs::Vec2<T>, mObj, mV)	{ ssvuj::convertArray(mObj, mV.x, mV.y); }													SSVUJ_CNV_SIMPLE_END();
	template<> SSVUJ_CNV_SIMPLE(ssvs::BitmapFontData, mObj, mV)		{ ssvuj::convertArray(mObj, mV.cellColumns, mV.cellWidth, mV.cellHeight, mV.cellStart); }	SSVUJ_CNV_SIMPLE_END();
	template<> SSVUJ_CNV_SIMPLE(sf::Color, mObj, mV)				{ ssvuj::convertArray(mObj, mV.r, mV.g, mV.b, mV.a); }										SSVUJ_CNV_SIMPLE_END();
	template<> SSVUJ_CNV_SIMPLE(ssvs::Input::Trigger, mObj, mV)		{ ssvuj::convert(mObj, mV.getCombos()); }													SSVUJ_CNV_SIMPLE_END();

	template<> struct Converter<ssvs::KKey>
	{
		using T = ssvs::KKey;
		inline static void fromObj(const Obj& mObj, T& mValue)	{ mValue = ssvs::getKKey(getExtr<std::string>(mObj)); }
		inline static void toObj(Obj& mObj, const T& mValue)	{ arch(mObj, ssvs::getKKeyName(mValue)); }
	};
	template<> struct Converter<ssvs::MBtn>
	{
		using T = ssvs::MBtn;
		inline static void fromObj(const Obj& mObj, T& mValue)	{ mValue = ssvs::getMBtn(getExtr<std::string>(mObj)); }
		inline static void toObj(Obj& mObj, const T& mValue)	{ arch(mObj, ssvs::getMBtnName(mValue)); }
	};
	template<> struct Converter<ssvs::Input::Combo>
	{
		using T = ssvs::Input::Combo;
		inline static void fromObj(const Obj& mObj, T& mValue)
		{
			for(const auto& i : mObj)
			{
				if(ssvs::isKKeyNameValid(getExtr<std::string>(i))) mValue.addKey(getExtr<ssvs::KKey>(i));
				else if(ssvs::isMBtnNameValid(getExtr<std::string>(i))) mValue.addBtn(getExtr<ssvs::MBtn>(i));
				else ssvu::lo("ssvs::getInputComboFromJSON") << "<" << i << "> is not a valid input name" << std::endl;
			}
		}
		inline static void toObj(Obj& mObj, const T& mValue)
		{
			auto i(0u);
			const auto& keys(mValue.getKeys());
			const auto& btns(mValue.getBtns());
			for(auto j(0u); j < ssvs::kKeyCount; ++j) if(ssvs::getKeyBit(keys, ssvs::KKey(j))) arch(mObj, i++, ssvs::KKey(j));
			for(auto j(0u); j < ssvs::mBtnCount; ++j) if(ssvs::getBtnBit(btns, ssvs::MBtn(j))) arch(mObj, i++, ssvs::MBtn(j));
		}
	};

	template<> struct Converter<ssvs::Tileset>
	{
		using T = ssvs::Tileset;
		inline static void fromObj(const Obj& mObj, T& mValue)
		{
			const auto& labels(getObj(mObj, "labels"));
			for(auto iY(0u); iY < getObjSize(labels); ++iY)
				for(auto iX(0u); iX < getObjSize(labels[iY]); ++iX)
					mValue.setLabel(getExtr<std::string>(labels[iY][iX]), {iX, iY});

			mValue.setTileSize(getExtr<ssvs::Vec2u>(mObj, "tileSize"));
		}
		inline static void toObj(Obj& mObj, const T& mValue)
		{
			arch(mObj, "tileSize", mValue.getTileSize());

			auto& labels(getObj(mObj, "labels"));
			for(const auto& l : mValue.getLabels()) arch(getObj(labels, l.second.y), l.second.x, l.first);
		}
	};
}

#endif
