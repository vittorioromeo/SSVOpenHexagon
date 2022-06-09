-- include useful files
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "utils.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "common.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "commonpatterns.lua")

preAdjustedThickness = 0

function adjustThicknessForLowDM(mult)
    preAdjustedThickness = THICKNESS

    if u_getDifficultyMult() < 1 then
        THICKNESS = THICKNESS * mult
    end
end

function restoreThicknessForLowDM()
    THICKNESS = preAdjustedThickness
end

-- this function adds a pattern to the timeline based on a key
function addPattern(mKey)
    if mKey == 1 and l_getSides() == 5 then
        -- mirror spiral looks bad with 5 sides
        mKey = 5
    end

        if mKey == 0 then pAltBarrage(u_rndInt(3, 5), 2)
    elseif mKey == 1 then
        adjustThicknessForLowDM(3)
        pMirrorSpiral(u_rndInt(2, 5), getHalfSides() - 3)
        restoreThicknessForLowDM()
    elseif mKey == 2 then pBarrageSpiral(u_rndInt(0, 3), 1, 1)
    elseif mKey == 3 then pInverseBarrage(0)
    elseif mKey == 4 then
        adjustThicknessForLowDM(3)
        pTunnel(u_rndInt(1, 3))
        restoreThicknessForLowDM()
    elseif mKey == 5 then
        adjustThicknessForLowDM(3)
        pSpiral(l_getSides() * u_rndInt(1, 2), 0)

        if l_getSpeedMult() >= 4.25 then
            t_wait(getPerfectDelayDM(THICKNESS) * 0.5)
        end

        restoreThicknessForLowDM()
    end
end

-- shuffle the keys, and then call them to add all the patterns
-- shuffling is better than randomizing - it guarantees all the patterns will be called
keys = { 0, 0, 1, 1, 2, 2, 3, 3, 4, 5, 5 }
shuffle(keys)
index = 0
achievementUnlocked = false
hardAchievementUnlocked = false

-- onInit is an hardcoded function that is called when the level is first loaded
function onInit()
    if u_getDifficultyMult() > 3 then
        l_setSpeedMult(1.40)
    else
        l_setSpeedMult(1.30)
    end

    l_setSpeedInc(0.125)
    l_setSpeedMax(4.75)

    if u_getDifficultyMult() > 3 then
        l_setRotationSpeedInc(0.1)
    else
        l_setRotationSpeedInc(0.04)
    end

    if u_getDifficultyMult() == 1 then
        l_setDelayMult(1.4)
    else
        l_setDelayMult(1.0)
    end

    l_setDelayInc(0)
    l_setFastSpin(0.0)
    l_setSides(6)

    if u_getDifficultyMult() > 3 then
        l_setSidesMin(6)
    else
        l_setSidesMin(5)
    end

    l_setSidesMax(6)
    l_setIncTime(15)

    enableSwapIfDMGreaterThan(3)
    disableSpeedIncIfDMGreaterThan(3)
end

-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()
    if u_getDifficultyMult() == 1 then
        e_messageAdd("tutorials are over", 130)
        e_messageAdd("good luck getting high scores!", 130)
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
    enableSwapIfSpeedGEThan(4.25);
end

-- onUnload is an hardcoded function that is called when the level is closed/restarted
function onUnload()
end

-- onUpdate is an hardcoded function that is called every frame
function onUpdate(mFrameTime)
    if not achievementUnlocked and l_getLevelTime() > 120 and u_getDifficultyMult() >= 1 then
        steam_unlockAchievement("a1_pointless")
        achievementUnlocked = true
    end

    if not hardAchievementUnlocked and l_getLevelTime() > 45 and u_getDifficultyMult() > 3 then
        steam_unlockAchievement("a25_pointless_hard")
        hardAchievementUnlocked = true
    end
end
