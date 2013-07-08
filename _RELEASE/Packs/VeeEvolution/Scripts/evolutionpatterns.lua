execScript("common.lua")
execScript("commonpatterns.lua")
execScript("utils.lua")
execScript("alternativepatterns.lua")
execScript("nextpatterns.lua")

function wallHMCurveAcc(mSide, mCurve, mCurveAcc, mCurveMin, mCurveMax, mCurvePingPong)
	wallHModCurveData(0.2, mSide, THICKNESS, mCurve, mCurveAcc, mCurveMin, mCurveMax, mCurvePingPong)
end

function wallHMCurve(mSide, mCurve)
	wallHMCurveAcc(mSide, mCurve, 0, 0, 0, false)
end

function hmcBarrageN(mSide, mNeighbors, mCurve, mCurveAcc, mCurveMin, mCurveMax, mCurvePingPong)
	for i = mNeighbors, getSides() - 2 - mNeighbors, 1 do
		wallHMCurveAcc(mSide + i + 1, mCurve, mCurveAcc, mCurveMin, mCurveMax, mCurvePingPong)
	end
end

function hmcBarrageS(mSide, mCurve, mCurveAcc, mCurveMin, mCurveMax, mCurvePingPong)
	hmcBarrageN(mSide, 0, mCurve, mCurveAcc, mCurveMin, mCurveMax, mCurvePingPong);
end

function hmcBarrage(mCurve, mCurveAcc, mCurveMin, mCurveMax, mCurvePingPong)
	hmcBarrageS(getRandomSide(), mCurve, mCurveAcc, mCurveMin, mCurveMax, mCurvePingPong);
end

function hmcSimpleBarrage(mCurve)
	hmcBarrageN(getRandomSide(), 0, mCurve, 0, 0, 0, false);
end

function hmcSimpleBarrageS(mSide, mCurve)
	hmcBarrageN(mSide, 0, mCurve, 0, 0, 0, false);
end

function hmcSimpleBarrageSNeigh(mSide, mCurve, mNeighbors)
	hmcBarrageN(mSide, mNeighbors, mCurve, 0, 0, 0, false);
end


function hmcSimpleTwirl(mTimes, mCurve, mCurveAdd)
	startSide = getRandomSide()
	currentSide = startSide
	loopDir = getRandomDir()
	delay = getPerfectDelay(THICKNESS) * 5.7
	j = 0
	
	currentCurve = mCurve	

	for i = 0, mTimes do
		hmcSimpleBarrageS(startSide + j, currentCurve)
		j = j + loopDir
		currentCurve = currentCurve + mCurveAdd
		wait(delay)
	end
end

function hmcSimpleCage(mCurve, mDir)
	side = getRandomSide()
	oppositeSide = side + getHalfSides()

	wallHMCurve(side, mCurve)
	wallHMCurve(oppositeSide, mCurve * mDir)
end

function hmcSimpleCageS(mCurve, mDir, mSide)
	oppositeSide = mSide + getHalfSides()

	wallHMCurve(mSide, mCurve)
	wallHMCurve(oppositeSide, mCurve * mDir)
end

function hmcSimpleSpinner(mCurve)
	side = getRandomSide()

	for i = 0, getSides() / 2, 1 do
		wallHMCurve(side + i * 2, mCurve)
	end
end

function hmcDefSpinner()
	wait(getPerfectDelay(THICKNESS) * 3.1)
	hmcSimpleSpinner(math.random(10, 20) / 10.0 * getRandomDir())
	wait(getPerfectDelay(THICKNESS) * 5)
end

function hmcDefBarrage()
	wait(getPerfectDelay(THICKNESS) * 3.1)
	hmcSimpleBarrage(math.random(10, 20) / 10.0 * getRandomDir())
	wait(getPerfectDelay(THICKNESS) * 5)
end

function hmcDef2Cage()
	side = getRandomSide()
	rndspd = math.random(10, 20) / 10.0

	wait(getPerfectDelay(THICKNESS) * 3.1)
	hmcSimpleCageS(rndspd, -1, side)
	wait(getPerfectDelay(THICKNESS) * 1.1)
	hmcSimpleCageS(rndspd, -1, side)
	wait(getPerfectDelay(THICKNESS) * 1.1)
	hmcSimpleCageS(rndspd, -1, side)
	wait(getPerfectDelay(THICKNESS) * 5)
end

function hmcSimpleBarrageSpiral(mTimes, mDelayMult, mStep, mCurve, mNeighbors)
	delay = getPerfectDelay(THICKNESS) * 5.6 * mDelayMult
	startSide = getRandomSide()
	loopDir = mStep * getRandomDir()	
	j = 0
	
	for i = 0, mTimes do
		hmcSimpleBarrageSNeigh(startSide + j, mCurve, mNeighbors)
		j = j + loopDir
		wait(delay)
		if(getSides() < 6) then wait(delay * 0.6) end
	end
	
	wait(getPerfectDelay(THICKNESS) * 6.1)
end

function hmcDefBarrageSpiral()
	hmcSimpleBarrageSpiral(math.random(2, 5), 1, 1, math.random(5, 15) / 10.0 * getRandomDir(), 0)
end

function hmcDefBarrageSpiralSpin()
	hmcSimpleBarrageSpiral(math.random(7, 14), 0.25, 1, math.random(5, 15) / 10.0 * getRandomDir(), 2)
end