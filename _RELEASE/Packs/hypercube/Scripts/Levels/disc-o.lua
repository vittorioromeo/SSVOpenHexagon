-- include useful files
u_execScript("utils.lua")
u_execScript("common.lua")
u_execScript("commonpatterns.lua")
u_execScript("nextpatterns.lua")
u_execScript("evolutionpatterns.lua")

-- this function adds a pattern to the timeline based on a key
function addPattern(mKey)
		if mKey == 0 then pAltBarrage(math.random(1, 3), 2) 
	elseif mKey == 1 then pMirrorSpiral(math.random(2, 4), 0)
	elseif mKey == 2 then pBarrageSpiral(math.random(0, 3), 1, 1)
	elseif mKey == 3 then pBarrageSpiral(math.random(0, 2), 1.2, 2)
	elseif mKey == 4 then pBarrageSpiral(2, 0.7, 1)
	elseif mKey == 5 then pInverseBarrage(0)
	elseif mKey == 6 then hmcDefBarrageSpiral()
	elseif mKey == 7 then pMirrorWallStrip(1, 0)
	elseif mKey == 8 then hmcDefSpinner()
	elseif mKey == 9 then hmcDefBarrage()
	elseif mKey == 10 then hmcDef2Cage()
	elseif mKey == 11 then hmcDefBarrageSpiralSpin()
	end
end

-- shuffle the keys, and then call them to add all the patterns
-- shuffling is better than randomizing - it guarantees all the patterns will be called
keys = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 9, 10, 10, 8, 8, 9, 9, 9, 9, 6, 11, 11, 10, 10 }
keys = shuffle(keys)
index = 0

specials = { "cage", "spinner", "barrage" }
special = "none"

-- onInit is an hardcoded function that is called when the level is first loaded
function onInit()
	l_setSpeedMult(1.7)
	l_setSpeedInc(0.15)
	l_setRotationSpeed(0.1)
	l_setRotationSpeedMax(0.4)
	l_setRotationSpeedInc(0.035)
	l_setDelayMult(1.2)
	l_setDelayInc(0.0)
	l_setFastSpin(0.0)
	l_setSides(6)
	l_setSidesMin(6)
	l_setSidesMax(6)
	l_setIncTime(15)

	l_setPulseMin(77)
	l_setPulseMax(95)
	l_setPulseSpeed(1.95)
	l_setPulseSpeedR(0.51)
	l_setPulseDelayMax(13)

	l_setBeatPulseMax(17)
	l_setBeatPulseDelayMax(27.8)

	l_setSwapEnabled(true)
	l_addTracked("special", "special")
end

-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()
end

-- onStep is an hardcoded function that is called when the level timeline is empty
-- onStep should contain your pattern spawning logic
function onStep()	
	if special == "none" then
		addPattern(keys[index])
		index = index + 1
 	
		if index - 1 == #keys then
			index = 1
		end
	elseif special == "cage" then
		addPattern(10)
	elseif special == "spinner" then
		addPattern(8)
	elseif special == "barrage" then
		addPattern(9)
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