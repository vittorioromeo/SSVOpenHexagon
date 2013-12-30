-- include useful files
u_execScript("utils.lua")
u_execScript("common.lua")
u_execScript("commonpatterns.lua")
u_execScript("nextpatterns.lua")
u_execScript("evolutionpatterns.lua")

-- this function adds a pattern to the timeline based on a key
function addPattern(mKey)
		if mKey == 0 then pAltBarrage(math.random(1, 2), 2) 
	elseif mKey == 1 then pBarrageSpiral(2, 0.6, 1)
	elseif mKey == 2 then pInverseBarrage(0)
	elseif mKey == 3 then hmcDefBarrageSpiralFast()
	elseif mKey == 4 then pWallExVortex(0, 1, 1)
	elseif mKey == 5 then pDMBarrageSpiral(math.random(2, 4), 0.4, 1)
	elseif mKey == 6 then pRandomBarrage(math.random(1, 3), 2.25)
	elseif mKey == 7 then pInverseBarrage(0)
	elseif mKey == 8 then pMirrorWallStrip(1, 0)
	elseif mKey == 9 then hmcDefSpinner()
	elseif mKey == 10 then hmcDefBarrageSpiral()
	elseif mKey == 11 then hmcDef2CageD()
	elseif mKey == 12 then hmcDefBarrageSpiralSpin()
	elseif mKey == 13 then hmcDefSpinnerSpiralAcc()
	elseif mKey == 14 then hmcDefBarrageSpiralRnd()
	elseif mKey == 15 then hmcDefBarrageInv()
	end
end

-- shuffle the keys, and then call them to add all the patterns
-- shuffling is better than randomizing - it guarantees all the patterns will be called
keys = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 }
keys = shuffle(keys)
index = 0

specials = { "cage", "spinner", "barrage", "spiral" }
special = "none"

-- onInit is an hardcoded function that is called when the level is first loaded
function onInit()
	l_setSpeedMult(2.7)
	l_setSpeedInc(0.04)
	l_setRotationSpeed(0.25)
	l_setRotationSpeedMax(0.4)
	l_setRotationSpeedInc(0.015)
	l_setDelayMult(1.35)
	l_setDelayInc(0.0)
	l_setFastSpin(71.0)
	l_setSides(6)
	l_setSidesMin(6)
	l_setSidesMax(6)
	l_setIncTime(10)

	l_setPulseMin(61)
	l_setPulseMax(80)
	l_setPulseSpeed(2.4)
	l_setPulseSpeedR(1.45)
	l_setPulseDelayMax(6.8)

	l_setBeatPulseMax(18)
	l_setBeatPulseDelayMax(18.8)

	l_setSwapEnabled(true)
	l_addTracked("special", "special")
end

-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()
	setCurveMult(0.85)
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
		addPattern(11)
		addPattern(9)
	elseif special == "spinner" then
		addPattern(14)
		addPattern(9)
	elseif special == "barrage" then
		addPattern(3)
		addPattern(14)
		addPattern(13)
		addPattern(15)
	elseif special == "spiral" then
		addPattern(12)
		addPattern(4)
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

-- continuous direction change (even if not on level increment)
dirChangeTime = 120

-- onUnload is an hardcoded function that is called when the level is closed/restarted
function onUnload()
end

-- onUpdate is an hardcoded function that is called every frame
function onUpdate(mFrameTime)
	dirChangeTime = dirChangeTime - mFrameTime;
	if dirChangeTime < 0 then
		-- do not change direction while fast spinning
		if u_isFastSpinning() == false then
			l_setRotationSpeed(l_getRotationSpeed() * -1.0)
			dirChangeTime = 200
		end
	end 
end