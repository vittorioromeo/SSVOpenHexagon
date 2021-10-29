-- include useful files
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "utils.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "common.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "commonpatterns.lua")

-- this function adds a pattern to the timeline based on a key
function addPattern(mKey)
    if mKey == 1 and l_getSides() ~= 6 then
        -- mirror spiral looks bad with odd sides
        mKey = 5
    end

    if mKey == 7 and l_getSides() ~= 6 then
        -- mirror wall strip looks bad with odd sides
        mKey = 5
    end

        if mKey ==  0 then pAltBarrage(u_rndInt(3, 5), 2)
    elseif mKey ==  1 then pMirrorSpiral(u_rndInt(3, 6), 0)
    elseif mKey ==  2 then pBarrageSpiral(u_rndInt(0, 3), 1, 1)
    elseif mKey ==  3 then pBarrageSpiral(u_rndInt(0, 2), 1.2, 2)
    elseif mKey ==  4 then pBarrageSpiral(2, 0.7, 1)
    elseif mKey ==  5 then pInverseBarrage(0)
    elseif mKey ==  6 then pTunnel(u_rndInt(1, 3))
    elseif mKey ==  7 then pMirrorWallStrip(1, 0)
    elseif mKey ==  8 then
        if l_getSides() > 5 then
            pWallExVortex(0, 1, 1)
        end
    elseif mKey ==  9 then pDMBarrageSpiral(u_rndInt(3, 6), 0.325 * (u_getDifficultyMult() ^ 0.56), 1)
    elseif mKey == 10 then pRandomBarrage(u_rndInt(2, 4), 2.25)
    end
end

-- shuffle the keys, and then call them to add all the patterns
-- shuffling is better than randomizing - it guarantees all the patterns will be called
keys = { 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 7, 7, 8, 9, 10, 10, 10 }
shuffle(keys)
index = 0
achievementUnlocked = false
hardAchievementUnlocked = false

-- onInit is an hardcoded function that is called when the level is first loaded
function onInit()
    if u_getDifficultyMult() > 2.0 then
        l_setSpeedMult(2.45)
    elseif u_getDifficultyMult() > 1.5 then
        l_setSpeedMult(2.55)
    else
        l_setSpeedMult(2.65)
    end

    l_setSpeedInc(0.1)
    l_setSpeedMax(4.2)
    l_setRotationSpeed(0.2)
    l_setRotationSpeedMax(0.9)

    if u_getDifficultyMult() > 1.5 then
        l_setRotationSpeedInc(0.1)
    else
        l_setRotationSpeedInc(0.05)
    end

    l_setDelayMult(1.1)
    l_setDelayInc(0.005)
    l_setDelayMax(1.22)
    l_setFastSpin(70.0)
    l_setSides(6)

    if u_getDifficultyMult() > 1.5 then
        l_setSidesMin(6)
        l_setSidesMax(6)
    else
        l_setSidesMin(5)
        l_setSidesMax(7)
    end

    l_setIncTime(15)

    l_setPulseInitialDelay(26.667)
    l_setPulseMin(70)
    l_setPulseMax(90.00025)
    l_setPulseSpeed(1.0)
    l_setPulseSpeedR(0.6)
    l_setPulseDelayMax(0)

    l_setBeatPulseMax(17)
    l_setBeatPulseDelayMax(26.667) -- BPM is 135
    l_setBeatPulseSpeedMult(0.45) -- Slows down the center going back to normal

    enableSwapIfDMGreaterThan(1.5)
    disableSpeedIncIfDMGreaterThan(1.5)
end

-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()
    if u_getDifficultyMult() == 1 then
        e_waitS(16)
        e_messageAdd("whoa!", 120)
        e_waitS(45)
        e_messageAddImportant("may the mayhem begin!", 130)
        s_setPulseInc(0.15)
    end
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
    enableSwapIfSpeedGEThan(3.8);
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

-- onUpdate is an hardcoded function that is called every frame
function onUpdate(mFrameTime)
    if not achievementUnlocked and l_getLevelTime() > 60 and u_getDifficultyMult() >= 1 then
        steam_unlockAchievement("a3_seconddim")
        achievementUnlocked = true
    end

    if not hardAchievementUnlocked and l_getLevelTime() > 20 and u_getDifficultyMult() > 2 then
        steam_unlockAchievement("a27_seconddim_hard")
        hardAchievementUnlocked = true
    end
end
