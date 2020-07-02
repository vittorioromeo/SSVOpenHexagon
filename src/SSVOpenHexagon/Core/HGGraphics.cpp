// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"
#include "SSVOpenHexagon/Utils/Color.hpp"
#include "SSVOpenHexagon/Core/HexagonGame.hpp"

#include <SSVStart/Utils/Vector2.hpp>

using namespace std;
using namespace sf;
using namespace ssvs;
using namespace hg::Utils;
using namespace ssvu;

namespace hg
{

void HexagonGame::draw()
{
    styleData.computeColors();

    window.clear(Color::Black);

    if(!status.hasDied)
    {
        if(levelStatus.cameraShake > 0)
        {
            const sf::Vector2f shake(
                getRndI(-levelStatus.cameraShake, levelStatus.cameraShake),
                getRndI(-levelStatus.cameraShake, levelStatus.cameraShake));

            backgroundCamera.setCenter(shake);
            overlayCamera.setCenter(
                shake + sf::Vector2f{Config::getWidth() / 2.f,
                            Config::getHeight() / 2.f});
        }
        else
        {
            backgroundCamera.setCenter(ssvs::zeroVec2f);
            overlayCamera.setCenter(sf::Vector2f{
                Config::getWidth() / 2.f, Config::getHeight() / 2.f});
        }
    }

    if(!Config::getNoBackground())
    {
        backgroundCamera.apply();
        styleData.drawBackground(window, ssvs::zeroVec2f, levelStatus);
    }

    backgroundCamera.apply();

    wallQuads3D.clear();
    playerTris3D.clear();
    wallQuads.clear();
    playerTris.clear();
    capTris.clear();

    for(CWall& w : walls)
    {
        w.draw(*this);
    }

    cwManager.draw(*this);

    if(status.started)
    {
        player.draw(*this, styleData.getCapColorResult());
    }

    if(Config::get3D())
    {
        const auto depth(styleData._3dDepth);
        const auto numWallQuads(wallQuads.size());
        const auto numPlayerTris(playerTris.size());

        wallQuads3D.reserve(numWallQuads * depth);
        playerTris3D.reserve(numPlayerTris * depth);

        const float effect{
            styleData._3dSkew * Config::get3DMultiplier() * status.pulse3D};

        const sf::Vector2f skew{1.f, 1.f + effect};
        backgroundCamera.setSkew(skew);

        const auto radRot(
            ssvu::toRad(backgroundCamera.getRotation()) + (ssvu::pi / 2.f));
        const auto sinRot(std::sin(radRot));
        const auto cosRot(std::cos(radRot));

        for(std::size_t i = 0; i < depth; ++i)
        {
            wallQuads3D.unsafe_emplace_other(wallQuads);
        }

        for(std::size_t i = 0; i < depth; ++i)
        {
            playerTris3D.unsafe_emplace_other(playerTris);
        }

        for(auto j(0); j < depth; ++j)
        {
            const float i(depth - j - 1);

            const float offset(styleData._3dSpacing *
                               (float(i + 1.f) * styleData._3dPerspectiveMult) *
                               (effect * 3.6f) * 1.4f);

            sf::Vector2f newPos(offset * cosRot, offset * sinRot);

            status.overrideColor = getColorDarkened(
                styleData.get3DOverrideColor(), styleData._3dDarkenMult);
            status.overrideColor.a /= styleData._3dAlphaMult;
            status.overrideColor.a -= i * styleData._3dAlphaFalloff;

            for(std::size_t k = j * numWallQuads; k < (j + 1) * numWallQuads;
                ++k)
            {
                wallQuads3D[k].position += newPos;
                wallQuads3D[k].color = status.overrideColor;
            }

            for(std::size_t k = j * numPlayerTris; k < (j + 1) * numPlayerTris;
                ++k)
            {
                playerTris3D[k].position += newPos;
                playerTris3D[k].color = status.overrideColor;
            }
        }
    }

    render(wallQuads3D);
    render(playerTris3D);
    render(wallQuads);
    render(playerTris);
    render(capTris);

    overlayCamera.apply();
    drawText();

    if(Config::getFlash())
    {
        render(flashPolygon);
    }

    if(mustTakeScreenshot)
    {
        window.saveScreenshot("screenshot.png");
        mustTakeScreenshot = false;
    }
}

void HexagonGame::initFlashEffect()
{
    flashPolygon.clear();
    flashPolygon.emplace_back(
        sf::Vector2f{-100.f, -100.f}, Color{255, 255, 255, 0});
    flashPolygon.emplace_back(sf::Vector2f{Config::getWidth() + 100.f, -100.f},
        Color{255, 255, 255, 0});
    flashPolygon.emplace_back(
        sf::Vector2f{Config::getWidth() + 100.f, Config::getHeight() + 100.f},
        Color{255, 255, 255, 0});
    flashPolygon.emplace_back(sf::Vector2f{-100.f, Config::getHeight() + 100.f},
        Color{255, 255, 255, 0});
}

void HexagonGame::updateText()
{
    os.str("");

    if(levelStatus.tutorialMode)
    {
        os << "TUTORIAL MODE\n";
    }
    else if(Config::getOfficial())
    {
        os << "OFFICIAL MODE\n";
    }

    if(Config::getDebug())
    {
        os << "DEBUG MODE\n";
    }

    if(status.started)
    {
        if(levelStatus.swapEnabled)
        {
            os << "SWAP ENABLED\n";
        }

        if(Config::getInvincible())
        {
            os << "INVINCIBILITY ON\n";
        }

        if(status.scoreInvalid)
        {
            os << "SCORE INVALIDATED (PERFORMANCE ISSUES)\n";
        }

        if(status.hasDied)
        {
            os << "PRESS R TO RESTART\n";
        }

        const auto& trackedVariables(levelStatus.trackedVariables);
        if(Config::getShowTrackedVariables() && !trackedVariables.empty())
        {
            os << "\n";
            for(const auto& t : trackedVariables)
            {
                if(!lua.doesVariableExist(t.variableName))
                {
                    continue;
                }
                string name{t.displayName};
                string var{lua.readVariable<string>(t.variableName)};
                Utils::uppercasify(name);
                Utils::uppercasify(var);
                os << name << ": " << var << "\n";
            }
        }
    }
    else
    {
        os << "ROTATE TO START\n";
        messageText.setString("ROTATE TO START");
    }

    os.flush();

    // Set in game timer text
    if (status.started)
	{
		timeText.setString(toStr(std::floor(status.getTimeSeconds() * 1000) / 1000.f));
	} else {
		timeText.setString("0");
	}
    timeText.setCharacterSize(
		ssvu::toNum<unsigned int>(70.f / Config::getZoomFactor()));
	timeText.setOrigin(-16, ssvu::toNum<int>(17.f / Config::getZoomFactor()));

    // Set information text
    text.setString(os.str());
    text.setCharacterSize(
        ssvu::toNum<unsigned int>(25.f / Config::getZoomFactor()));
    text.setOrigin(-8, 8);

    // Set FPS Text, if option is enabled.
    if(Config::getShowFPS())
    {
		fpsText.setString(toStr(window.getFPS()));
		fpsText.setCharacterSize(
			ssvu::toNum<unsigned int>(25.f / Config::getZoomFactor()));
		fpsText.setOrigin(-8, -ssvu::toNum<int>(735.f / Config::getZoomFactor()) + 8);
	}

    messageText.setCharacterSize(
        ssvu::toNum<unsigned int>(38.f / Config::getZoomFactor()));
    messageText.setOrigin(getGlobalWidth(messageText) / 2.f, 0);
}

void HexagonGame::drawText()
{
    Color offsetColor{getColor(0)};
    if(Config::getBlackAndWhite())
    {
        offsetColor = Color::Black;
    }

    if(Config::getDrawTextOutlines())
    {
		timeText.setOutlineColor(offsetColor);
        text.setOutlineColor(offsetColor);
		fpsText.setOutlineColor(offsetColor);
        
		timeText.setOutlineThickness(2.f);
		text.setOutlineThickness(1.f);
		fpsText.setOutlineThickness(1.f);
    } else {
		timeText.setOutlineThickness(0.f);
		text.setOutlineThickness(0.f);
		fpsText.setOutlineThickness(0.f);
	}

    timeText.setFillColor(getColorMain());
	text.setPosition(tl_txt_pos);
	render(timeText);

    text.setFillColor(getColorMain());
    text.setPosition(tl_txt_pos);
    render(text);

    if(Config::getShowFPS()) 
	{
		fpsText.setFillColor(getColorMain());
		fpsText.setPosition(tl_txt_pos);
		render(fpsText);
	}

    if(messageText.getString() == "")
    {
        return;
    }

    if(Config::getDrawTextOutlines())
    {
        messageText.setOutlineColor(offsetColor);
        messageText.setOutlineThickness(1.f);
    } else {
        messageText.setOutlineThickness(0.f);
    }

    messageText.setPosition(
        sf::Vector2f{Config::getWidth() / 2.f, Config::getHeight() / 6.f});
    messageText.setFillColor(getColorMain());
    render(messageText);
}

} // namespace hg
