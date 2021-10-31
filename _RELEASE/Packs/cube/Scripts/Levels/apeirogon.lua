-- include useful files
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "utils.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "common.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "commonpatterns.lua")

-- this function adds a pattern to the timeline based on a key
function addPattern(mKey)
    if mKey == 4 and l_getSides() ~= 6 then
        -- mirror wall strip looks bad with odd sides
        mKey = 2
    end

    if mKey == 8 and l_getSides() ~= 6 then
        -- mirror spiral double looks bad with odd sides
        mKey = 0
    end

    if mKey == 9 and l_getSides() ~= 6 then
        -- mirror spiral looks bad with odd sides
        mKey = 7
    end

        if mKey == 0 then pAltBarrage(u_rndInt(3, 4), 2)
    elseif mKey == 1 then pBarrageSpiral(3, 0.6, 1)
    elseif mKey == 2 then pInverseBarrage(0)
    elseif mKey == 3 then pTunnel(u_rndInt(1, 3))
    elseif mKey == 4 then pMirrorWallStrip(1, 0)
    elseif mKey == 5 then
        if l_getSides() > 5 then
            pWallExVortex(0, u_rndInt(1, 2), 1)
        end
    elseif mKey == 6 then
        if u_getDifficultyMult() < 1.0 then
            local oldThickness = THICKNESS
            THICKNESS = 60
            pDMBarrageSpiral(u_rndInt(2, 4), 0.12, 1)
            THICKNESS = oldThickness
        else
            pDMBarrageSpiral(u_rndInt(3, 6), 0.295 * (u_getDifficultyMult() ^ 0.56), 1)
        end
    elseif mKey == 7 then pRandomBarrage(u_rndInt(2, 5), 2.25)
    elseif mKey == 8 then pMirrorSpiralDouble(u_rndInt(4, 6), 0)
    elseif mKey == 9 then pMirrorSpiral(u_rndInt(2, 4), 0)
    end
end

-- shuffle the keys, and then call them to add all the patterns
-- shuffling is better than randomizing - it guarantees all the patterns will be called
keys = { 0, 0, 1, 1, 2, 2, 3, 4, 4, 5, 6, 7, 7, 7, 8, 9, 9 }
shuffle(keys)
index = 0
achievementUnlocked = false
hardAchievementUnlocked = false

-- onInit is an hardcoded function that is called when the level is first loaded
function onInit()
    if u_getDifficultyMult() > 1.5 then
        l_setSpeedMult(3.6)
    else
        l_setSpeedMult(2.9)
    end

    l_setSpeedInc(0.13)
    l_setSpeedMax(3.6)
    l_setRotationSpeed(0.3)
    l_setRotationSpeedMax(0.9)

    if u_getDifficultyMult() > 1.5 then
        l_setRotationSpeedInc(0.1)
    else
        l_setRotationSpeedInc(0.04)
    end

    l_setDelayMult(1.1)
    l_setDelayInc(0.0)
    l_setFastSpin(71.0)
    l_setSides(6)

    if u_getDifficultyMult() > 1.5 then
        l_setSidesMin(6)
        l_setSidesMax(6)
    else
        l_setSidesMin(5)
        l_setSidesMax(7)
    end

    l_setIncTime(15)

    l_setPulseInitialDelay(19.672)
    l_setPulseMin(64)
    l_setPulseMax(84)
    l_setPulseSpeed(2.0)
    l_setPulseSpeedR(1.5)
    l_setPulseDelayMax(16.0111)

    l_setBeatPulseMax(24)
    l_setBeatPulseDelayMax(19.672) -- BPM is 183
    l_setBeatPulseSpeedMult(1.35) -- Slows down the center going back to normal

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
    enableSwapIfSpeedGEThan(3.4);
    if (u_getSpeedMultDM() >= 4.5 and l_getSidesMin() == 5) then
        e_messageAddImportant("Speed >= 4.5\nPentagon removed!", 120)
        if (l_getSides() == 5) then
            l_setSides(6)
        end
        l_setSidesMin(6)
    end
end

-- onUnload is an hardcoded function that is called when the level is closed/restarted
function onUnload()
end

-- continuous direction change (even if not on level increment)
dirChangeTime = 100

-- onUpdate is an hardcoded function that is called every frame
function onUpdate(mFrameTime)
    dirChangeTime = dirChangeTime - mFrameTime;
    if dirChangeTime < 0 then
        -- do not change direction while fast spinning
        if u_isFastSpinning() == false then
            l_setRotationSpeed(l_getRotationSpeed() * -1.0)
            dirChangeTime = 300
        end
    end

    if not achievementUnlocked and l_getLevelTime() > 60 and u_getDifficultyMult() >= 1 then
        steam_unlockAchievement("a4_apeirogon")
        achievementUnlocked = true
    end

    if not hardAchievementUnlocked and l_getLevelTime() > 45 and u_getDifficultyMult() > 1.5 then
        steam_unlockAchievement("a28_apeirogon_hard")
        hardAchievementUnlocked = true
    end
end
