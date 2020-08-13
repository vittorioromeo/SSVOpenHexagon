-- include useful files
u_execScript("utils.lua")
u_execScript("common.lua")
u_execScript("commonpatterns.lua")
u_execScript("nextpatterns.lua")
u_execScript("evolutionpatterns.lua")

-- shuffle the keys, and then call them to add all the patterns
-- shuffling is better than randomizing - it guarantees all the patterns will be called
keys = { 0 }
shuffle(keys)
index = 0
achievementUnlocked = false

smin = 2
smax = 2

level = 1
incrementTime = 10

range = "("..(smin * 2).."/"..(smax * 2).."]"

function slitherSpiralAcc()
	t_wait(getPerfectDelayDM(THICKNESS) * 2.1)
	t_wait(getPerfectDelayDM(THICKNESS) * 2.1)
	local side = getRandomSide()

	local acc = math.random(50, 90) / 500.0 * getRandomDir()
	local minimum = math.random(15, 21) / 10.0 * -1
	local maximum = -minimum

	t_wait(getPerfectDelayDM(THICKNESS) * 3.1)

	for i = 0, math.random(6, 10) do
		hmcSimpleSpinnerSAcc(side, 0, acc, minimum, maximum, true)
		t_wait(getPerfectDelay(THICKNESS) * 0.55)
	end

	t_wait(getPerfectDelayDM(THICKNESS) * 5.3)
end

-- onInit is an hardcoded function that is called when the level is first loaded
function onInit()
	l_setSpeedMult(1.7)
	l_setSpeedInc(0.1)
	l_setSpeedMax(2.9)

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
	l_setPulseSpeed(3.3)
	l_setPulseSpeedR(1.30)
	l_setPulseDelayMax(38)

	l_setBeatPulseMax(15)
	l_setBeatPulseDelayMax(23.4)

	l_setSwapEnabled(true)
	l_addTracked("level", "level")
	l_addTracked("next at", "incrementTime")
	l_addTracked("range", "range")
end

-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()
	m_messageAdd("remember, you can focus with lshift!", 150)
end

-- onStep is an hardcoded function that is called when the level timeline is empty
-- onStep should contain your pattern spawning logic
function onStep()
	l_setSides(math.random(smin, smax) * 2)
	slitherSpiralAcc()
end


-- onIncrement is an hardcoded function that is called when the level difficulty is incremented
function onIncrement()
	level = level + 1
	incrementTime = incrementTime + 5
	m_messageAddImportant("level: "..(level).." / time: "..incrementTime, 150)

	if smax < 4 then
		smax = smax + 1;
	else
		smin = smin + 1;
		smax = smin;
	end

	range = "("..(smin * 2).."/"..(smax * 2).."]"
	m_messageAddImportant("Range: "..range, 100)

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
		if u_isFastSpinning() == false then
			l_setRotationSpeed(l_getRotationSpeed() * -1.0)
			dirChangeTime = 400
		end
	end

	if not achievementUnlocked and l_getLevelTime() > 60 and u_getDifficultyMult() >= 1 then
		steam_unlockAchievement("a16_slither")
		achievementUnlocked = true
	end
end
