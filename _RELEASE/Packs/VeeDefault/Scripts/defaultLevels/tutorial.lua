-- include useful files
execScript("utils.lua")
execScript("common.lua")
execScript("commonpatterns.lua")

-- this function adds a pattern to the timeline based on a key
function addPattern(mKey)
	if mKey == 0 then pBarrageSpiral(math.random(1, 2), 1, 1) 
	elseif mKey == 1 then pInverseBarrage(0)
	elseif mKey == 2 then pAltBarrage(math.random(1, 3), 2)
	end
end

-- shuffle the keys, and then call them to add all the patterns
-- shuffling is better than randomizing - it guarantees all the patterns will be called
keys = { 0, 1, 2 }
keys = shuffle(keys)
index = 0

-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()
	tutorialMode()

	messageImportantAdd("welcome to open hexagon 2", 130)
	messageImportantAdd("use left/right to rotate", 130)
	messageImportantAdd("avoid the walls!", 130)
	eventStopTimeS(6) eventWaitS(6)
	
	eventStopTimeS(8) eventWaitUntilS(12)
	messageImportantAdd("great job!", 130)
	messageImportantAdd("after a while, things get harder", 130)
	messageImportantAdd("get to 45 seconds to win!", 130)

	eventWaitUntilS(42)
	messageImportantAdd("well done!", 130)
	messageImportantAdd("now play some real levels!", 138)

	eventWaitUntilS(45)
	eventKill()
end

-- onStep is an hardcoded function that is called when the level timeline is empty
-- onStep should contain your pattern spawning logic
function onStep()	
	addPattern(keys[index])
	index = index + 1
	
	if index - 1 == table.getn(keys) then
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