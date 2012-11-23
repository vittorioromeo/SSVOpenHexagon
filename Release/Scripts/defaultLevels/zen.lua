-- include useful files
execScript("utils.lua")
execScript("common.lua")
execScript("commonpatterns.lua")

-- this function is a non-default pattern, unique to this level
function pCoolPattern()
	startSide = getRandomSide()	
	delay = getPerfectDelay(THICKNESS) * 5.0
		
	for i = 0, 1 do
		wall(startSide + getSides() / 2, THICKNESS * 5)
		cBarrage(startSide - 1)
		wait(delay)
		wall(startSide, THICKNESS * 6)
		cBarrage(startSide + getSides() / 2 - 1)
		wait(delay)
	end
	
	wait(getPerfectDelay(THICKNESS) * 3.0)
	
	for i = 0, 1 do
		wall(startSide, THICKNESS * 5)
		cBarrage(startSide - 1)
		wait(delay)
		wall(startSide + getSides() / 2, THICKNESS * 6)
		cBarrage(startSide + getSides() / 2 - 1)
		wait(delay)
	end
	
	wait(getPerfectDelay(THICKNESS) * 5.0)
end

-- this function adds a pattern to the timeline based on a key
function addPattern(mKey)
	if mKey == 0 then pTunnel(2) 
	elseif mKey == 1 then pWallExVortex(math.random(0, 1), math.random(1, 2), 1)
	elseif mKey == 2 then pInverseBarrage(0)
	elseif mKey == 3 then pMirrorWallStrip(1, 0) 
	elseif mKey == 4 then pCoolPattern()
	end
end

-- shuffle the keys, and then call them to add all the patterns
-- shuffling is better than randomizing - it guarantees all the patterns will be called
keys = { 0, 1, 2, 3, 4 }
keys = shuffle(keys)

for i = 0, 4 do
	addPattern(keys[i])
end

-- end - the file will restart from the beginning when the timeline is clear
