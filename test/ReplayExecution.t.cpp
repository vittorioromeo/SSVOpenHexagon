// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/Core/HexagonGame.hpp"

#include "TestUtils.hpp"

#include <random>
#include <stdexcept>
#include <array>

int main()
try
{
    constexpr std::array levels{                             //
        "ohvrvanilla_vittorio_romeo_cube_1_pointless",       //
        "ohvrvanilla_vittorio_romeo_cube_1_seconddimension", //
        "ohvrvanilla_vittorio_romeo_cube_1_apeirogon"};

    hg::Config::loadConfig({});

    auto assets = std::make_unique<hg::HGAssets>(
        nullptr /* steamManager */, true /* headless */);

    for(int i = 0; i < 100; ++i)
    {
        auto hg = std::make_unique<hg::HexagonGame>(nullptr /* steamManager */,
            nullptr /* discordManager */, *assets, nullptr /* window */,
            nullptr /* client */);

        double score;
        double score2;

        hg->onReplayCreated = [&](const hg::replay_file& newRf) {
            auto hg2 = std::make_unique<hg::HexagonGame>(
                nullptr /* steamManager */, nullptr /* discordManager */,
                *assets, nullptr /* window */, nullptr /* client */);

            score2 = hg2->runReplayUntilDeathAndGetScore(newRf);
        };

        hg->newGame("ohvrvanilla_vittorio_romeo_cube_1",
            levels[i % levels.size()], true /* firstPlay */, 1.f /* diffMult */,
            /* mExecuteLastReplay */ false);

        hg->setMustStart(true);
        score = hg->executeGameUntilDeath();

        std::cerr << score << " == " << score2 << std::endl;
        TEST_ASSERT_EQ(score, score2);
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
