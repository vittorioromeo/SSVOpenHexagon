-- include useful files
u_execScript("utils.lua")
u_execScript("common.lua")
u_execScript("commonpatterns.lua")
u_execScript("nextpatterns.lua")
u_execScript("evolutionpatterns.lua")

curveSpeed = 1
achievementUnlocked = false

-- onInit is an hardcoded function that is called when the level is first loaded
function onInit()
	l_setSpeedMult(2.91)
	l_setSpeedInc(0.0)
	l_setRotationSpeed(0.0)
	l_setRotationSpeedMax(0.0)
	l_setRotationSpeedInc(0.0)
	l_setDelayMult(1.35)
	l_setDelayInc(0.0)
	l_setFastSpin(0.0)
	l_setSides(15)
	l_setSidesMin(15)
	l_setSidesMax(15)
	l_setIncTime(10)

	l_setWallSkewLeft(0)

	l_setPulseMin(60)
	l_setPulseMax(60)
	l_setPulseSpeed(0)
	l_setPulseSpeedR(0)
	l_setPulseDelayMax(6.8)

	l_setBeatPulseMax(20)
	l_setBeatPulseDelayMax(27.2)

	l_setSwapEnabled(true)
	l_addTracked("curveSpeed", "curve speed")
end

-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()
end

-- onStep is an hardcoded function that is called when the level timeline is empty
-- onStep should contain your pattern spawning logic
function onStep()
	hmcSimpleBarrageSNeigh(getRandomSide(), getRandomDir() * curveSpeed, 2)
	t_wait(getPerfectDelayDM(THICKNESS) * 6.22)
end


-- onIncrement is an hardcoded function that is called when the level difficulty is incremented
function onIncrement()
	if curveSpeed < 3 then
		curveSpeed = curveSpeed + 0.4
		m_messageAddImportant("Curve speed: "..curveSpeed, 120)
	end
end

-- onUnload is an hardcoded function that is called when the level is closed/restarted
function onUnload()
end

-- onUpdate is an hardcoded function that is called every frame
function onUpdate(mFrameTime)
	if not achievementUnlocked and l_getLevelTime() > 90 and u_getDifficultyMult() >= 1 then
		steam_unlockAchievement("a19_centrifugalforce")
		achievementUnlocked = true
	end
end
