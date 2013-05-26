-- include useful files
execScript("utils.lua")
execScript("common.lua")
execScript("commonpatterns.lua")

-- this function adds a pattern to the timeline based on a key
function addPattern(mKey)
		if mKey == 0 then cWallEx(math.random(0, getSides()), math.random(1, 2)) wait(getPerfectDelay(THICKNESS) * 2.5)
	elseif mKey == 1 then pMirrorSpiralDouble(math.random(1, 2), 4)
	elseif mKey == 2 then rWallEx(math.random(0, getSides()), math.random(1, 2)) wait(getPerfectDelay(THICKNESS) * 2.8)
	elseif mKey == 3 then pMirrorWallStrip(1, 2)
	elseif mKey == 4 then rWallEx(math.random(0, getSides()), 1) wait(getPerfectDelay(THICKNESS) * 2.3)
	elseif mKey == 5 then cWallEx(math.random(0, getSides()), 7) wait(getPerfectDelay(THICKNESS) * 2.7)
	end
end

-- shuffle the keys, and then call them to add all the patterns
-- shuffling is better than randomizing - it guarantees all the patterns will be called
keys = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 4, 4, 5, 5, 5, 5 }
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
end

-- onUnload is an hardcoded function that is called when the level is closed/restarted
function onUnload()
end

-- continuous direction change (even if not on level increment)
dirChangeTime = 150

-- onUpdate is an hardcoded function that is called every frame
function onUpdate(mFrameTime)
	dirChangeTime = dirChangeTime - mFrameTime;
	if dirChangeTime < 0 then
		-- do not change direction while fast spinning
		if isFastSpinning() == false then
			setLevelValueFloat("rotation_speed", getLevelValueFloat("rotation_speed") * -1)
			dirChangeTime = 100
		end
	end 
end