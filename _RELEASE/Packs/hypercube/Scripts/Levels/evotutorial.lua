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

    l_setPulseMin(64)
    l_setPulseMax(84)
    l_setPulseSpeed(1.05)
    l_setPulseSpeedR(1.35)
    l_setPulseDelayMax(7)

    l_setBeatPulseMax(15)
    l_setBeatPulseDelayMax(110)

    l_setSwapEnabled(true)

    l_setTutorialMode(true)
    l_setIncEnabled(false)
end

swappedOnce = false
swapTime = false

-- onCursorSwap is executed whenever the player executes a successful 180° swap
function onCursorSwap()
    if (swapTime and not swappedOnce) then
        swappedOnce = true
        e_clearMessages()
    end
end

-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()
    e_waitS(1)
    e_messageAddImportant("Welcome to the evolution tutorial", 150)
    e_messageAddImportant("Today you'll be introduced to...", 120)
    e_messageAddImportant("swapping, accelerating walls, and curving walls!", 300)
    e_wait(570 + 120)
    e_messageAddImportant("Swapping allows you to instantly rotate 180 degrees!", 200)
    e_messageAddImportant("You can only swap when your player\nblinks red and yellow!", 250)
    e_wait(450)

    e_eval([[swapTime = true]])
    e_messageAddImportant("Try swapping now!\nPress space or middle mouse button to swap", 10000)
end

function partTwo()
    e_messageAddImportant("Awesome!", 120)
    e_messageAddImportant("After swapping, you must wait before\nswapping again!", 250)
    e_messageAddImportant("Let's introduce a swap pattern.", 180);
    e_messageAddImportant("For this pattern, you must swap to survive", 180)
    e_messageAddImportant("Swap at the right moment!", 180)
    e_wait(180 * 3 + 120 + 250 + 120)
    e_messageAddImportant("Excellent! You know how to swap!", 120)
    t_wait(610)
    pSwapBarrage(getRandomSide(), 2)

    e_messageAddImportant("Let's talk about accelerating walls", 180);
    e_messageAddImportant("These walls can change speed over time", 180);
    e_messageAddImportant("They can either accelerate...", 180);
    e_messageAddImportant("Decelerate...", 180);
    e_messageAddImportant("Or go completely backwards to trick you!", 180);
    e_wait(180 * 5 + 120 + 120)

    t_wait(760);
    pACBarrageAccelerate();
    t_wait(120);
    pACBarrageDecelerate();
    t_wait(100);
    pACBarrageDeception(3, 1);

    e_messageAddImportant("But wait, it gets crazier!", 120);
    e_messageAddImportant("Let's focus on curving walls", 120)
    e_messageAddImportant("they can be simple...", 120)
    e_wait(300 + 410)

    t_wait(300)
    hmcSimpleBarrage(0, 1)
    t_wait(100)
    hmcSimpleBarrage(0, -1)
    t_wait(50)
    hmcSimpleBarrage(0, math.random(1, 6))
    t_wait(100)
    hmcSimpleBarrage(0, -math.random(1, 6))
    t_wait(80)
    hmcSimpleBarrage(0, math.random(1, 6))
    t_wait(80)
    hmcSimpleBarrage(0, math.random(-6, 6))

    t_wait(50)
    e_messageAddImportant("...in various patterns...", 130)
    e_wait(100 + 120 * 5 + 80)
    t_wait(130)

    hmcSimpleTwirl(5, math.random(-3, 3), 0)
    t_wait(50)
    hmcSimpleTwirl(5, -2, 1)

    e_messageAddImportant("...or can accelerate!", 130)
    e_wait(130 + 340)
    t_wait(130)

    hmcBarrage(0, 0.05, -1.5, 3, true)
    t_wait(80)
    hmcBarrage(0, -0.05, -3, 3, true)
    t_wait(100)
    hmcBarrage(0, 0.1, -2, 2, true)
    t_wait(100)
    hmcBarrage(0, 0.1, -3, 3, true)
    t_wait(200)

    e_messageAddImportant("they can also do crazy stuff!", 180)
    e_wait(120 * 9)

    hmcSimpleCage(2, 1)
    t_wait(80)
    hmcSimpleCage(2, -1)
    t_wait(100)
    hmcSimpleCage(2, 1)
    hmcSimpleCage(2, 1)
    t_wait(100)
    hmcSimpleCage(2, 1)
    hmcSimpleCage(2, -1)
    t_wait(100)
    hmcSimpleSpinner(getRandomSide(), 1)
    t_wait(100)
    hmcGrowBarrage(getRandomSide(), 0)
    t_wait(100)
    hmcGrowBarrage(getRandomSide(), 3)
    t_wait(100)
    hmcGrowBarrage(getRandomSide(), 6)
    t_wait(100)
    hmcAssembleBarrage(getRandomSide())
    t_wait(100)
    hmcAssembleBarrage(getRandomSide())
    t_wait(100)
    hmcAssembleBarrage(getRandomSide())
    t_wait(700)

    e_messageAddImportant("Well done!", 130)
    e_messageAddImportant("You should be prepared to take on Hypercube!", 300)
    e_messageAddImportant("Have fun!", 1000)

    e_wait(340)
    e_kill()
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
    --print(l_getPulse());
    if (swappedOnce and swapTime) then
        partTwo()
        swapTime = false
    end
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
