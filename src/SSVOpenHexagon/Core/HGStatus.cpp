#include "SSVOpenHexagon/Core/HGStatus.hpp"

namespace hg {

float HexagonGameStatus::getIncrementTimeSeconds()
{
    if(isTimePaused())
    {
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            pauseTp - incrementTp);
        return static_cast<float>(ms.count()) / 1000.f;
    }
    else
    {
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            lastTp - incrementTp);
        return static_cast<float>(ms.count()) / 1000.f;
    }
}

float HexagonGameStatus::getTimeSeconds()
{
    if(isTimePaused())
    {
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            pauseTp - startTp);
        return static_cast<float>(ms.count()) / 1000.f;
    }
    else
    {
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            lastTp - startTp);
        return static_cast<float>(ms.count()) / 1000.f;
    }
}

bool HexagonGameStatus::isTimePaused()
{
    return pauseDuration > (lastTp - pauseTp);
}

void HexagonGameStatus::pauseTime(float seconds)
{
    if(isTimePaused())
    {
        pauseDuration +=
            std::chrono::milliseconds(static_cast<int>(seconds * 1000.f));
    }
    else
    {
        pauseTp = lastTp;
        pauseDuration =
            std::chrono::milliseconds(static_cast<int>(seconds * 1000.f));
    }
}

void HexagonGameStatus::resetIncrementTime()
{
    if(isTimePaused())
    {
        incrementTp = pauseTp;
    }
    else
    {
        incrementTp = lastTp;
    }
}

void HexagonGameStatus::updateTime()
{
    const bool wasPaused = isTimePaused();

    lastTp = std::chrono::steady_clock::now();
    if(wasPaused && !isTimePaused())
    {
        startTp += pauseDuration;
        pauseTp = startTp;
        pauseDuration = 0ms;
    }
}
