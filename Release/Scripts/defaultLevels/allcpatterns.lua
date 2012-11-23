-- include useful files
execScript("utils.lua")
execScript("common.lua")
execScript("commonpatterns.lua")

-- this function adds a pattern to the timeline based on a key
function addPattern(mKey)
		if mKey == 0 then pAltBarrage(math.random(2, 4), 2) 
	elseif mKey == 1 then pMirrorSpiral(math.random(3, 6), 0)
	elseif mKey == 2 then pBarrageSpiral(math.random(0, 4), 1, math.random(1, 2))
	elseif mKey == 3 then pWallExVortex(0, 1, 1)
	elseif mKey == 4 then pInverseBarrage(0)
	elseif mKey == 5 then pMirrorWallStrip(1, 0)
	elseif mKey == 6 then pTunnel(math.random(1, 3))
	end
end

-- shuffle the keys, and then call them to add all the patterns
-- shuffling is better than randomizing - it guarantees all the patterns will be called
keys = { 0, 0, 0, 1, 1, 2, 2, 3, 4, 5, 5, 5, 6 }
keys = shuffle(keys)

for i = 0, table.getn(keys) do
	addPattern(keys[i])
end

-- end - the file will restart from the beginning when the timeline is clear
