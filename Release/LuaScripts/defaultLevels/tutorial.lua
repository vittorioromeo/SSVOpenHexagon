-- include useful files
execFile("utils.lua")
execFile("common.lua")
execFile("commonpatterns.lua")

-- this function adds a pattern to the timeline based on a key
function addPattern(mKey)
	if mKey == 0 then pBarrageSpiral(math.random(1, 2), 1) 
	elseif mKey == 1 then pInverseBarrage(0)
	elseif mKey == 2 then pAltBarrage(math.random(2, 4), 2)
	end
end

-- shuffle the keys, and then call them to add all the patterns
-- shuffling is better than randomizing - it guarantees all the patterns will be called
keys = { 0, 1, 2 }
keys = shuffle(keys)

for i = 0, 2 do
	addPattern(keys[i])
end

-- end - the file will restart from the beginning when the timeline is clear
