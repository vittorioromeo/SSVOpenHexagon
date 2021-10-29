-- include useful files
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "utils.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "common.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "commonpatterns.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "nextpatterns.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "evolutionpatterns.lua")

gap = 7
minGap = 4

-- this function adds a pattern to the timeline based on a key
function addPattern(mKey)
        if mKey == 0 then cBarrageN(getRandomSide(), gap) t_wait(getPerfectDelay(THICKNESS) * 6)
    elseif mKey == 1 then hmcSimpleBarrageSNeigh(getRandomSide(), 0, gap) t_wait(getPerfectDelay(THICKNESS) * 6)
    elseif mKey == 2 then
        if (u_getDifficultyMult() < 1) then
            cSwapBarrageN(getRandomSide(), gap, 1.25)
        else
            cSwapBarrageN(getRandomSide(), gap, .75)
        end
        t_wait(getPerfectDelay(THICKNESS) * 6)
    end
end

-- shuffle the keys, and then call them to add all the patterns
-- shuffling is better than randomizing - it guarantees all the patterns will be called
keys = { 0, 0, 0, 1, 1, 1, 2, 2 }
shuffle(keys)
index = 0
achievementUnlocked = false
hardAchievementUnlocked = false

-- onInit is an hardcoded function that is called when the level is first loaded
function onInit()
    l_setSpeedMult(3.0)
    l_setSpeedInc(0.0)
    l_setRotationSpeed(0.22)
    l_setRotationSpeedMax(0.75)
    l_setRotationSpeedInc(0.03)
    l_setDelayMult(1.5)
    l_setDelayInc(0.0)
    l_setFastSpin(71.0)
    l_setSides(32)
    l_setSidesMin(32)
    l_setSidesMax(32)
    l_setIncTime(15)

    l_setWallSkewLeft(15)

    l_setPulseMin(60.98)
    l_setPulseMax(80.06)
    l_setPulseSpeed(3.597)
    l_setPulseSpeedR(1.448)
    l_setPulseDelayMax(7.23)

    l_setBeatPulseMax(19)
    l_setBeatPulseDelayMax(25.714)

    l_setSwapEnabled(true)
    l_setSwapCooldownMult(0.7/u_getDifficultyMult())
    l_addTracked("gap", "gap size")

    if(u_getDifficultyMult() >= 1.59) then
        gap = 9
        minGap = 5
    elseif(u_getDifficultyMult() >= 1.39) then
        gap = 8
        minGap = 4
    else
        gap = 7
        minGap = 3
    end
end

-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()
    syncCurveWithRotationSpeed(0, 0)
    e_messageAdd("Remember to swap!", 120)
    if (u_getDifficultyMult() > 1) then
        minGap = 5
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
    if gap > minGap then
        gap = gap -1
        e_messageAddImportant("Gap size: "..gap, 120)
    end
end

-- onUnload is an hardcoded function that is called when the level is closed/restarted
function onUnload()
end

-- onUpdate is an hardcoded function that is called every frame
function onUpdate(mFrameTime)
    if not achievementUnlocked and l_getLevelTime() > 45 and u_getDifficultyMult() >= 1 then
        steam_unlockAchievement("a18_reppaws")
        achievementUnlocked = true
    end

    if not hardAchievementUnlocked and l_getLevelTime() > 30 and u_getDifficultyMult() > 1.5 then
        steam_unlockAchievement("a44_reppaws_hard")
        hardAchievementUnlocked = true
    end
end
