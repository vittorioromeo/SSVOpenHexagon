-- include useful files
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "utils.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "common.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "commonpatterns.lua")

-- this function adds a pattern to the timeline based on a key
function addPattern(mKey)
        if mKey == 0 then cWallEx(u_rndInt(0, l_getSides()), u_rndInt(1, 2)) t_wait(getPerfectDelay(THICKNESS) * 2.5)
    elseif mKey == 1 then pMirrorSpiralDouble(u_rndInt(1, 2), 4)
    elseif mKey == 2 then rWallEx(u_rndInt(0, l_getSides()), u_rndInt(0, 1)) t_wait(getPerfectDelay(THICKNESS) * 2.8)
    elseif mKey == 3 then pMirrorWallStrip(1, 2)
    elseif mKey == 4 then rWallEx(u_rndInt(0, l_getSides()), 1) t_wait(getPerfectDelay(THICKNESS) * 2.3)
    elseif mKey == 5 then cWallEx(u_rndInt(0, l_getSides()), 5) t_wait(getPerfectDelay(THICKNESS) * 2.7)
    end
end

-- shuffle the keys, and then call them to add all the patterns
-- shuffling is better than randomizing - it guarantees all the patterns will be called
keys = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 4, 4, 5, 5, 5, 5 }
shuffle(keys)
index = 0
achievementUnlocked = false
hardAchievementUnlocked = false

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

-- onInit is an hardcoded function that is called when the level is first loaded
function onInit()
    l_setSpeedMult(3.4)
    l_setSpeedInc(0.10)
    l_setSpeedMax(4.3)
    l_setRotationSpeed(0.25)
    l_setRotationSpeedMax(1)

    if u_getDifficultyMult() > 1.5 then
        l_setRotationSpeedInc(0.1)
    else
        l_setRotationSpeedInc(0.04)
    end

    l_setDelayMult(1.0)
    l_setDelayInc(-0.01)
    l_setDelayMin(0.9)
    l_setFastSpin(80.0)
    l_setSides(24)
    l_setSidesMin(20)
    l_setSidesMax(28)
    l_setIncTime(15)

    l_setPulseMin(68)
    l_setPulseMax(82.93)
    l_setPulseSpeed(3.6)
    l_setPulseSpeedR(1.4)
    l_setPulseDelayMax(7)

    l_setBeatPulseMax(15)
    l_setBeatPulseDelayMax(21.818)

    enableSwapIfDMGreaterThan(1.5)
    disableSpeedIncIfDMGreaterThan(1.5)
end

-- onIncrement is an hardcoded function that is called when the level difficulty is incremented
function onIncrement()
    enableSwapIfSpeedGEThan(4);
end

-- onUnload is an hardcoded function that is called when the level is closed/restarted
function onUnload()
end

-- continuous direction change (even if not on level increment)
dirChangeTime = 180

-- onUpdate is an hardcoded function that is called every frame
function onUpdate(mFrameTime)
    dirChangeTime = dirChangeTime - mFrameTime;
    if dirChangeTime < 0 then
        -- do not change direction while fast spinning
        if u_isFastSpinning() == false then
            l_setRotationSpeed(l_getRotationSpeed() * -1.0)
            dirChangeTime = 180
        end
    end

    if not achievementUnlocked and l_getLevelTime() > 60 and u_getDifficultyMult() >= 1 then
        steam_unlockAchievement("a7_pi")
        achievementUnlocked = true
    end

    if not hardAchievementUnlocked and l_getLevelTime() > 30 and u_getDifficultyMult() > 1.5 then
        steam_unlockAchievement("a31_pi_hard")
        hardAchievementUnlocked = true
    end
end
