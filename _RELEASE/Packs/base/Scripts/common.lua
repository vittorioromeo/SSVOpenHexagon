-- COMMON GLOBAL VARIABLES
THICKNESS = 40.0

-- COMMON FUNCTIONS

-- enableSwapIfDMGreaterThan: If the player chooses to play a level that exceeds the given difficulty, turn on swap.
function enableSwapIfDMGreaterThan(mDM)
    if(u_getDifficultyMult() > mDM) then
        e_messageAdd("difficulty > " ..mDM.. "\nswap enabled!", 65)
        l_setSwapEnabled(true)
    end
end

-- enableSwapIfSpeedGEThan: If the player reaches a given speed on a level, turn on swap.
function enableSwapIfSpeedGEThan(mSpeed)
    if (u_getSpeedMultDM() >= mSpeed and not l_getSwapEnabled()) then
        e_messageAddImportant("Speed >= "..mSpeed.."\nswap enabled!", 120)
        l_setSwapEnabled(true)
    end
end

-- disableIncIfDMGreaterThan: If the player chooses to play a level that exceeds the given difficulty, disable incrementing.
function disableIncIfDMGreaterThan(mDM)
    if(u_getDifficultyMult() > mDM) then
        e_messageAdd("difficulty > " ..mDM.. "\nincrement disabled!", 65)
        l_setIncEnabled(false)
    end
end

-- disableSpeedIncIfDMGreaterThan: If the player chooses to play a level that exceeds the given difficulty, make speed not increase upon incrementing.
function disableSpeedIncIfDMGreaterThan(mDM)
    if(u_getDifficultyMult() > mDM) then
        e_messageAdd("difficulty > " ..mDM.. "\nspeed increment disabled!", 65)
        l_setSpeedInc(0.0)
    end
end

-- getHalfSides: returns half the number of sides (integer)
function getHalfSides() return math.ceil(l_getSides() / 2) end

-- getRandomSide: returns random mSide
function getRandomSide() return u_rndInt(0, l_getSides() - 1) end

-- getPlayerSide: gets the current side that the player is in
function getPlayerSide()
    local playerPosition = math.deg(u_getPlayerAngle())
    local sideLength = (360 / l_getSides())
    local offset = sideLength / 2

    return math.floor((playerPosition + offset) % 360 / sideLength)
end

-- getRandomDir: returns either 1 or -1
function getRandomDir()
    if u_rndInt(0, 1) == 0 then return 1 end
    return -1
end

-- getPerfectDelay: returns time to wait for two walls to be next to each other
function getPerfectDelay(mThickness) return mThickness / (5.02 * u_getSpeedMultDM()) * u_getDelayMultDM() end

-- getPerfectDelayDM's implementation has now been implemented into getPerfectDelay. Keeping the name for compatibility reasons.
getPerfectDelayDM = getPerfectDelay

-- getPerfectThickness: returns a good THICKNESS value in relation to human reflexes
function getPerfectThickness(mThickness) return mThickness * u_getSpeedMultDM() end

-- getSideDistance: returns shortest distance from a side to another
function getSideDistance(mSide1, mSide2)
    if (mSide1 == mSide2) then
        return 0
    end
    -- Pick out the lower and higher side
    local low = math.min(mSide1, mSide2)
    local high = math.max(mSide1, mSide2)
    -- We need to get both high and low positive for modulus to work properly
    -- We only need to check the lower number for this.
    if (low < 0) then
        high = high - low
        low = 0
    end
    -- Calculate the difference and make any last minute adjustments accordingly
    local diff = (high - low) % l_getSides()
    if (diff > getHalfSides()) then
        return l_getSides() - diff
    end
    return diff
end

-- cWall: creates a wall with the common THICKNESS
function cWall(mSide) w_wall(mSide, THICKNESS) end

-- oWall: creates a wall opposite to the mSide passed
function oWall(mSide) cWall(mSide + getHalfSides()) end

-- rWall: union of cwall and owall (created 2 walls facing each other)
function rWall(mSide)
    cWall(mSide)
    oWall(mSide)
end

-- cWallEx: creates a wall with mExtra walls attached to it
function cWallEx(mSide, mExtra)
    cWall(mSide)
    local exLoopDir = 1

    if mExtra < 0 then exLoopDir = -1 end
    for i = 0, mExtra, exLoopDir do cWall(mSide + i) end
end

-- oWallEx: creates a wall with mExtra walls opposite to mSide
function oWallEx(mSide, mExtra)
    cWallEx(mSide + getHalfSides(), mExtra)
end

-- rWallEx: union of cwallex and owallex
function rWallEx(mSide, mExtra)
    cWallEx(mSide, mExtra)
    oWallEx(mSide, mExtra)
end

-- cBarrageN: spawns a barrage of walls, with a free mSide plus mNeighbors
function cBarrageN(mSide, mNeighbors)
    for i = mNeighbors, l_getSides() - 2 - mNeighbors, 1 do
        cWall(mSide + i + 1)
    end
end

-- cBarrage: spawns a barrage of walls, with a single free mSide
function cBarrage(mSide) cBarrageN(mSide, 0) end

-- cBarrageOnlyN: spawns a barrage of wall, with only free mNeighbors
function cBarrageOnlyN(mSide, mNeighbors)
    cWall(mSide)
    cBarrageN(mSide, mNeighbors)
end

-- cAltBarrage: spawns a barrage of alternate walls
function cAltBarrage(mSide, mStep)
    for i = 0, l_getSides(), mStep do
        cWall(mSide + i)
    end
end

-- cSwapBarrageN: A barrage with mNeighbors to have an opening pocket, forcing the players to go into the pocket and swap.
function cSwapBarrageN(mSide, mNeighbors, mDelayMult)
    mDelayMult = mDelayMult or 1
    local myThickness = getPerfectThickness(THICKNESS) * 2.5 * mDelayMult
    local delay = getPerfectDelay(myThickness - THICKNESS) / u_getDelayMultDM()
    cBarrageN(mSide, mNeighbors + 1) -- Create the initial barrage
    w_wall(mSide + 1 + mNeighbors, myThickness)
    w_wall(mSide + l_getSides() - 1 - mNeighbors, myThickness)
    t_wait(delay)
    for i = -mNeighbors, mNeighbors do
        cWall(i + mSide)
    end
end

-- cSwapBarrage: A barrage, but the opening is instead a pocket that is fully enclosed, forcing the player to swap to escape the pocket.
function cSwapBarrage(mSide, mDelayMult)
    mDelayMult = mDelayMult or 1
    local myThickness = getPerfectThickness(THICKNESS) * 2.5 * mDelayMult
    local delay = getPerfectDelay(myThickness - THICKNESS) / u_getDelayMultDM()
    cBarrageN(mSide, 1)
    w_wall(mSide + 1, myThickness)
    w_wall(mSide + l_getSides() - 1, myThickness)
    t_wait(delay)
    cWall(mSide)
end

-- cSwapCorridor: A barrage, but the neighbors of the opening have elongated thicknesses
function cSwapCorridor(mSide, mEnding)
    mEnding = mEnding or false
    local myThickness = getPerfectThickness(THICKNESS) * 4
    local delay = getPerfectDelay(myThickness - THICKNESS) / u_getDelayMultDM()
    cBarrageN(mSide, 1)
    w_wall(mSide + 1, myThickness)
    w_wall(mSide + l_getSides() - 1, myThickness)
    t_wait(delay)
    if (mEnding) then
        cWall(mSide)
    end
    t_wait(getPerfectDelay(THICKNESS) / u_getDelayMultDM())
end
