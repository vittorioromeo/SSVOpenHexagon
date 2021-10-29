-- include useful files
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "utils.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "common.lua")

THICKNESS = 800.0;

local hueModifier = 0.2
local sync = false
local syncRndMin = 0
local syncRndMax = 0

local curveMult = 1

function wallHMCurveAcc(mSide, mCurve, mCurveAcc, mCurveMin, mCurveMax, mCurvePingPong)
    if sync == true then
        mCurve = l_getRotationSpeed() * 10.0
        mCurve = mCurve + (u_rndInt(syncRndMin, syncRndMax) / 100.0)
    end

    w_wallHModCurveData(hueModifier, mSide, THICKNESS, mCurve * (u_getDifficultyMult() ^ 0.25) * curveMult, mCurveAcc, mCurveMin, mCurveMax, mCurvePingPong)
end

function wallHMCurve(mSide, mCurve)
    wallHMCurveAcc(mSide, mCurve, 0, 0, 0, false)
end

function hmcSimpleSpinner(mCurve)
    local side = getRandomSide()

    for i = 0, l_getSides() / 2, 1 do
        wallHMCurve(side + i * 2, mCurve)
    end
end

function hmcDefSpinner()
    hmcSimpleSpinner(u_rndInt(10, 45) / 10.0 * getRandomDir())
    t_wait(getPerfectDelay(THICKNESS) * 1.2)
end

-- onInit is an hardcoded function that is called when the level is first loaded
function onInit()
    l_setSpeedMult(1.7)
    l_setSpeedInc(0.15)
    l_setSpeedMax(2.9)
    l_setRotationSpeed(0.1)
    l_setRotationSpeedMax(0.415)
    l_setRotationSpeedInc(0.035)
    l_setDelayMult(1.2)
    l_setDelayInc(0.0)
    l_setFastSpin(0.0)
    l_setSides(6)
    l_setSidesMin(6)
    l_setSidesMax(6)
    l_setIncTime(15)

    l_setPulseMin(77)
    l_setPulseMax(95)
    l_setPulseSpeed(1.95)
    l_setPulseSpeedR(0.51)
    l_setPulseDelayMax(13)

    l_setBeatPulseMax(17)
    l_setBeatPulseDelayMax(27.8)

    l_setSwapEnabled(true)
end

-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()
end

-- onStep is an hardcoded function that is called when the level timeline is empty
-- onStep should contain your pattern spawning logic
function onStep()
    hmcDefSpinner()
    t_wait(50)
end

-- onIncrement is an hardcoded function that is called when the level difficulty is incremented
function onIncrement()
end

-- onUnload is an hardcoded function that is called when the level is closed/restarted
function onUnload()
end

-- onUpdate is an hardcoded function that is called every frame
function onUpdate(mFrameTime)
end
