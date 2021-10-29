-- include useful files
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "utils.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "common.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "commonpatterns.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "nextpatterns.lua")

extra = 0
level = 1
incrementTime = 10

-- this function adds a pattern to the timeline based on a key
function addPattern(mKey)
        if mKey == 0 then pTrapBarrage(u_rndInt(0, l_getSides()))
    elseif mKey == 1 then pTrapBarrageDouble(u_rndInt(0, l_getSides()))
    elseif mKey == 2 then pTrapBarrageInverse(u_rndInt(0, l_getSides()))
    elseif mKey == 3 then pTrapBarrageAlt(u_rndInt(0, l_getSides()))
    end
end

-- shuffle the keys, and then call them to add all the patterns
-- shuffling is better than randomizing - it guarantees all the patterns will be called
keys = { 0, 0, 1, 1, 2, 2, 3, 3 }
shuffle(keys)
index = 0
achievementUnlocked = false
hardAchievementUnlocked = false

-- onInit is an hardcoded function that is called when the level is first loaded
function onInit()
    l_setSpeedMult(2.0)
    l_setSpeedInc(0.0)
    l_setRotationSpeed(0.27)
    l_setRotationSpeedMax(0.4)
    l_setRotationSpeedInc(0.045)
    l_setDelayMult(1.1)
    l_setDelayInc(0.0)
    l_setFastSpin(71.0)
    l_setSides(4)
    l_setSidesMin(0)
    l_setSidesMax(0)
    l_setIncTime(10)

    l_setPulseMin(64)
    l_setPulseMax(84.99)
    l_setPulseSpeed(1.036)
    l_setPulseSpeedR(1.353)
    l_setPulseDelayMax(13.2)

    l_setBeatPulseMax(16)
    l_setBeatPulseDelayMax(24.489)

    l_addTracked("level", "level")
    l_addTracked("lowerBound", "min")
    l_addTracked("upperBound", "max")
    l_enableRndSideChanges(false)

    enableSwapIfDMGreaterThan(1.5)
end

-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()
    e_messageAddImportant("level: "..(extra + 1).." / time: "..incrementTime, 170)
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
    extra = extra + 1
    level = extra + 1
    incrementTime = incrementTime + 5
    l_setSides(l_getSides() + 1)
    l_setIncTime(incrementTime)
    e_messageAddImportant("level: "..(extra + 1).." / time: "..incrementTime, 170)
end

-- onUnload is an hardcoded function that is called when the level is closed/restarted
function onUnload()
end

-- continuous direction change (even if not on level increment)
dirChangeTime = 400
hueIMin = 0.0
hueIMax = 22.0
hueIStep = 0.0065

-- onUpdate is an hardcoded function that is called every frame
function onUpdate(mFrameTime)
    dirChangeTime = dirChangeTime - mFrameTime;
    if dirChangeTime < 0 then
        -- do not change direction while fast spinning
        if u_isFastSpinning() == false then
            l_setRotationSpeed(l_getRotationSpeed() * -1.0)
            dirChangeTime = 400
        end
    end

    if not achievementUnlocked and l_getLevelTime() > 60 and u_getDifficultyMult() >= 1 then
        steam_unlockAchievement("a17_polyhedrug")
        achievementUnlocked = true
    end

    if not hardAchievementUnlocked and l_getLevelTime() > 45 and u_getDifficultyMult() > 1.5 then
        steam_unlockAchievement("a43_polyhedrug_hard")
        hardAchievementUnlocked = true
    end

    s_setHueInc(s_getHueInc() + hueIStep)
    if(s_getHueInc() > hueIMax) then hueIStep = hueIStep * -1 end
    if(s_getHueInc() < hueIMin) then hueIStep = hueIStep * -1 end
end
