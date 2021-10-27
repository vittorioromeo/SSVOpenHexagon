-- include useful files
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "utils.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "common.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "commonpatterns.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "nextpatterns.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "evolutionpatterns.lua")

-- this function adds a pattern to the timeline based on a key
function addPattern(mKey)
    -- Normal Palette
        if mKey == 0 then pAltBarrage(math.random(2, 4), 2)
    elseif mKey == 1 then pMirrorSpiral(math.random(2, 4), 0)
    elseif mKey == 2 then pBarrageSpiral(math.random(0, 3), 1, 1)
    elseif mKey == 3 then pBarrageSpiral(math.random(0, 2), 1.2, 2)
    elseif mKey == 4 then pBarrageSpiral(2, 0.7, 1)
    elseif mKey == 5 then pInverseBarrage(0)
    elseif mKey == 6 then hmpBarrageSpiral(math.random(1, 3), 2, clamp(level + 1, 2, 6))
    elseif mKey == 7 then pMirrorWallStrip(1, 0)
    elseif mKey == 8 then hmpSpinner(1, clamp(level, 1, 6))
    elseif mKey == 9 then hmpBarrage(1, clamp(level, 1, 6))
    elseif mKey == 10 then hmcDef2Cage()
    elseif mKey == 11 then hmpBarrageSpiralSpin(math.random(7, 14), 2, clamp(level + 1, 2, 6))
    elseif mKey == 12 then hmpGrowTunnel(math.random(2, 3))
    elseif mKey == 13 then
        hmcGrowBarrage(getRandomSide(), getRandomSide())
        t_wait(getPerfectDelay(getPerfectThickness(THICKNESS)) * 4.5)
    elseif mKey == 14 then pSwapBarrage(getRandomSide())
    elseif mKey == 15 then pSwapCorridor(math.random(2, 3))
    elseif mKey == 16 then hmpTwirl(math.random(2, 4), clamp(level + 1, 2, 5), 1)
    -- Specials Palette
    elseif mKey == 101 then
        hmpBarrageStop(3, 12)
        if (u_getDifficultyMult() > 1) then
            t_wait(getPerfectDelay(THICKNESS) * 3)
        end
    elseif mKey == 102 then hmpSwarm(clamp(level, 1, 6), 3 * clamp(level, 1, 6), true, level > 10, false)
    end
end

--globalHueModifier = 0
level = 1;

-- shuffle the keys, and then call them to add all the patterns
-- shuffling is better than randomizing - it guarantees all the patterns will be called
keys = { 0, 0, 1, 1, 2, 3, 3, 4, 5, 5, 6, 6, 7, 7, 8, 8, 8, 9, 9, 9, 9, 10, 10, 10, 11, 12, 13, 13, 14, 14, 15, 16, 16 }
specialKeys = {} -- For specials.
shuffle(keys)
index = 1
specialIndex = 1
achievementUnlocked = false
hardAchievementUnlocked = false

specials = { "spinner", "barrage", "grow", "swarm" }
shuffle(specials)
currSpecial = 1
special = "none"

-- onInit is an hardcoded function that is called when the level is first loaded
function onInit()
    l_setSpeedMult(1.7)
    l_setSpeedInc(0)
    l_setSpeedMax(3.5)
    l_setRotationSpeed(0.1)
    l_setRotationSpeedMax(0.415)
    l_setRotationSpeedInc(0)
    l_setDelayMult(1.2)
    l_setDelayInc(0.0)
    l_setFastSpin(0.0)
    l_setSides(6)
    l_setSidesMin(6)
    l_setSidesMax(6)
    l_setIncTime(15)

    l_setPulseMin(77)
    l_setPulseMax(95)
    l_setPulseSpeed(1.937)
    l_setPulseSpeedR(0.524)
    l_setPulseDelayMax(13.05)
    l_setPulseInitialDelay(28.346) -- skip a beat to match with the clap

    l_setBeatPulseMax(17)
    l_setBeatPulseDelayMax(28.346)

    l_setSwapEnabled(true)
    l_setSwapCooldownMult(1.4/u_getSpeedMultDM());
    l_addTracked("special", "special")
end

-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()
end

-- onStep is an hardcoded function that is called when the level timeline is empty
-- onStep should contain your pattern spawning logic
function onStep()
    if (special == "none") then
        addPattern(keys[index])
        index = index + 1
        if index - 1 == #keys then
            index = 1
            shuffle(keys)
        end
    else
        addPattern(specialKeys[specialIndex])
        if (#specialKeys > 1) then
            specialIndex = specialIndex + 1;

            if specialIndex - 1 == #specialKeys then
                specialIndex = 1
                shuffle(specialKeys)
            end
        end
    end
end

local constantFlag = false

-- onIncrement is an hardcoded function that is called when the level difficulty is incremented
function onIncrement()
    if special == "none" then
        special = specials[currSpecial]
        e_messageAddImportant("Special: "..special, 120)
        currSpecial = currSpecial + 1
        if (currSpecial - 1 == #specials) then
            currSpecial = 1
            shuffle(specials)
        end
        specialIndex = 1
        if (special == "cage") then
            specialKeys = {10};
        elseif (special == "spinner") then
            specialKeys = {8};
        elseif (special == "barrage") then
            specialKeys = {101};
        elseif (special == "grow") then
            specialKeys = {13};
        elseif (special == "swarm") then
            specialKeys = {102};
            if (level > 10 and not constantFlag) then
                e_messageAddImportant("Constant Speed Enabled!", 90)
                constantFlag = true
            end
        end
        shuffle(specialKeys)
        l_setSpeedInc(0.1)
        l_setRotationSpeedInc(0.035)
    else
        special = "none"
        level = level + 1;
        l_setSpeedInc(0)
        l_setRotationSpeedInc(0)
    end
    l_setSwapCooldownMult(1.7/u_getSpeedMultDM());
end

-- onUnload is an hardcoded function that is called when the level is closed/restarted
function onUnload()
end

-- onUpdate is an hardcoded function that is called every frame
function onUpdate(mFrameTime)
    if not achievementUnlocked and l_getLevelTime() > 90 and u_getDifficultyMult() >= 1 then
        steam_unlockAchievement("a12_disco")
        achievementUnlocked = true
    end

    if not hardAchievementUnlocked and l_getLevelTime() > 40 and u_getDifficultyMult() > 2.5 then
        steam_unlockAchievement("a38_disco_hard")
        hardAchievementUnlocked = true
    end
end
