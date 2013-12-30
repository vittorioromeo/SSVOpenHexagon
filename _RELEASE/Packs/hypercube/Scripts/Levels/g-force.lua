-- include useful files
u_execScript("utils.lua")
u_execScript("common.lua")
u_execScript("commonpatterns.lua")
u_execScript("nextpatterns.lua")
u_execScript("evolutionpatterns.lua")

function gforceBarrage()
	cBarrage(getRandomSide()) 
	t_wait(getPerfectDelayDM(THICKNESS) * 6.1)
end

function gforceBarrageAssault()
	cBarrage(getRandomSide()) 
	t_wait(getPerfectDelayDM(THICKNESS) * 3.1)
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

-- onInit is an hardcoded function that is called when the level is first loaded
function onInit()
	l_setSpeedMult(2.1)
	l_setSpeedInc(0.16)
	l_setRotationSpeed(0.12)
	l_setRotationSpeedMax(0.4)
	l_setRotationSpeedInc(0.035)
	l_setDelayMult(1.9)
	l_setDelayInc(0.0)
	l_setFastSpin(0.0)
	l_setSides(4)
	l_setSidesMin(4)
	l_setSidesMax(4)
	l_setIncTime(10)

	l_setWallSkewLeft(-15)

	l_setPulseMin(77)
	l_setPulseMax(95)
	l_setPulseSpeed(2.95)
	l_setPulseSpeedR(0.5)
	l_setPulseDelayMax(13)

	l_setBeatPulseMax(19)
	l_setBeatPulseDelayMax(14.2)

	l_setSwapEnabled(true)
	l_addTracked("special", "special")
end

-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()
end

-- onStep is an hardcoded function that is called when the level timeline is empty
-- onStep should contain your pattern spawning logic
function onStep()	
	if special == "incongruence" then
		l_setSides(math.random(4, 5))
	else
		l_setSides(4)
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
	
	if index - 1 == #keys then
		index = 1
	end
end


-- onIncrement is an hardcoded function that is called when the level difficulty is incremented
function onIncrement()
	specials = shuffle(specials)

	if special == "none" then
		special = specials[1]
		m_messageAddImportant("Special: "..special, 120)
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