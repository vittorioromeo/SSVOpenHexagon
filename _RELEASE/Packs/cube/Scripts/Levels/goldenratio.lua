-- include useful files
u_execScript("utils.lua")
u_execScript("common.lua")
u_execScript("commonpatterns.lua")

-- this function adds a pattern to the timeline based on a key
function addPattern(mKey)
	if mKey == 0 then pBarrageSpiral(math.random(5, 9), 0.41, 1)
	elseif mKey == 1 then pMirrorSpiralDouble(math.random(8, 10), 0)
	elseif mKey == 2 then pMirrorSpiral(math.random(2, 5), 0)
	end
end

-- shuffle the keys, and then call them to add all the patterns
-- shuffling is better than randomizing - it guarantees all the patterns will be called
keys = { 0, 0, 1, 1, 2 }
keys = shuffle(keys)
index = 0

-- onInit is an hardcoded function that is called when the level is first loaded
function onInit()
	l_setSpeedMult(1.7)
	l_setSpeedInc(0.1)
	l_setRotationSpeed(0.0)
	l_setRotationSpeedMax(0.4)
	l_setRotationSpeedInc(0.1)
	l_setDelayMult(1.0)
	l_setDelayInc(0.0)
	l_setFastSpin(50.0)
	l_setSides(6)
	l_setSidesMin(5)
	l_setSidesMax(7)
	l_setIncTime(10)

	l_setPulseMin(60)
	l_setPulseMax(87)
	l_setPulseSpeed(1.2)
	l_setPulseSpeedR(1)
	l_setPulseDelayMax(12.9)

	l_setBeatPulseMax(17)
	l_setBeatPulseDelayMax(26.2)

	l_setWallSkewRight(-20)
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