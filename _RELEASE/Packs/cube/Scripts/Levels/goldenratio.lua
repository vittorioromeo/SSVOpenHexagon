-- include useful files
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "utils.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "common.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "commonpatterns.lua")

-- this function adds a pattern to the timeline based on a key
function addPattern(mKey)
    if mKey == 0 then pBarrageSpiral(u_rndInt(5, 9), 0.41, 1)
    elseif mKey == 1 then pMirrorSpiralDouble(u_rndInt(8, 10), 0)
    elseif mKey == 2 then pMirrorSpiral(u_rndInt(2, 5), 0)
    elseif mKey == 3 then pSpiral(l_getSides() * u_rndInt(1, 3), 0)
    end
end

-- shuffle the keys, and then call them to add all the patterns
-- shuffling is better than randomizing - it guarantees all the patterns will be called
keys = { 0, 0, 1, 1, 2, 3 }
shuffle(keys)
index = 0
achievementUnlocked = false

-- onInit is an hardcoded function that is called when the level is first loaded
function onInit()
    l_setSpeedMult(1.7)
    l_setSpeedInc(0.1)
    l_setSpeedMax(3.1) -- I would do pi, but I think there will be some cases that may be impossible
    l_setRotationSpeed(0)
    l_setRotationSpeedMax(1)
    l_setRotationSpeedInc(0.1)
    l_setDelayMult(1.0)
    l_setDelayInc(0.0)
    l_setFastSpin(50.0)
    l_setSides(6)
    l_setSidesMin(5)
    l_setSidesMax(7)
    l_setIncTime(10)

    l_setPulseMin(60)
    -- l_setPulseMax(87)
    -- l_setPulseSpeed(1.2)
    -- l_setPulseSpeedR(1)
    -- l_setPulseDelayMax(12.9)

    l_setBeatPulseMax(17)
    l_setBeatPulseDelayMax(24.1)

    l_setWallSkewRight(-20)
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
end
