-- include useful files
execScript("utils.lua")
execScript("common.lua")
execScript("commonpatterns.lua")

level = 0
levelTracked = 1
incrementTime = 3

-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()	
	messageImportantAdd("level: "..(level + 1).." / time: "..incrementTime, 170)
end

-- onStep is an hardcoded function that is called when the level timeline is empty
-- onStep should contain your pattern spawning logic
function onStep()
	disableRandomSideChanges()	
	cBarrage(getRandomSide())
	wait(getPerfectDelay(THICKNESS) * 6.55)
end

-- onInit is an hardcoded function that is called when the level is first loaded
function onInit()
	l_setSpeedMult(2.25)
	l_setSpeedInc(0.0)
	l_setRotationSpeed(0.0)
	l_setRotationSpeedMax(0.0)
	l_setRotationSpeedInc(0.0)
	l_setDelayMult(1.0)
	l_setDelayInc(0.0)
	l_setFastSpin(0.0)
	l_setSides(3)
	l_setSidesMin(0)
	l_setSidesMax(0)
	l_setIncTime(5)

	l_setPulseMin(58)
	l_setPulseMax(90)
	l_setPulseSpeed(2.2)
	l_setPulseSpeedR(0.65)
	l_setPulseDelayMax(1)

	l_setBeatPulseMax(17)
	l_setBeatPulseDelayMax(23.8)

	l_setRadiusMin(40)
	l_addTracked("levelTracked", "level")
end

-- onIncrement is an hardcoded function that is called when the level difficulty is incremented
function onIncrement()	
	playSound("beep.ogg")
	playSound("VeeEndurance_test.ogg")
	
	level = level + 1
	levelTracked = level + 1
	incrementTime = incrementTime + 2
	
	setLevelSides(getSides() + 1)
	setLevelIncrementTime(incrementTime)
		
	messageImportantAdd("level: "..(level + 1).." / time: "..incrementTime, 170)
end

-- onUnload is an hardcoded function that is called when the level is closed/restarted
function onUnload()
end

-- onUpdate is an hardcoded function that is called every frame
function onUpdate(mFrameTime)
end