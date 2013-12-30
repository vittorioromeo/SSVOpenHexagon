-- include useful files
u_execScript("utils.lua")
u_execScript("common.lua")
u_execScript("commonpatterns.lua")

-- this function adds a pattern to the timeline based on a key
function addPattern(mKey)
		if mKey == 0 then pAltBarrage(math.random(2, 4), 2) 
	elseif mKey == 1 then pMirrorSpiral(math.random(3, 6), 0)
	elseif mKey == 2 then pBarrageSpiral(math.random(0, 3), 1, 1)
	elseif mKey == 3 then pBarrageSpiral(math.random(0, 2), 1.2, 2)
	elseif mKey == 4 then pBarrageSpiral(2, 0.7, 1)
	elseif mKey == 5 then pInverseBarrage(0)
	elseif mKey == 6 then pTunnel(math.random(1, 3))
	elseif mKey == 7 then pMirrorWallStrip(1, 0)
	end
end

-- shuffle the keys, and then call them to add all the patterns
-- shuffling is better than randomizing - it guarantees all the patterns will be called
keys = { 0, 0, 1, 1, 2, 2, 3, 4, 5, 5, 6, 7, 7 }
keys = shuffle(keys)
index = 0

-- onInit is an hardcoded function that is called when the level is first loaded
function onInit()
	l_setSpeedMult(1.74)
	l_setSpeedInc(0.18)
	l_setRotationSpeed(0.13)
	l_setRotationSpeedMax(0.4)
	l_setRotationSpeedInc(0.04)
	l_setDelayMult(1.0)
	l_setDelayInc(-0.03)
	l_setFastSpin(0.0)
	l_setSides(6)
	l_setSidesMin(5)
	l_setSidesMax(6)
	l_setIncTime(15)

	l_setPulseMin(75)
	l_setPulseMax(91)
	l_setPulseSpeed(1.5)
	l_setPulseSpeedR(0.6)
	l_setPulseDelayMax(9)

	l_setBeatPulseMax(17)
	l_setBeatPulseDelayMax(23.8)

	enableSwapIfDMGreaterThan(2)
	disableIncIfDMGreaterThan(2.5)
end

-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()
end

-- onStep is an hardcoded function that is called when the level timeline is empty
-- onStep should contain your pattern spawning logic
function onStep()	
	addPattern(keys[index])
	index = index + 1
 	
	if index - 1 == #keys then
		index = 1
	end
end

-- onIncrement is an hardcoded function that is called when the level difficulty is incremented
function onIncrement()
end

-- onUnload is an hardcoded function that is called when the level is closed/restarted
function onUnload()
end

-- onUpdate is an hardcoded function that is called every frame
function onUpdate(mFrameTime)
end