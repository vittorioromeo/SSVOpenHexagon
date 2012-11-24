execScript("common.lua")

function pAltMirrorSpiral(mTimes, mExtra)
	oldThickness = THICKNESS
	THICKNESS = getPerfectThickness(THICKNESS)
	delay = getPerfectDelay(THICKNESS)
	startSide = getRandomSide()
	loopDir = getRandomDir()	
	for k = 0, table.getn(mTimes) do
		for i = 0, mTimes[k] do
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