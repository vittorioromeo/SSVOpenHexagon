execScript("common.lua")
execScript("commonpatterns.lua")
execScript("utils.lua")
execScript("alternativepatterns.lua")

function wallSAdj(mSide, mAdj) wallAdj(mSide, THICKNESS, mAdj) end

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