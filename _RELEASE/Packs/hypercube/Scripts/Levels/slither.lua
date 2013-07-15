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

smin = 2
smax = 2

level = 1
incrementTime = 10

range = "("..(smin * 2).."/"..(smax * 2).."]"

-- onInit is an hardcoded function that is called when the level is first loaded
function onInit()
	l_setSpeedMult(1.7)
	l_setSpeedInc(0.1)
	l_setRotationSpeed(0.2)
	l_setRotationSpeedMax(0.4)
	l_setRotationSpeedInc(0.035)
	l_setDelayMult(1.1)
	l_setDelayInc(0.0)
	l_setFastSpin(0.0)
	l_setSides(3)
	l_setSidesMin(3)
	l_setSidesMax(3)
	l_setIncTime(10)

	l_setWallAngleLeft(-25)

	l_setPulseMin(60)
	l_setPulseMax(80)
	l_setPulseSpeed(3.6)
	l_setPulseSpeedR(1.45)
	l_setPulseDelayMax(7)

	l_setBeatPulseMax(15)
	l_setBeatPulseDelayMax(21.8)

	l_setSwapEnabled(true)
	l_addTracked("level", "level")
	l_addTracked("next at", "incrementTime")
	l_addTracked("range", "range")
end

-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()
	messageAdd("remember, you can focus with lshift!", 150)
end

-- onStep is an hardcoded function that is called when the level timeline is empty
-- onStep should contain your pattern spawning logic
function onStep()	
	l_setSides(math.random(smin, smax) * 2)
	hmcDefSpinnerSpiralAcc()
end


-- onIncrement is an hardcoded function that is called when the level difficulty is incremented
function onIncrement()
	level = level + 1
	incrementTime = incrementTime + 5
	messageImportantAdd("level: "..(level).." / time: "..incrementTime, 150)

	if smax < 4 then
		smax = smax + 1;
	else
		smin = smin + 1;
		smax = smin;
	end

	range = "("..(smin * 2).."/"..(smax * 2).."]"
	messageImportantAdd("Range: "..range, 100)

	l_setSides(l_getSides() + 2)
	l_setIncTime(incrementTime)
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
		if isFastSpinning() == false then
			l_setRotationSpeed(l_getRotationSpeed() * -1.0)
			dirChangeTime = 400
		end
	end 
end