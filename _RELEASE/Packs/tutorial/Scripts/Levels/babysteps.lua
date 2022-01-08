-- include useful files
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "utils.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "common.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "commonpatterns.lua")

-- this function adds a pattern to the timeline based on a key
function addPattern(mKey)
    if mKey == 0 then pBarrageSpiral(u_rndInt(1, 2), 1, 1)
    elseif mKey == 1 then pInverseBarrage(0)
    elseif mKey == 2 then pAltBarrage(u_rndInt(2, 4), 2)
    elseif mKey == 3 then pSpiral(12, 0)
    end
end

-- shuffle the keys, and then call them to add all the patterns
-- shuffling is better than randomizing - it guarantees all the patterns will be called
keys = { 0, 1, 2, 3 }
shuffle(keys)
index = 0
achievementUnlocked = false
challengeFailed = false
challengeFailedText = "yes, so far..."

step0 = false
step1 = false
step2 = false
step25 = false
step3 = false
step35 = false
step4 = false
step5 = false
step6 = false

step0_trRotateLeft = "nope"
step0_trRotateRight = "nope"

step0_completionTime = 0
step3_completionTime = 0

step1_trFocus = "nope"

-- onInit is an hardcoded function that is called when the level is first loaded
function onInit()
    l_setSpeedMult(0.65)
    l_setSpeedInc(0.15)
    l_setRotationSpeed(0.04)
    l_setRotationSpeedMax(0.4)
    l_setRotationSpeedInc(0.04)
    l_setDelayMult(1.65)
    l_setDelayInc(0.0)
    l_setFastSpin(0.0)
    l_setSides(6)
    l_setSidesMin(6)
    l_setSidesMax(6)
    l_setIncTime(1500)
    l_setTutorialMode(true)

    l_setBeatPulseMax(14)
    l_setBeatPulseDelayMax(21.95) -- BPM is 164, 3600/164 is 21.95
    l_setBeatPulseSpeedMult(0.45) -- Slows down the center going back to normal
end

-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()
end

-- onStep is an hardcoded function that is called when the level timeline is empty
-- onStep should contain your pattern spawning logic
function onStep()
    if not step35 then
        return
    end

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

-- onInput is a hardcoded function invoked when the player executes input
function onInput(mFrameTime, mMovement, mFocus, mSwap)
    if not step25 and step1 then
        if mMovement == -1 and step0_trRotateLeft ~= "yes!" then
            a_playSound("blip.ogg")
            step0_trRotateLeft = "yes!"
        end

        if mMovement == 1 and step0_trRotateRight ~= "yes!"  then
            a_playSound("blip.ogg")
            step0_trRotateRight = "yes!"
        end
    end

    if step25 then
        if mFocus then
            if step1_trFocus ~= "yes!" then
                a_playSound("blip.ogg")
                step1_trFocus = "yes!"
            end

            if mMovement == -1 and step0_trRotateLeft ~= "yes!" then
                a_playSound("blip.ogg")
                step0_trRotateLeft = "yes!"
            end

            if mMovement == 1 and step0_trRotateRight ~= "yes!" then
                a_playSound("blip.ogg")
                step0_trRotateRight = "yes!"
            end
        end
    end
end

-- onUpdate is an hardcoded function that is called every frame
function onUpdate(mFrameTime)
    if not step0 then
        step0 = true

        l_resetTime()

        e_messageAddImportant("welcome to open hexagon 2!", 120)
        e_messageAddImportant("let's learn about the controls", 120)
        e_messageAddImportant("use left/right to rotate around the center", 200)
        e_messageAddImportant("try it now!", 120)
        e_stopTimeS(9)
    end

    if not step1 and l_getLevelTime() > 0.2 then
        step1 = true

        l_addTracked("step0_trRotateLeft", "rotated counter-clockwise")
        l_addTracked("step0_trRotateRight", "rotated clockwise")
    end

    if step1 and not step2 and step0_trRotateLeft == "yes!" and step0_trRotateRight == "yes!" then
        step2 = true
        step0_completionTime = l_getLevelTime()

        l_clearTracked()

        e_messageAddImportant("well done!", 120)
        e_messageAddImportant("you can slow down by focusing", 160)
        e_messageAddImportant("by default, use left shift", 180)
        e_messageAddImportant("try it while rotating!", 140)
        e_stopTimeS(8)
    end

    if not step25 and step2 and l_getLevelTime() > step0_completionTime + 0.2 then
        step25 = true

        step0_trRotateLeft = "nope"
        step0_trRotateRight = "nope"

        l_addTracked("step0_trRotateLeft", "rotated ccw while focusing")
        l_addTracked("step0_trRotateRight", "rotated cw while focusing")
        l_addTracked("step1_trFocus", "focused (left shift)")
    end

    if step25 and not step3 and step0_trRotateLeft == "yes!" and step0_trRotateRight == "yes!" and step1_trFocus == "yes!" then
        step3 = true
        step3_completionTime = l_getLevelTime()

        l_clearTracked()

        e_stopTimeS(7)
        e_messageAddImportant("great!", 120)
        e_messageAddImportant("the goal of open hexagon is to survive", 180)
        e_messageAddImportant("avoid the walls!", 120)
    end

    if not step35 and step3 and l_getLevelTime() > step3_completionTime + 0.2 then
        step35 = true


        l_addTracked("challengeFailedText", "survived until the end")

        l_resetTime()
        l_setIncTime(15)
    end

    if step35 and not step4 and l_getLevelTime() > 30 then
        step4 = true

        e_stopTimeS(5)

        if not challengeFailed then
            e_messageAddImportant("great job!", 110)
        else
            e_messageAddImportant("nice try...", 110)
        end

        e_messageAddImportant("after a while, things get harder", 140)
        e_messageAddImportant("get to 45 seconds to complete the tutorial!", 160)
    end

    if step4 and not step5 and l_getLevelTime() > 43 then
        step5 = true

        u_clearWalls()
        l_setIncEnabled(false);

        if not achievementUnlocked then
            steam_unlockAchievement("a0_babysteps")
            achievementUnlocked = true
        end

        if challengeFailed == false then
            a_playPackSound("fanfare.ogg")
            e_messageAddImportant("well done! you survived!", 120)
            e_messageAddImportant("now play some real levels!", 180)
        else
            a_playPackSound("failure.ogg")
            e_messageAddImportant("nice try, but not quite...", 120)
            e_messageAddImportant("try again without hitting any wall!", 220)
        end
    end

    if step5 and not step6 and l_getLevelTime() > 45 then
        step6 = true
        e_kill()
    end
end

-- onPreDeath is an hardcoded function that is called when the player is killed, even
-- in tutorial mode
function onPreDeath()
    if challengeFailed == false and l_getLevelTime() < 42 then
        challengeFailed = true
        challengeFailedText = "not this time!"
        e_messageAddImportant("whoops!", 60)
    end
end
