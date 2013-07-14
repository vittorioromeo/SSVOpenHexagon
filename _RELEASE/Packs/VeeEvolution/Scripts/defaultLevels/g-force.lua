-- include useful files
execScript("utils.lua")
execScript("common.lua")
execScript("commonpatterns.lua")
execScript("nextpatterns.lua")
execScript("evolutionpatterns.lua")

function gforceBarrage()
	cBarrage(getRandomSide()) 
	wait(getPerfectDelayDM(THICKNESS) * 6.1)
end

function gforceBarrageAssault()
	cBarrage(getRandomSide()) 
	wait(getPerfectDelayDM(THICKNESS) * 3.1)
end


-- this function adds a pattern to the timeline based on a key
function addPattern(mKey)
		if mKey == 0 then hmcDefAccelBarrage()
	elseif mKey == 1 then gforceBarrage()
	end
end

-- shuffle the keys, and then call them to add all the patterns
-- shuffling is better than randomizing - it guarantees all the patterns will be called
keys = { 0, 1 }
keys = shuffle(keys)
index = 0

specials = { "double", "assault", "incongruence", "dizzy" } 
special = "none"

-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()
	addTracked("special", "special")
end

-- onStep is an hardcoded function that is called when the level timeline is empty
-- onStep should contain your pattern spawning logic
function onStep()	
	if special == "incongruence" then
		setLevelSides(math.random(4, 5))
	else
		setLevelSides(4)
	end

	if special == "assault" then
		gforceBarrageAssault()
		return
	end

	if special == "dizzy" then
		addPattern(0)
		return
	end

	if special ~= "double" then
		addPattern(keys[index])
	else
		addPattern(keys[index])
		addPattern(keys[index])
	end

	index = index + 1
	
	if index - 1 == table.getn(keys) then
		index = 1
	end
end


-- onIncrement is an hardcoded function that is called when the level difficulty is incremented
function onIncrement()
	specials = shuffle(specials)

	if special == "none" then
		special = specials[1]
		messageImportantAdd("Special: "..special, 120)
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