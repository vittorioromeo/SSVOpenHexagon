-- include useful files
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "utils.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "common.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "commonpatterns.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "nextpatterns.lua")

-- this function adds a pattern to the timeline based on a key
function addPattern(mKey)
        if mKey == 0 then pBarrage()
    elseif mKey == 1 then pRCBarrageDouble()
    elseif mKey == 2 then pBarrageSpiral(math.random(1, 2), 0.8)
    elseif mKey == 3 then
        if (l_getSides() > 8) then
            pAltBarrage(math.random(2, 3), 3, 0.75)
        else
            pAltBarrage(math.random(2, 3), 2, 0.75)
        end
    -- "Dynamic" Patterns
    elseif mKey == 4 then pRCAscendBarrageRandom(lowerBound, upperBound)
    elseif mKey == 5 then pRCAscendBarrage(getRandomSide(), lowerBound, upperBound)
    elseif mKey == 6 then pRCDynamicAltBarrage(2, math.random(3, 4), lowerBound, upperBound)
    end
end

-- shuffle the keys, and then call them to add all the patterns
-- shuffling is better than randomizing - it guarantees all the patterns will be called
keys = { 0, 0, 0, 1, 1, 2, 3 }
shuffle(keys)
index = 0
lowerBound = 4
upperBound = 6
achievementUnlocked = false
hardAchievementUnlocked = false

-- onInit is an hardcoded function that is called when the level is first loaded
function onInit()
    l_setSpeedMult(2.7)
    l_setSpeedInc(0.1)
    l_setSpeedMax(3.8) -- A lot of the difficulty is coming from the changing sides. Speed isn't too important here.
    l_setRotationSpeed(0.27)
    l_setRotationSpeedMax(0.5)
    l_setRotationSpeedInc(0.045)
    l_setDelayMult(1.1)
    l_setDelayInc(0)
    l_setFastSpin(71.0)
    l_setSides(6)
    l_setSidesMin(0)
    l_setSidesMax(0)
    l_setIncTime(15)

    l_setPulseMin(64)
    l_setPulseMax(84)
    l_setPulseSpeed(1.05)
    l_setPulseSpeedR(1.34)
    l_setPulseDelayMax(1.74)

    l_setBeatPulseMax(15)
    l_setBeatPulseDelayMax(21.428)

    l_addTracked("lowerBound", "min")
    l_addTracked("upperBound", "max")
    l_enableRndSideChanges(false)

    enableSwapIfDMGreaterThan(1.5)
    l_setSwapCooldownMult(1 / u_getDifficultyMult())
end

-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()
    if (u_getDifficultyMult() >= 1) then
        keys[#keys + 1] = 4
        keys[#keys + 1] = 4
        if (u_getDifficultyMult() > 1) then
            e_messageAdd("Difficulty > 1\n\"Dynamic\" patterns enabled!", 120)
            keys[#keys + 1] = 5
            keys[#keys + 1] = 6
        end
        shuffle(keys)
    end
    e_messageAddImportant("Sides: "..lowerBound.." - "..upperBound, 120)
end

-- onStep is an hardcoded function that is called when the level timeline is empty
-- onStep should contain your pattern spawning logic
function onStep()
    l_setSides(u_rndInt(lowerBound, upperBound))
    addPattern(keys[index])
    index = index + 1

    if index - 1 == #keys then
        index = 1
        shuffle(keys)
    end
end

-- onIncrement is an hardcoded function that is called when the level difficulty is incremented
function onIncrement()
    lowerBound = math.floor(u_rndInt(4, 6))
    upperBound = math.floor(lowerBound + u_rndInt(1, 4))
    e_messageAddImportant("Sides: "..lowerBound.." - "..upperBound, 120)
    t_clear()
end

-- onUnload is an hardcoded function that is called when the level is closed/restarted
function onUnload()
end

-- continuous direction change (even if not on level increment)
dirChangeTime = 400
hueIMin = 0.0
hueIMax = 15.0
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

    if not achievementUnlocked and l_getLevelTime() > 90 and u_getDifficultyMult() >= 1 then
        steam_unlockAchievement("a15_incongruence")
        achievementUnlocked = true
    end

    if not hardAchievementUnlocked and l_getLevelTime() > 45 and u_getDifficultyMult() > 1.5 then
        steam_unlockAchievement("a41_incongruence_hard")
        hardAchievementUnlocked = true
    end

    s_setHueInc(s_getHueInc() + hueIStep)
    if(s_getHueInc() > hueIMax) then hueIStep = hueIStep * -1 end
    if(s_getHueInc() < hueIMin) then hueIStep = hueIStep * -1 end
end
