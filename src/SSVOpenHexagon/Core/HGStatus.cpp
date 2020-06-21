#include "SSVOpenHexagon/Core/HGStatus.hpp"

using namespace hg;
typedef std::chrono::duration<float> duration;

float HexagonGameStatus::getIncrementTimeSeconds() {
    if(isTimePaused()) {
        return duration(pauseTms - incrementTms).count();
    } else {
        return duration(lastTickTms - incrementTms).count();
    }
}

float HexagonGameStatus::getTimeSeconds() {
    if(isTimePaused()) {
        return duration(pauseTms - startTms).count();
    } else {
        return duration(lastTickTms - startTms).count();
    }
}

bool HexagonGameStatus::isTimePaused() {
    return std::chrono::steady_clock::now() - pauseTms < pauseLength;
}

void HexagonGameStatus::pauseTime(float seconds) {
    if(!isTimePaused()) {
        pauseTms = lastTickTms;
    }
    pauseLength = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration<float>(seconds));
}

void HexagonGameStatus::resetIncrementTime() {
    incrementTms = lastTickTms;
}

void HexagonGameStatus::updateTime() {
    bool wasPaused = isTimePaused();

    lastTickTms = std::chrono::steady_clock::now();
    if(wasPaused && !isTimePaused()) {
        startTms += pauseLength;
        pauseTms = startTms;
    }
}