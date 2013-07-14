-- include useful files
execScript("utils.lua")
execScript("common.lua")
execScript("commonpatterns.lua")

level = 0
levelTracked = 1
incrementTime = 3

-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()	
	addTracked("levelTracked", "level")
	messageImportantAdd("level: "..(level + 1).." / time: "..incrementTime, 170)
end

-- onStep is an hardcoded function that is called when the level timeline is empty
-- onStep should contain your pattern spawning logic
function onStep()
	disableRandomSideChanges()	
	cBarrage(getRandomSide())
	wait(getPerfectDelay(THICKNESS) * 6.55)
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