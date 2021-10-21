-- include useful files
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "utils.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "common.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "commonpatterns.lua")

-- this function adds a pattern to the timeline based on a key
function addPattern(mKey)
    local oldT = THICKNESS

    if u_getDifficultyMult() > 1.5 then
        THICKNESS = THICKNESS * 0.91
    elseif u_getDifficultyMult() < 1 then
        THICKNESS = THICKNESS * 1.98
    end

    if mKey == 0 then pMirrorSpiralDouble(u_rndInt(4, 6), 0)
    elseif mKey == 1 then pMirrorSpiral(u_rndInt(3, 4), 0)
    elseif mKey == 2 then
        local ot = THICKNESS
        local rd = getRandomDir()
        THICKNESS = THICKNESS * 0.85
        pSpiral(l_getSides(), 1, rd)
        pSpiral(l_getSides(), 1, rd * -1)
        THICKNESS = ot
    end

    THICKNESS = oldT
end

-- shuffle the keys, and then call them to add all the patterns
-- shuffling is better than randomizing - it guarantees all the patterns will be called
keys = { 0, 0, 1, 1, 2, 2 }
shuffle(keys)
index = 0
achievementUnlocked = false
hardAchievementUnlocked = false

-- onInit is an hardcoded function that is called when the level is first loaded
function onInit()
    if u_getDifficultyMult() > 1.5 then
        l_setSpeedMult(3.5)
    else
        l_setSpeedMult(2.5)
    end

    l_setSpeedInc(0.1)
    l_setSpeedMax(4.5)
    l_setRotationSpeed(0)
    l_setRotationSpeedMax(1)
    l_setRotationSpeedInc(0.1)
    l_setDelayMult(1.0)
    l_setDelayInc(0.0)
    l_setFastSpin(75.0)
    l_setSides(6)
    l_setSidesMin(6)
    l_setSidesMax(6)
    l_setIncTime(10)

    l_setPulseMin(66.82)
    l_setPulseMax(96.21)
    l_setPulseSpeed(1.631)
    l_setPulseSpeedR(3.146)
    l_setPulseDelayMax(71.95)

    l_setBeatPulseMax(17)
    l_setBeatPulseDelayMax(24.8275)

    l_setWallSkewRight(-15)
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
        steam_unlockAchievement("a9_ratio")
        achievementUnlocked = true
    end

    if not hardAchievementUnlocked and l_getLevelTime() > 45 and u_getDifficultyMult() > 1.5 then
        steam_unlockAchievement("a33_ratio_hard")
        hardAchievementUnlocked = true
    end
end
