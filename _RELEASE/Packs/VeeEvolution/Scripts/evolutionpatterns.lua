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
	hmcBarrageN(mSide, mCurve, mCurveAcc, mCurveMin, mCurveMax, mCurvePingPong);
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

function hmcSimpleSpinner(mCurve)
	side = getRandomSide()

	for i = 0, getSides() / 2, 1 do
		wallHMCurve(side + i * 2, mCurve)
	end
end