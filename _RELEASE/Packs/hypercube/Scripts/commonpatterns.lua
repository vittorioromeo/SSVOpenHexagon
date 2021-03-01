u_execScript("common.lua")

-- pAltBarrage: spawns a series of cAltBarrage
function pAltBarrage(mTimes, mStep)
	local delay = getPerfectDelayDM(THICKNESS) * 5.6

	for i = 0, mTimes do
		cAltBarrage(i, mStep)
		t_wait(delay)
	end

	t_wait(delay)
end

-- pSpiral: spawns a spiral of cWallEx
function pSpiral(mTimes, mExtra)
	local oldThickness = THICKNESS
	THICKNESS = getPerfectThickness(THICKNESS)
	local delay = getPerfectDelay(THICKNESS)
	local startSide = getRandomSide()
	local loopDir = getRandomDir()
	local j = 0

	for i = 0, mTimes do
		cWallEx(startSide + j, mExtra)
		j = j + loopDir
		t_wait(delay)
	end

	THICKNESS = oldThickness

	t_wait(getPerfectDelayDM(THICKNESS) * 6.5)
end

-- pMirrorSpiral: spawns a spiral of rWallEx
function pMirrorSpiral(mTimes, mExtra)
	local oldThickness = THICKNESS
	THICKNESS = getPerfectThickness(THICKNESS)
	local delay = getPerfectDelay(THICKNESS)
	local startSide = getRandomSide()
	local loopDir = getRandomDir()
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
    local oldThickness = THICKNESS
    THICKNESS = getPerfectThickness(THICKNESS)
    local delay = getPerfectDelayDM(THICKNESS)
    local startSide = getRandomSide()
    local loopDir = getRandomDir()
    local j = 0

    for i = 0, mTimes do
        rWallEx(startSide + j, mExtra)
        j = j + loopDir
        t_wait(delay)
    end

    rWallEx(startSide + j, mExtra)
    t_wait(delay * 0.9)

    for i = 0, mTimes + 1 do
        rWallEx(startSide + j, mExtra)
        j = j - loopDir
        t_wait(delay)
    end

    THICKNESS = oldThickness
    t_wait(getPerfectDelayDM(THICKNESS) * 7.5)
end

-- pBarrageSpiral: spawns a spiral of cBarrage
function pBarrageSpiral(mTimes, mDelayMult, mStep)
	local delay = getPerfectDelayDM(THICKNESS) * 5.6 * mDelayMult
	local startSide = getRandomSide()
	local loopDir = mStep * getRandomDir()
	local j = 0

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
	local delay = (getPerfectDelayDM(THICKNESS) * 5.42) * (mDelayMult / (u_getDifficultyMult() ^ 0.4)) * (u_getSpeedMultDM() ^ 0.35)
	local startSide = getRandomSide()
	local loopDir = mStep * getRandomDir()
	local j = 0

	for i = 0, mTimes do
		cBarrage(startSide + j)
		j = j + loopDir
		t_wait(delay)
		if(l_getSides() < 6) then t_wait(delay * 0.49) end
	end

	t_wait(getPerfectDelayDM(THICKNESS) * (6.76 * (u_getDifficultyMult() ^ 0.7)))
end

-- pWallExVortex: spawns left-left right-right spiral patters
function pWallExVortex(mTimes, mStep, mExtraMult)
	local delay = getPerfectDelayDM(THICKNESS) * 5.0
	local startSide = getRandomSide()
	local loopDir = getRandomDir()
	local currentSide = startSide

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
	local delay = getPerfectDelayDM(THICKNESS) * 9.9
	local startSide = getRandomSide()

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
	local side = getRandomSide()
	local oldSide = 0

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
	local delay = getPerfectDelayDM(THICKNESS) * 3.65
	local startSide = getRandomSide()

	for i = 0, mTimes do
		rWallEx(startSide, mExtra)
		t_wait(delay)
	end

	t_wait(getPerfectDelayDM(THICKNESS) * 5.00)
end

-- pTunnel: forces you to circle around a very thick wall
function pTunnel(mTimes)
	local oldThickness = THICKNESS
	local myThickness = getPerfectThickness(THICKNESS)
	local delay = getPerfectDelay(myThickness) * 5
	local startSide = getRandomSide()
	local loopDir = getRandomDir()

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
