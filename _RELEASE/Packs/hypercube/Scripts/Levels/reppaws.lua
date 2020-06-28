-- include useful files
u_execScript("utils.lua")
u_execScript("common.lua")
u_execScript("commonpatterns.lua")
u_execScript("nextpatterns.lua")
u_execScript("evolutionpatterns.lua")

gap = 6

-- this function adds a pattern to the timeline based on a key
function addPattern(mKey)
		if mKey == 0 then cBarrageN(getRandomSide(), gap) t_wait(getPerfectDelayDM(THICKNESS) * 6)
	elseif mKey == 1 then hmcSimpleBarrageSNeigh(getRandomSide(), 0, gap) t_wait(getPerfectDelayDM(THICKNESS) * 6)
	end
end

-- shuffle the keys, and then call them to add all the patterns
-- shuffling is better than randomizing - it guarantees all the patterns will be called
keys = { 0, 0, 0, 1, 1, 1 }
keys = shuffle(keys)
index = 0

-- onInit is an hardcoded function that is called when the level is first loaded
function onInit()
	l_setSpeedMult(3.0)
	l_setSpeedInc(0.0)
	l_setRotationSpeed(0.22)
	l_setRotationSpeedMax(0.4)
	l_setRotationSpeedInc(0.0)
	l_setDelayMult(1.35)
	l_setDelayInc(0.0)
	l_setFastSpin(71.0)
	l_setSides(32)
	l_setSidesMin(32)
	l_setSidesMax(32)
	l_setIncTime(10)

	l_setWallSkewLeft(15)

	l_setPulseMin(61)
	l_setPulseMax(80)
	l_setPulseSpeed(3.6)
	l_setPulseSpeedR(1.45)
	l_setPulseDelayMax(6.8)

	l_setBeatPulseMax(20)
	l_setBeatPulseDelayMax(26.1)

	l_setSwapEnabled(true)
	l_addTracked("gap", "gap size")
end

-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()
	syncCurveWithRotationSpeed(0, 0)
	m_messageAdd("remember, swap with spacebar!", 120)
end

-- onStep is an hardcoded function that is called when the level timeline is empty
-- onStep should contain your pattern spawning logic
function onStep()	
	addPattern(keys[index])

	index = index + 1
	
	if index - 1 == #keys then
		index = 1
		keys = shuffle(keys)
	end
end


-- onIncrement is an hardcoded function that is called when the level difficulty is incremented
function onIncrement()
	if gap > 2 then
		gap = gap -1
		m_messageAddImportant("Gap size: "..gap, 120)
	end
end

-- onUnload is an hardcoded function that is called when the level is closed/restarted
function onUnload()
end

-- onUpdate is an hardcoded function that is called every frame
function onUpdate(mFrameTime)
end