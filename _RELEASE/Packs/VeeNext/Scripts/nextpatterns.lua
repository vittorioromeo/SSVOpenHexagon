execScript("common.lua")
execScript("commonpatterns.lua")
execScript("utils.lua")
execScript("alternativepatterns.lua")

function wallSAdj(mSide, mAdj) wallAdj(mSide, THICKNESS, mAdj) end
function wallSAcc(mSide, mAdj, mAcc, mMinSpd, mMaxSpd) wallAcc(mSide, THICKNESS, mAdj, mAcc * (getDifficultyMult()), mMinSpd, mMaxSpd) end

function pTrapBarrage(mSide)
	delay = getPerfectDelay(THICKNESS) * 3.7
		
	cBarrage(mSide)
	wait(delay * 3)
	wallSAdj(mSide, 1.9)

	wait(delay * 2.5)
end

function pTrapBarrageDouble(mSide)
	delay = getPerfectDelay(THICKNESS) * 3.7
	side2 = mSide + getHalfSides();
	
	for i = 0, getSides() - 1 do
		currentSide = mSide + i
		if((currentSide ~= mSide) and (currentSide ~= side2)) then cWall(currentSide) end
	end

	wait(delay * 3)
	wallSAdj(mSide, 1.9)
	wallSAdj(side2, 1.9)
	
	wait(delay * 2.5)
end

function pTrapBarrageInverse(mSide)
	delay = getPerfectDelay(THICKNESS) * 3.7
	
	cWall(mSide)	
	wait(delay * 3)

	for i = 0, getSides() - 1 do
		currentSide = mSide + i
		if(currentSide ~= mSide) then wallSAdj(currentSide, 1.9) end
	end

	wait(delay * 2.5)
end

function pTrapBarrageAlt(mSide)
	delay = getPerfectDelay(THICKNESS) * 3.7

	for i = 0, getSides() - 1 do
		currentSide = mSide + i
		if(currentSide % 2 ~= 0) then cWall(currentSide) end
	end

	wait(delay * 3)

	for i = 0, getSides() - 1 do
		currentSide = mSide + i
		if(currentSide % 2 == 0) then wallSAdj(currentSide, 1.9) end
	end

	wait(delay * 2.5)
end

function pTrapSpiral(mSide)
	delay = getPerfectDelay(THICKNESS) * 3.7
	loopDir = getRandomDir()		

	if(getSides() < 6) then delay = delay + 4 end

	for i = 0, getSides() + getHalfSides() do
		currentSide = (mSide + i) * loopDir
		for j = 0, getHalfSides() do wallSAdj(currentSide + j, 1.2 + (i / 7.9)) end
		wait((delay * 0.75) - (i * 0.45) + 3)
	end

	wait(delay * 2.5)
end

function pRCBarrage()
	currentSides = getLevelValueInt("sides")
	delay = getPerfectDelay(THICKNESS) * 3.7
	startSide = math.random(0, 10)
	for i = 0, currentSides - 2 do
		currentSide = startSide + i
		cWall(currentSide)
	end
	wait(delay * 2.5)
end

function pRCBarrageDouble()
	currentSides = getLevelValueInt("sides")
	delay = getPerfectDelay(THICKNESS) * 3.7
	startSide = math.random(0, 10)
	for i = 0, currentSides - 2 do
		currentSide = startSide + i
		holeSide = startSide + i + (currentSides / 2)
		if(i ~= holeSide) then cWall(currentSide) end
	end
	wait(delay * 2.5)
end

function pRCBarrageSpin()
	currentSides = getLevelValueInt("sides")
	delay = getPerfectDelay(THICKNESS) * 3.7
	startSide = math.random(0, 10)
	loopDir = getRandomDir()
	for j = 0, 2 do
		for i = 0, currentSides - 2 do
			currentSide = startSide + i
			cWall(currentSide + (j * loopDir))
		end
		wait(delay + 1)
	end
	wait(delay * 2.5)
end

function pACBarrage()
	currentSides = getLevelValueInt("sides")
	delay = getPerfectDelay(THICKNESS) * 3.7
	startSide = math.random(0, 10)
	for i = 0, currentSides - 2 do
		currentSide = startSide + i
		wallSAcc(currentSide, 9 + math.random(0, 1), -1.1, 1, 12)
	end
	wait(delay * 2.5)
end

function pACBarrageMulti()
	currentSides = getLevelValueInt("sides")
	delay = getPerfectDelay(THICKNESS) * 3.7
	startSide = math.random(0, 10)
	for i = 0, currentSides - 2 do
		currentSide = startSide + i
		wallSAcc(currentSide, 10, -1.09, 0.31, 10)
		wallSAcc(currentSide, 0, 0.05, 0, 4.0)
		wallSAcc(currentSide, 0, 0.09, 0, 4.0)
		wallSAcc(currentSide, 0, 0.12, 0, 4.0)
	end
	wait(delay * 8)
end

function pACBarrageMultiAltDir()
	currentSides = getLevelValueInt("sides")
	delay = getPerfectDelayDM(THICKNESS) * 4
	mdiff = 1 + math.abs(1 - getDifficultyMult())
	startSide = math.random(0, 10)
	loopDir = getRandomDir()
	for i = 0, currentSides + getHalfSides() do
		currentSide = startSide + i * loopDir
		wallSAcc(currentSide, 10, -1.095, 0.40, 10)
		wait((delay / 2.21) * (mdiff * 1.29))
		wallSAcc(currentSide + (getHalfSides() * loopDir), 0, 0.128, 0, 1.4)
	end
	wait(delay * 8)
end