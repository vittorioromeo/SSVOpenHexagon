-- include useful files
execScript("utils.lua")
execScript("common.lua")
execScript("commonpatterns.lua")

extra = 0
incrementTime = 5

-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()
	log("level onLoad")
end

-- onStep is an hardcoded function that is called when the level timeline is empty
-- onStep should contain your pattern spawning logic
function onStep()
	log("level onStep")
	
	rWallEx(getRandomSide(), extra)
	wait((getPerfectDelay(THICKNESS) - ((math.abs(6 - getSides())) * 1.13)) * (4 + extra / 1.25))
end

-- onIncrement is an hardcoded function that is called when the level difficulty is incremented
function onIncrement()
	log("level onIncrement")	
	
	playSound("beep")
	
	extra = extra + 1
	incrementTime = incrementTime + 2
	
	setLevelValueInt("sides", getSides() + 2)
	setLevelValueInt("increment_time", incrementTime)
	
	messageImportantAdd("level: "..extra.." / time: "..incrementTime, 170)
end

-- onUnload is an hardcoded function that is called when the level is closed/restarted
function onUnload()
	log("level onUnload")	
end
