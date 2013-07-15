u_execScript("common.lua")

-- pAltBarrage: spawns a series of cAltBarrage
function pAltBarrage(mTimes, mStep)
	delay = getPerfectDelayDM(THICKNESS) * 5.6
	
	for i = 0, mTimes do
		cAltBarrage(i, mStep)
		t_wait(delay)
	end
	
	t_wait(delay)
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
		t_wait(delay)
	end
	
	THICKNESS = oldThickness
	
	t_wait(getPerfectDelayDM(THICKNESS) * 6.5)
end

-- pMirrorSpiralDouble: spawns a spiral of rWallEx where you need to change direction
function pMirrorSpiralDouble(mTimes, mExtra)
	oldThickness = THICKNESS
	THICKNESS = getPerfectThickness(THICKNESS)
	delay = getPerfectDelayDM(THICKNESS)
	startSide = getRandomSide()
	currentSide = startSide
	loopDir = getRandomDir()	
	j = 0
	
	for i = 0, mTimes do
		rWallEx(startSide + j, mExtra)
		j = j + loopDir
		t_wait(delay)
	end
	
	rWallEx(startSide + j, mExtra)
	t_wait(delay * 0.9)
	
	rWallEx(startSide + j, mExtra)
	t_wait(delay * 0.9)
	
	loopDir = loopDir * -1
	
	for i = 0, mTimes + 1 do
		currentSide = currentSide + loopDir;
		rWallEx(currentSide + j - 1, mExtra)
		j = j + loopDir
		t_wait(delay)
	end
	
	THICKNESS = oldThickness
	t_wait(getPerfectDelayDM(THICKNESS) * 7.5)
end

-- pBarrageSpiral: spawns a spiral of cBarrage
function pBarrageSpiral(mTimes, mDelayMult, mStep)
	delay = getPerfectDelayDM(THICKNESS) * 5.6 * mDelayMult
	startSide = getRandomSide()
	loopDir = mStep * getRandomDir()	
	j = 0
	
	for i = 0, mTimes do
		cBarrage(startSide + j)
		j = j + loopDir
		t_wait(delay)
		if(l_getSides() < 6) then t_wait(delay * 0.6) end
	end
	
	t_wait(getPerfectDelayDM(THICKNESS) * 6.1)
end

-- pDMBarrageSpiral: spawns a spiral of cBarrage, with static delay
function pDMBarrageSpiral(mTimes, mDelayMult, mStep)
	delay = (getPerfectDelayDM(THICKNESS) * 5.42) * (mDelayMult / (u_getDifficultyMult() ^ 0.4)) * (u_getSpeedMultDM() ^ 0.35)
	startSide = getRandomSide()
	loopDir = mStep * getRandomDir()	
	j = 0
	
	for i = 0, mTimes do
		cBarrage(startSide + j)
		j = j + loopDir
		t_wait(delay)
		if(l_getSides() < 6) then t_wait(delay * 0.49) end
	end
	
	t_wait(getPerfectDelayDM(THICKNESS) * (6.7 * (u_getDifficultyMult() ^ 0.7)))
end

-- pWallExVortex: spawns left-left right-right spiral patters
function pWallExVortex(mTimes, mStep, mExtraMult)
	delay = getPerfectDelayDM(THICKNESS) * 5.0 
	startSide = getRandomSide()
	loopDir = getRandomDir()
	currentSide = startSide
	
	for j = 0, mTimes do
		for i = 0, mStep do
			currentSide = currentSide + loopDir
			rWallEx(currentSide, loopDir * mExtraMult)
			t_wait(delay)
		end
		
		loopDir = loopDir * -1
		
		for i = 0, mStep + 1 do
			currentSide = currentSide + loopDir;
			rWallEx(currentSide, loopDir * mExtraMult)
			t_wait(delay)
		end
	end
	
	t_wait(getPerfectDelayDM(THICKNESS) * 5.5)
end

-- pInverseBarrage: spawns two barrages who force you to turn 180 degrees
function pInverseBarrage(mTimes)
	delay = getPerfectDelayDM(THICKNESS) * 9.9
	startSide = getRandomSide()
	
	for i = 0, mTimes do
		cBarrage(startSide)
		t_wait(delay)
		if(l_getSides() < 6) then t_wait(delay * 0.8) end
		cBarrage(startSide + getHalfSides())
		t_wait(delay)
	end
	
	t_wait(getPerfectDelayDM(THICKNESS) * 2.5)
end

-- pRandomBarrage: spawns barrages with random side, and waits humanly-possible times depending on the sides distance
function pRandomBarrage(mTimes, mDelayMult)
	side = getRandomSide()
	oldSide = 0
	
	for i = 0, mTimes do	
		cBarrage(side)
		oldSide = side
		side = getRandomSide()
		t_wait(getPerfectDelayDM(THICKNESS) * (2 + (getSideDistance(side, oldSide)*mDelayMult)))
	end
	
	t_wait(getPerfectDelayDM(THICKNESS) * 5.6)
end

-- pMirrorWallStrip: spawns rWalls close to one another on the same side
function pMirrorWallStrip(mTimes, mExtra)
	delay = getPerfectDelayDM(THICKNESS) * 3.65
	startSide = getRandomSide()
	
	for i = 0, mTimes do
		rWallEx(startSide, mExtra)
		t_wait(delay)
	end
	
	t_wait(getPerfectDelayDM(THICKNESS) * 5.00)
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
			w_wall(startSide, myThickness + 5 * u_getSpeedMultDM() * delay)
		end
		
		cBarrage(startSide + loopDir)
		t_wait(delay)
		
		loopDir = loopDir * -1
	end
	
	THICKNESS = oldThickness
end