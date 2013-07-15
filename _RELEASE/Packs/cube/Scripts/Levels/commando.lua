-- include useful files
execScript("utils.lua")
execScript("common.lua")
execScript("commonpatterns.lua")
            
extra = 0
level = 1
incrementTime = 5

-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()	
	disableRandomSideChanges()
	messageImportantAdd("level: "..(extra + 1).." / time: "..incrementTime, 170)
end

-- onStep is an hardcoded function that is called when the level timeline is empty
-- onStep should contain your pattern spawning logic
function onStep()	
	rWallEx(getRandomSide(), extra)
	wait(getPerfectDelay(THICKNESS) * 6)
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
	l_setSides(4)
	l_setSidesMin(0)
	l_setSidesMax(0)
	l_setIncTime(5)

	l_setPulseMin(75)
	l_setPulseMax(91)
	l_setPulseSpeed(2)
	l_setPulseSpeedR(1)
	l_setPulseDelayMax(0.7)

	l_setBeatPulseMax(17)
	l_setBeatPulseDelayMax(23.8)
	
	l_addTracked("level", "level")
end

-- onIncrement is an hardcoded function that is called when the level difficulty is incremented
function onIncrement()	
	playSound("beep.ogg")
	playSound("VeeEndurance_test.ogg")
	
	extra = extra + 1
	level = extra + 1
	incrementTime = incrementTime + 2
	
	setLevelSides(getSides() + 2)
	setLevelIncrementTime(incrementTime)
		
	messageImportantAdd("level: "..(extra + 1).." / time: "..incrementTime, 170)
end

-- onUnload is an hardcoded function that is called when the level is closed/restarted
function onUnload()
end

-- onUpdate is an hardcoded function that is called every frame
function onUpdate(mFrameTime)
end