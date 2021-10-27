-- include useful files
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "utils.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "common.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "commonpatterns.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "nextpatterns.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "evolutionpatterns.lua")

local side = 0
local sideSpawn = 0;

-- this function adds a pattern to the timeline based on a key
function addPattern(mKey)
        if mKey == 0 then pAltBarrage(math.random(2, 4), 2)
    elseif mKey == 1 then pBarrageSpiral(2, 0.6, 1)
    elseif mKey == 2 then pInverseBarrage(0)
    elseif mKey == 3 then hmpTunnelDynamic(math.random(2, 3))
    elseif mKey == 4 then pWallExVortex(0, 1, 1)
    elseif mKey == 5 then pDMBarrageSpiral(math.random(2, 4), 0.4, 1)
    elseif mKey == 6 then pRandomBarrage(math.random(1, 3), 2.25)
    elseif mKey == 7 then pInverseBarrage(0)
    elseif mKey == 8 then pMirrorWallStrip(1, 0)
    elseif mKey == 9 then hmpSpinner(1, clamp(level, 1, 6))
    elseif mKey == 10 then hmpBarrageSpiral(math.random(1, 3), 2, clamp(level + 1, 2, 5))
    elseif mKey == 11 then hmcDef2CageD()
    elseif mKey == 12 then hmpBarrageSpiralSpin(math.random(4, 8), 2, clamp(level + 1, 2, 5))
    elseif mKey == 13 then hmpBarrageSpiralStop(math.random(2, 4), 2, 6);
    elseif mKey == 14 then hmcBarrageInv(1, clamp(level + 1, 2, 5))
    elseif mKey == 15 then hmpStripeSnakeBarrage(getRandomSide(), math.random(3, 6))
    elseif mKey == 16 then hmpStripeSnakeAltBarrage(math.random(5, 8), 2)
    elseif mKey == 17 then hmpGrowTunnel(math.random(2, 3))
    elseif mKey == 18 then hmpAssembleTunnel(math.random(2, 3))
    elseif mKey == 19 then pSwapBarrage(getRandomSide())
    elseif mKey == 20 then pSwapCorridor(math.random(2, 3))
    elseif mKey == 21 then hmpTwirl(math.random(2, 3), clamp(level + 1, 2, 5), 1)

    -- Special-exclusive patterns
    -- Assemble Pattern
    elseif mKey == 101 then
        hmcAssembleBarrage(getRandomSide(), 1, 4)
        t_wait(getPerfectDelay(getPerfectThickness(THICKNESS)) * 4.5)
    -- Chaser Pattern
    elseif mKey == 102 then
        hmpChaserAltBarrage(math.random(1, 3), 2, math.random(getHalfSides(), l_getSides() - 1), true);
    -- Turnaround pattern
    elseif mKey == 103 then
        sideSpawn = side + math.random(0, 1) * getHalfSides();
        local revealChance = math.random(1, 3)
        hmcTurnaroundSector(sideSpawn, revealChance == 1)
    -- Alternate pattern
    elseif mKey == 104 then
        hmpAlternate()
    -- Tunnel exclusive pattern
    elseif mKey == 105 then
        hmpTunnelSpinner(math.random(1, 3), 2, clamp(level + 1, 2, 5))
    elseif mKey == 106 then
        hmpSwarm(clamp(level, 1, 6), 3 * clamp(level, 1, 6), true, true, level > 14)
    -- Consistency pattern
    elseif mKey == 107 then
        hmpTwirl(10, clamp(level, 1, 4), 1)
    end
end

level = 1;

-- shuffle the keys, and then call them to add all the patterns
-- shuffling is better than randomizing - it guarantees all the patterns will be called
keys = { 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 9, 10, 11, 12, 13, 14, 15, 16, 16, 16, 17, 18, 19, 19, 19, 20, 20, 21, 21 }
specialKeys = {} -- For specials.
shuffle(keys)
index = 1
specialIndex = 1
achievementUnlocked = false
hardAchievementUnlocked = false

specials = { "assemble", "turnaround", "alternate", "chaser", "tunnel", "swarm", "consistency"}
shuffle(specials)
currSpecial = 1;
special = "none"

-- onInit is an hardcoded function that is called when the level is first loaded
function onInit()
    l_setSpeedMult(2.7)
    l_setSpeedInc(0)
    l_setSpeedMax(3.5)
    l_setRotationSpeed(0.25)
    l_setRotationSpeedMax(0.7)
    l_setRotationSpeedInc(0)
    l_setDelayMult(1.35)
    l_setDelayInc(0.0)
    l_setFastSpin(71.0)
    l_setSides(6)
    l_setSidesMin(6)
    l_setSidesMax(6)
    l_setIncTime(20)

    l_setPulseMin(61.01)
    l_setPulseMax(80.48)
    l_setPulseSpeed(2.4)
    l_setPulseSpeedR(1.449)
    l_setPulseDelayMax(6.8)

    l_setBeatPulseMax(18)
    l_setBeatPulseDelayMax(28.346)

    l_setSwapEnabled(true)
    l_setSwapCooldownMult(0.6)
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

local accelFlag = false

-- onIncrement is an hardcoded function that is called when the level difficulty is incremented
function onIncrement()
    if (special == "none") then
        special = specials[currSpecial]
        currSpecial = currSpecial + 1
        if (currSpecial - 1 == #specials) then
            currSpecial = 1
            shuffle(specials)
        end
        specialIndex = 1
        -- If branch to set up keys
        if (special == "turnaround") then
            side = getRandomSide();
            specialKeys = {103};
        elseif (special == "assemble") then
            specialKeys = {101};
        elseif (special == "alternate") then
            specialKeys = {104};
        elseif (special == "chaser") then
            specialKeys = {102};
        elseif (special == "tunnel") then
            specialKeys = {3, 17, 18, 105};
        elseif (special == "swarm") then
            specialKeys = {106};
            if (level > 14 and not accelFlag) then
                e_messageAddImportant("Accelerating Speed Enabled!", 90)
                accelFlag = true
            end
        elseif (special == "consistency") then
            specialKeys = {107}
        end
        shuffle(specialKeys);
        l_setIncTime(10);
        e_messageAddImportant("Special: "..special, 90);

        -- Enable the speed and rotation increment
        l_setSpeedInc(0.05)
        l_setRotationSpeedInc(0.015)
    else
        level = level + 1;
        special = "none";
        l_setIncTime(20);
        -- Disable the speed and rotation increment
        l_setSpeedInc(0)
        l_setRotationSpeedInc(0)
    end
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
            dirChangeTime = 200
        end
    end

    if not achievementUnlocked and l_getLevelTime() > 60 and u_getDifficultyMult() >= 1 then
        steam_unlockAchievement("a20_massacre")
        achievementUnlocked = true
    end

    if not hardAchievementUnlocked and l_getLevelTime() > 30 and u_getDifficultyMult() > 1.5 then
        steam_unlockAchievement("a46_massacre_hard")
        hardAchievementUnlocked = true
    end
end
