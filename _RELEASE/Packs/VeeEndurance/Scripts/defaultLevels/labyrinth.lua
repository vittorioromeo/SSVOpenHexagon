-- include useful files
execScript("utils.lua")
execScript("common.lua")
execScript("commonpatterns.lua")

level = 0
incrementTime = 3

-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()	
	messageImportantAdd("level: "..(level + 1).." / time: "..incrementTime, 170)
end

-- onStep is an hardcoded function that is called when the level timeline is empty
-- onStep should contain your pattern spawning logic
function onStep()	
	cBarrage(getRandomSide())
	wait(getPerfectDelay(THICKNESS) * 6.55)
end

-- onIncrement is an hardcoded function that is called when the level difficulty is incremented
function onIncrement()	
	playSound("beep.ogg")
	playSound("VeeEndurance_test.ogg")
	
	level = level + 1
	incrementTime = incrementTime + 2
	
	setLevelValueInt("sides", getSides() + 1)
	setLevelValueInt("increment_time", incrementTime)
	--if level < 6 then setLevelValueFloat("rotation_speed", getLevelValueFloat("rotation_speed") + 0.1) end
		
	messageImportantAdd("level: "..(level + 1).." / time: "..incrementTime, 170)
end

-- onUnload is an hardcoded function that is called when the level is closed/restarted
function onUnload()
end

-- onUpdate is an hardcoded function that is called every frame
function onUpdate(mFrameTime)
end