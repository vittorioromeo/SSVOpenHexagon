u_execScript("common.lua")
u_execScript("commonpatterns.lua")
u_execScript("utils.lua")
u_execScript("alternativepatterns.lua")

-- getPerfectAccelDM: Returns a constant that is used to perfectly adjust wallAcc acceleration to speed and difficulty multiplier
function getPerfectAccelDM()
    local requiredDecel = (u_getSpeedMultDM() ^ 2.02);
    local diffAdjust = (u_getDifficultyMult() ^ 0.65);
    if (u_getDifficultyMult() < 1) then
        if (u_getDifficultyMult() < 0.5) then
            diffAdjust = 1.2 * (0.256289 * math.sin(u_getDifficultyMult()) - 36.5669 * math.cos(u_getDifficultyMult()) + 36.5674);
        else
            diffAdjust = 1 - (-2 * u_getDifficultyMult() + 2) ^ 2 / 2.7;
        end
    end
    return diffAdjust * requiredDecel
end

-- wallSAdj: Returns a wall with common THICKNESS and multiplies the speed by mAdj
function wallSAdj(mSide, mAdj) w_wallAdj(mSide, THICKNESS, mAdj) end

-- wallSAcc: Returns a wallAcc perfectly adjusted to speed and difficulty multiplier
function wallSAcc(mSide, mAdj, mAcc, mMinSpd, mMaxSpd)
    local speedMult = u_getSpeedMultDM()
    local adjust = getPerfectAccelDM()
    w_wallAcc(mSide, THICKNESS, mAdj * speedMult, mAcc * adjust, mMinSpd, mMaxSpd)
end

-- wallSHAcc: Creates a wallHModSpeedData adjusted the perfect acceleration adjustment
function wallSHAcc(mHMod, mSide, mAdj, mAcc, mMinSpd, mMaxSpd, mPingPong)
    local speedMult = u_getSpeedMultDM()
    local adjust = getPerfectAccelDM()
    w_wallHModSpeedData(mHMod, mSide, THICKNESS, mAdj * speedMult, mAcc * adjust, mMinSpd, mMaxSpd, mPingPong)
end

-----------------------------------------
-- TRAP PATTERN FUNCTIONS (POLYHEDRUG) --
-----------------------------------------
-- These are patterns that have openings, but after a specified amount of delay, a faster wall spawns to speed through the gap.
-- This tells the player to not immediately orient their cursor to the gap, and they must wait before they can.

-- A trap version of a barrage
-- mSide: The side the pattern will spawn on
function pTrapBarrage(mSide)
    local delay = getPerfectDelay(THICKNESS) * 3.7

    cBarrage(mSide)
    t_wait(delay * 3)
    wallSAdj(mSide, 1.9)

    t_wait(delay * 2.5)
end

-- A trap barrage with two gaps, both on opposing sides of each other
-- mSide: The side the pattern will spawn on
function pTrapBarrageDouble(mSide)
    local delay = getPerfectDelay(THICKNESS) * 3.7
    local side2 = mSide + getHalfSides();

    for i = 0, l_getSides() - 1 do
        local currentSide = mSide + i
        if((currentSide ~= mSide) and (currentSide ~= side2)) then cWall(currentSide) end
    end

    t_wait(delay * 3)
    wallSAdj(mSide, 1.9)
    wallSAdj(side2, 1.9)

    t_wait(delay * 2.5)
end

-- pTrapBarrage, but the whole barrage is speeding instead of the "trap".
-- mSide: The side the pattern will spawn on
function pTrapBarrageInverse(mSide)
    local delay = getPerfectDelay(THICKNESS) * 3.7

    cWall(mSide)
    t_wait(delay * 3)

    for i = 0, l_getSides() - 1 do
        local currentSide = mSide + i
        if(currentSide ~= mSide) then wallSAdj(currentSide, 1.9) end
    end

    t_wait(delay * 2.5)
end

-- A trap version of an alt barrage
-- mSide: The side the pattern will spawn on
function pTrapBarrageAlt(mSide)
    local delay = getPerfectDelay(THICKNESS) * 3.7

    for i = 0, l_getSides() - 1 do
        local currentSide = mSide + i
        if(currentSide % 2 ~= 0) then cWall(currentSide) end
    end

    t_wait(delay * 3)

    for i = 0, l_getSides() - 1 do
        local currentSide = mSide + i
        if(currentSide % 2 == 0) then wallSAdj(currentSide, 1.9) end
    end

    t_wait(delay * 2.5)
end

--------------------------------------------
-- RANDOM CONTROL PATTERNS (Incongruence) --
--------------------------------------------

-- Spawns a barrage with two opposing holes instead of one
function pRCBarrageDouble()
    local currentSides = l_getSides()
    local delay = getPerfectDelay(THICKNESS) * 3.7
    local startSide = getRandomSide()

    for i = 0, currentSides - 2 do
        local currentSide = startSide + i
        local holeSide = startSide + i + (currentSides / 2)
        if(i ~= holeSide) then cWall(currentSide) end
    end
    t_wait(delay * 2.5)
end

-- Summons a series of barrages, but with the side count ascending, adding one new side for each barrage summoned.
-- This as a result can cause the barrages to either spiral or stay still, depending on the side it spawns on
-- This pattern can be unpredictable in it's nature.
function pRCAscendBarrage(mSide, mStartSides, mEndSides)
    local delay = getPerfectDelay(THICKNESS) * 5.6
    for i = mStartSides, mEndSides do
        t_eval([[l_setSides(]]..i..[[)]])
        for r = 0, i - 2 do
            cWall(r + mSide)
        end
        t_wait(delay)
    end
    t_wait(delay)
end

-- Summons a series of barrages in randomized positions, with the side count ascending per barrage
-- mStartSides: The starting number of sides for the pattern
-- mEndSides: The ending number of sides for the pattern
function pRCAscendBarrageRandom(mStartSides, mEndSides)
    local delay = getPerfectDelay(THICKNESS) * 8.5
    for i = mStartSides, mEndSides do
        t_eval([[l_setSides(]]..i..[[)]])
        local offset = math.random(1, i)
        for r = 0, i - 2 do
            cWall(r + offset)
        end
        t_wait(delay)
    end
    t_wait(delay)
end

-- Summons a series of alt barrages, but the side count randomizes per alt barrage to make the pattern much harder to navigate
-- mStep: How many sides to move per wall spawn.
-- mTimes: How many alt barrage segments to make
-- mLowerSides: The lowest side count to select at random
-- mUpperSides: The highest side count to select at random
function pRCDynamicAltBarrage(mStep, mTimes, mLowerSides, mUpperSides)
    local delay = getPerfectDelay(THICKNESS) * 5.6
    local prevSides = 0

    for r = 0, mTimes do
        local sides = math.random(mLowerSides, mUpperSides)
        if (sides ~= prevSides) then
            t_eval([[l_setSides(]]..sides..[[)]])
        end
        prevSides = sides
        for i = 0, sides, mStep do
            cWall(i + r)
        end
        t_wait(delay)
    end
    t_wait(delay)
end

-------------------------------------------
-- ACCELERATING PATTERNS (ACCELERADIANT) --
-------------------------------------------

-- pACBarrageAccelerate: A barrage that will slowly increase in speed over time, accelerating.
function pACBarrageAccelerate()
    local currentSides = l_getSides()
    local delay = getPerfectDelay(THICKNESS) * 3.7
    local startSide = getRandomSide()

    for i = 0, currentSides - 2 do
        local currentSide = startSide + i
        wallSAcc(currentSide, 2, 0.015, 1, 10)
    end
    t_wait(delay * 2.5)
end

-- pACBarrageDecelerate: A barrage that will quickly approach the player and decelerate.
function pACBarrageDecelerate()
    local currentSides = l_getSides()
    local delay = getPerfectDelay(THICKNESS) * 3.7
    local startSide = getRandomSide()

    for i = 0, currentSides - 2 do
        local currentSide = startSide + i
        wallSAcc(currentSide, 10, -0.22, 0.6, 10)
    end
    t_wait(delay * 3.5)
end

-- pACBarrageDeception: A barrage that never interacts with the player, going in and out of the view, tricking the player.
function pACBarrageDeception(mAdjMult, mAccMult)
    mAdjMult = mAdjMult or 1;
    mAccMult = mAccMult or 1;

    local currentSides = l_getSides()
    local delay = getPerfectDelay(THICKNESS) * 3.7
    local startSide = getRandomSide()

    for i = 0, currentSides - 2 do
        local currentSide = startSide + i
        wallSAcc(currentSide, 2.8 * mAdjMult, -0.05 * mAccMult, -2, 5)
    end
    t_wait(delay * 4)
end

-- pACInverseBarrage: Two decelerating barrages, one slower than the other to create an inverse barrage
function pACInverseBarrage()
    local currentSides = l_getSides()
    local delay = getPerfectDelay(THICKNESS) * 3.7
    local startSide = getRandomSide()

    for i = 0, currentSides - 2 do
        local currentSide = startSide + i
        wallSAcc(currentSide, 10, -0.21, 0.6, 10)
    end
    for i = 0, currentSides - 2 do
        local currentSide = startSide + getHalfSides() + i
        wallSAcc(currentSide, 10, -0.28, 0.6, 10)
    end
    t_wait(delay * 6.5)
end

-- pACBarrageMulti: A decelerating barrage, followed by a trail of accelerating barrages for visual effect
function pACBarrageMulti()
    local currentSides = l_getSides()
    local delay = getPerfectDelay(THICKNESS) * 3.7
    local startSide = math.random(0, 10)

    for i = 0, currentSides - 2 do
        local currentSide = startSide + i
        wallSAcc(currentSide, 10, -0.21, 0.31, 10)
        wallSAcc(currentSide, 0, 0.01, 0, 2.0)
        wallSAcc(currentSide, 0, 0.02, 0, 2.0)
        wallSAcc(currentSide, 0, 0.025, 0, 2.0)
    end
    t_wait(delay * 8)
end

-- pACSpiral: A pSpiral that decelerates, followed by an accelerating barrage similar to pACBarrageMulti
function pACSpiral()
    local currentSides = l_getSides()
    local oldThickness = THICKNESS;
    THICKNESS = getPerfectThickness(THICKNESS);
    local delay = getPerfectDelay(THICKNESS) * 3
    local startSide = math.random(0, 10)
    local loopDir = getRandomDir()

    for i = 0, currentSides + getHalfSides() do
        local currentSide = startSide + i * loopDir
        wallSAcc(currentSide, 10, -0.2, 0.40, 10)
        t_wait((delay / 2.1))
        wallSAcc(currentSide + (getHalfSides() * loopDir), 0, 0.128, 0, 1.4)
    end
    THICKNESS = oldThickness;
    t_wait(delay * 3)
end

-- A series of alt barrages that decelerate.
-- mTimes: The number of alt barrage series to create
function pACAltBarrage(mTimes)
    local currentSides = l_getSides()
    local delay = getPerfectDelay(THICKNESS) * 3.7

    for t = 1, mTimes do
        for i = 1, currentSides, 2 do
            wallSAcc(i, 10, -0.21, 0.6, 10)
        end
        t_wait(delay * 1.6);
        for i = 0, currentSides - 1, 2 do
            wallSAcc(i, 10, -0.21, 0.6, 10)
        end
        t_wait(delay * 1.6);
    end

    t_wait(delay * 3.5)
end

-- pACAltBarrageMulti: Alt Barrage with similar behavior to pACBarrageMulti
function pACAltBarrageMulti()
    local currentSides = l_getSides()
    local delay = getPerfectDelay(THICKNESS) * 3.7
    local offset = math.random(0, 1);

    for i = 1, currentSides, 2 do
        wallSAcc(i + offset, 10, -0.2, 0.5, 10)
        wallSAcc(i + offset, 0, 0.01, 0, 4.0)
        wallSAcc(i + offset, 0, 0.015, 0, 4.0)
        wallSAcc(i + offset, 0, 0.02, 0, 4.0)
    end

    t_wait(delay * 8)
end

-- Reveals randomly positioned Alt Barrages for the player to navigate
-- mTimes: The number of alt barrages to create
function pACAltBarrageReveal(mTimes)
    local currentSides = l_getSides()
    local delay = getPerfectDelay(THICKNESS) * 3.7
    local chooser = 0;

    for t = 1, mTimes do
        chooser = math.random(0, 1);
        for i = 1, currentSides do
            if (i % 2 == chooser) then
                wallSAcc(i, 10, -0.21, 0.6, 10)
            else
                wallSAcc(i, 10, -0.21, -2, 10)
            end
        end
        t_wait(delay * 1.6);
    end

    t_wait(delay * 3.5)
end

-- A tunnel pattern that reveals random barrages for the player to navigate
-- mTimes: The number of tunnel segments to create
function pACTunnelReveal(mTimes)
    local currentSides = l_getSides()
    local oldThickness = THICKNESS
    local myThickness = getPerfectThickness(THICKNESS);
    local delay = getPerfectDelay(myThickness) * 5
    local startSide = getRandomSide()
    local loopDir = getRandomDir()

    for t = 0, mTimes do
        THICKNESS = myThickness + 5 * u_getSpeedMultDM() * delay
        if t < mTimes then
            wallSAcc(startSide, 10, -0.22, 0.6, 10)
        end

        THICKNESS = oldThickness

        local revealSide = math.random(0, currentSides - 1)
        if (revealSide == startSide) then
            revealSide = (revealSide + getRandomDir()) % currentSides;
        end
        for i = 0, currentSides - 1 do
            if (i == revealSide) then
                wallSAcc(i, 10, -0.22, -2, 10)
            else
                wallSAcc(i, 10, -0.22, 0.6, 10)
            end
        end
        t_wait(delay)
        loopDir = loopDir * -1
    end

    THICKNESS = oldThickness;
    t_wait(delay)
end
