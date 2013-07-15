-- common variables
THICKNESS = 40.0;

-- getHalfSides: returns half the number of sides (integer)
function getHalfSides() return math.ceil(getSides() / 2) end

-- getRandomSide: returns random mSide
function getRandomSide() return math.random(0, getSides() - 1) end

-- getRandomDir: returns either 1 or -1
function getRandomDir()
	if math.random(0, 100) > 50 then return 1 end
	return -1
end

-- getPerfectDelay: returns time to wait for two walls to be next to each other
function getPerfectDelay(mThickness) return mThickness / (5.02 * getSpeedMult()) * getDelayMult() end

-- getPerfectDelayDM: returns getPerfectDelay calculated with difficulty mutliplier
function getPerfectDelayDM(mThickness) return mThickness / (5.02 * (getSpeedMult() / (getDifficultyMult() ^ 0.65))) * (getDelayMult() * (getDifficultyMult() ^ 0.10)) end

-- getPerfectThickness: returns a good THICKNESS value in relation to human reflexes
function getPerfectThickness(mThickness) return mThickness * getSpeedMult() end

-- getSideDistance: returns shortest distance from a side to another
function getSideDistance(mSide1, mSide2)
	start = mSide1	
	rightSteps = 0
	while start ~= mSide2 do
		rightSteps = rightSteps + 1
		start = start + 1
		if start > getSides() - 1 then start = 0 end
	end
	
	start = mSide1	
	leftSteps = 0
	while start ~= mSide2 do
		leftSteps = leftSteps + 1
		start = start - 1
		if start < 0 then start = getSides() - 1 end
	end
	
	if rightSteps < leftSteps then return rightSteps end
	return leftSteps
end

-- cWall: creates a wall with the common THICKNESS
function cWall(mSide) wall(mSide, THICKNESS) end

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
	for i = mNeighbors, getSides() - 2 - mNeighbors, 1 do
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
	for i = 0, getSides() / mStep, 1 do
		cWall(mSide + i * mStep)
	end
end