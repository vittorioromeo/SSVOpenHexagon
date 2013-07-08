-- include useful files
execScript("utils.lua")
execScript("common.lua")
execScript("commonpatterns.lua")
execScript("nextpatterns.lua")
execScript("evolutionpatterns.lua")

-- this function adds a pattern to the timeline based on a key
function addPattern(mKey)
		if mKey == 0 then pAltBarrage(math.random(1, 3), 2) 
	end
end

-- shuffle the keys, and then call them to add all the patterns
-- shuffling is better than randomizing - it guarantees all the patterns will be called
keys = { 0 }
keys = shuffle(keys)
index = 0

specials = { "none" }
special = "none"

smin = 3
smax = 3

level = 1
incrementTime = 10

range = "("..smin.."/"..smax.."]"

-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()
	messageAdd("remember you can focus with lshift!", 150)
end

-- onStep is an hardcoded function that is called when the level timeline is empty
-- onStep should contain your pattern spawning logic
function onStep()	
	setLevelValueInt("sides", math.random(smin, smax))
	hmcDefSpinnerSpiralAcc()
end


-- onIncrement is an hardcoded function that is called when the level difficulty is incremented
function onIncrement()
	level = level + 1
	incrementTime = incrementTime + 5
	messageImportantAdd("level: "..(level).." / time: "..incrementTime, 150)

	if smax < 7 then
		smax = smax + 1;
	else
		smin = smin + 1;
		smax = smin;
	end

	range = "("..smin.."/"..smax.."]"
	messageImportantAdd("Range: "..range, 100)

	setLevelValueInt("sides", getSides() + 2)
	setLevelValueInt("increment_time", incrementTime)

	specials = shuffle(specials)

	if special == "none" then
		special = specials[1]
		messageImportantAdd("Special: "..special, 100)
	else
		special = "none"
	end
end

-- onUnload is an hardcoded function that is called when the level is closed/restarted
function onUnload()
end

-- onUpdate is an hardcoded function that is called every frame
function onUpdate(mFrameTime)
end