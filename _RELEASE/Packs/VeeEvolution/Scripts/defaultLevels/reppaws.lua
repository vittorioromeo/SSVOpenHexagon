-- include useful files
execScript("utils.lua")
execScript("common.lua")
execScript("commonpatterns.lua")
execScript("nextpatterns.lua")
execScript("evolutionpatterns.lua")

gap = 4

-- this function adds a pattern to the timeline based on a key
function addPattern(mKey)
		if mKey == 0 then pMirrorSpiralDouble(math.random(1, 2), 4)
	elseif mKey == 1 then rWallEx(math.random(0, getSides()), math.random(1, 2)) wait(getPerfectDelay(THICKNESS) * 2.8)
	elseif mKey == 2 then pMirrorWallStrip(1, 2)
	elseif mKey == 3 then cBarrageN(getRandomSide(), gap) wait(getPerfectDelay(THICKNESS) * 6)
	elseif mKey == 4 then hmcSimpleBarrageSNeigh(getRandomSide(), 0.07 * getRandomSide(), gap) wait(getPerfectDelay(THICKNESS) * 6)
	end
end

-- shuffle the keys, and then call them to add all the patterns
-- shuffling is better than randomizing - it guarantees all the patterns will be called
keys = { 0, 1, 2, 3, 3, 3, 3, 4, 4, 4, 4 }
keys = shuffle(keys)
index = 0

-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()
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
	if gap > 2 then
		gap = gap -1
		messageImportantAdd("Gap size: "..gap, 120)
	end
end

-- onUnload is an hardcoded function that is called when the level is closed/restarted
function onUnload()
end

-- onUpdate is an hardcoded function that is called every frame
function onUpdate(mFrameTime)
end