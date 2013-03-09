-- include useful files
execScript("utils.lua")
execScript("common.lua")
execScript("commonpatterns.lua")
            
extra = 0
incrementTime = 5

-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()	
	messageImportantAdd("level: "..(extra + 1).." / time: "..incrementTime, 170)
end

-- onStep is an hardcoded function that is called when the level timeline is empty
-- onStep should contain your pattern spawning logic
function onStep()	
	rWallEx(getRandomSide(), extra)
	wait(getPerfectDelay(THICKNESS) * 6)
end

-- onIncrement is an hardcoded function that is called when the level difficulty is incremented
function onIncrement()	
	playSound("beep.ogg")
	playSound("VeeEndurance_test.ogg")
	
	extra = extra + 1
	incrementTime = incrementTime + 2
	
	setLevelValueInt("sides", getSides() + 2)
	setLevelValueInt("increment_time", incrementTime)
		
	messageImportantAdd("level: "..(extra + 1).." / time: "..incrementTime, 170)
end

-- onUnload is an hardcoded function that is called when the level is closed/restarted
function onUnload()
end

-- onUpdate is an hardcoded function that is called every frame
function onUpdate(mFrameTime)
end