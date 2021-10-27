-- include useful files
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "utils.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "common.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "commonpatterns.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "nextpatterns.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "evolutionpatterns.lua")

-- shuffle the keys, and then call them to add all the patterns
-- shuffling is better than randomizing - it guarantees all the patterns will be called
keys = { 0, 0, 0, 1, 2 }
shuffle(keys)
index = 0
achievementUnlocked = false
hardAchievementUnlocked = false

smin = 2
smax = 2
completed = false

level = 1
incrementTime = 10

range = "("..(smin * 2).."/"..(smax * 2).."]"

function slitherSpiralAcc()
    t_wait(getPerfectDelay(THICKNESS) * 2.1)
    t_wait(getPerfectDelay(THICKNESS) * 2.1)
    local side = getRandomSide()

    local acc = u_rndInt(50, 90) / 500.0 * getRandomDir()
    local minimum = u_rndInt(15, 21) / 10.0 * -1
    local maximum = -minimum

    t_wait(getPerfectDelay(THICKNESS) * 3.1)

    for i = 0, u_rndInt(6, 10) do
        hmcSimpleSpinnerSAcc(side, 0, acc, minimum, maximum, true)
        t_wait(getPerfectDelay(THICKNESS) * 0.55)
    end

    t_wait(getPerfectDelay(THICKNESS) * 5.3)
end

-- this function adds a pattern to the timeline based on a key
function addPattern(mKey)
    if (mKey == 0) then slitherSpiralAcc()
    elseif (mKey == 1) then
        hmpStripeSnakeBarrage(getRandomSide(), u_rndInt(5, 10), getHalfSides(), l_getSides())
        t_wait(getPerfectDelay(THICKNESS) * 2)
    elseif (mKey == 2) then
        hmpStripeSnakeAltBarrage(u_rndInt(5, 10), 2, getHalfSides(), l_getSides())
        t_wait(getPerfectDelay(THICKNESS) * 2)
    end
end

-- onInit is an hardcoded function that is called when the level is first loaded
function onInit()
    l_setSpeedMult(1.8)
    l_setSpeedInc(0) -- This will be set later.
    l_setSpeedMax(2.9)

    l_setRotationSpeed(0.2)
    l_setRotationSpeedMax(0.5)
    l_setRotationSpeedInc(0.035)

    l_setDelayMult(1.1)
    l_setDelayInc(0.0)

    l_setFastSpin(0.0)

    l_setSides(3)
    l_setSidesMin(3)
    l_setSidesMax(3)

    l_setIncTime(10)

    l_setWallAngleLeft(-25)

    l_setPulseMin(49.98)
    l_setPulseMax(88.73)
    l_setPulseSpeed(2.439)
    l_setPulseSpeedR(0.668)
    l_setPulseDelayMax(0.08)

    l_setBeatPulseMax(16)
    l_setBeatPulseDelayMax(24.657)

    l_setSwapEnabled(true)
    l_addTracked("level", "level")
    l_addTracked("next at", "incrementTime")
    l_addTracked("range", "range")
end

-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()
    e_messageAdd("remember, you can focus with lshift!", 150)
end

-- onStep is an hardcoded function that is called when the level timeline is empty
-- onStep should contain your pattern spawning logic
function onStep()
    l_setSides(math.random(smin, smax) * 2)
    addPattern(keys[index])
    index = index + 1
    if index - 1 == #keys then
        index = 1
        shuffle(keys)
    end
end

-- onIncrement is an hardcoded function that is called when the level difficulty is incremented
function onIncrement()
    level = level + 1
    e_messageAddImportant("level: "..(level).." / time: "..incrementTime, 120)
    if (not completed) then
        incrementTime = incrementTime + 5
        smin = smin + 1
        if (smin > smax) then
            smin = 2
            smax = smax + 1
            if (smax > 4) then
                completed = true
                l_setSpeedInc(0.1) -- Enable the speed increment
                incrementTime = 30
            end
        end
    end
    if (completed) then
        smin = math.random(2, 3)
        smax = clamp(smin + math.random(0, 3), smin, 5)
    end

    range = "("..(smin * 2).."/"..(smax * 2).."]"
    e_messageAddImportant("Range: "..range, 100)

    l_setSides(l_getSides() + 2)
    l_setIncTime(incrementTime)
end

-- continuous direction change (even if not on level increment)
dirChangeTime = 120

-- onUnload is an hardcoded function that is called when the level is closed/restarted
function onUnload()
end

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
        steam_unlockAchievement("a16_slither")
        achievementUnlocked = true
    end

    if not hardAchievementUnlocked and l_getLevelTime() > 30 and u_getDifficultyMult() > 1.8 then
        steam_unlockAchievement("a42_slither_hard")
        hardAchievementUnlocked = true
    end
end
