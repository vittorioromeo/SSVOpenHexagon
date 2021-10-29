u_execScript("common.lua")
u_execScript("commonpatterns.lua")
u_execScript("utils.lua")
u_execScript("alternativepatterns.lua")
u_execScript("nextpatterns.lua")

---------------
-- CONSTANTS --
---------------

globalHueModifier = 0.2 -- Constant used to shift the color offset to work with the style's hue.
local sync = false -- Constant used to sync curving walls with that rotation speed
local syncRndMin = 0 -- Minimum random displacement from the synced speed
local syncRndMax = 0 -- Maximum random displacement from the synced speed

local curveMult = 1 -- Constant used to multiply the global curve speed of curving walls

---------------
-- FUNCTIONS --
---------------

-- syncCurveToSideDistance: Returns an appropriate constant that, if applied to a curving wall travelling at constant speed, will cause it go one full side.
function syncCurveToSideDistance()
    return (.2 * u_getSpeedMultDM() + 0.005) * (6 / l_getSides()) * (u_getDifficultyMult() ^ -.25);
end

-- getPerfectCurveDecel: Returns a constant that, if applied to a curving wall, will make the wall stop accelerating on a side panel.
function getPerfectCurveDecel()
    -- Coordinates tested: (0, 0), (1, .134), (1.5, .295), (2, .52), (2.5, .81), (3, 1.162), (3.5, 1.579), (4, 2.06)
    -- Cubic Formula (full accuracy): 0.000233918 x^3 + 0.125985 x^2 + 0.00730493 x + 0.0000701754
    -- Current formula has ~100% accuracy
    return (0.127378 * u_getSpeedMultDM() ^ 2 + 0.00526331 * u_getSpeedMultDM() + 0.000431373) * (6 / l_getSides());
end

-- getRndMinDM: Returns a random integer of a number where the minimum is influenced by the difficulty multiplier
function getRndMinDM(mNum)
    return math.random(math.floor(mNum - (u_getDifficultyMult() ^ 3)), math.ceil(mNum))
end

-- getRndMaxDM: Returns a random integer of a number where the maximum is influenced by the difficulty multiplier
function getRndMaxDM(mNum)
    return math.random(math.floor(mNum), math.ceil(mNum + (u_getDifficultyMult() ^ 2.25)))
end

-- The function to call when wanting to sync curving walls with the rotation speed.
-- mRndMin (OPTIONAL): Sets the syncRndMin const to this value
-- mRndMax (OPTIONAL): Sets the syncRndMax const to this value
function syncCurveWithRotationSpeed(mRndMin, mRndMax)
    sync = true
    syncRndMin = mRndMin or 0
    syncRndMax = mRndMax or 0
end

-- The function to call when wanting to alter curveMult constant
-- mMult: Sets the curveMult const to this value
function setCurveMult(mMult)
    curveMult = mMult
end

-- Creates a curving wall, adhering to the constants presented by this script.
-- mSide: The side to spawn the curving wall on
-- mCurve: The curving speed the wall will travel at
-- mCurveAcc: How much acceleration to apply to the curving wall
-- mCurveMin: The lowest curving speed the wall is allowed to go
-- mCurveMax: The highest curving speed the wall is allowed to go
-- mCurvePingPong: If accelerated to mCurveMin or mCurveMax, should the acceleration switch directions and go the other way?
-- mThickness (OPTIONAL): The thickness of the curving wall.
function wallHMCurveAcc(mSide, mCurve, mCurveAcc, mCurveMin, mCurveMax, mCurvePingPong, mThickness)
    mThickness = mThickness or THICKNESS
    if sync == true then
        mCurve = l_getRotationSpeed() * (10.0 / syncCurveToSideDistance())
        mCurve = mCurve + (math.random(syncRndMin, syncRndMax) / 100.0)
    end
    w_wallHModCurveData(globalHueModifier, mSide, mThickness, mCurve * (u_getDifficultyMult() ^ 0.25) * curveMult * syncCurveToSideDistance(), mCurveAcc, mCurveMin, mCurveMax, mCurvePingPong)
end

-- A simplification of wallHMCurve, only allowing customization of the curve speed
-- mSide: The side to spawn the curving wall on
-- mCurve: The curving speed the wall will travel at
-- mThickness (OPTIONAL): The thickness of the curving wall.
function wallHMCurve(mSide, mCurve, mThickness)
    mThickness = mThickness or THICKNESS
    wallHMCurveAcc(mSide, mCurve, 0, 0, 0, false, mThickness)
end

-- A preset of wallHMCurveAcc where getPerfectCurveDecel is used to spawn a wall to stop acceleration on a side panel.
-- mSide: The side that the wall will stop on.
-- mOffset: How much offset from mSide the wall will have when spawned. A higher offset will make a faster wall.
-- mThickness (OPTIONAL): The thickness of the curving wall.
function wallHMStop(mSide, mOffset, mThickness)
    mThickness = mThickness or THICKNESS
    local curveAcc = getPerfectCurveDecel();
    if (getSign(mOffset) == -1) then
        wallHMCurveAcc(mSide + mOffset, 2 * -mOffset, curveAcc * mOffset / 100, 0, 2 * -mOffset, false, mThickness);
    else
        wallHMCurveAcc(mSide + mOffset, -2 * mOffset, curveAcc * mOffset / 100, -2 * mOffset, 0, false, mThickness);
    end
end

------------------------------------------------------
-- BEGINNING OF HMC FUNCTIONS (Hue Modified Common) --
------------------------------------------------------

-- A barrage with additional adjacent mNeighbors that composes of curving walls.
-- mSide: The side to spawn the curving wall on
-- mNeighbors: The expansion of the barrage opening by this many adjacent neighbors
-- mCurve: The curving speed the wall will travel at
-- mCurveAcc: How much acceleration to apply to the curving wall
-- mCurveMin: The lowest curving speed the wall is allowed to go
-- mCurveMax: The highest curving speed the wall is allowed to go
-- mCurvePingPong: If accelerated to mCurveMin or mCurveMax, should the acceleration switch directions and go the other way?
function hmcBarrageN(mSide, mNeighbors, mCurve, mCurveAcc, mCurveMin, mCurveMax, mCurvePingPong)
    for i = mNeighbors, l_getSides() - 2 - mNeighbors, 1 do
        wallHMCurveAcc(mSide + i + 1, mCurve, mCurveAcc, mCurveMin, mCurveMax, mCurvePingPong)
    end
end

-- hmcBarrageS: hmcBarrageN, but there are no additional neighbors, and the gap is only 1 side wide.
function hmcBarrageS(mSide, mCurve, mCurveAcc, mCurveMin, mCurveMax, mCurvePingPong)
    hmcBarrageN(mSide, 0, mCurve, mCurveAcc, mCurveMin, mCurveMax, mCurvePingPong);
end

-- hmcBarrage: hmcBarrageS, but the side is chosen at random for you.
function hmcBarrage(mCurve, mCurveAcc, mCurveMin, mCurveMax, mCurvePingPong)
    hmcBarrageS(getRandomSide(), mCurve, mCurveAcc, mCurveMin, mCurveMax, mCurvePingPong);
end

-- hmcBarrageStop: A barrage that comes to a full stop, similar to how wallHMStop works.
function hmcBarrageStop(mSide, mOffset, mNeighbors)
    mNeighbors = mNeighbors or 0
    local curveAcc = getPerfectCurveDecel();
    if (getSign(mOffset) == -1) then
        hmcBarrageN(mSide + mOffset, mNeighbors, 2 * -mOffset, curveAcc * mOffset / 100, 0, 2 * -mOffset, false);
    else
        hmcBarrageN(mSide + mOffset, mNeighbors, -2 * mOffset, curveAcc * mOffset / 100, -2 * mOffset, 0, false);
    end
end

-- A simple curving barrage.
-- mSide (OPTIONAL, BUT HIGHLY RECOMMENDED): The side the barrage starts at
-- mCurve (OPTIONAL): The constant curving speed the barrage will travel
-- mNeighbors (OPTIONAL): The expansion of the barrage opening by this many adjacent neighbors
function hmcSimpleBarrage(mSide, mCurve, mNeighbors)
    mSide = mSide or 0
    mCurve = mCurve or 0
    mNeighbors = mNeighbors or 0
    hmcBarrageN(mSide, mNeighbors, mCurve, 0, 0, 0, false);
end

-- Compatibility mappings for legacy functions that did the same thing.
hmcSimpleBarrageS = hmcSimpleBarrage
hmcSimpleBarrageSNeigh = hmcSimpleBarrage

-- A series of barrages that curve at mCurve speed, increasing each iteration by mCurveAdd to recreate a barrage spiral
-- mTimes: How many repetitions of barrages there should be
-- mCurve: The starting curve speed for the pattern
-- mCurveAdd: How much to add to the curving speed each iteration
function hmcSimpleTwirl(mTimes, mCurve, mCurveAdd)
    local startSide = getRandomSide()
    local delay = getPerfectDelay(THICKNESS) * 5.7
    local currentCurve = mCurve

    for i = 0, mTimes do
        hmcSimpleBarrageS(startSide, currentCurve)
        currentCurve = currentCurve + mCurveAdd
        t_wait(delay)
    end
end

-- A wall that curves at mCurve speed, with an opposing wall that travel towards it,
-- "caging" the player if not aware of the movements.
-- mCurve: The curve speed for the pattern
-- mDir: The direction of the curving for the opposing wall.
-- mSide (OPTIONAL): Which side the pattern spawns on.
function hmcSimpleCage(mCurve, mDir, mSide)
    mSide = mSide or getRandomSide()
    local oppositeSide = mSide + getHalfSides()

    wallHMCurve(mSide, mCurve)
    wallHMCurve(oppositeSide, mCurve * mDir)
end

hmcSimpleCageS = hmcSimpleCage

-- An alt barrage that travels at a constant curving speed (Don't ask. Vee came up with the name).
-- mSide: The side the spinner starts at
-- mCurve: The constant curving speed the spinner will travel
-- mStep (OPTIONAL): How much to iterate before spawning the next wall
function hmcSimpleSpinner(mSide, mCurve, mStep)
    mStep = mStep or 2
    for i = 0, l_getSides(), mStep do
        wallHMCurve(mSide + i, mCurve)
    end
end

hmcSimpleSpinnerS = hmcSimpleSpinner

-- hmcSimpleSpinnerSAcc: A spinner but with more options in regards to accelerating the walls.
function hmcSimpleSpinnerSAcc(mSide, mCurve, mCurveAcc, mCurveMin, mCurveMax, mCurvePingPong)
    for i = 0, l_getSides() / 2, 1 do
        wallHMCurveAcc(mSide + i * 2, mCurve, mCurveAcc, mCurveMin, mCurveMax, mCurvePingPong)
    end
end

-- hmcGrowBarrage: Given a specific position to pivot, all walls will spawn there and spread out from that position to make a barrage.
function hmcGrowBarrage(mSide, mPivot)
    for i = 0, l_getSides() - 2 do
        wallHMStop(mSide + i + 1, mPivot - i);
    end
end

-- hmcAssembleBarrage: All individual walls will be randomly offset and then assemble together to create a barrage.
function hmcAssembleBarrage(mSide, mLower, mUpper)
    mLower = mLower or 1
    mUpper = mUpper or l_getSides()
    for i = 0, l_getSides() - 2 do
        wallHMStop(mSide + i + 1, math.random(mLower, mUpper) * getRandomDir());
    end
end

-- hmcDef2Cage: Spawns a series of hmcSimpleCage to create a trailing effect
function hmcDef2Cage()
    local side = getRandomSide()
    local rndspd = math.random(1, 5)

    t_wait(getPerfectDelay(THICKNESS) * 5.2)
    hmcSimpleCageS(rndspd, -1, side)
    t_wait(getPerfectDelay(THICKNESS) * 1.1)
    hmcSimpleCageS(rndspd, -1, side)
    t_wait(getPerfectDelay(THICKNESS) * 1.1)
    hmcSimpleCageS(rndspd, -1, side)
    t_wait(getPerfectDelay(THICKNESS) * 5.3)
end

-- hmcDef2CageD: A doubled version of hmcDef2Cage, creating another one opposite of the first pattern
function hmcDef2CageD()
    local side = getRandomSide()
    local oppositeSide = getHalfSides() + side
    local rndspd = math.random(1, 5)

    t_wait(getPerfectDelay(THICKNESS) * 5.2)
    hmcSimpleCageS(rndspd, -1, side)
    t_wait(getPerfectDelay(THICKNESS) * 1.1)
    hmcSimpleCageS(rndspd, -1, side)
    t_wait(getPerfectDelay(THICKNESS) * 1.1)
    hmcSimpleCageS(rndspd, -1, side)
    t_wait(getPerfectDelay(THICKNESS) * 6.0)
    hmcSimpleCageS(rndspd, -1, oppositeSide)
    t_wait(getPerfectDelay(THICKNESS) * 1.1)
    hmcSimpleCageS(rndspd, -1, oppositeSide)
    t_wait(getPerfectDelay(THICKNESS) * 1.1)
    hmcSimpleCageS(rndspd, -1, oppositeSide)
    t_wait(getPerfectDelay(THICKNESS) * 9.2)
end

-- A barrage spiral that travels at a constant curving speed.
-- mTimes: The amount of barrages to spawn during a barrage
-- mDelayMult: A custom delay multiplier used to customize spacing between barrages
-- mStep: How much sides to increment from barrage to barrage
-- mCurve: The curve speed for the pattern
-- mNeighbors (OPTIONAL): The expansion of the barrage opening by this many adjacent neighbors
function hmcSimpleBarrageSpiral(mTimes, mDelayMult, mStep, mCurve, mNeighbors)
    mNeighors = mNeighbors or 0
    local delay = getPerfectDelay(THICKNESS) * 6.2 * mDelayMult
    local startSide = getRandomSide()
    local loopDir = mStep * getRandomDir()
    local j = 0

    for i = 0, mTimes do
        hmcSimpleBarrageSNeigh(startSide + j, mCurve, mNeighbors)
        j = j + loopDir
        t_wait(delay)
        if(l_getSides() < 6) then t_wait(delay * 0.7) end
    end

    t_wait(getPerfectDelay(THICKNESS) * 6.1)
end

-- getPerfectDelay now has the same implementation as getPerfectDelay, so this pattern is now redundant.
hmcSimpleBarrageSpiralStatic = hmcSimpleBarrageSpiral

-- hmcBarrageInv: pInverseBarrage but with curving walls.
function hmcBarrageInv(mMinCurve, mMaxCurve)
    t_wait(getPerfectDelay(THICKNESS) * 2.0)
    local delay = getPerfectDelay(THICKNESS) * 8
    local side = getRandomSide()
    local rndSpd = math.random(mMinCurve, mMaxCurve);
    local oppositeSide = getRandomSide() + getHalfSides()

    hmcSimpleBarrageSNeigh(side, rndSpd * getRandomDir(), 0)
    t_wait(delay)

    hmcSimpleBarrageSNeigh(oppositeSide, rndSpd * getRandomDir(), 0)
    t_wait(delay)
end

-- hmcTunnelDynamic: A single segment tunnel piece where the tunnel (the thick wall) moves into a random position.
function hmcTunnelDynamic(mBarrageSide, mTunnelSide, mOffset)
    local offset = mOffset or math.random(3, l_getSides());
    local myThickness = getPerfectThickness(THICKNESS)
    local delay = getPerfectDelay(myThickness) * 4

    cBarrage(mBarrageSide);
    t_wait(getPerfectDelay(THICKNESS) - 1)
    wallHMStop(mTunnelSide % l_getSides(), offset, myThickness + 6 * u_getSpeedMultDM() * delay - myThickness * 2)
    t_wait(delay)
end

-- hmcTurnaroundSector: Spawns a segment with two openings. One of the openings will have a dead end and force the player to swap.
-- mReveal determines whether the dead end should reveal itself from a hidden spot.
function hmcTurnaroundSector(mSide, mReveal)
    mReveal = mReveal or false;

    local myThickness = getPerfectThickness(THICKNESS)
    local delay = getPerfectDelay(myThickness) * 2.5

    if (mReveal) then
        wallHMStop(mSide, math.random(1, l_getSides()) * getRandomDir());
    else
        cWall(mSide)
    end
    for i = 1, getHalfSides() - 1 do
        w_wall(mSide + i, myThickness + 3.8 * u_getSpeedMultDM() * delay)
        w_wall(mSide + i, -myThickness * 1.5)
    end
    for i = getHalfSides() + 1, l_getSides() - 1 do
        w_wall(mSide + i, myThickness + 3.8 * u_getSpeedMultDM() * delay)
        w_wall(mSide + i, -myThickness * 1.5)
    end
    t_wait(delay)
end

--------------------------------------------------------
-- BEGINNING OF HMP FUNCTIONS (Hue Modified Patterns) --
--------------------------------------------------------

-- A patternized hmcSimpleBarrage, with a randomized side placement and optional speed randomization set by the user
-- mMin (OPTIONAL): The lowest speed the barrage can travel
-- mMax (OPTIONAL): The highest speed the barrage can travel
function hmpBarrage(mMin, mMax)
    mMin = mMin or 1;
    mMax = mMax or l_getSides() - 1;
    hmcSimpleBarrage(getRandomSide(), math.random(mMin, mMax) * getRandomDir())
    t_wait(getPerfectDelay(THICKNESS) * 8.1)
end

-- A patternized hmcBarrageStop, with a randomized side placement and optional offset randomization set by the user
-- mOffsetMin (OPTIONAL): The lowest offset a barrage can be placed
-- mOffsetMax (OPTIONAL): The highest offset a barrage can be placed
function hmpBarrageStop(mOffsetMin, mOffsetMax)
    mOffsetMin = mOffsetMin or 2;
    mOffsetMax = mOffsetMax or l_getSides() * 2;
    hmcBarrageStop(getRandomSide(), math.random(mOffsetMin, mOffsetMax) * getRandomDir());
    t_wait(getPerfectDelay(THICKNESS) * 9)
end

-- A patternized hmcSimpleSpinner, with a randomized side placement and optional speed randomization set by the user
-- mMin (OPTIONAL): The lowest speed the barrage can travel
-- mMax (OPTIONAL): The highest speed the barrage can travel
function hmpSpinner(mMin, mMax)
    mMin = mMin or 1;
    mMax = mMax or l_getSides() - 1;
    hmcSimpleSpinner(getRandomSide(), math.random(mMin, mMax) * getRandomDir())
    t_wait(getPerfectDelay(THICKNESS) * 9.1)
end

-- hmpTwirl: A patternized hmcSimpleTwirl that has a shorter delay between barrages and curve and curveAdd must go in the same direction.
function hmpTwirl(mTimes, mCurve, mCurveAdd)
    local startSide = getRandomSide()
    local delay = getPerfectDelay(THICKNESS) * 4
    local dir = getRandomDir()
    local currentCurve = mCurve * dir

    for i = 0, mTimes do
        hmcSimpleBarrageS(startSide, currentCurve)
        currentCurve = currentCurve + mCurveAdd * dir
        t_wait(delay)
    end
    t_wait(delay * 1.5)
end

-- A patternized hmcSimpleBarrageSpiral, with a randomized side placement and optional speed randomization set by the user
-- mTimes (OPTIONAL): The amount of barrages to spawn
-- mMinCurve (OPTIONAL): The lowest speed the pattern can travel
-- mMaxCurve (OPTIONAL): The highest speed the pattern can travel
function hmpBarrageSpiral(mTimes, mMinCurve, mMaxCurve)
    mTimes = mTimes or math.random(1, 3);
    mMinCurve = mMinCurve or 1;
    mMaxCurve = mMaxCurve or l_getSides() - 1;
    hmcSimpleBarrageSpiral(mTimes, 1, 1, math.random(mMinCurve, mMaxCurve) * getRandomDir(), 0)
end

-- An hmcSimpleBarrageSpiral, but it uses hmcBarrageStop instead of normal hmcBarrages.
-- mTimes (OPTIONAL): The amount of barrages to spawn
-- mMinCurve (OPTIONAL): The lowest offset each barrage can travel
-- mMaxCurve (OPTIONAL): The highest offset each barrage can travel
function hmpBarrageSpiralStop(mTimes, mMinCurve, mMaxCurve)
    mTimes = mTimes or math.random(1, 3);
    mMinCurve = mMinCurve or 1;
    mMaxCurve = mMaxCurve or l_getSides() - 1;

    local side = getRandomSide()
    local loopDir = getRandomDir()
    local delay = getPerfectDelay(THICKNESS) * 6.2

    for i = 0, mTimes do
        hmcBarrageStop(side + i * loopDir, math.random(mMinCurve, mMaxCurve) * getRandomDir());
        t_wait(delay)
        if(l_getSides() < 6) then t_wait(delay * 0.7) end
    end

    t_wait(getPerfectDelay(THICKNESS) * 6.1)
end

-- hmpBarrageSpiralSpin: A subset of hmcSimpleBarrageSpiral to spawn barrages with 25% the normal delay
function hmpBarrageSpiralSpin(mTimes, mMinCurve, mMaxCurve)
    mTimes = mTimes or math.random(7, 14);
    mMinCurve = mMinCurve or 1;
    mMaxCurve = mMaxCurve or l_getSides() - 1;
    hmcSimpleBarrageSpiralStatic(mTimes, 0.25, 1, math.random(mMinCurve, mMaxCurve) * getRandomDir(), 2)
end

-- hmpDefAccelBarrage: A barrage that is randomized in curving speed, minimum, and maximum accelerations.
function hmpDefAccelBarrage()
    t_wait(getPerfectDelay(THICKNESS) * 1.5)
    local c = math.random(50, 100) / 1000.0 * getRandomDir()
    local minimum = math.random(5, 35) / 10.0 * -1
    local maximum = math.random(5, 35) / 10.0
    hmcBarrage(0, c, minimum, maximum, true)
    t_wait(getPerfectDelay(THICKNESS) * 6.1)
end

-- hmpTunnelDynamic: A segment of hmcTunnelDynamics in a single pattern, written to avoid impossible outcomes.
function hmpTunnelDynamic(mTimes, mLower, mUpper)
    mLower = mLower or 2
    mUpper = mUpper or l_getSides();
    local barrageSide, prevSide, tunnelSide;
    for i = 1, mTimes + 1 do
        if (prevSide ~= nil) then
            repeat barrageSide = getRandomSide() until barrageSide ~= prevSide;
        else
            barrageSide = getRandomSide();
        end
        repeat tunnelSide = getRandomSide() until tunnelSide ~= barrageSide;
        if (i < mTimes + 1) then
            hmcTunnelDynamic(barrageSide, tunnelSide, math.random(mLower, mUpper) * getRandomDir());
        else
            cBarrage(barrageSide);
        end
        prevSide = tunnelSide;
    end

    t_wait(getPerfectDelay(getPerfectThickness(THICKNESS)) * 5.6)
end

-- hmpStripeSnakeBarrage: Given a specific offset, this pattern creates repetitions of a barrage with a given offset to drift from.
function hmpStripeSnakeBarrage(mSide, mRepetitions, mLower, mUpper)
    mLower = mLower or l_getSides()
    mUpper = mUpper or l_getSides() * 2
    local offset = math.random(mLower, mUpper) * getRandomDir()
    local delay = getPerfectDelay(THICKNESS)
    for r = 0, mRepetitions do
        for i = 0, l_getSides() - 2 do
            wallHMStop(mSide + i + 1, offset)
        end
        t_wait(delay)
    end
    t_wait(delay * 7)
end

-- hmpStripeSnakeBarrage: Given a specific offset, this pattern creates repetitions of a single alt barrage with a given offset to drift from.
function hmpStripeSnakeAltBarrage(mRepetitions, mStep, mLower, mUpper)
    mLower = mLower or l_getSides()
    mUpper = mUpper or l_getSides() * 2
    local side = getRandomSide()
    local offset = math.random(mLower, mUpper) * getRandomDir()
    local delay = getPerfectDelay(THICKNESS)
    for r = 0, mRepetitions do
        for i = 0, l_getSides(), mStep do
            wallHMStop(side + i, offset)
        end
        t_wait(delay)
    end
    t_wait(delay * 7)
end

-- hmpGrowTunnel: A tunnel pattern where the barrages consist of hmcGrowBarrages instead of regular ones.
function hmpGrowTunnel(mTimes)
    local myThickness = getPerfectThickness(THICKNESS)
    local delay = getPerfectDelay(myThickness) * 4.5
    local startSide = getRandomSide()
    local barrageSide, offset;

    for i = 0, mTimes do
        repeat barrageSide = getRandomSide() until barrageSide ~= startSide
        offset = (l_getSides() - (barrageSide - startSide + 1)) % (l_getSides())
        hmcGrowBarrage(barrageSide, offset)
        w_wall(startSide, -myThickness) -- Wall to prevent visual artifacts
        if i < mTimes then
            w_wall(startSide, myThickness + 5 * u_getSpeedMultDM() * delay)
        else
            w_wall(startSide, myThickness)
        end
        t_wait(delay)
    end
end

-- hmpAssembleTunnel: A tunnel pattern where the barrages consist of hmcAssembleBarrages instead of regular ones.
function hmpAssembleTunnel(mTimes, mLower, mUpper)
    mLower = mLower or 1
    mUpper = mUpper or getHalfSides()

    local myThickness = getPerfectThickness(THICKNESS)
    local delay = getPerfectDelay(myThickness) * 4.5
    local startSide = getRandomSide()
    local barrageSide;

    for i = 0, mTimes do
        repeat barrageSide = getRandomSide() until barrageSide ~= startSide
        hmcAssembleBarrage(barrageSide, mLower, mUpper)
        w_wall(startSide, -myThickness) -- Wall to prevent visual artifacts
        if i < mTimes then
            w_wall(startSide, myThickness + 5 * u_getSpeedMultDM() * delay)
        else
            w_wall(startSide, myThickness)
        end
        t_wait(delay)
    end
end

-- hmpChaserAltBarrage: pAltBarrage, but a huge tunnel curving wall moves throughout the pattern, restricting where the player moves.
function hmpChaserAltBarrage(mTimes, mStep, mSpeed, mTail)
    mSpeed = mSpeed or math.random(getHalfSides(), l_getSides());
    mTail = mTail or false -- If true, the tunnel extends past the last alt barrage. Done for connectivity purposes
    mTail = mTail and 1 or 0 -- Convert boolean to integer
    local myThickness = getPerfectThickness(THICKNESS)
    local delay = getPerfectDelay(myThickness) * 3

    -- Create the tunnel wall (chaser)
    -- For the side, we want to make sure that the chaser is on one of the first alt barrage walls (which is odd numbers)
    -- This makes the pattern slightly easier to predict and doesn't close out one of the openings.
    wallHMCurveAcc((getRandomSide() * 2 + 1 - mSpeed), mSpeed * getRandomDir(), 0, 0, 0, false, (THICKNESS * (mTimes - 1 + mTail)) + delay * 12.5 * (u_getDifficultyMult() ^ .65) * (mTimes - 1 + mTail));
    for i = 1, mTimes do
        cAltBarrage(i, mStep)
        t_wait(delay)
    end
end

-- hmpAlternate: A simple pattern involving one spinning barrage and one normal barrage.
function hmpAlternate()
    local curveSide = getRandomSide();
    local force = math.random(2, 4);

    hmcSimpleBarrageS(curveSide, force * getRandomDir())
    t_wait(getPerfectDelay(THICKNESS) * 10)
    cBarrage(getRandomSide());
    t_wait(getPerfectDelay(THICKNESS) * 10)
end

-- hmpTunnelSpinner: A typical tunnel pattern that contains spinners (curving alt barrages)
function hmpTunnelSpinner(mTimes, mLower, mUpper)
    mLower = mLower or 1
    mUpper = mUpper or getHalfSides()

    local myThickness = getPerfectThickness(THICKNESS)
    local delay = getPerfectDelay(myThickness) * 4.5
    local startSide = getRandomSide()

    for i = 0, mTimes do
        hmcSimpleSpinner(getRandomSide(), math.random(mLower, mUpper) * getRandomDir())
        w_wall(startSide, -myThickness) -- Wall to prevent visual artifacts
        if i < mTimes then
            w_wall(startSide, myThickness + 5 * u_getSpeedMultDM() * delay)
        else
            w_wall(startSide, myThickness)
        end
        t_wait(delay)
    end
end

-- hmpSwarm: A pattern that sends a flurry of single curving walls segments for the player to avoid.
-- The pattern takes in 3 parameters to allow which walls should be contained in the swarm or not.
-- An optional delay parameter can be supplied to lower or raise the delay between segments
function hmpSwarm(mLower, mUpper, mDecel, mConstant, mAccel, mDelayMult)
    mDecel = mDecel or true
    mConstant = mConstant or false
    mAccel = mAccel or false
    mDelayMult = mDelayMult or 1
    local delay = getPerfectDelay(getPerfectThickness(THICKNESS)) * 2 * mDelayMult

    -- Constuct wall pool
    local wall_pool = {}
    if mDecel then
        wall_pool[#wall_pool + 1] = "d"
    end
    if mConstant then
        wall_pool[#wall_pool + 1] = "c"
    end
    if mAccel then
        wall_pool[#wall_pool + 1] = "a"
    end
    -- If all patterns are false, abruptly end the function
    if (#wall_pool == 0) then
        return
    end
    for i = 1, 10 do
        local option = wall_pool[math.random(1, #wall_pool)]
        local offset = math.random(mLower, mUpper) * getRandomDir()
        if (option == "d") then
            wallHMStop(getRandomSide(), offset)
        elseif (option == "c") then
            wallHMCurveAcc(getRandomSide(), offset, 0, 0, 0, false)
        elseif (option == "a") then
            local c = math.random(100, 300) / 500.0 * getRandomDir()
            wallHMCurveAcc(getRandomSide(), 0, c, -offset, offset, true)
        end
        t_wait(delay)
    end
end
