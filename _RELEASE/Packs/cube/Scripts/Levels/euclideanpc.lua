-- include useful files
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "utils.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "common.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "commonpatterns.lua")

-- this function adds a pattern to the timeline based on a key
function addPattern(mKey)
        if mKey ==  0 then pAltBarrage(u_rndInt(3, 5), 2)
    elseif mKey ==  1 then pMirrorSpiral(u_rndInt(3, 6), 0)
    elseif mKey ==  2 then pBarrageSpiral(u_rndInt(0, 3), 1, 1)
    elseif mKey ==  3 then pBarrageSpiral(u_rndInt(0, 2), 1.2, 2)
    elseif mKey ==  4 then pBarrageSpiral(2, 0.7, 1)
    elseif mKey ==  5 then pInverseBarrage(0)
    elseif mKey ==  6 then pTunnel(u_rndInt(1, 3))
    elseif mKey ==  7 then pMirrorWallStrip(1, 0)
    elseif mKey ==  8 then pWallExVortex(0, 1, 1)
    elseif mKey ==  9 then pDMBarrageSpiral(u_rndInt(2, 4), 0.25 * (u_getDifficultyMult() ^ 0.56), 1)
    elseif mKey == 10 then pRandomBarrage(u_rndInt(2, 4), 2.25)
    elseif mKey == 11 then pMirrorSpiralDouble(u_rndInt(4, 8), 0)
    end
end

-- shuffle the keys, and then call them to add all the patterns
-- shuffling is better than randomizing - it guarantees all the patterns will be called
keys = { 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 7, 7, 8, 9, 10, 10, 10, 11, 11 }
shuffle(keys)
index = 0
achievementUnlocked = false
hardAchievementUnlocked = false

-- onInit is an hardcoded function that is called when the level is first loaded
function onInit()
    l_setSpeedMult(2.7)
    l_setSpeedInc(0.02)
    l_setSpeedMax(3.5)
    l_setRotationSpeed(0.25)
    l_setRotationSpeedMax(1)

    if u_getDifficultyMult() > 1.5 then
        l_setRotationSpeedInc(0.1)
    else
        l_setRotationSpeedInc(0.04)
    end

    l_setDelayMult(1.0)
    l_setDelayInc(0.0)
    l_setFastSpin(85.0)
    l_setSides(8)
    l_setSidesMin(8)
    l_setSidesMax(8)
    l_setIncTime(15)

    l_setPulseMin(60)
    l_setPulseMax(88.2)
    l_setPulseSpeed(1.269)
    l_setPulseSpeedR(0.838)
    l_setPulseDelayMax(12.9)

    l_setBeatPulseMax(17)
    l_setBeatPulseDelayMax(22.92)

    enableSwapIfDMGreaterThan(1.5)
    disableSpeedIncIfDMGreaterThan(1.5)
end

-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()
end

-- onStep is an hardcoded function that is called when the level timeline is empty
-- onStep should contain your pattern spawning logic
function onStep()
    addPattern(keys[index])
    index = index + 1

    if index - 1 == #keys then
        index = 1
        shuffle(keys)
    end
end

-- onIncrement is an hardcoded function that is called when the level difficulty is incremented
function onIncrement()
end

-- onUnload is an hardcoded function that is called when the level is closed/restarted
function onUnload()
end

-- onUpdate is an hardcoded function that is called every frame
function onUpdate(mFrameTime)
    if not achievementUnlocked and l_getLevelTime() > 60 and u_getDifficultyMult() >= 1 then
        steam_unlockAchievement("a6_euclidian")
        achievementUnlocked = true
    end

    if not hardAchievementUnlocked and l_getLevelTime() > 30 and u_getDifficultyMult() > 1.5 then
        steam_unlockAchievement("a30_euclidian_hard")
        hardAchievementUnlocked = true
    end
end
