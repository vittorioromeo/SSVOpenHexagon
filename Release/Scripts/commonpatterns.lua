execScript("common.lua")

-- pAltBarrage: spawns a series of cAltBarrage
function pAltBarrage(mTimes, mStep)
	delay = getPerfectDelay(THICKNESS) * 4.6
	
	for i = 0, mTimes do
		cAltBarrage(i, mStep)
		wait(delay)
	end
	
	wait(delay)
end

-- pMirrorSpiral: spawns a spiral of rWallEx
function pMirrorSpiral(mTimes, mExtra)
	oldThickness = THICKNESS
	THICKNESS = getPerfectThickness(THICKNESS)
	delay = getPerfectDelay(THICKNESS)
	startSide = getRandomSide()
	loopDir = getRandomDir()	
	j = 0
	
	for i = 0, mTimes do
		rWallEx(startSide + j, mExtra)
		j = j + loopDir
		wait(delay)
	end
	
	THICKNESS = oldThickness
	
	wait(getPerfectDelay(THICKNESS) * 6.5)
end

-- pBarrageSpiral: spawns a spiral of cBarrage
function pBarrageSpiral(mTimes, mDelayMult)
	delay = getPerfectDelay(THICKNESS) * 4.6 * mDelayMult
	startSide = getRandomSide()
	loopDir = getRandomDir()	
	j = 0
	
	for i = 0, mTimes do
		cBarrage(startSide + j)
		j = j + loopDir
		wait(delay)
	end
	
	wait(getPerfectDelay(THICKNESS) * 5.1)
end

-- pWallExVortex: spawns left-left right-right spiral patters
function pWallExVortex(mTimes, mStep, mExtraMult)
	delay = getPerfectDelay(THICKNESS) * 4.0 
	startSide = getRandomSide()
	loopDir = getRandomDir()
	currentSide = startSide
	
	for j = 0, mTimes do
		for i = 0, mStep do
			currentSide = currentSide + loopDir
			rWallEx(currentSide, loopDir * mExtraMult)
			wait(delay)
		end
		
		loopDir = loopDir * -1
		
		for i = 0, mStep + 1 do
			currentSide = currentSide + loopDir;
			rWallEx(currentSide, loopDir * mExtraMult)
			wait(delay)
		end
	end
	
	wait(getPerfectDelay(THICKNESS) * 4.5)
end

-- pInverseBarrage: spawns two barrages who force you to turn 180 degrees
function pInverseBarrage(mTimes)
	delay = getPerfectDelay(THICKNESS) * 8.9
	startSide = getRandomSide()
	
	for i = 0, mTimes do
		cBarrage(startSide)
		wait(delay)
		cBarrage(startSide + getHalfSides())
		wait(delay)
	end
	
	wait(getPerfectDelay(THICKNESS) * 1.5)
end

-- pMirrorWallStrip: spawns rWalls close to one another on the same side
function pMirrorWallStrip(mTimes, mExtra)
	delay = getPerfectDelay(THICKNESS) * 2.5
	startSide = getRandomSide()
	
	for i = 0, mTimes do
		rWallEx(startSide, mExtra)
		wait(delay)
	end
	
	delay = getPerfectDelay(THICKNESS) * 0.85
end

-- pTunnel: forces you to circle around a very thick wall
function pTunnel(mTimes)
	oldThickness = THICKNESS
	myThickness = getPerfectThickness(THICKNESS)
	delay = getPerfectDelay(myThickness) * 5
	startSide = getRandomSide()
	loopDir = getRandomDir()
	
	THICKNESS = myThickness
	
	for i = 0, mTimes do
		if i < mTimes then
			wall(startSide, myThickness + 5 * getSpeedMult() * delay)
		end
		
		cBarrage(startSide + loopDir)
		wait(delay)
		
		loopDir = loopDir * -1
	end
	
	THICKNESS = oldThickness
end