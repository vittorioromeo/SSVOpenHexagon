// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Data/ProfileData.hpp"

#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/Global/Version.hpp"

#include "SSVOpenHexagon/Core/HexagonGame.hpp"

#include "TestUtils.hpp"

#include <array>
#include <optional>
#include <random>
#include <stdexcept>

int main()
try
{
    constexpr std::array packs{
        "ohvrvanilla_vittorio_romeo_cube_1",         //
        "ohvrvanilla_vittorio_romeo_experimental_1", //
        "ohvrvanilla_vittorio_romeo_orthoplex_1",    //
        "ohvrvanilla_vittorio_romeo_orthoplex_1"     //
    };

    constexpr std::array levels{
        "ohvrvanilla_vittorio_romeo_cube_1_apeirogon",         //
        "ohvrvanilla_vittorio_romeo_experimental_1_autotest0", //
        "ohvrvanilla_vittorio_romeo_orthoplex_1_arcadia",      //
        "ohvrvanilla_vittorio_romeo_orthoplex_1_bipolarity"    //
    };

    hg::Config::loadConfig({});

    hg::HGAssets assets{nullptr /* steamManager */, true /* headless */};

    hg::ProfileData fakeProfile{hg::GAME_VERSION, "testProfile", {}, {}};
    assets.addLocalProfile(std::move(fakeProfile));
    assets.pSetCurrent("testProfile");

    const auto doTest = [&](int i, bool differentHG, ssvs::GameWindow* gw)
    {
        hg::HexagonGame hg{
            nullptr /* steamManager */,   //
            nullptr /* discordManager */, //
            assets,                       //
            nullptr /* audio */,          //
            gw,                           //
            nullptr /* client */          //
        };

        if(getRndBool())
        {
            hg.executeRandomInputs = true;
        }
        else
        {
            hg.alwaysSpinRight = true;
        }

        std::optional<hg::replay_file> rf;

        hg.onDeathReplayCreated = [&](const hg::replay_file& newRf)
        { rf.emplace(newRf); };

        hg.newGame(packs[i % packs.size()], levels[i % levels.size()],
            true /* firstPlay */, 1.f /* diffMult */,
            /* mExecuteLastReplay */ false);

        hg.setMustStart(true);
        const double score =
            hg.executeGameUntilDeath(
                  1 /* maxProcessingSeconds */, 1.f /* timescale */)
                .value()
                .playedTimeSeconds;

        TEST_ASSERT(rf.has_value());

        std::optional<hg::HexagonGame::GameExecutionResult> score2;
        if(differentHG)
        {
            hg::HexagonGame hg2{
                nullptr /* steamManager */,   //
                nullptr /* discordManager */, //
                assets,                       //
                nullptr /* audio */,          //
                nullptr /* window */,         //
                nullptr /* client */          //
            };

            score2 = hg2.runReplayUntilDeathAndGetScore(
                rf.value(), 1 /* maxProcessingSeconds */, 1.f /* timescale */);
        }
        else
        {
            score2 = hg.runReplayUntilDeathAndGetScore(
                rf.value(), 1 /* maxProcessingSeconds */, 1.f /* timescale */);
        }

        TEST_ASSERT(score2.has_value());
        const double replayPlayedTimeSeconds = score2.value().playedTimeSeconds;

        // std::cerr << score << " == " << replayPlayedTimeSeconds << std::endl;

        TEST_ASSERT_EQ(score, replayPlayedTimeSeconds);
    };

    for(int i = 0; i < 100; ++i)
    {
        doTest(i, false, nullptr);
        doTest(i, true, nullptr);
    }

    return 0;
}
catch(const std::runtime_error& e)
{
    std::cerr << "EXCEPTION: " << e.what() << std::endl;
}
catch(...)
{
    std::cerr << "EXCEPTION: unknown" << std::endl;
}
