u_execScript("common.lua")

function pAltMirrorSpiral(mTimes, mExtra)
	oldThickness = THICKNESS
	THICKNESS = getPerfectThickness(THICKNESS)
	delay = getPerfectDelay(THICKNESS)
	startSide = getRandomSide()
	loopDir = getRandomDir()	
	for k = 1, #mTimes do
		for i = 1, mTimes[k] do
			rWallEx(startSide, mExtra)
			if (k % 2) == 0 then
				startSide = startSide + loopDir
			else
				startSide = startSide - loopDir
			end
			t_wait(delay)
		end 
	end

	THICKNESS = oldThickness
	
	t_wait(getPerfectDelay(THICKNESS) * 6.5)
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
			w_wall(startSide, myThickness + 5 * l_getSpeedMult() * delay)
		end
		
		cBarrageN(startSide + loopDir,mFree)
		t_wait(delay)
		
		loopDir = loopDir * -1
	end
	
	THICKNESS = oldThickness
end

function cycle(mSides)
	eArray = {}
	j = getRandomSide()
	for i = 1, mSides do 
		eArray[i] = (i + j) % mSides + 1
	end
	return eArray
end

function pLadder(mTimes,mArray,myThickness)

	delay = getPerfectDelay(myThickness)

	local eArray = {}
	l = 1
	s = (#mArray)/l_getSides()
	t = math.random(0,100)

	for i = 1, mTimes do
		q = (i+t) % s + 1
		for k = 1, l_getSides() do
			if(mArray[(q-1)*l_getSides() + k] ~= 0) then
				eArray[l] = 1
			else
				eArray[l] = 0
			end
			l = l + 1
		end
		
		if i ~= mTimes then
			for j = 1, 3 do
				for k = 1,l_getSides() do
					if(mArray[(q-1)*l_getSides() + k] == 2) then
						eArray[l] = 1
					else
						eArray[l] = 0
					end
					l = l + 1
				end
			end
		end
	end

	patternizer(eArray,myThickness)
	t_wait(delay*2)
	
end

function patternizer(mArray,myThickness)
	delay = getPerfectDelay(myThickness)
	eArray = cycle(l_getSides())

	j = math.floor(#mArray / l_getSides())
	
	for i = 1, j do
		for k = 1, l_getSides() do
			if mArray[(i - 1)*l_getSides() + k] == 1 then
				w_wall(eArray[k], myThickness)
			end
		end
		t_wait(delay)
	end
end