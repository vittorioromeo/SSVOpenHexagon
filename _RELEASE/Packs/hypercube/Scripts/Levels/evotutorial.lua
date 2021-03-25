-- include useful files
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "utils.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "common.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "commonpatterns.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "nextpatterns.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "evolutionpatterns.lua")

-- this function adds a pattern to the timeline based on a key
function addPattern(mKey)
        if mKey == 0 then cBarrage(0)
    elseif mKey == 1 then hmcBarrageN(0, 0, 0, 0.05, -3.8, 2.7, true); t_wait(55)
    elseif mKey == 2 then hmcBarrageN(0, 0, 0, -0.05, -2.7, 3.8, true); t_wait(55)
    end
end

-- shuffle the keys, and then call them to add all the patterns
-- shuffling is better than randomizing - it guarantees all the patterns will be called
keys = { 1, 1, 2, 2 }
shuffle(keys)
index = 0
achievementUnlocked = false

-- onInit is an hardcoded function that is called when the level is first loaded
function onInit()
    l_setSpeedMult(1.1)
    l_setSpeedInc(0.045)
    l_setRotationSpeed(0.1)
    l_setRotationSpeedMax(0.4)
    l_setRotationSpeedInc(0.045)
    l_setDelayMult(1.0)
    l_setDelayInc(0.0)
    l_setFastSpin(71.0)
    l_setSides(6)
    l_setSidesMin(5)
    l_setSidesMax(7)
    l_setIncTime(0)

    l_setWallSkewLeft(18)

    l_setPulseMin(68)
    l_setPulseMax(82.93)
    l_setPulseSpeed(1.521)
    l_setPulseSpeedR(1.4)
    l_setPulseDelayMax(7)

    l_setBeatPulseMax(15)
    l_setBeatPulseDelayMax(27.48)

    l_setSwapEnabled(true)

    l_setTutorialMode(true)
    l_setIncEnabled(false)
end

swappedOnce = false

-- onCursorSwap is executed whenever the player executes a successful 180° swap
function onCursorSwap()
    if swappedOnce == false then
        u_log("swap detected!")
        swappedOnce = true
    end
end

-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()
    e_messageAddImportant("welcome to the evolution tutorial", 120)
    e_messageAddImportant("today you'll be introduced to...", 120)
    e_messageAddImportant("1. swapping!", 100)
    e_messageAddImportant("2. curving walls!", 100)
    e_messageAddImportant("", 120)
    e_messageAddImportant("press space or middle mouse button\nto swap", 250)
    e_messageAddImportant("it allows you to rotate 180 degrees!", 200)
    e_messageAddImportant("", 120)

    e_messageAddImportant("now: curving walls", 120)
    e_messageAddImportant("they can be simple...", 120)
    e_messageAddImportant("", 120 * 3 + 80)

    t_wait(135 * 8)
    hmcSimpleBarrage(1)
    t_wait(100)
    hmcSimpleBarrage(-1)
    t_wait(50)
    hmcSimpleBarrage(1)
    t_wait(100)
    hmcSimpleBarrage(-2.5)
    t_wait(80)
    hmcSimpleBarrage(2.5)
    t_wait(80)
    hmcSimpleBarrage(3)

    t_wait(50)
    e_messageAddImportant("...in various patterns...", 130)
    e_messageAddImportant("", 120 * 5 + 80)
    t_wait(130)

    hmcSimpleTwirl(5, 1, 0)
    t_wait(50)
    hmcSimpleTwirl(5, -2.5, 0.3)

    e_messageAddImportant("...or can accellerate!", 130)
    e_messageAddImportant("", 120 * 4 + 40)
    t_wait(130)

    hmcBarrage(0, 0.05, -1.5, 3, true)
    t_wait(80)
    hmcBarrage(0, -0.05, -3, 3, true)
    t_wait(100)
    hmcBarrage(0, 0.1, -2, 2, true)
    t_wait(100)
    hmcBarrage(0, 0.1, -3, 3, true)
    t_wait(200)

    e_messageAddImportant("they can also do crazy stuff!", 130)
    e_messageAddImportant("", 120 * 8 + 50)

    hmcSimpleCage(2.5, 1)
    t_wait(80)
    hmcSimpleCage(2.5, -1)
    t_wait(100)
    hmcSimpleCage(2.5, 1)
    hmcSimpleCage(2.5, 1)
    t_wait(100)
    hmcSimpleCage(2.5, 1)
    hmcSimpleCage(2.5, -1)
    t_wait(100)
    hmcSimpleSpinner(1)
    t_wait(100)
    hmcSimpleSpinner(-2)
    t_wait(100)
    hmcSimpleSpinner(3)
    t_wait(100)
    hmcSimpleCage(1.5, 1)
    hmcSimpleCage(2.5, 1)
    t_wait(100)
    hmcSimpleCage(1.5, 1)
    hmcSimpleCage(2.5, -1)
    t_wait(100)
    hmcSimpleSpinner(1)
    hmcSimpleSpinner(1.2)
    t_wait(100)
    hmcSimpleSpinner(1)
    hmcSimpleSpinner(-1.2)
    t_wait(500)

    e_messageAddImportant("now play some real levels!", 138)
    e_messageAddImportant("good luck!", 130)

    t_kill()
end

-- onStep is an hardcoded function that is called when the level timeline is empty
-- onStep should contain your pattern spawning logic
function onStep()
end

-- onIncrement is an hardcoded function that is called when the level difficulty is incremented
function onIncrement()
end

-- onUnload is an hardcoded function that is called when the level is closed/restarted
function onUnload()
end

-- continuous direction change (even if not on level increment)
dirChangeTime = 600

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

    if not achievementUnlocked and l_getLevelTime() > 69 then
        steam_unlockAchievement("a11_evotutorial")
        achievementUnlocked = true
    end
end
