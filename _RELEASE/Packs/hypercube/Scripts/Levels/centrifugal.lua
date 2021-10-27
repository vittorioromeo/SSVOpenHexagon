-- include useful files
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "utils.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "common.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "commonpatterns.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "nextpatterns.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "evolutionpatterns.lua")

curveSpeed = 15
achievementUnlocked = false
hardAchievementUnlocked = false

-- onInit is an hardcoded function that is called when the level is first loaded
function onInit()
    l_setSpeedMult(2.8)
    l_setSpeedInc(0.0)
    l_setRotationSpeed(0.0)
    l_setRotationSpeedMax(0.0)
    l_setRotationSpeedInc(0.0)
    l_setDelayMult(1.35)
    l_setDelayInc(0.0)
    l_setFastSpin(0.0)
    l_setSides(30)
    l_setSidesMin(30)
    l_setSidesMax(30)
    l_setIncTime(10)

    l_setWallSkewLeft(0)

    l_setPulseMin(60)
    l_setPulseMax(60)
    l_setPulseSpeed(0)
    l_setPulseSpeedR(0)
    l_setPulseDelayMax(6.8)

    l_setBeatPulseMax(20)
    l_setBeatPulseDelayMax(27.2)

    l_setSwapEnabled(true)
    l_setSwapCooldownMult(0.8/u_getDifficultyMult())
    l_addTracked("curveSpeed", "curve speed")
end

-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()
    if (u_getDifficultyMult() > 1) then
        e_messageAdd("Difficulty > 1\nWalls travel constantly!", 120)
    end
end

-- onStep is an hardcoded function that is called when the level timeline is empty
-- onStep should contain your pattern spawning logic
function onStep()
    if (u_getDifficultyMult() > 1) then
        hmcSimpleBarrageSNeigh(getRandomSide(), getRandomDir() * curveSpeed / 1.5, 4)
        t_wait(getPerfectDelay(THICKNESS) * 7.5)
    else
        hmcBarrageStop(getRandomSide(), getRandomDir() * curveSpeed, 4)
        t_wait(getPerfectDelay(THICKNESS) * 7)
    end

end


-- onIncrement is an hardcoded function that is called when the level difficulty is incremented
function onIncrement()
    curveSpeed = curveSpeed + 5
    e_messageAddImportant("Curve speed: "..curveSpeed, 120)
end

-- onUnload is an hardcoded function that is called when the level is closed/restarted
function onUnload()
end

-- onUpdate is an hardcoded function that is called every frame
function onUpdate(mFrameTime)
    if not achievementUnlocked and l_getLevelTime() > 45 and u_getDifficultyMult() >= 1 then
        steam_unlockAchievement("a19_centrifugalforce")
        achievementUnlocked = true
    end

    if not hardAchievementUnlocked and l_getLevelTime() > 30 and u_getDifficultyMult() > 1.5 then
        steam_unlockAchievement("a45_centrifugalforce_hard")
        hardAchievementUnlocked = true
    end
end
