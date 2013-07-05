-- include useful files
execScript("utils.lua")
execScript("common.lua")
execScript("commonpatterns.lua")
execScript("nextpatterns.lua")
execScript("evolutionpatterns.lua")

-- this function adds a pattern to the timeline based on a key
function addPattern(mKey)
		if mKey == 0 then cBarrage(0)
	elseif mKey == 1 then hmcBarrageN(0, 0, 0, 0.05, -3.8, 2.7, true); wait(55)
	elseif mKey == 2 then hmcBarrageN(0, 0, 0, -0.05, -2.7, 3.8, true); wait(55)
	end
end

-- shuffle the keys, and then call them to add all the patterns
-- shuffling is better than randomizing - it guarantees all the patterns will be called
keys = { 1, 1, 2, 2 }
keys = shuffle(keys)
index = 0

-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()
	tutorialMode()
	stopIncrement()

	messageImportantAdd("welcome to the evolution tutorial", 120)
	messageImportantAdd("today you'll be introduced to...", 120)
	messageImportantAdd("1. curving walls", 100)
	messageImportantAdd("2. restrictions", 100)
	messageImportantAdd("let's start with curving walls!", 120)

	messageImportantAdd("they can be simple...", 120)
	messageImportantAdd("", 120 * 5 + 80)
	
	wait(120 * 5)
	hmcSimpleBarrage(1)
	wait(100)
	hmcSimpleBarrage(-1)
	wait(50)
	hmcSimpleBarrage(1)
	wait(100)
	hmcSimpleBarrage(-2.5)
	wait(80)
	hmcSimpleBarrage(2.5)
	wait(80)
	hmcSimpleBarrage(3)

	wait(50)
	messageImportantAdd("...in various patterns...", 130)
	messageImportantAdd("", 120 * 5 + 80)
	wait(130)

	hmcSimpleTwirl(5, 1, 0)
	wait(50)
	hmcSimpleTwirl(5, -2.5, 0.3)

	messageImportantAdd("...or can accellerate!", 130)
	messageImportantAdd("", 120 * 5 + 80)
	wait(130)

	hmcBarrage(0, 0.05, -1.5, 3, true)
	wait(80)
	hmcBarrage(0, -0.05, -3, 3, true)
	wait(100)
	hmcBarrage(0, 0.1, -2, 2, true)
	wait(100)
	hmcBarrage(0, 0.1, -3, 3, true)
	wait(200)

	messageImportantAdd("they can also do crazy stuff", 120)
	messageImportantAdd("", 120 * 5 + 80)

	hmcSimpleCage(2.5, 1)
	wait(80)
	hmcSimpleCage(2.5, -1)
	wait(100)
	hmcSimpleCage(2.5, 1)
	hmcSimpleCage(2.5, 1)
	wait(100)
	hmcSimpleCage(2.5, 1)
	hmcSimpleCage(2.5, -1)
	wait(100)
	hmcSimpleSpinner(1)
	wait(100)
	hmcSimpleSpinner(-2)
	wait(100)
	hmcSimpleSpinner(3)
	wait(100)
	hmcSimpleCage(1.5, 1)
	hmcSimpleCage(2.5, 1)
	wait(100)
	hmcSimpleCage(1.5, 1)
	hmcSimpleCage(2.5, -1)
	wait(100)
	hmcSimpleSpinner(1)
	hmcSimpleSpinner(1.2)
	wait(100)
	hmcSimpleSpinner(1)
	hmcSimpleSpinner(-1.2)
end

-- onStep is an hardcoded function that is called when the level timeline is empty
-- onStep should contain your pattern spawning logic
function onStep()	
	-- addPattern(keys[index])
	-- index = index + 1
	
	-- if index - 1 == table.getn(keys) then
--		index = 1
--	end
end

-- onIncrement is an hardcoded function that is called when the level difficulty is incremented
function onIncrement()
end

-- onUnload is an hardcoded function that is called when the level is closed/restarted
function onUnload()
end

-- continuous direction change (even if not on level increment)
dirChangeTime = 400
hueIMin = 0.0
hueIMax = 22.0
hueIStep = 0.0065

-- onUpdate is an hardcoded function that is called every frame
function onUpdate(mFrameTime)
	dirChangeTime = dirChangeTime - mFrameTime;
	if dirChangeTime < 0 then
		-- do not change direction while fast spinning
		if isFastSpinning() == false then
			setLevelValueFloat("rotation_speed", getLevelValueFloat("rotation_speed") * -1)
			dirChangeTime = 400
		end
	end 

	setStyleValueFloat("hue_increment", getStyleValueFloat("hue_increment") + hueIStep)
	if(getStyleValueFloat("hue_increment") > hueIMax) then hueIStep = hueIStep * -1 end
	if(getStyleValueFloat("hue_increment") < hueIMin) then hueIStep = hueIStep * -1 end
end