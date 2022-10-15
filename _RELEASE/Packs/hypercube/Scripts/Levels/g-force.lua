-- include useful files
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "utils.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "common.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "commonpatterns.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "nextpatterns.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "evolutionpatterns.lua")

function gforceBarrage()
    cBarrage(getRandomSide())
    t_wait(getPerfectDelay(THICKNESS) * 6.1)
end

function gforceBarrageAssault()
    cBarrage(getRandomSide())
    t_wait(getPerfectDelay(THICKNESS) * 3.5)
end


-- this function adds a pattern to the timeline based on a key
function addPattern(mKey)
        if mKey == 0 then hmpDefAccelBarrage()
    elseif mKey == 1 then gforceBarrage()
    end
end

-- shuffle the keys, and then call them to add all the patterns
-- shuffling is better than randomizing - it guarantees all the patterns will be called
keys = { 0, 1 }
shuffle(keys)
index = 0
achievementUnlocked = false
hardAchievementUnlocked = false

specials = { "double", "assault", "incongruence", "dizzy" }
shuffle(specials)
currSpecial = 1
special = "none"

-- onInit is an hardcoded function that is called when the level is first loaded
function onInit()
    l_setSpeedMult(2.1)
    l_setSpeedInc(0) -- Disable it for right now
    l_setSpeedMax(3.9)
    l_setRotationSpeed(0.12)
    l_setRotationSpeedMax(0.65)
    l_setRotationSpeedInc(0.0175)
    l_setDelayMult(1.9)
    l_setDelayInc(0.0)
    l_setFastSpin(0.0)
    l_setSides(4)
    l_setSidesMin(4)
    l_setSidesMax(4)
    l_setIncTime(10)

    l_setWallSkewLeft(-15)

    l_setPulseInitialDelay(13.953 * 2)
    l_setPulseMin(67.62)
    l_setPulseMax(95)
    l_setPulseSpeed(2.791)
    l_setPulseSpeedR(0.83)
    l_setPulseDelayMax(13.01)

    l_setBeatPulseMax(17)
    l_setBeatPulseDelayMax(13.953)

    l_setSwapEnabled(true)
    l_setSwapCooldownMult(1.9/u_getSpeedMultDM())
    l_addTracked("special", "special")
end

-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()
end

-- onStep is an hardcoded function that is called when the level timeline is empty
-- onStep should contain your pattern spawning logic
function onStep()
    if special == "incongruence" then
        l_setSides(u_rndInt(4, 5))
    else
        l_setSides(4)
    end

    if special == "assault" then
        gforceBarrageAssault()
        return
    end

    if special == "dizzy" then
        addPattern(0)
        return
    end

    if special ~= "double" then
        addPattern(keys[index])
    else
        addPattern(keys[index])
        addPattern(keys[index])
    end

    index = index + 1

    if index - 1 == #keys then
        index = 1
        shuffle(keys)
    end
end


-- onIncrement is an hardcoded function that is called when the level difficulty is incremented
function onIncrement()
    if special == "none" then
        special = specials[currSpecial]
        currSpecial = currSpecial + 1
        if (currSpecial - 1 == #specials) then
            currSpecial = 1
            shuffle(specials)
        end
        e_messageAddImportant("Special: "..special, 120)
        l_setSpeedInc(0.16)
    else
        special = "none"
        l_setSpeedInc(0)
    end
    if (special == "assault") then
        l_setSwapCooldownMult(1/u_getSpeedMultDM()) -- Assault has tighter spacing so we need lower swap cooldown.
    else
        l_setSwapCooldownMult(1.9/u_getSpeedMultDM())
    end
end

-- onUnload is an hardcoded function that is called when the level is closed/restarted
function onUnload()
end

-- onUpdate is an hardcoded function that is called every frame
function onUpdate(mFrameTime)
    if not achievementUnlocked and l_getLevelTime() > 90 and u_getDifficultyMult() >= 1 then
        steam_unlockAchievement("a14_gforce")
        achievementUnlocked = true
    end

    if not hardAchievementUnlocked and l_getLevelTime() > 40 and u_getDifficultyMult() > 2.5 then
        steam_unlockAchievement("a40_gforce_hard")
        hardAchievementUnlocked = true
    end
end
