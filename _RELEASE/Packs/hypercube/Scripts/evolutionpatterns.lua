u_execScript("common.lua")
u_execScript("commonpatterns.lua")
u_execScript("utils.lua")
u_execScript("alternativepatterns.lua")
u_execScript("nextpatterns.lua")

hueModifier = 0.2
sync = false
syncRndMin = 0
syncRndMax = 0

curveMult = 1

function syncCurveWithRotationSpeed(mRndMin, mRndMax)
	sync = true
	syncRndMin = mRndMin
	syncRndMax = mRndMax
end

function setCurveMult(mMult)
	curveMult = mMult
end

function wallHMCurveAcc(mSide, mCurve, mCurveAcc, mCurveMin, mCurveMax, mCurvePingPong)
	if sync == true then
		mCurve = l_getRotationSpeed() * 10.0
		mCurve = mCurve + (math.random(syncRndMin, syncRndMax) / 100.0)
	end

	w_wallHModCurveData(hueModifier, mSide, THICKNESS, mCurve * (u_getDifficultyMult() ^ 0.25) * curveMult, mCurveAcc, mCurveMin, mCurveMax, mCurvePingPong)
end

function wallHMCurve(mSide, mCurve)
	wallHMCurveAcc(mSide, mCurve, 0, 0, 0, false)
end

function hmcBarrageN(mSide, mNeighbors, mCurve, mCurveAcc, mCurveMin, mCurveMax, mCurvePingPong)
	for i = mNeighbors, l_getSides() - 2 - mNeighbors, 1 do
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
		t_wait(delay)
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

	for i = 0, l_getSides() / 2, 1 do
		wallHMCurve(side + i * 2, mCurve)
	end
end

function hmcSimpleSpinnerS(mSide, mCurve)
	for i = 0, l_getSides() / 2, 1 do
		wallHMCurve(mSide + i * 2, mCurve)
	end
end

function hmcSimpleSpinnerSAcc(mSide, mCurve, mCurveAcc, mCurveMin, mCurveMax, mCurvePingPong)
	for i = 0, l_getSides() / 2, 1 do
		wallHMCurveAcc(mSide + i * 2, mCurve, mCurveAcc, mCurveMin, mCurveMax, mCurvePingPong)
	end
end

function hmcDefSpinner()
	t_wait(getPerfectDelayDM(THICKNESS) * 3.2)
	hmcSimpleSpinner(math.random(10, 19) / 10.0 * getRandomDir())
	t_wait(getPerfectDelayDM(THICKNESS) * 5.9)
end

function hmcDefBarrage()
	t_wait(getPerfectDelayDM(THICKNESS) * 3.1)
	hmcSimpleBarrage(math.random(10, 20) / 10.0 * getRandomDir())
	t_wait(getPerfectDelayDM(THICKNESS) * 5)
end

function hmcDef2Cage()
	t_wait(getPerfectDelayDM(THICKNESS) * 2.1)
	side = getRandomSide()
	rndspd = math.random(10, 20) / 10.0

	t_wait(getPerfectDelayDM(THICKNESS) * 3.1)
	hmcSimpleCageS(rndspd, -1, side)
	t_wait(getPerfectDelayDM(THICKNESS) * 1.1)
	hmcSimpleCageS(rndspd, -1, side)
	t_wait(getPerfectDelayDM(THICKNESS) * 1.1)
	hmcSimpleCageS(rndspd, -1, side)
	t_wait(getPerfectDelayDM(THICKNESS) * 5.3)
end

function hmcDef2CageD()
	t_wait(getPerfectDelayDM(THICKNESS) * 2.1)

	side = getRandomSide()
	oppositeSide = getHalfSides() + side
	rndspd = math.random(10, 17) / 10.0

	t_wait(getPerfectDelayDM(THICKNESS) * 3.1)
	hmcSimpleCageS(rndspd, -1, side)
	t_wait(getPerfectDelayDM(THICKNESS) * 1.1)
	hmcSimpleCageS(rndspd, -1, side)
	t_wait(getPerfectDelayDM(THICKNESS) * 1.1)
	hmcSimpleCageS(rndspd, -1, side)
	t_wait(getPerfectDelayDM(THICKNESS) * 6.0)
	hmcSimpleCageS(rndspd, -1, oppositeSide)
	t_wait(getPerfectDelayDM(THICKNESS) * 1.1)
	hmcSimpleCageS(rndspd, -1, oppositeSide)
	t_wait(getPerfectDelayDM(THICKNESS) * 1.1)
	hmcSimpleCageS(rndspd, -1, oppositeSide)
	t_wait(getPerfectDelayDM(THICKNESS) * 9.2)
end

function hmcSimpleBarrageSpiral(mTimes, mDelayMult, mStep, mCurve, mNeighbors)
	delay = getPerfectDelayDM(THICKNESS) * 6.2 * mDelayMult
	startSide = getRandomSide()
	loopDir = mStep * getRandomDir()	
	j = 0
	
	for i = 0, mTimes do
		hmcSimpleBarrageSNeigh(startSide + j, mCurve, mNeighbors)
		j = j + loopDir
		t_wait(delay)
		if(l_getSides() < 6) then t_wait(delay * 0.7) end
	end
	
	t_wait(getPerfectDelayDM(THICKNESS) * 6.1)
end

function hmcSimpleBarrageSpiralRnd(mTimes, mDelayMult, mCurve, mNeighbors)
	delay = getPerfectDelayDM(THICKNESS) * 6.2 * mDelayMult
	startSide = getRandomSide()
	
	for i = 0, mTimes do
		hmcSimpleBarrageSNeigh(getRandomSide(), mCurve, mNeighbors)
		t_wait(delay)
		if(l_getSides() < 6) then t_wait(delay * 0.7) end
	end
	
	t_wait(getPerfectDelayDM(THICKNESS) * 6.1)
end

function hmcSimpleBarrageSpiralStatic(mTimes, mDelayMult, mStep, mCurve, mNeighbors)
	delay = getPerfectDelay(THICKNESS) * 5.6 * mDelayMult
	startSide = getRandomSide()
	loopDir = mStep * getRandomDir()	
	j = 0
	
	for i = 0, mTimes do
		hmcSimpleBarrageSNeigh(startSide + j, mCurve, mNeighbors)
		j = j + loopDir
		t_wait(delay)
		if(l_getSides() < 6) then t_wait(delay * 0.6) end
	end
	
	t_wait(getPerfectDelayDM(THICKNESS) * 6.1)
end

function hmcDefBarrageSpiral()
	hmcSimpleBarrageSpiral(math.random(1, 3), 1, 1, math.random(5, 15) / 10.0 * getRandomDir(), 0)
end

function hmcDefBarrageSpiralRnd()
	hmcSimpleBarrageSpiralRnd(math.random(1, 3), 1, math.random(5, 15) / 10.0 * getRandomDir(), 0)
end

function hmcDefBarrageSpiralFast()
	hmcSimpleBarrageSpiral(math.random(1, 3), 0.8, 1, math.random(5, 15) / 10.0 * getRandomDir(), 0)
end

function hmcDefBarrageSpiralSpin()
	hmcSimpleBarrageSpiralStatic(math.random(7, 14), 0.25, 1, math.random(5, 18) / 10.0 * getRandomDir(), 2)
end

function hmcDefBarrageInv()
	t_wait(getPerfectDelayDM(THICKNESS) * 2.0)
	delay = getPerfectDelay(THICKNESS) * 5.6 
	side = getRandomSide()
	rndspd = math.random(10, 20) / 10.0
	oppositeSide = getRandomSide() + getHalfSides()

	hmcSimpleBarrageSNeigh(side, rndspd * getRandomDir(), 0)
	t_wait(delay)

	hmcSimpleBarrageSNeigh(oppositeSide, rndspd * getRandomDir(), 0)
	t_wait(delay)
end

function hmcDefAccelBarrage()
	t_wait(getPerfectDelayDM(THICKNESS) * 1.5)
	c = math.random(50, 100) / 1000.0 * getRandomDir()
	min = math.random(5, 35) / 10.0 * -1
	max = math.random(5, 35) / 10.0
	hmcBarrage(0, c, min, max, true)
	t_wait(getPerfectDelayDM(THICKNESS) * 6.1)
end

function hmcDefAccelBarrageDouble()
	t_wait(getPerfectDelayDM(THICKNESS) * 1.5)
	c = math.random(50, 100) / 1000.0 * getRandomDir()
	min = math.random(5, 35) / 10.0 * -1
	max = math.random(5, 35) / 10.0
	hmcBarrage(0, c, min, max, true)
	t_wait(getPerfectDelayDM(THICKNESS) * 2.1)
	hmcBarrage(0, c, min, max, true)
	t_wait(getPerfectDelayDM(THICKNESS) * 6.1)
end

function hmcDefSpinnerSpiral()
	t_wait(getPerfectDelayDM(THICKNESS) * 1.5)
	side = getRandomSide()
	c = math.random(10, 20) / 10.0 * getRandomDir()

	t_wait(getPerfectDelayDM(THICKNESS) * 3.1)

	for i = 0, math.random(4, 8) do
		hmcSimpleSpinnerS(side, c)
		t_wait(getPerfectDelayDM(THICKNESS) * 1.15)
	end

	t_wait(getPerfectDelayDM(THICKNESS) * 5)
end

function getRndMinDM(mNum)
	return math.random(mNum - u_getDifficultyMult() ^ 3, mNum)
end

function getRndMaxDM(mNum)
	return math.random(mNum, mNum + u_getDifficultyMult() ^ 2.25)
end

function hmcDefSpinnerSpiralAcc()
	t_wait(getPerfectDelayDM(THICKNESS) * 2.1)
	t_wait(getPerfectDelayDM(THICKNESS) * 2.1)
	side = getRandomSide()
	acc = math.random(getRndMinDM(50), getRndMaxDM(100)) / 1000.0 * getRandomDir()
	min = math.random(getRndMinDM(12), getRndMaxDM(28)) / 10.0 * -1
	max = math.random(getRndMinDM(12), getRndMaxDM(28)) / 10.0

	t_wait(getPerfectDelayDM(THICKNESS) * 3.1)

	for i = 0, math.random(4, 8) do
		hmcSimpleSpinnerSAcc(side, 0, acc, min, max, true)
		t_wait(getPerfectDelay(THICKNESS) * 0.8)
	end

	t_wait(getPerfectDelayDM(THICKNESS) * 5.3)
end