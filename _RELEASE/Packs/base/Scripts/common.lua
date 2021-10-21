-- common variables
THICKNESS = 40.0

function enableSwapIfDMGreaterThan(mDM)
    if(u_getDifficultyMult() > mDM) then
        e_messageAdd("difficulty > " ..mDM.. "\nswap enabled!", 65)
        l_setSwapEnabled(true)
    end
end

function enableSwapIfSpeedGEThan(mSpeed)
    if (u_getSpeedMultDM() >= mSpeed and not l_getSwapEnabled()) then
        e_messageAddImportant("Speed >= "..mSpeed.."\nswap enabled!", 120)
        l_setSwapEnabled(true)
    end
end

function disableIncIfDMGreaterThan(mDM)
    if(u_getDifficultyMult() > mDM) then
        e_messageAdd("difficulty > " ..mDM.. "\nincrement disabled!", 65)
        l_setIncEnabled(false)
    end
end

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
    if u_rndInt(0, 100) > 50 then return 1 end
    return -1
end

-- getPerfectDelay: returns time to wait for two walls to be next to each other
function getPerfectDelay(mThickness) return mThickness / (5.02 * u_getSpeedMultDM()) * u_getDelayMultDM() end

-- getPerfectDelayDM: returns getPerfectDelay calculated with difficulty mutliplier
function getPerfectDelayDM(mThickness) return mThickness / (5.02 * u_getSpeedMultDM()) * u_getDelayMultDM() end

-- getPerfectThickness: returns a good THICKNESS value in relation to human reflexes
function getPerfectThickness(mThickness) return mThickness * u_getSpeedMultDM() end

-- getSideDistance: returns shortest distance from a side to another
function getSideDistance(mSide1, mSide2)
    start = mSide1
    rightSteps = 0
    while start ~= mSide2 do
        rightSteps = rightSteps + 1
        start = start + 1
        if start > l_getSides() - 1 then start = 0 end
    end

    start = mSide1
    leftSteps = 0
    while start ~= mSide2 do
        leftSteps = leftSteps + 1
        start = start - 1
        if start < 0 then start = l_getSides() - 1 end
    end

    if rightSteps < leftSteps then return rightSteps end
    return leftSteps
end

-- cWall: creates a wall with the common THICKNESS or mThickness (optional)
function cWall(mSide, mThickness, ...) w_wall(mSide, mThickness or THICKNESS) end

-- oWall: creates a wall opposite to the mSide passed
function oWall(mSide, mThickness, ...) cWall(mSide + getHalfSides(), mThickness, ...) end

-- rWall: union of cwall and owall (created 2 walls facing each other)
function rWall(mSide, mThickness, ...)
    cWall(mSide, mThickness, ...)
    oWall(mSide, mThickness, ...)
end

-- cWallEx: creates a wall with mExtra walls attached to it
function cWallEx(mSide, mExtra, mThickness, ...)
    local loopDir = mExtra > 0 and 1 or -1
    for i = 0, mExtra, loopDir do cWall(mSide + i, mThickness, ...) end
end

-- oWallEx: creates a wall with mExtra walls opposite to mSide
function oWallEx(mSide, mExtra, mThickness, ...)
    cWallEx(mSide + getHalfSides(), mExtra, mThickness, ...)
end

-- rWallEx: union of cwallex and owallex
function rWallEx(mSide, mExtra, mThickness, ...)
    cWallEx(mSide, mExtra, mThickness, ...)
    oWallEx(mSide, mExtra, mThickness, ...)
end

-- cBarrageN: spawns a barrage of walls, with a free mSide plus mNeighbors
function cBarrageN(mSide, mNeighbors, mThickness, ...)
    for i = mNeighbors, l_getSides() - 2 - mNeighbors do
        cWall(mSide + i + 1, mThickness, ...)
    end
end

-- cBarrage: spawns a barrage of walls, with a single free mSide
function cBarrage(mSide, mThickness, ...) cBarrageN(mSide, 0, mThickness, ...) end

-- cBarrageOnlyN: spawns a barrage of wall, with only free mNeighbors
function cBarrageOnlyN(mSide, mNeighbors, mThickness, ...)
    cWall(mSide, mThickness, ...)
    cBarrageN(mSide, mNeighbors, mThickness, ...)
end

-- cAltBarrage: spawns a barrage of alternate walls
function cAltBarrage(mSide, mStep, mThickness, ...)
    for i = 0, l_getSides() / mStep, 1 do
        cWall(mSide + i * mStep, mThickness, ...)
    end
end