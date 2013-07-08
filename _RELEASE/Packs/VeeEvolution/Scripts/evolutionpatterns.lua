execScript("common.lua")
execScript("commonpatterns.lua")
execScript("utils.lua")
execScript("alternativepatterns.lua")
execScript("nextpatterns.lua")

function wallHMCurveAcc(mSide, mCurve, mCurveAcc, mCurveMin, mCurveMax, mCurvePingPong)
	wallHModCurveData(0.2, mSide, THICKNESS, mCurve * (getDifficultyMult() ^ 0.25), mCurveAcc, mCurveMin, mCurveMax, mCurvePingPong)
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
	delay = getPerfectDelayDM(THICKNESS) * 5.7
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

function hmcSimpleSpinnerS(mSide, mCurve)
	for i = 0, getSides() / 2, 1 do
		wallHMCurve(mSide + i * 2, mCurve)
	end
end

function hmcSimpleSpinnerSAcc(mSide, mCurve, mCurveAcc, mCurveMin, mCurveMax, mCurvePingPong)
	for i = 0, getSides() / 2, 1 do
		wallHMCurveAcc(mSide + i * 2, mCurve, mCurveAcc, mCurveMin, mCurveMax, mCurvePingPong)
	end
end

function hmcDefSpinner()
	wait(getPerfectDelayDM(THICKNESS) * 3.1)
	hmcSimpleSpinner(math.random(10, 20) / 10.0 * getRandomDir())
	wait(getPerfectDelayDM(THICKNESS) * 5)
end

function hmcDefBarrage()
	wait(getPerfectDelayDM(THICKNESS) * 3.1)
	hmcSimpleBarrage(math.random(10, 20) / 10.0 * getRandomDir())
	wait(getPerfectDelayDM(THICKNESS) * 5)
end

function hmcDef2Cage()
	side = getRandomSide()
	rndspd = math.random(10, 20) / 10.0

	wait(getPerfectDelayDM(THICKNESS) * 3.1)
	hmcSimpleCageS(rndspd, -1, side)
	wait(getPerfectDelayDM(THICKNESS) * 1.1)
	hmcSimpleCageS(rndspd, -1, side)
	wait(getPerfectDelayDM(THICKNESS) * 1.1)
	hmcSimpleCageS(rndspd, -1, side)
	wait(getPerfectDelayDM(THICKNESS) * 5)
end

function hmcSimpleBarrageSpiral(mTimes, mDelayMult, mStep, mCurve, mNeighbors)
	delay = getPerfectDelayDM(THICKNESS) * 5.6 * mDelayMult
	startSide = getRandomSide()
	loopDir = mStep * getRandomDir()	
	j = 0
	
	for i = 0, mTimes do
		hmcSimpleBarrageSNeigh(startSide + j, mCurve, mNeighbors)
		j = j + loopDir
		wait(delay)
		if(getSides() < 6) then wait(delay * 0.6) end
	end
	
	wait(getPerfectDelayDM(THICKNESS) * 6.1)
end

function hmcSimpleBarrageSpiralStatic(mTimes, mDelayMult, mStep, mCurve, mNeighbors)
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
	
	wait(getPerfectDelayDM(THICKNESS) * 6.1)
end

function hmcDefBarrageSpiral()
	hmcSimpleBarrageSpiral(math.random(2, 5), 1, 1, math.random(5, 15) / 10.0 * getRandomDir(), 0)
end

function hmcDefBarrageSpiralSpin()
	hmcSimpleBarrageSpiralStatic(math.random(7, 14), 0.25, 1, math.random(5, 18) / 10.0 * getRandomDir(), 2)
end

function hmcDefAccelBarrage()
	c = math.random(50, 100) / 1000.0 * getRandomDir()
	min = math.random(5, 35) / 10.0 * -1
	max = math.random(5, 35) / 10.0
	hmcBarrage(0, c, min, max, true)
	wait(getPerfectDelayDM(THICKNESS) * 6.1)
end

function hmcDefAccelBarrageDouble()
	c = math.random(50, 100) / 1000.0 * getRandomDir()
	min = math.random(5, 35) / 10.0 * -1
	max = math.random(5, 35) / 10.0
	hmcBarrage(0, c, min, max, true)
	wait(getPerfectDelayDM(THICKNESS) * 2.1)
	hmcBarrage(0, c, min, max, true)
	wait(getPerfectDelayDM(THICKNESS) * 6.1)
end

function hmcDefSpinnerSpiral()
	side = getRandomSide()
	c = math.random(10, 20) / 10.0 * getRandomDir()

	wait(getPerfectDelayDM(THICKNESS) * 3.1)

	for i = 0, math.random(4, 8) do
		hmcSimpleSpinnerS(side, c)
		wait(getPerfectDelayDM(THICKNESS) * 1.15)
	end

	wait(getPerfectDelayDM(THICKNESS) * 5)
end

function getRndDM(mNum)
	return math.random(mNum - getDifficultyMult() ^ 1.25, mNum + getDifficultyMult() ^ 1.25)
end

function hmcDefSpinnerSpiralAcc()
	side = getRandomSide()
	acc = math.random(50, 100) / 1000.0 * getRandomDir()
	min = math.random(getRndDM(12), getRndDM(28)) / 10.0 * -1
	max = math.random(getRndDM(12), getRndDM(28)) / 10.0

	wait(getPerfectDelayDM(THICKNESS) * 3.1)

	for i = 0, math.random(4, 8) do
		hmcSimpleSpinnerSAcc(side, 0, acc, min, max, true)
		wait(getPerfectDelay(THICKNESS) * 0.8)
	end

	wait(getPerfectDelayDM(THICKNESS) * 5)
end