-- include useful files
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "utils.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "common.lua")

-- audio time
timer = "0s"

-- onInit is an hardcoded function that is called when the level is first loaded
function onInit()
    l_setSpeedMult(2)
    l_setRotationSpeed(0.25)
    l_setDelayMult(1)
    l_setSides(6)

    l_setPulseMin(60)
    l_setPulseMax(60)
    l_setPulseSpeed(1.0)
    l_setPulseSpeedR(0.6)
    l_setPulseDelayMax(0)

    l_setBeatPulseMax(25)
    l_setBeatPulseDelayMax(getBPMToBeatPulseDelay(131))

    l_setIncEnabled(false)
    l_enableRndSideChanges(false)

    l_addTracked("timer", "Playback")
end

-- wall offset calculation to match the beat
-- becomes offset when difficulty multiplier value is not 1
function calculateWallOffset(speedMult)
    currentSpeedMult = speedMult or l_getSpeedMult()
    return (l_getWallSpawnDistance() - (l_getRadiusMin() * l_getPulse() / l_getPulseMin()) + l_getBeatPulse()) / (5 * currentSpeedMult * u_getDifficultyMult() ^ 0.65)
end

-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()
    -- 131 BPM 1/4 in ms
    BEAT_OFFSET = 458

    -- timed message offset in ms
    MESSAGE_OFFSET = 10000

    -- initial offset to respect start delay
    offset = calculateWallOffset()

    -- timed message
    a_evalToMs(MESSAGE_OFFSET, [[
        e_messageAddImportant("Timed message at " .. tostring(MESSAGE_OFFSET / 1000) ..
        " seconds\n(took " .. tostring(a_getMusicMilliseconds() - MESSAGE_OFFSET) ..
        " milliseconds)", 250)
    ]])

    -- first pattern (with initial delay)
    for i = 1, 10, 1
    do
        a_evalToMs(offset, [[ cWall(u_rndInt(1, l_getSides())) ]])
        offset = offset + BEAT_OFFSET
    end

    -- second pattern
    for i = 1, 16, 1
    do
        a_evalToMs(offset, [[ cBarrageOnlyN(u_rndInt(1, l_getSides()), 2) ]])
        offset = offset + BEAT_OFFSET
    end

    -- first pattern to loop back at the start
    for i = 1, 6, 1
    do
        a_evalToMs(offset, [[ cWall(u_rndInt(1, l_getSides())) ]])
        offset = offset + BEAT_OFFSET
    end

    CUSTOM_SPEED_MULT = 2

    offset = calculateWallOffset(CUSTOM_SPEED_MULT) + (BEAT_OFFSET * 14)

    -- additional pattern between 1/4 beat
    for i = 1, 8, 1
    do
        a_evalToMs(offset, [[
            w_wallHModSpeedData(1, u_rndInt(1, l_getSides()), THICKNESS, CUSTOM_SPEED_MULT, 0, 0, 0, 0)
        ]])
        -- a_evalToMs(offset, [[ w_wallAdj(u_rndInt(1, l_getSides()), THICKNESS, CUSTOM_SPEED_MULT) ]])
        offset = offset + (BEAT_OFFSET * 2)
    end

    offset = BEAT_OFFSET * 17

    -- additional effects between 1/4 beat
    for i = 1, 8, 1
    do
        a_evalToMs(offset, [[ u_setFlashEffect(75) ]])
        offset = offset + (BEAT_OFFSET * 2)
    end
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

-- onUpdate is an hardcoded function that is called every frame
function onUpdate(mFrameTime)
    timer = tostring(a_getMusicSeconds()) .. " s"
end
