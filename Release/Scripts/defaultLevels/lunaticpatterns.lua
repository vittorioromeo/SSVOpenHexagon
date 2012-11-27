-- include useful files
execScript("utils.lua")
execScript("common.lua")
execScript("commonpatterns.lua")

-- this function adds a pattern to the timeline based on a key
function addPattern(mKey)
		if mKey == 0 then pAltBarrage(math.random(2, 3), 2) 
	elseif mKey == 1 then pBarrageSpiral(3, 0.6, 1)
	elseif mKey == 2 then pInverseBarrage(0)
	elseif mKey == 3 then pTunnel(math.random(1, 3))
	elseif mKey == 4 then pMirrorWallStrip(1, 0)
	elseif mKey == 5 then pWallExVortex(0, math.random(1, 2), 1)
	elseif mKey == 6 then pBarrageSpiral(math.random(5, 9), 0.41, 1)
	elseif mKey == 7 then pRandomBarrage(math.random(2, 5), 2.25)
	elseif mKey == 8 then pMirrorSpiralDouble(math.random(8, 10), 0)
	elseif mKey == 9 then pMirrorSpiral(math.random(2, 5), 0)
	end
end

-- shuffle the keys, and then call them to add all the patterns
-- shuffling is better than randomizing - it guarantees all the patterns will be called
keys = { 0, 0, 1, 1, 2, 2, 3, 4, 4, 5, 6, 7, 7, 7, 8, 9, 9 }
keys = shuffle(keys)
index = 0

-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()
	log("level onLoad")
end

-- onStep is an hardcoded function that is called when the level timeline is empty
-- onStep should contain your pattern spawning logic
function onStep()
	log("level onStep")
	
	addPattern(keys[index])
	index = index + 1
	
	if index - 1 == table.getn(keys) then
		index = 1
	end
end

-- onIncrement is an hardcoded function that is called when the level difficulty is incremented
function onIncrement()
	log("level onIncrement")	
end

-- onUnload is an hardcoded function that is called when the level is closed/restarted
function onUnload()
	log("level onUnload")	
end
