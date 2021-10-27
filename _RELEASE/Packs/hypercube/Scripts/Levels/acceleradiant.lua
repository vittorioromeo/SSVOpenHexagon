-- include useful files
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "utils.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "common.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "commonpatterns.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "nextpatterns.lua")

-- this function adds a pattern to the timeline based on a key
function addPattern(mKey)
        if mKey == 0 then pACBarrageDecelerate()
    elseif mKey == 1 then pACBarrageMulti()
    elseif mKey == 2 then pACSpiral()
    elseif mKey == 3 and u_getDifficultyMult() >= 1 then pACBarrageDeception()
    elseif mKey == 4 then pACAltBarrage(math.random(2, 3))
    elseif mKey == 5 then pACAltBarrageMulti()
    elseif mKey == 6 then pACAltBarrageReveal(math.random(3, 4))
    elseif mKey == 7 then pACInverseBarrage();
    elseif mKey == 8 then pACTunnelReveal(math.random(2, 4));
    end
end

-- shuffle the keys, and then call them to add all the patterns
-- shuffling is better than randomizing - it guarantees all the patterns will be called
keys = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 3, 3, 4, 5, 6, 7, 7, 8}
shuffle(keys)
index = 0
achievementUnlocked = false
hardAchievementUnlocked = false

-- onInit is an hardcoded function that is called when the level is first loaded
function onInit()
    l_setSpeedMult(2.2)
    l_setSpeedInc(0.1)
    l_setSpeedMax(4.5);
    l_setRotationSpeed(0.27)
    l_setRotationSpeedMax(0.45)
    l_setRotationSpeedInc(0.045)
    l_setDelayMult(1.1)
    l_setFastSpin(71.0)
    l_setSides(6)
    l_setSidesMin(5)
    l_setSidesMax(7)
    l_setIncTime(15)

    l_setPulseMin(64)
    l_setPulseMax(84)
    l_setPulseSpeed(1.05)
    l_setPulseSpeedR(1.34)
    l_setPulseDelayMax(7)

    l_setBeatPulseMax(15)
    l_setBeatPulseDelayMax(21.8)

    enableSwapIfDMGreaterThan(1.4)
    l_setSwapCooldownMult(1 / u_getDifficultyMult());
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

-- continuous direction change (even if not on level increment)
dirChangeTime = 400
hueIMin = 0.0
hueIMax = 22.0
hueIStep = 6.5

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
        steam_unlockAchievement("a13_acceleradiant")
        achievementUnlocked = true
    end

    if not hardAchievementUnlocked and l_getLevelTime() > 30 and u_getDifficultyMult() > 2.2 then
        steam_unlockAchievement("a39_acceleradiant_hard")
        hardAchievementUnlocked = true
    end

    s_setHueInc(s_getHueInc() + hueIStep * mFrameTime/FPS)
    if(s_getHueInc() > hueIMax) then hueIStep = hueIStep * -1 end
    if(s_getHueInc() < hueIMin) then hueIStep = hueIStep * -1 end
end
