execScript("common.lua")

function pAltMirrorSpiral(mTimes, mExtra)
	oldThickness = THICKNESS
	THICKNESS = getPerfectThickness(THICKNESS)
	delay = getPerfectDelay(THICKNESS)
	startSide = getRandomSide()
	loopDir = getRandomDir()	
	for k = 1, table.getn(mTimes) do
		for i = 1, mTimes[k] do
			rWallEx(startSide, mExtra)
			if (k % 2) == 0 then
				startSide = startSide + loopDir
			else
				startSide = startSide - loopDir
			end
			wait(delay)
		end 
	end

	THICKNESS = oldThickness
	
	wait(getPerfectDelay(THICKNESS) * 6.5)
end

function randomArray(mNumber,mLower,mUpper)
	a = {}
	for k = 1, mNumber do
		a[k] = math.random(mLower,mUpper)
	end
	return a
end

function pAltTunnel(mTimes,mFree)
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
		
		cBarrageN(startSide + loopDir,mFree)
		wait(delay)
		
		loopDir = loopDir * -1
	end
	
	THICKNESS = oldThickness
end

function pLadder(mTimes,mArray,mCycles)
	oldThickness = THICKNESS
	myThickness = getPerfectThickness(THICKNESS)
	delay = getPerfectDelay(myThickness)

	THICKNESS = myThickness

	for i = 1, mTimes do

		k = i % mCycles + 1
		for j = 1, getSides() do
			if(mArray[(k-1)*getSides() + j] ~= 0) then
				cWall(j)
			end
		end
		wait(delay)
		
		if i < mTimes then
			for j = 1, getSides() do
				if(mArray[(k-1)*getSides() + j] == 2) then
					wall(j, myThickness + 25 * getSpeedMult() * delay)
				end
			end
			wait(delay*5)
		end
		wait(delay)
	end

	wait(delay*2)
	
	THICKNESS = oldThickness
end