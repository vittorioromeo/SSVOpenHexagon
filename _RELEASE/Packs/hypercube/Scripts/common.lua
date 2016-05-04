-- common variables
THICKNESS = 40.0;

function enableSwapIfDMGreaterThan(mDM)
	if(u_getDifficultyMult() > mDM) then
		m_messageAdd(" difficulty > " ..mDM.. "\nswap enabled!", 65)
		l_setSwapEnabled(true)
	end	
end

function disableIncIfDMGreaterThan(mDM)
	if(u_getDifficultyMult() > mDM) then
		m_messageAdd(" difficulty > " ..mDM.. "\nincrement disabled!", 65)
		l_setIncEnabled(false)
	end	
end

-- getHalfSides: returns half the number of sides (integer)
function getHalfSides() return math.ceil(l_getSides() / 2) end

-- getRandomSide: returns random mSide
function getRandomSide() return math.random(0, l_getSides() - 1) end

-- getRandomDir: returns either 1 or -1
function getRandomDir()
	if math.random(0, 100) > 50 then return 1 end
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
	cWall(mSide);
	loopDir = 1;
	
	if mExtra < 0 then loopDir = -1 end
	for i = 0, mExtra, loopDir do cWall(mSide + i) end
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
	for i = 0, l_getSides() / mStep, 1 do
		cWall(mSide + i * mStep)
	end
end