u_execScript("common.lua")
u_execScript("commonpatterns.lua")
u_execScript("utils.lua")
u_execScript("alternativepatterns.lua")
u_execScript("nextpatterns.lua")

local hueModifier = 0.2
local sync = false
local syncRndMin = 0
local syncRndMax = 0

local curveMult = 1

function syncCurveWithRotationSpeed(mRndMin, mRndMax)
    sync = true
    syncRndMin = mRndMin
    syncRndMax = mRndMax
end

function setCurveMult(mMult)
    curveMult = mMult
end

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

function hmcBarrageN(mSide, mNeighbors, mCurve, mCurveAcc, mCurveMin, mCurveMax, mCurvePingPong)
    for i = mNeighbors, l_getSides() - 2 - mNeighbors, 1 do
        wallHMCurveAcc(mSide + i + 1, mCurve, mCurveAcc, mCurveMin, mCurveMax, mCurvePingPong)
    end
end

function hmcBarrageS(mSide, mCurve, mCurveAcc, mCurveMin, mCurveMax, mCurvePingPong)
    hmcBarrageN(mSide, 0, mCurve, mCurveAcc, mCurveMin, mCurveMax, mCurvePingPong);
end

function hmcBarrage(mCurve, mCurveAcc, mCurveMin, mCurveMax, mCurvePingPong)
    hmcBarrageS(getRandomSide(), mCurve, mCurveAcc, mCurveMin, mCurveMax, mCurvePingPong);
end

function hmcSimpleBarrage(mCurve)
    hmcBarrageN(getRandomSide(), 0, mCurve, 0, 0, 0, false);
end

function hmcSimpleBarrageS(mSide, mCurve)
    hmcBarrageN(mSide, 0, mCurve, 0, 0, 0, false);
end

function hmcSimpleBarrageSNeigh(mSide, mCurve, mNeighbors)
    hmcBarrageN(mSide, mNeighbors, mCurve, 0, 0, 0, false);
end


function hmcSimpleTwirl(mTimes, mCurve, mCurveAdd)
    local startSide = getRandomSide()
    local currentSide = startSide
    local loopDir = getRandomDir()
    local delay = getPerfectDelayDM(THICKNESS) * 5.7
    local j = 0

    local currentCurve = mCurve

    for i = 0, mTimes do
        hmcSimpleBarrageS(startSide + j, currentCurve)
        j = j + loopDir
        currentCurve = currentCurve + mCurveAdd
        t_wait(delay)
    end
end

function hmcSimpleCage(mCurve, mDir)
    local side = getRandomSide()
    local oppositeSide = side + getHalfSides()

    wallHMCurve(side, mCurve)
    wallHMCurve(oppositeSide, mCurve * mDir)
end

function hmcSimpleCageS(mCurve, mDir, mSide)
    local oppositeSide = mSide + getHalfSides()

    wallHMCurve(mSide, mCurve)
    wallHMCurve(oppositeSide, mCurve * mDir)
end

function hmcSimpleSpinner(mCurve)
    local side = getRandomSide()

    for i = 0, l_getSides() / 2, 1 do
        wallHMCurve(side + i * 2, mCurve)
    end
end

function hmcSimpleSpinnerS(mSide, mCurve)
    for i = 0, l_getSides() / 2, 1 do
        wallHMCurve(mSide + i * 2, mCurve)
    end
end

function hmcSimpleSpinnerSAcc(mSide, mCurve, mCurveAcc, mCurveMin, mCurveMax, mCurvePingPong)
    for i = 0, l_getSides() / 2, 1 do
        wallHMCurveAcc(mSide + i * 2, mCurve, mCurveAcc, mCurveMin, mCurveMax, mCurvePingPong)
    end
end

function hmcDefSpinner()
    t_wait(getPerfectDelayDM(THICKNESS) * 3.2)
    hmcSimpleSpinner(u_rndInt(10, 19) / 10.0 * getRandomDir())
    t_wait(getPerfectDelayDM(THICKNESS) * 5.9)
end

function hmcDefBarrage()
    t_wait(getPerfectDelayDM(THICKNESS) * 3.1)
    hmcSimpleBarrage(u_rndInt(10, 20) / 10.0 * getRandomDir())
    t_wait(getPerfectDelayDM(THICKNESS) * 5)
end

function hmcDef2Cage()
    t_wait(getPerfectDelayDM(THICKNESS) * 2.1)
    local side = getRandomSide()
    local rndspd = u_rndInt(10, 20) / 10.0

    t_wait(getPerfectDelayDM(THICKNESS) * 3.1)
    hmcSimpleCageS(rndspd, -1, side)
    t_wait(getPerfectDelayDM(THICKNESS) * 1.1)
    hmcSimpleCageS(rndspd, -1, side)
    t_wait(getPerfectDelayDM(THICKNESS) * 1.1)
    hmcSimpleCageS(rndspd, -1, side)
    t_wait(getPerfectDelayDM(THICKNESS) * 5.3)
end

function hmcDef2CageD()
    t_wait(getPerfectDelayDM(THICKNESS) * 2.1)

    local side = getRandomSide()
    local oppositeSide = getHalfSides() + side
    local rndspd = u_rndInt(10, 17) / 10.0

    t_wait(getPerfectDelayDM(THICKNESS) * 3.1)
    hmcSimpleCageS(rndspd, -1, side)
    t_wait(getPerfectDelayDM(THICKNESS) * 1.1)
    hmcSimpleCageS(rndspd, -1, side)
    t_wait(getPerfectDelayDM(THICKNESS) * 1.1)
    hmcSimpleCageS(rndspd, -1, side)
    t_wait(getPerfectDelayDM(THICKNESS) * 6.0)
    hmcSimpleCageS(rndspd, -1, oppositeSide)
    t_wait(getPerfectDelayDM(THICKNESS) * 1.1)
    hmcSimpleCageS(rndspd, -1, oppositeSide)
    t_wait(getPerfectDelayDM(THICKNESS) * 1.1)
    hmcSimpleCageS(rndspd, -1, oppositeSide)
    t_wait(getPerfectDelayDM(THICKNESS) * 9.2)
end

function hmcSimpleBarrageSpiral(mTimes, mDelayMult, mStep, mCurve, mNeighbors)
    local delay = getPerfectDelayDM(THICKNESS) * 6.2 * mDelayMult
    local startSide = getRandomSide()
    local loopDir = mStep * getRandomDir()
    local j = 0

    for i = 0, mTimes do
        hmcSimpleBarrageSNeigh(startSide + j, mCurve, mNeighbors)
        j = j + loopDir
        t_wait(delay)
        if(l_getSides() < 6) then t_wait(delay * 0.7) end
    end

    t_wait(getPerfectDelayDM(THICKNESS) * 6.1)
end

function hmcSimpleBarrageSpiralRnd(mTimes, mDelayMult, mCurve, mNeighbors)
    local delay = getPerfectDelayDM(THICKNESS) * 6.2 * mDelayMult
    local startSide = getRandomSide()

    for i = 0, mTimes do
        hmcSimpleBarrageSNeigh(getRandomSide(), mCurve, mNeighbors)
        t_wait(delay)
        if(l_getSides() < 6) then t_wait(delay * 0.7) end
    end

    t_wait(getPerfectDelayDM(THICKNESS) * 6.1)
end

function hmcSimpleBarrageSpiralStatic(mTimes, mDelayMult, mStep, mCurve, mNeighbors)
    local delay = getPerfectDelay(THICKNESS) * 5.6 * mDelayMult
    local startSide = getRandomSide()
    local loopDir = mStep * getRandomDir()
    local j = 0

    for i = 0, mTimes do
        hmcSimpleBarrageSNeigh(startSide + j, mCurve, mNeighbors)
        j = j + loopDir
        t_wait(delay)
        if(l_getSides() < 6) then t_wait(delay * 0.6) end
    end

    t_wait(getPerfectDelayDM(THICKNESS) * 6.1)
end

function hmcDefBarrageSpiral()
    hmcSimpleBarrageSpiral(u_rndInt(1, 3), 1, 1, u_rndInt(5, 15) / 10.0 * getRandomDir(), 0)
end

function hmcDefBarrageSpiralRnd()
    hmcSimpleBarrageSpiralRnd(u_rndInt(1, 3), 1, u_rndInt(5, 15) / 10.0 * getRandomDir(), 0)
end

function hmcDefBarrageSpiralFast()
    hmcSimpleBarrageSpiral(u_rndInt(1, 3), 0.8, 1, u_rndInt(5, 15) / 10.0 * getRandomDir(), 0)
end

function hmcDefBarrageSpiralSpin()
    hmcSimpleBarrageSpiralStatic(u_rndInt(7, 14), 0.25, 1, u_rndInt(5, 18) / 10.0 * getRandomDir(), 2)
end

function hmcDefBarrageInv()
    t_wait(getPerfectDelayDM(THICKNESS) * 2.0)
    local delay = getPerfectDelay(THICKNESS) * 5.6
    local side = getRandomSide()
    local rndspd = u_rndInt(10, 20) / 10.0
    local oppositeSide = getRandomSide() + getHalfSides()

    hmcSimpleBarrageSNeigh(side, rndspd * getRandomDir(), 0)
    t_wait(delay)

    hmcSimpleBarrageSNeigh(oppositeSide, rndspd * getRandomDir(), 0)
    t_wait(delay)
end

function hmcDefAccelBarrage()
    t_wait(getPerfectDelayDM(THICKNESS) * 1.5)
    local c = u_rndInt(50, 100) / 1000.0 * getRandomDir()
    local minimum = u_rndInt(5, 35) / 10.0 * -1
    local maximum = u_rndInt(5, 35) / 10.0
    hmcBarrage(0, c, minimum, maximum, true)
    t_wait(getPerfectDelayDM(THICKNESS) * 6.1)
end

function hmcDefAccelBarrageDouble()
    t_wait(getPerfectDelayDM(THICKNESS) * 1.5)
    local c = u_rndInt(50, 100) / 1000.0 * getRandomDir()
    local minimum = u_rndInt(5, 35) / 10.0 * -1
    local maximum = u_rndInt(5, 35) / 10.0
    hmcBarrage(0, c, minimum, maximum, true)
    t_wait(getPerfectDelayDM(THICKNESS) * 2.1)
    hmcBarrage(0, c, minimum, maximum, true)
    t_wait(getPerfectDelayDM(THICKNESS) * 6.1)
end

function hmcDefSpinnerSpiral()
    t_wait(getPerfectDelayDM(THICKNESS) * 1.5)
    local side = getRandomSide()
    local c = u_rndInt(10, 20) / 10.0 * getRandomDir()

    t_wait(getPerfectDelayDM(THICKNESS) * 3.1)

    for i = 0, u_rndInt(4, 8) do
        hmcSimpleSpinnerS(side, c)
        t_wait(getPerfectDelayDM(THICKNESS) * 1.15)
    end

    t_wait(getPerfectDelayDM(THICKNESS) * 5)
end

function getRndMinDM(mNum)
    return u_rndInt(math.floor(mNum - (u_getDifficultyMult() ^ 3)), math.ceil(mNum))
end

function getRndMaxDM(mNum)
    return u_rndInt(math.floor(mNum), math.ceil(mNum + (u_getDifficultyMult() ^ 2.25)))
end

function hmcDefSpinnerSpiralAcc()
    t_wait(getPerfectDelayDM(THICKNESS) * 2.1)
    t_wait(getPerfectDelayDM(THICKNESS) * 2.1)
    local side = getRandomSide()

    local acc = u_rndInt(getRndMinDM(50), getRndMaxDM(100)) / 1000.0 * getRandomDir()
    local minimum = u_rndInt(getRndMinDM(12), getRndMaxDM(28)) / 10.0 * -1
    local maximum = u_rndInt(getRndMinDM(12), getRndMaxDM(28)) / 10.0



    t_wait(getPerfectDelayDM(THICKNESS) * 3.1)

    for i = 0, u_rndInt(4, 8) do
        hmcSimpleSpinnerSAcc(side, 0, acc, minimum, maximum, true)
        t_wait(getPerfectDelay(THICKNESS) * 0.8)
    end

    t_wait(getPerfectDelayDM(THICKNESS) * 5.3)
end
