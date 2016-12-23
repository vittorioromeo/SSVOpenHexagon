// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"
#include "SSVOpenHexagon/Core/HexagonGame.hpp"

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
                int x{0 + getRndI(-levelStatus.cameraShake,
                              levelStatus.cameraShake)};
                int y{0 + getRndI(-levelStatus.cameraShake,
                              levelStatus.cameraShake)};
                backgroundCamera.setCenter(Vec2f(x, y));
                overlayCamera.setCenter(
                    Vec2f(x, y) +
                    Vec2f{Config::getWidth() / 2.f, Config::getHeight() / 2.f});
            }
            else
            {
                backgroundCamera.setCenter(ssvs::zeroVec2f);
                overlayCamera.setCenter(
                    Vec2f{Config::getWidth() / 2.f, Config::getHeight() / 2.f});
            }
        }

        if(!Config::getNoBackground())
        {
            backgroundCamera.apply();
            styleData.drawBackground(window, ssvs::zeroVec2f, getSides());
        }

        backgroundCamera.apply();

        wallQuads.clear();
        playerTris.clear();
        capTris.clear();

        for (auto& w : walls) w.draw();
        if(status.started) player.draw();

        if(Config::get3D())
        {
            const auto owqSz(wallQuads.size());
            const auto optSz(playerTris.size());
            wallQuads.reserve(owqSz * styleData._3dDepth);
            playerTris.reserve(optSz * styleData._3dDepth);

            float effect{
                styleData._3dSkew * Config::get3DMultiplier() * status.pulse3D};

            Vec2f skew{1.f, 1.f + effect};
            backgroundCamera.setSkew(skew);

            const auto radRot(
                ssvu::toRad(backgroundCamera.getRotation()) + (ssvu::pi / 2.f));
            const auto sinRot(std::sin(radRot));
            const auto cosRot(std::cos(radRot));

            for(auto v(0u); v < owqSz * styleData._3dDepth; ++v)
                wallQuads.emplace_back(wallQuads[v % owqSz]);
            for(auto v(0u); v < optSz * styleData._3dDepth; ++v)
                playerTris.emplace_back(playerTris[v % optSz]);

            int lastWQ(0);
            int lastPT(0);

            for(auto j(0); j < styleData._3dDepth; ++j)
            {
                auto i(styleData._3dDepth - j - 1);
                auto offset(styleData._3dSpacing *
                            (float(i + 1.f) * styleData._3dPerspectiveMult) *
                            (effect * 3.6f) * 1.4f);
                Vec2f newPos(offset * cosRot, offset * sinRot);

                status.overrideColor = getColorDarkened(
                    styleData.get3DOverrideColor(), styleData._3dDarkenMult);
                status.overrideColor.a /= styleData._3dAlphaMult;
                status.overrideColor.a -= i * styleData._3dAlphaFalloff;

                for(auto k(0u); k < owqSz; ++k)
                {
                    auto& x(wallQuads[lastWQ++]);
                    x.position += newPos;
                    x.color = status.overrideColor;
                }

                for(auto k(0u); k < optSz; ++k)
                {
                    auto& x(playerTris[lastPT++]);
                    x.position += newPos;
                    x.color = status.overrideColor;
                }
            }
        }

        render(wallQuads);
        render(playerTris);
        render(capTris);

        overlayCamera.apply();
        drawText();

        if(Config::getFlash()) render(flashPolygon);
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
            Vec2f{-100.f, -100.f}, Color{255, 255, 255, 0});
        flashPolygon.emplace_back(
            Vec2f{Config::getWidth() + 100.f, -100.f}, Color{255, 255, 255, 0});
        flashPolygon.emplace_back(
            Vec2f{Config::getWidth() + 100.f, Config::getHeight() + 100.f},
            Color{255, 255, 255, 0});
        flashPolygon.emplace_back(Vec2f{-100.f, Config::getHeight() + 100.f},
            Color{255, 255, 255, 0});
    }

    void HexagonGame::updateText()
    {
        os.str("");

        if(Config::getShowFPS()) os << "FPS: " << window.getFPS() << "\n";
        if(status.started)
            os << "time: " << toStr(status.currentTime).substr(0, 5) << "\n";

        if(levelStatus.tutorialMode)
            os << "tutorial mode\n";
        else if(Config::getOfficial())
            os << "official mode\n";

        if(Config::getDebug()) os << "debug mode\n";

        if(status.started)
        {
            if(levelStatus.swapEnabled) os << "swap enabled\n";
            if(Config::getInvincible()) os << "invincibility on\n";
            if(status.scoreInvalid)
                os << "score invalidated (performance issues)\n";
            if(status.hasDied) os << "press r to restart\n";

            const auto& trackedVariables(levelStatus.trackedVariables);
            if(Config::getShowTrackedVariables() && !trackedVariables.empty())
            {
                os << "\n";
                for(const auto& t : trackedVariables)
                {
                    if(!lua.doesVariableExist(t.variableName)) continue;
                    string var{lua.readVariable<string>(t.variableName)};
                    os << t.displayName << ": " << var << "\n";
                }
            }
        }
        else
        {
            os << "rotate to start\n";
            messageText.setString("rotate to start");
        }

        os.flush();

        text.setString(os.str());
        text.setCharacterSize(
            ssvu::toNum<unsigned int>(25.f / Config::getZoomFactor()));
        text.setOrigin(0, 0);

        messageText.setOrigin(getGlobalWidth(messageText) / 2.f, 0);
    }

    void HexagonGame::drawText()
    {
        Color offsetColor{getColor(1)};
        if(Config::getBlackAndWhite()) offsetColor = Color::Black;

        if(Config::getDrawTextOutlines())
        {
            text.setFillColor(offsetColor);
            for(const auto& o : txt_offsets)
            {
                text.setPosition(txt_pos + o);
                render(text);
            }
        }

        text.setFillColor(getColorMain());
        text.setPosition(txt_pos);
        render(text);



        if(messageText.getString() == "") return;

        if(Config::getDrawTextOutlines())
        {
            messageText.setFillColor(offsetColor);
            for(const auto& o : txt_offsets)
            {
                messageText.setPosition(
                    Vec2f{Config::getWidth() / 2.f, Config::getHeight() / 6.f} +
                    o);
                render(messageText);
            }
        }

        messageText.setPosition(
            Vec2f{Config::getWidth() / 2.f, Config::getHeight() / 6.f});
        messageText.setFillColor(getColorMain());
        render(messageText);
    }
}
